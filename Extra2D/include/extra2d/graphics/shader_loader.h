#pragma once

#include <extra2d/core/types.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace extra2d {

// ============================================================================
// Shader加载结果
// ============================================================================
struct ShaderLoadResult {
    bool success = false;
    std::string errorMessage;
    std::string vertSource;
    std::string fragSource;
    std::vector<std::string> dependencies;
};

// ============================================================================
// Shader元数据
// ============================================================================
struct ShaderMetadata {
    std::string name;
    std::string vertPath;
    std::string fragPath;
    std::string combinedPath;
    uint64_t lastModified = 0;
    std::vector<std::string> defines;
    std::unordered_map<std::string, std::string> uniforms;
};

// ============================================================================
// ShaderLoader接口 - 支持多种文件格式加载
// ============================================================================
class IShaderLoader {
public:
    virtual ~IShaderLoader() = default;

    /**
     * @brief 从分离文件加载Shader (.vert + .frag)
     * @param name Shader名称
     * @param vertPath 顶点着色器文件路径
     * @param fragPath 片段着色器文件路径
     * @return 加载结果
     */
    virtual ShaderLoadResult loadFromSeparateFiles(
        const std::string& name,
        const std::string& vertPath,
        const std::string& fragPath) = 0;

    /**
     * @brief 从组合文件加载Shader (.shader)
     * @param path 组合Shader文件路径
     * @return 加载结果
     */
    virtual ShaderLoadResult loadFromCombinedFile(const std::string& path) = 0;

    /**
     * @brief 从源码字符串加载Shader
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 加载结果
     */
    virtual ShaderLoadResult loadFromSource(
        const std::string& vertSource,
        const std::string& fragSource) = 0;

    /**
     * @brief 处理Shader源码中的#include指令
     * @param source 原始源码
     * @param baseDir 基础目录
     * @param outDependencies 输出依赖列表
     * @return 处理后的源码
     */
    virtual std::string processIncludes(
        const std::string& source,
        const std::string& baseDir,
        std::vector<std::string>& outDependencies) = 0;

    /**
     * @brief 应用预处理器定义
     * @param source 原始源码
     * @param defines 预处理器定义列表
     * @return 处理后的源码
     */
    virtual std::string applyDefines(
        const std::string& source,
        const std::vector<std::string>& defines) = 0;

    /**
     * @brief 获取Shader元数据
     * @param path Shader文件路径
     * @return 元数据
     */
    virtual ShaderMetadata getMetadata(const std::string& path) = 0;
};

// ============================================================================
// 默认ShaderLoader实现
// ============================================================================
class ShaderLoader : public IShaderLoader {
public:
    ShaderLoader();
    ~ShaderLoader() override = default;

    /**
     * @brief 从分离文件加载Shader (.vert + .frag)
     * @param name Shader名称
     * @param vertPath 顶点着色器文件路径
     * @param fragPath 片段着色器文件路径
     * @return 加载结果
     */
    ShaderLoadResult loadFromSeparateFiles(
        const std::string& name,
        const std::string& vertPath,
        const std::string& fragPath) override;

    /**
     * @brief 从组合文件加载Shader (.shader)
     * @param path 组合Shader文件路径
     * @return 加载结果
     */
    ShaderLoadResult loadFromCombinedFile(const std::string& path) override;

    /**
     * @brief 从源码字符串加载Shader
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 加载结果
     */
    ShaderLoadResult loadFromSource(
        const std::string& vertSource,
        const std::string& fragSource) override;

    /**
     * @brief 处理Shader源码中的#include指令
     * @param source 原始源码
     * @param baseDir 基础目录
     * @param outDependencies 输出依赖列表
     * @return 处理后的源码
     */
    std::string processIncludes(
        const std::string& source,
        const std::string& baseDir,
        std::vector<std::string>& outDependencies) override;

    /**
     * @brief 应用预处理器定义
     * @param source 原始源码
     * @param defines 预处理器定义列表
     * @return 处理后的源码
     */
    std::string applyDefines(
        const std::string& source,
        const std::vector<std::string>& defines) override;

    /**
     * @brief 获取Shader元数据
     * @param path Shader文件路径
     * @return 元数据
     */
    ShaderMetadata getMetadata(const std::string& path) override;

    /**
     * @brief 添加include搜索路径
     * @param path 搜索路径
     */
    void addIncludePath(const std::string& path);

    /**
     * @brief 读取文件内容
     * @param filepath 文件路径
     * @return 文件内容字符串
     */
    static std::string readFile(const std::string& filepath);

    /**
     * @brief 获取文件修改时间
     * @param filepath 文件路径
     * @return 修改时间戳
     */
    static uint64_t getFileModifiedTime(const std::string& filepath);

    /**
     * @brief 检查文件是否存在
     * @param filepath 文件路径
     * @return 存在返回true，否则返回false
     */
    static bool fileExists(const std::string& filepath);

private:
    std::vector<std::string> includePaths_;
    std::unordered_map<std::string, std::string> includeCache_;

    /**
     * @brief 解析组合Shader文件
     * @param content 文件内容
     * @param outVert 输出顶点着色器源码
     * @param outFrag 输出片段着色器源码
     * @param outMetadata 输出元数据
     * @return 解析成功返回true，失败返回false
     */
    bool parseCombinedFile(const std::string& content,
                          std::string& outVert,
                          std::string& outFrag,
                          ShaderMetadata& outMetadata);

    /**
     * @brief 解析元数据JSON块
     * @param jsonContent JSON内容
     * @param outMetadata 输出元数据
     * @return 解析成功返回true，失败返回false
     */
    bool parseMetadata(const std::string& jsonContent, ShaderMetadata& outMetadata);

    /**
     * @brief 查找include文件路径
     * @param includeName include文件名
     * @param baseDir 基础目录
     * @return 找到的完整路径，未找到返回空字符串
     */
    std::string findIncludeFile(const std::string& includeName, const std::string& baseDir);
};

} // namespace extra2d
