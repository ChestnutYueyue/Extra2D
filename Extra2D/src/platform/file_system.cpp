#include <extra2d/platform/file_system.h>
#include <extra2d/utils/logger.h>

#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <direct.h>
    #include <shlwapi.h>
    #pragma comment(lib, "shlwapi.lib")
#else
    #include <unistd.h>
    #include <libgen.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <limits.h>
#endif

namespace extra2d {

// ============================================================================
// 平台特定辅助函数
// ============================================================================

#ifdef PLATFORM_WINDOWS
    static const char PATH_SEPARATOR = '\\';
    static const char PATH_SEPARATOR_ALT = '/';
#else
    static const char PATH_SEPARATOR = '/';
    static const char PATH_SEPARATOR_ALT = '\\';
#endif

static std::string normalizeSeparators(const std::string& path) {
    std::string result = path;
    std::replace(result.begin(), result.end(), PATH_SEPARATOR_ALT, PATH_SEPARATOR);
    return result;
}

// ============================================================================
// 资源根目录
// ============================================================================

std::string FileSystem::getResourceRoot() {
#ifdef PLATFORM_SWITCH
    return "romfs:/";
#else
    // PC 端：优先使用可执行文件目录下的 assets 文件夹
    std::string exeDir = getExecutableDirectory();
    std::string assetsDir = combinePath(exeDir, "assets");
    
    if (directoryExists(assetsDir)) {
        return assetsDir;
    }
    
    // 备选：当前工作目录
    std::string cwd = getCurrentWorkingDirectory();
    assetsDir = combinePath(cwd, "assets");
    
    if (directoryExists(assetsDir)) {
        return assetsDir;
    }
    
    // 默认返回当前工作目录
    return cwd;
#endif
}

// ============================================================================
// 路径解析
// ============================================================================

std::string FileSystem::resolvePath(const std::string& relativePath) {
    std::string normalized = normalizeSeparators(relativePath);
    
#ifdef PLATFORM_SWITCH
    // Switch: 添加 romfs:/ 前缀
    if (normalized.find("romfs:/") == 0) {
        return normalized;
    }
    return "romfs:/" + normalized;
#else
    // PC: 如果已经是绝对路径，直接返回
#ifdef PLATFORM_WINDOWS
    if (normalized.size() >= 2 && normalized[1] == ':') {
        return normalized;
    }
#else
    if (!normalized.empty() && normalized[0] == '/') {
        return normalized;
    }
#endif
    
    // 组合资源根目录和相对路径
    return combinePath(getResourceRoot(), normalized);
#endif
}

// ============================================================================
// 文件/目录检查
// ============================================================================

bool FileSystem::fileExists(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFREG) != 0;
}

bool FileSystem::directoryExists(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFDIR) != 0;
}

// ============================================================================
// 路径操作
// ============================================================================

std::string FileSystem::getExecutableDirectory() {
    std::string exePath;
    
#ifdef PLATFORM_WINDOWS
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        exePath = buffer;
    }
#elif defined(PLATFORM_SWITCH)
    // Switch: 返回当前工作目录（不支持获取可执行文件路径）
    return getCurrentWorkingDirectory();
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        exePath = buffer;
    }
#endif
    
    if (!exePath.empty()) {
        size_t lastSep = exePath.find_last_of("/\\");
        if (lastSep != std::string::npos) {
            return exePath.substr(0, lastSep);
        }
    }
    
    return getCurrentWorkingDirectory();
}

std::string FileSystem::getCurrentWorkingDirectory() {
#ifdef PLATFORM_WINDOWS
    char buffer[MAX_PATH];
    DWORD len = GetCurrentDirectoryA(MAX_PATH, buffer);
    if (len > 0 && len < MAX_PATH) {
        return std::string(buffer);
    }
#else
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
#endif
    return ".";
}

std::string FileSystem::combinePath(const std::string& base, const std::string& relative) {
    if (base.empty()) {
        return normalizeSeparators(relative);
    }
    if (relative.empty()) {
        return normalizeSeparators(base);
    }
    
    std::string result = normalizeSeparators(base);
    std::string rel = normalizeSeparators(relative);
    
    // 移除末尾的分隔符
    while (!result.empty() && result.back() == PATH_SEPARATOR) {
        result.pop_back();
    }
    
    // 移除开头的分隔符
    size_t relStart = 0;
    while (relStart < rel.size() && rel[relStart] == PATH_SEPARATOR) {
        ++relStart;
    }
    
    if (relStart < rel.size()) {
        result += PATH_SEPARATOR;
        result += rel.substr(relStart);
    }
    
    return result;
}

std::string FileSystem::getFileName(const std::string& path) {
    std::string normalized = normalizeSeparators(path);
    size_t lastSep = normalized.find_last_of(PATH_SEPARATOR);
    if (lastSep != std::string::npos) {
        return normalized.substr(lastSep + 1);
    }
    return normalized;
}

std::string FileSystem::getFileExtension(const std::string& path) {
    std::string fileName = getFileName(path);
    size_t lastDot = fileName.find_last_of('.');
    if (lastDot != std::string::npos && lastDot > 0) {
        return fileName.substr(lastDot);
    }
    return "";
}

std::string FileSystem::getDirectoryName(const std::string& path) {
    std::string normalized = normalizeSeparators(path);
    size_t lastSep = normalized.find_last_of(PATH_SEPARATOR);
    if (lastSep != std::string::npos) {
        return normalized.substr(0, lastSep);
    }
    return ".";
}

std::string FileSystem::normalizePath(const std::string& path) {
    return normalizeSeparators(path);
}

// ============================================================================
// 文件读写
// ============================================================================

std::string FileSystem::readFileText(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        E2D_LOG_ERROR("Failed to open file: {}", path);
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<uint8_t> FileSystem::readFileBytes(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        E2D_LOG_ERROR("Failed to open file: {}", path);
        return {};
    }
    
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    return buffer;
}

int64_t FileSystem::getFileSize(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return -1;
    }
    return static_cast<int64_t>(st.st_size);
}

// ============================================================================
// 目录操作
// ============================================================================

bool FileSystem::createDirectory(const std::string& path) {
    std::string normalized = normalizeSeparators(path);
    
#ifdef PLATFORM_WINDOWS
    int result = _mkdir(normalized.c_str());
#else
    int result = mkdir(normalized.c_str(), 0755);
#endif
    
    return result == 0 || errno == EEXIST;
}

bool FileSystem::createDirectories(const std::string& path) {
    std::string normalized = normalizeSeparators(path);
    
    if (normalized.empty()) {
        return true;
    }
    
    // 逐级创建目录
    size_t pos = 0;
    while (pos < normalized.size()) {
        pos = normalized.find(PATH_SEPARATOR, pos + 1);
        if (pos == std::string::npos) {
            pos = normalized.size();
        }
        
        std::string subPath = normalized.substr(0, pos);
        if (!subPath.empty() && !directoryExists(subPath)) {
            if (!createDirectory(subPath)) {
                return false;
            }
        }
    }
    
    return true;
}

} // namespace extra2d
