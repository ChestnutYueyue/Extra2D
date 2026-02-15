#include <extra2d/graphics/shader_loader.h>
#include <extra2d/utils/logger.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace extra2d {

namespace fs = std::filesystem;

/**
 * @brief 构造函数，初始化Shader加载器
 */
ShaderLoader::ShaderLoader() {
}

/**
 * @brief 从分离文件加载Shader (.vert + .frag)
 * @param name Shader名称
 * @param vertPath 顶点着色器文件路径
 * @param fragPath 片段着色器文件路径
 * @return 加载结果
 */
ShaderLoadResult ShaderLoader::loadFromSeparateFiles(
    const std::string& name,
    const std::string& vertPath,
    const std::string& fragPath) {
    
    ShaderLoadResult result;

    if (!fileExists(vertPath)) {
        result.errorMessage = "Vertex shader file not found: " + vertPath;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    if (!fileExists(fragPath)) {
        result.errorMessage = "Fragment shader file not found: " + fragPath;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    std::string vertSource = readFile(vertPath);
    std::string fragSource = readFile(fragPath);

    if (vertSource.empty()) {
        result.errorMessage = "Failed to read vertex shader file: " + vertPath;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    if (fragSource.empty()) {
        result.errorMessage = "Failed to read fragment shader file: " + fragPath;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    fs::path vertDir = fs::path(vertPath).parent_path();
    fs::path fragDir = fs::path(fragPath).parent_path();

    vertSource = processIncludes(vertSource, vertDir.string(), result.dependencies);
    fragSource = processIncludes(fragSource, fragDir.string(), result.dependencies);

    result.vertSource = vertSource;
    result.fragSource = fragSource;
    result.success = true;

    return result;
}

/**
 * @brief 从组合文件加载Shader (.shader)
 * @param path 组合Shader文件路径
 * @return 加载结果
 */
ShaderLoadResult ShaderLoader::loadFromCombinedFile(const std::string& path) {
    ShaderLoadResult result;

    if (!fileExists(path)) {
        result.errorMessage = "Shader file not found: " + path;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    std::string content = readFile(path);
    if (content.empty()) {
        result.errorMessage = "Failed to read shader file: " + path;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    ShaderMetadata metadata;
    std::string vertSource, fragSource;

    if (!parseCombinedFile(content, vertSource, fragSource, metadata)) {
        result.errorMessage = "Failed to parse combined shader file: " + path;
        E2D_LOG_ERROR("{}", result.errorMessage);
        return result;
    }

    fs::path baseDir = fs::path(path).parent_path();
    vertSource = processIncludes(vertSource, baseDir.string(), result.dependencies);
    fragSource = processIncludes(fragSource, baseDir.string(), result.dependencies);

    result.vertSource = vertSource;
    result.fragSource = fragSource;
    result.success = true;

    return result;
}

/**
 * @brief 从源码字符串加载Shader
 * @param vertSource 顶点着色器源码
 * @param fragSource 片段着色器源码
 * @return 加载结果
 */
ShaderLoadResult ShaderLoader::loadFromSource(
    const std::string& vertSource,
    const std::string& fragSource) {
    
    ShaderLoadResult result;
    result.vertSource = vertSource;
    result.fragSource = fragSource;
    result.success = true;
    return result;
}

/**
 * @brief 处理Shader源码中的#include指令
 * @param source 原始源码
 * @param baseDir 基础目录
 * @param outDependencies 输出依赖列表
 * @return 处理后的源码
 */
std::string ShaderLoader::processIncludes(
    const std::string& source,
    const std::string& baseDir,
    std::vector<std::string>& outDependencies) {
    
    std::string result;
    std::istringstream stream(source);
    std::string line;

    while (std::getline(stream, line)) {
        size_t includePos = line.find("#include");
        if (includePos != std::string::npos) {
            size_t startQuote = line.find('"', includePos);
            size_t endQuote = line.find('"', startQuote + 1);
            
            if (startQuote != std::string::npos && endQuote != std::string::npos) {
                std::string includeName = line.substr(startQuote + 1, endQuote - startQuote - 1);
                std::string includePath = findIncludeFile(includeName, baseDir);
                
                if (!includePath.empty()) {
                    auto cacheIt = includeCache_.find(includePath);
                    std::string includeContent;
                    
                    if (cacheIt != includeCache_.end()) {
                        includeContent = cacheIt->second;
                    } else {
                        includeContent = readFile(includePath);
                        includeCache_[includePath] = includeContent;
                    }
                    
                    outDependencies.push_back(includePath);
                    result += includeContent;
                    result += "\n";
                    continue;
                } else {
                    E2D_LOG_WARN("Include file not found: {}", includeName);
                }
            }
        }
        
        result += line;
        result += "\n";
    }

    return result;
}

/**
 * @brief 应用预处理器定义
 * @param source 原始源码
 * @param defines 预处理器定义列表
 * @return 处理后的源码
 */
std::string ShaderLoader::applyDefines(
    const std::string& source,
    const std::vector<std::string>& defines) {
    
    if (defines.empty()) {
        return source;
    }

    std::string defineBlock;
    for (const auto& def : defines) {
        defineBlock += "#define " + def + "\n";
    }

    std::string result;
    std::istringstream stream(source);
    std::string line;
    bool inserted = false;

    while (std::getline(stream, line)) {
        if (!inserted && (line.find("#version") != std::string::npos || 
                          line.find("precision") != std::string::npos)) {
            result += line + "\n";
            continue;
        }
        
        if (!inserted) {
            result += defineBlock;
            inserted = true;
        }
        
        result += line + "\n";
    }

    return result;
}

/**
 * @brief 获取Shader元数据
 * @param path Shader文件路径
 * @return 元数据
 */
ShaderMetadata ShaderLoader::getMetadata(const std::string& path) {
    ShaderMetadata metadata;
    
    if (!fileExists(path)) {
        return metadata;
    }

    metadata.combinedPath = path;
    metadata.lastModified = getFileModifiedTime(path);
    
    fs::path p(path);
    metadata.name = p.stem().string();

    return metadata;
}

/**
 * @brief 添加include搜索路径
 * @param path 搜索路径
 */
void ShaderLoader::addIncludePath(const std::string& path) {
    if (std::find(includePaths_.begin(), includePaths_.end(), path) == includePaths_.end()) {
        includePaths_.push_back(path);
    }
}

/**
 * @brief 读取文件内容
 * @param filepath 文件路径
 * @return 文件内容字符串
 */
std::string ShaderLoader::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

/**
 * @brief 获取文件修改时间
 * @param filepath 文件路径
 * @return 修改时间戳
 */
uint64_t ShaderLoader::getFileModifiedTime(const std::string& filepath) {
#ifdef __SWITCH__
    (void)filepath;
    return 1;
#else
    try {
        auto ftime = fs::last_write_time(filepath);
        auto sctp = std::chrono::time_point_cast<std::chrono::seconds>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        return static_cast<uint64_t>(sctp.time_since_epoch().count());
    } catch (...) {
        return 0;
    }
#endif
}

/**
 * @brief 检查文件是否存在
 * @param filepath 文件路径
 * @return 存在返回true，否则返回false
 */
bool ShaderLoader::fileExists(const std::string& filepath) {
    return fs::exists(filepath);
}

/**
 * @brief 解析组合Shader文件
 * @param content 文件内容
 * @param outVert 输出顶点着色器源码
 * @param outFrag 输出片段着色器源码
 * @param outMetadata 输出元数据
 * @return 解析成功返回true，失败返回false
 */
bool ShaderLoader::parseCombinedFile(const std::string& content,
                                     std::string& outVert,
                                     std::string& outFrag,
                                     ShaderMetadata& outMetadata) {
    enum class Section {
        None,
        Meta,
        Vertex,
        Fragment
    };

    Section currentSection = Section::None;
    std::string metaContent;
    std::string vertContent;
    std::string fragContent;

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        std::string trimmedLine = line;
        size_t start = trimmedLine.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) {
            trimmedLine = trimmedLine.substr(start);
        }
        size_t end = trimmedLine.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            trimmedLine = trimmedLine.substr(0, end + 1);
        }

        if (trimmedLine == "#meta") {
            currentSection = Section::Meta;
            continue;
        } else if (trimmedLine == "#vertex") {
            currentSection = Section::Vertex;
            continue;
        } else if (trimmedLine == "#fragment") {
            currentSection = Section::Fragment;
            continue;
        }

        switch (currentSection) {
            case Section::Meta:
                metaContent += line + "\n";
                break;
            case Section::Vertex:
                vertContent += line + "\n";
                break;
            case Section::Fragment:
                fragContent += line + "\n";
                break;
            default:
                break;
        }
    }

    if (vertContent.empty() || fragContent.empty()) {
        return false;
    }

    if (!metaContent.empty()) {
        parseMetadata(metaContent, outMetadata);
    }

    outVert = vertContent;
    outFrag = fragContent;
    return true;
}

/**
 * @brief 解析元数据JSON块
 * @param jsonContent JSON内容
 * @param outMetadata 输出元数据
 * @return 解析成功返回true，失败返回false
 */
bool ShaderLoader::parseMetadata(const std::string& jsonContent, ShaderMetadata& outMetadata) {
    std::string content = jsonContent;
    
    size_t start = content.find('{');
    size_t end = content.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return false;
    }
    
    content = content.substr(start, end - start + 1);

    auto extractString = [&content](const std::string& key) -> std::string {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = content.find(searchKey);
        if (keyPos == std::string::npos) {
            return "";
        }
        
        size_t colonPos = content.find(':', keyPos);
        if (colonPos == std::string::npos) {
            return "";
        }
        
        size_t quoteStart = content.find('"', colonPos);
        if (quoteStart == std::string::npos) {
            return "";
        }
        
        size_t quoteEnd = content.find('"', quoteStart + 1);
        if (quoteEnd == std::string::npos) {
            return "";
        }
        
        return content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
    };

    outMetadata.name = extractString("name");
    return true;
}

/**
 * @brief 查找include文件路径
 * @param includeName include文件名
 * @param baseDir 基础目录
 * @return 找到的完整路径，未找到返回空字符串
 */
std::string ShaderLoader::findIncludeFile(const std::string& includeName, const std::string& baseDir) {
    fs::path basePath(baseDir);
    fs::path includePath = basePath / includeName;
    
    if (fs::exists(includePath)) {
        return includePath.string();
    }

    for (const auto& searchPath : includePaths_) {
        includePath = fs::path(searchPath) / includeName;
        if (fs::exists(includePath)) {
            return includePath.string();
        }
    }

    return "";
}

} // namespace extra2d
