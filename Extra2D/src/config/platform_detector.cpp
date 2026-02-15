#include <extra2d/config/platform_detector.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/utils/logger.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <psapi.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#include <pwd.h>
#include <cstdlib>
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace extra2d {

/**
 * @brief 检测当前运行平台
 * 使用编译时宏判断当前平台类型
 * @return 当前平台的类型枚举值
 */
PlatformType PlatformDetector::detect() {
#ifdef _WIN32
    return PlatformType::Windows;
#elif defined(__SWITCH__)
    return PlatformType::Switch;
#elif defined(__linux__)
    return PlatformType::Linux;
#elif defined(__APPLE__)
    return PlatformType::macOS;
#else
    return PlatformType::Windows;
#endif
}

/**
 * @brief 获取当前平台名称字符串
 * @return 平台名称（如 "Windows", "Linux", "macOS", "Switch"）
 */
const char* PlatformDetector::platformName() {
    return platformName(detect());
}

/**
 * @brief 获取指定平台类型的名称
 * @param type 平台类型
 * @return 平台名称字符串
 */
const char* PlatformDetector::platformName(PlatformType type) {
    switch (type) {
        case PlatformType::Windows: return "Windows";
        case PlatformType::Switch:  return "Nintendo Switch";
        case PlatformType::Linux:   return "Linux";
        case PlatformType::macOS:   return "macOS";
        case PlatformType::Auto:    return "Auto";
        default:                    return "Unknown";
    }
}

/**
 * @brief 检查当前平台是否为桌面平台
 * @return 如果是桌面平台返回 true
 */
bool PlatformDetector::isDesktopPlatform() {
    PlatformType type = detect();
    return type == PlatformType::Windows ||
           type == PlatformType::Linux ||
           type == PlatformType::macOS;
}

/**
 * @brief 检查当前平台是否为游戏主机平台
 * @return 如果是游戏主机平台返回 true
 */
bool PlatformDetector::isConsolePlatform() {
    return detect() == PlatformType::Switch;
}

/**
 * @brief 检查当前平台是否为移动平台
 * @return 如果是移动平台返回 true
 */
bool PlatformDetector::isMobilePlatform() {
    return false;
}

/**
 * @brief 获取当前平台的能力
 * @return 平台能力结构
 */
PlatformCapabilities PlatformDetector::capabilities() {
    return capabilities(detect());
}

/**
 * @brief 获取指定平台的能力
 * @param type 平台类型
 * @return 平台能力结构
 */
PlatformCapabilities PlatformDetector::capabilities(PlatformType type) {
    switch (type) {
        case PlatformType::Windows:
            return getWindowsCapabilities();
        case PlatformType::Switch:
            return getSwitchCapabilities();
        case PlatformType::Linux:
            return getLinuxCapabilities();
        case PlatformType::macOS:
            return getMacOSCapabilities();
        default:
            return getWindowsCapabilities();
    }
}

/**
 * @brief 获取当前平台的默认配置
 * @return 平台默认的应用配置
 */
AppConfig PlatformDetector::platformDefaults() {
    return platformDefaults(detect());
}

/**
 * @brief 获取指定平台的默认配置
 * @param type 平台类型
 * @return 平台默认的应用配置
 */
AppConfig PlatformDetector::platformDefaults(PlatformType type) {
    switch (type) {
        case PlatformType::Windows:
            return getWindowsDefaults();
        case PlatformType::Switch:
            return getSwitchDefaults();
        case PlatformType::Linux:
            return getLinuxDefaults();
        case PlatformType::macOS:
            return getMacOSDefaults();
        default:
            return AppConfig::createDefault();
    }
}

/**
 * @brief 获取当前平台的推荐分辨率
 * @param width 输出宽度
 * @param height 输出高度
 */
void PlatformDetector::getRecommendedResolution(int& width, int& height) {
    PlatformCapabilities caps = capabilities();
    width = caps.preferredScreenWidth;
    height = caps.preferredScreenHeight;
}

/**
 * @brief 获取当前平台的默认 DPI
 * @return 默认 DPI 值
 */
float PlatformDetector::getDefaultDPI() {
    return capabilities().defaultDPI;
}

/**
 * @brief 检查当前平台是否支持指定功能
 * @param feature 功能名称
 * @return 如果支持返回 true
 */
bool PlatformDetector::supportsFeature(const std::string& feature) {
    PlatformCapabilities caps = capabilities();
    
    if (feature == "windowed") return caps.supportsWindowed;
    if (feature == "fullscreen") return caps.supportsFullscreen;
    if (feature == "borderless") return caps.supportsBorderless;
    if (feature == "cursor") return caps.supportsCursor;
    if (feature == "cursor_hide") return caps.supportsCursorHide;
    if (feature == "dpi_awareness") return caps.supportsDPIAwareness;
    if (feature == "vsync") return caps.supportsVSync;
    if (feature == "multi_monitor") return caps.supportsMultiMonitor;
    if (feature == "clipboard") return caps.supportsClipboard;
    if (feature == "gamepad") return caps.supportsGamepad;
    if (feature == "touch") return caps.supportsTouch;
    if (feature == "keyboard") return caps.supportsKeyboard;
    if (feature == "mouse") return caps.supportsMouse;
    if (feature == "resize") return caps.supportsResize;
    if (feature == "high_dpi") return caps.supportsHighDPI;
    
    return false;
}

/**
 * @brief 获取系统内存大小
 * @return 系统内存大小（MB），如果无法获取返回 0
 */
int PlatformDetector::getSystemMemoryMB() {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return static_cast<int>(status.ullTotalPhys / (1024 * 1024));
    }
    return 0;
#elif defined(__SWITCH__)
    return 4096;
#elif defined(__linux__)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<int>(info.totalram * info.mem_unit / (1024 * 1024));
    }
    return 0;
#elif defined(__APPLE__)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    int64_t memSize = 0;
    size_t length = sizeof(memSize);
    if (sysctl(mib, 2, &memSize, &length, nullptr, 0) == 0) {
        return static_cast<int>(memSize / (1024 * 1024));
    }
    return 0;
#else
    return 0;
#endif
}

/**
 * @brief 获取 CPU 核心数
 * @return CPU 核心数
 */
int PlatformDetector::getCPUCoreCount() {
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return static_cast<int>(sysinfo.dwNumberOfProcessors);
#elif defined(__SWITCH__)
    return 4;
#elif defined(__linux__) || defined(__APPLE__)
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    return static_cast<int>(cores > 0 ? cores : 1);
#else
    return 1;
#endif
}

/**
 * @brief 检查是否支持多线程渲染
 * @return 如果支持返回 true
 */
bool PlatformDetector::supportsMultithreadedRendering() {
#ifdef __SWITCH__
    return false;
#else
    return getCPUCoreCount() >= 2;
#endif
}

/**
 * @brief 获取平台特定的配置路径
 * @param appName 应用名称
 * @return 配置文件目录路径
 */
std::string PlatformDetector::getConfigPath(const std::string& appName) {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\" + appName + "\\config";
    }
    return ".\\config";
#elif defined(__SWITCH__)
    return "sdmc:/config/" + appName;
#elif defined(__linux__)
    const char* configHome = getenv("XDG_CONFIG_HOME");
    if (configHome && configHome[0] != '\0') {
        return std::string(configHome) + "/" + appName;
    }
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/.config/" + appName;
    }
    return "./config";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/Library/Application Support/" + appName + "/config";
    }
    return "./config";
#else
    return "./config";
#endif
}

/**
 * @brief 获取平台特定的存档路径
 * @param appName 应用名称
 * @return 存档文件目录路径
 */
std::string PlatformDetector::getSavePath(const std::string& appName) {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\" + appName + "\\saves";
    }
    return ".\\saves";
#elif defined(__SWITCH__)
    return "sdmc:/saves/" + appName;
#elif defined(__linux__)
    const char* dataHome = getenv("XDG_DATA_HOME");
    if (dataHome && dataHome[0] != '\0') {
        return std::string(dataHome) + "/" + appName + "/saves";
    }
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/.local/share/" + appName + "/saves";
    }
    return "./saves";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/Library/Application Support/" + appName + "/saves";
    }
    return "./saves";
#else
    return "./saves";
#endif
}

/**
 * @brief 获取平台特定的缓存路径
 * @param appName 应用名称
 * @return 缓存文件目录路径
 */
std::string PlatformDetector::getCachePath(const std::string& appName) {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\" + appName + "\\cache";
    }
    return ".\\cache";
#elif defined(__SWITCH__)
    return "sdmc:/cache/" + appName;
#elif defined(__linux__)
    const char* cacheHome = getenv("XDG_CACHE_HOME");
    if (cacheHome && cacheHome[0] != '\0') {
        return std::string(cacheHome) + "/" + appName;
    }
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/.cache/" + appName;
    }
    return "./cache";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/Library/Caches/" + appName;
    }
    return "./cache";
#else
    return "./cache";
#endif
}

/**
 * @brief 获取平台特定的日志路径
 * @param appName 应用名称
 * @return 日志文件目录路径
 */
std::string PlatformDetector::getLogPath(const std::string& appName) {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, path))) {
        return std::string(path) + "\\" + appName + "\\logs";
    }
    return ".\\logs";
#elif defined(__SWITCH__)
    return "sdmc:/logs/" + appName;
#elif defined(__linux__)
    const char* cacheHome = getenv("XDG_CACHE_HOME");
    if (cacheHome && cacheHome[0] != '\0') {
        return std::string(cacheHome) + "/" + appName + "/logs";
    }
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/.cache/" + appName + "/logs";
    }
    return "./logs";
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) home = pwd->pw_dir;
    }
    if (home) {
        return std::string(home) + "/Library/Logs/" + appName;
    }
    return "./logs";
#else
    return "./logs";
#endif
}

/**
 * @brief 获取平台特定的资源路径（Shader、纹理等）
 * Switch平台使用romfs，其他平台使用相对路径
 * @param appName 应用名称
 * @return 资源目录路径
 */
std::string PlatformDetector::getResourcePath(const std::string& appName) {
#ifdef __SWITCH__
    (void)appName;
    return "romfs:/";
#else
    (void)appName;
    return "./resources/";
#endif
}

/**
 * @brief 获取平台特定的Shader路径
 * @param appName 应用名称
 * @return Shader目录路径
 */
std::string PlatformDetector::getShaderPath(const std::string& appName) {
#ifdef __SWITCH__
    (void)appName;
    return "romfs:/shaders/";
#else
    (void)appName;
    return "./shaders/";
#endif
}

/**
 * @brief 获取平台特定的Shader缓存路径
 * Switch平台使用sdmc，其他平台使用系统缓存目录
 * @param appName 应用名称
 * @return Shader缓存目录路径
 */
std::string PlatformDetector::getShaderCachePath(const std::string& appName) {
#ifdef __SWITCH__
    std::string name = appName.empty() ? "extra2d" : appName;
    return "sdmc:/cache/" + name + "/shaders/";
#else
    return getCachePath(appName.empty() ? "extra2d" : appName) + "/shaders/";
#endif
}

/**
 * @brief 检查平台是否使用romfs（只读文件系统）
 * @return 使用romfs返回true
 */
bool PlatformDetector::usesRomfs() {
#ifdef __SWITCH__
    return true;
#else
    return false;
#endif
}

/**
 * @brief 检查平台是否支持热重载
 * Switch平台不支持热重载（romfs只读）
 * @return 支持热重载返回true
 */
bool PlatformDetector::supportsHotReload() {
#ifdef __SWITCH__
    return false;
#else
    return true;
#endif
}

/**
 * @brief 检查平台是否为小端字节序
 * @return 如果是小端字节序返回 true
 */
bool PlatformDetector::isLittleEndian() {
    union {
        uint32_t i;
        char c[4];
    } test = {0x01020304};
    return test.c[0] == 0x04;
}

/**
 * @brief 检查平台是否为大端字节序
 * @return 如果是大端字节序返回 true
 */
bool PlatformDetector::isBigEndian() {
    return !isLittleEndian();
}

/**
 * @brief 获取平台信息摘要
 * @return 平台信息字符串
 */
std::string PlatformDetector::getPlatformSummary() {
    std::string summary;
    summary += "Platform: ";
    summary += platformName();
    summary += "\n";
    summary += "Memory: ";
    summary += std::to_string(getSystemMemoryMB());
    summary += " MB\n";
    summary += "CPU Cores: ";
    summary += std::to_string(getCPUCoreCount());
    summary += "\n";
    summary += "Endianness: ";
    summary += isLittleEndian() ? "Little Endian" : "Big Endian";
    summary += "\n";
    summary += "Desktop Platform: ";
    summary += isDesktopPlatform() ? "Yes" : "No";
    summary += "\n";
    summary += "Console Platform: ";
    summary += isConsolePlatform() ? "Yes" : "No";
    summary += "\n";
    summary += "Recommended Resolution: ";
    int width, height;
    getRecommendedResolution(width, height);
    summary += std::to_string(width);
    summary += "x";
    summary += std::to_string(height);
    summary += "\n";
    summary += "Default DPI: ";
    summary += std::to_string(static_cast<int>(getDefaultDPI()));
    return summary;
}

PlatformCapabilities PlatformDetector::getWindowsCapabilities() {
    PlatformCapabilities caps;
    caps.supportsWindowed = true;
    caps.supportsFullscreen = true;
    caps.supportsBorderless = true;
    caps.supportsCursor = true;
    caps.supportsCursorHide = true;
    caps.supportsDPIAwareness = true;
    caps.supportsVSync = true;
    caps.supportsMultiMonitor = true;
    caps.supportsClipboard = true;
    caps.supportsGamepad = true;
    caps.supportsTouch = false;
    caps.supportsKeyboard = true;
    caps.supportsMouse = true;
    caps.supportsResize = true;
    caps.supportsHighDPI = true;
    caps.maxTextureSize = 16384;
    caps.preferredScreenWidth = 1920;
    caps.preferredScreenHeight = 1080;
    caps.defaultDPI = 96.0f;
    return caps;
}

PlatformCapabilities PlatformDetector::getLinuxCapabilities() {
    PlatformCapabilities caps;
    caps.supportsWindowed = true;
    caps.supportsFullscreen = true;
    caps.supportsBorderless = true;
    caps.supportsCursor = true;
    caps.supportsCursorHide = true;
    caps.supportsDPIAwareness = true;
    caps.supportsVSync = true;
    caps.supportsMultiMonitor = true;
    caps.supportsClipboard = true;
    caps.supportsGamepad = true;
    caps.supportsTouch = false;
    caps.supportsKeyboard = true;
    caps.supportsMouse = true;
    caps.supportsResize = true;
    caps.supportsHighDPI = true;
    caps.maxTextureSize = 16384;
    caps.preferredScreenWidth = 1920;
    caps.preferredScreenHeight = 1080;
    caps.defaultDPI = 96.0f;
    return caps;
}

PlatformCapabilities PlatformDetector::getMacOSCapabilities() {
    PlatformCapabilities caps;
    caps.supportsWindowed = true;
    caps.supportsFullscreen = true;
    caps.supportsBorderless = true;
    caps.supportsCursor = true;
    caps.supportsCursorHide = true;
    caps.supportsDPIAwareness = true;
    caps.supportsVSync = true;
    caps.supportsMultiMonitor = true;
    caps.supportsClipboard = true;
    caps.supportsGamepad = true;
    caps.supportsTouch = false;
    caps.supportsKeyboard = true;
    caps.supportsMouse = true;
    caps.supportsResize = true;
    caps.supportsHighDPI = true;
    caps.maxTextureSize = 16384;
    caps.preferredScreenWidth = 1920;
    caps.preferredScreenHeight = 1080;
    caps.defaultDPI = 144.0f;
    return caps;
}

PlatformCapabilities PlatformDetector::getSwitchCapabilities() {
    PlatformCapabilities caps;
    caps.supportsWindowed = false;
    caps.supportsFullscreen = true;
    caps.supportsBorderless = false;
    caps.supportsCursor = false;
    caps.supportsCursorHide = false;
    caps.supportsDPIAwareness = false;
    caps.supportsVSync = true;
    caps.supportsMultiMonitor = false;
    caps.supportsClipboard = false;
    caps.supportsGamepad = true;
    caps.supportsTouch = true;
    caps.supportsKeyboard = false;
    caps.supportsMouse = false;
    caps.supportsResize = false;
    caps.supportsHighDPI = false;
    caps.maxTextureSize = 8192;
    caps.preferredScreenWidth = 1920;
    caps.preferredScreenHeight = 1080;
    caps.defaultDPI = 96.0f;
    return caps;
}

AppConfig PlatformDetector::getWindowsDefaults() {
    AppConfig config = AppConfig::createDefault();
    config.window.highDPI = true;
    config.window.resizable = true;
    config.render.vsync = true;
    config.render.targetFPS = 60;
    return config;
}

AppConfig PlatformDetector::getLinuxDefaults() {
    AppConfig config = AppConfig::createDefault();
    config.window.resizable = true;
    config.render.vsync = true;
    return config;
}

AppConfig PlatformDetector::getMacOSDefaults() {
    AppConfig config = AppConfig::createDefault();
    config.window.highDPI = true;
    config.window.resizable = true;
    config.render.vsync = true;
    return config;
}

AppConfig PlatformDetector::getSwitchDefaults() {
    AppConfig config = AppConfig::createDefault();
    config.window.width = 1920;
    config.window.height = 1080;
    config.window.mode = WindowMode::Fullscreen;
    config.window.resizable = false;
    config.window.highDPI = false;
    config.render.vsync = true;
    config.render.targetFPS = 60;
    config.input.enableVibration = true;
    config.input.maxGamepads = 2;
    return config;
}

}
