#include <extra2d/config/config_loader.h>
#include <extra2d/utils/logger.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>

namespace extra2d {

/**
 * @brief 去除字符串首尾空白字符
 * @param str 输入字符串
 * @return 去除空白后的字符串
 */
static std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    size_t end = str.length();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return str.substr(start, end - start);
}

/**
 * @brief 将字符串转换为小写
 * @param str 输入字符串
 * @return 小写字符串
 */
static std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

/**
 * @brief 将字符串转换为窗口模式枚举
 * @param modeStr 窗口模式字符串
 * @return 窗口模式枚举值
 */
static WindowMode stringToWindowMode(const std::string& modeStr) {
    std::string lower = toLower(modeStr);
    if (lower == "fullscreen") return WindowMode::Fullscreen;
    if (lower == "borderless") return WindowMode::Borderless;
    return WindowMode::Windowed;
}

/**
 * @brief 将窗口模式枚举转换为字符串
 * @param mode 窗口模式枚举值
 * @return 窗口模式字符串
 */
static std::string windowModeToString(WindowMode mode) {
    switch (mode) {
        case WindowMode::Fullscreen: return "fullscreen";
        case WindowMode::Borderless: return "borderless";
        default: return "windowed";
    }
}

/**
 * @brief 将字符串转换为渲染后端类型枚举
 * @param backendStr 后端类型字符串
 * @return 渲染后端类型枚举值
 */
static BackendType stringToBackendType(const std::string& backendStr) {
    std::string lower = toLower(backendStr);
    if (lower == "opengl") return BackendType::OpenGL;
    return BackendType::OpenGL;
}

/**
 * @brief 将渲染后端类型枚举转换为字符串
 * @param backend 渲染后端类型枚举值
 * @return 后端类型字符串
 */
static std::string backendTypeToString(BackendType backend) {
    switch (backend) {
        case BackendType::OpenGL: return "opengl";
        default: return "opengl";
    }
}

/**
 * @brief INI 数据存储结构
 */
using IniData = std::map<std::string, std::map<std::string, std::string>>;

/**
 * @brief 解析 INI 内容到数据结构
 * @param content INI 内容字符串
 * @param data 输出的 INI 数据
 * @return 解析结果
 */
static ConfigLoadResult parseIniContent(const std::string& content, IniData& data) {
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;
    int lineNumber = 0;
    
    while (std::getline(stream, line)) {
        ++lineNumber;
        line = trim(line);
        
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        if (line[0] == '[') {
            size_t endBracket = line.find(']');
            if (endBracket == std::string::npos) {
                return ConfigLoadResult::error("INI 解析错误: 缺少右括号", lineNumber);
            }
            currentSection = trim(line.substr(1, endBracket - 1));
            if (data.find(currentSection) == data.end()) {
                data[currentSection] = std::map<std::string, std::string>();
            }
        } else {
            size_t equalPos = line.find('=');
            if (equalPos == std::string::npos) {
                continue;
            }
            
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            
            if (currentSection.empty()) {
                return ConfigLoadResult::error("INI 解析错误: 键值对不在任何节中", lineNumber);
            }
            
            data[currentSection][key] = value;
        }
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 获取 INI 值（带默认值）
 * @param data INI 数据
 * @param section 节名
 * @param key 键名
 * @param defaultValue 默认值
 * @return 值字符串
 */
static std::string getIniValue(const IniData& data, const std::string& section, 
                               const std::string& key, const std::string& defaultValue = "") {
    auto sectionIt = data.find(section);
    if (sectionIt == data.end()) {
        return defaultValue;
    }
    auto keyIt = sectionIt->second.find(key);
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    return keyIt->second;
}

/**
 * @brief 检查 INI 值是否存在
 * @param data INI 数据
 * @param section 节名
 * @param key 键名
 * @return 是否存在
 */
static bool hasIniValue(const IniData& data, const std::string& section, const std::string& key) {
    auto sectionIt = data.find(section);
    if (sectionIt == data.end()) {
        return false;
    }
    return sectionIt->second.find(key) != sectionIt->second.end();
}

// ============================================================================
// IniConfigLoader 实现
// ============================================================================

/**
 * @brief 从 INI 文件加载配置
 * @param filepath 配置文件路径
 * @param config 输出的配置对象
 * @return 加载结果
 */
ConfigLoadResult IniConfigLoader::load(const std::string& filepath, AppConfig& config) {
    E2D_LOG_INFO("正在从 INI 文件加载配置: {}", filepath);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法打开配置文件: {}", filepath);
        return ConfigLoadResult::error("无法打开配置文件: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    return loadFromString(content, config);
}

/**
 * @brief 保存配置到 INI 文件
 * @param filepath 配置文件路径
 * @param config 要保存的配置对象
 * @return 保存结果
 */
ConfigSaveResult IniConfigLoader::save(const std::string& filepath, const AppConfig& config) {
    E2D_LOG_INFO("正在保存配置到 INI 文件: {}", filepath);
    
    std::string content = saveToString(config);
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法创建配置文件: {}", filepath);
        return ConfigSaveResult::error("无法创建配置文件: " + filepath);
    }
    
    file << content;
    file.close();
    
    E2D_LOG_INFO("配置已成功保存到: {}", filepath);
    return ConfigSaveResult::ok();
}

/**
 * @brief 从 INI 字符串加载配置
 * @param content INI 内容字符串
 * @param config 输出的配置对象
 * @return 加载结果
 */
ConfigLoadResult IniConfigLoader::loadFromString(const std::string& content, AppConfig& config) {
    IniData data;
    auto result = parseIniContent(content, data);
    if (result.hasError()) {
        return result;
    }
    
    if (hasIniValue(data, "app", "name")) {
        config.appName = getIniValue(data, "app", "name");
    }
    if (hasIniValue(data, "app", "version")) {
        config.appVersion = getIniValue(data, "app", "version");
    }
    if (hasIniValue(data, "app", "organization")) {
        config.organization = getIniValue(data, "app", "organization");
    }
    
    if (hasIniValue(data, "window", "title")) {
        config.window.title = getIniValue(data, "window", "title");
    }
    if (hasIniValue(data, "window", "width")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "width"), value, "window.width");
        if (res.hasError()) return res;
        config.window.width = value;
    }
    if (hasIniValue(data, "window", "height")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "height"), value, "window.height");
        if (res.hasError()) return res;
        config.window.height = value;
    }
    if (hasIniValue(data, "window", "minWidth")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "minWidth"), value, "window.minWidth");
        if (res.hasError()) return res;
        config.window.minWidth = value;
    }
    if (hasIniValue(data, "window", "minHeight")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "minHeight"), value, "window.minHeight");
        if (res.hasError()) return res;
        config.window.minHeight = value;
    }
    if (hasIniValue(data, "window", "maxWidth")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "maxWidth"), value, "window.maxWidth");
        if (res.hasError()) return res;
        config.window.maxWidth = value;
    }
    if (hasIniValue(data, "window", "maxHeight")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "maxHeight"), value, "window.maxHeight");
        if (res.hasError()) return res;
        config.window.maxHeight = value;
    }
    if (hasIniValue(data, "window", "mode")) {
        config.window.mode = stringToWindowMode(getIniValue(data, "window", "mode"));
    }
    if (hasIniValue(data, "window", "resizable")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "resizable"), value, "window.resizable");
        if (res.hasError()) return res;
        config.window.resizable = value;
    }
    if (hasIniValue(data, "window", "borderless")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "borderless"), value, "window.borderless");
        if (res.hasError()) return res;
        config.window.borderless = value;
    }
    if (hasIniValue(data, "window", "alwaysOnTop")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "alwaysOnTop"), value, "window.alwaysOnTop");
        if (res.hasError()) return res;
        config.window.alwaysOnTop = value;
    }
    if (hasIniValue(data, "window", "centered")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "centered"), value, "window.centered");
        if (res.hasError()) return res;
        config.window.centered = value;
    }
    if (hasIniValue(data, "window", "posX")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "posX"), value, "window.posX");
        if (res.hasError()) return res;
        config.window.posX = value;
    }
    if (hasIniValue(data, "window", "posY")) {
        int value;
        auto res = parseInt(getIniValue(data, "window", "posY"), value, "window.posY");
        if (res.hasError()) return res;
        config.window.posY = value;
    }
    if (hasIniValue(data, "window", "hideOnClose")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "hideOnClose"), value, "window.hideOnClose");
        if (res.hasError()) return res;
        config.window.hideOnClose = value;
    }
    if (hasIniValue(data, "window", "minimizeOnClose")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "minimizeOnClose"), value, "window.minimizeOnClose");
        if (res.hasError()) return res;
        config.window.minimizeOnClose = value;
    }
    if (hasIniValue(data, "window", "opacity")) {
        float value;
        auto res = parseFloat(getIniValue(data, "window", "opacity"), value, "window.opacity");
        if (res.hasError()) return res;
        config.window.opacity = value;
    }
    if (hasIniValue(data, "window", "transparentFramebuffer")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "transparentFramebuffer"), value, "window.transparentFramebuffer");
        if (res.hasError()) return res;
        config.window.transparentFramebuffer = value;
    }
    if (hasIniValue(data, "window", "highDPI")) {
        bool value;
        auto res = parseBool(getIniValue(data, "window", "highDPI"), value, "window.highDPI");
        if (res.hasError()) return res;
        config.window.highDPI = value;
    }
    if (hasIniValue(data, "window", "contentScale")) {
        float value;
        auto res = parseFloat(getIniValue(data, "window", "contentScale"), value, "window.contentScale");
        if (res.hasError()) return res;
        config.window.contentScale = value;
    }
    
    if (hasIniValue(data, "render", "backend")) {
        config.render.backend = stringToBackendType(getIniValue(data, "render", "backend"));
    }
    if (hasIniValue(data, "render", "targetFPS")) {
        int value;
        auto res = parseInt(getIniValue(data, "render", "targetFPS"), value, "render.targetFPS");
        if (res.hasError()) return res;
        config.render.targetFPS = value;
    }
    if (hasIniValue(data, "render", "vsync")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "vsync"), value, "render.vsync");
        if (res.hasError()) return res;
        config.render.vsync = value;
    }
    if (hasIniValue(data, "render", "tripleBuffering")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "tripleBuffering"), value, "render.tripleBuffering");
        if (res.hasError()) return res;
        config.render.tripleBuffering = value;
    }
    if (hasIniValue(data, "render", "multisamples")) {
        int value;
        auto res = parseInt(getIniValue(data, "render", "multisamples"), value, "render.multisamples");
        if (res.hasError()) return res;
        config.render.multisamples = value;
    }
    if (hasIniValue(data, "render", "sRGBFramebuffer")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "sRGBFramebuffer"), value, "render.sRGBFramebuffer");
        if (res.hasError()) return res;
        config.render.sRGBFramebuffer = value;
    }
    if (hasIniValue(data, "render", "clearColorR")) {
        float value;
        auto res = parseFloat(getIniValue(data, "render", "clearColorR"), value, "render.clearColorR");
        if (res.hasError()) return res;
        config.render.clearColor.r = value;
    }
    if (hasIniValue(data, "render", "clearColorG")) {
        float value;
        auto res = parseFloat(getIniValue(data, "render", "clearColorG"), value, "render.clearColorG");
        if (res.hasError()) return res;
        config.render.clearColor.g = value;
    }
    if (hasIniValue(data, "render", "clearColorB")) {
        float value;
        auto res = parseFloat(getIniValue(data, "render", "clearColorB"), value, "render.clearColorB");
        if (res.hasError()) return res;
        config.render.clearColor.b = value;
    }
    if (hasIniValue(data, "render", "clearColorA")) {
        float value;
        auto res = parseFloat(getIniValue(data, "render", "clearColorA"), value, "render.clearColorA");
        if (res.hasError()) return res;
        config.render.clearColor.a = value;
    }
    if (hasIniValue(data, "render", "maxTextureSize")) {
        int value;
        auto res = parseInt(getIniValue(data, "render", "maxTextureSize"), value, "render.maxTextureSize");
        if (res.hasError()) return res;
        config.render.maxTextureSize = value;
    }
    if (hasIniValue(data, "render", "textureAnisotropy")) {
        int value;
        auto res = parseInt(getIniValue(data, "render", "textureAnisotropy"), value, "render.textureAnisotropy");
        if (res.hasError()) return res;
        config.render.textureAnisotropy = value;
    }
    if (hasIniValue(data, "render", "wireframeMode")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "wireframeMode"), value, "render.wireframeMode");
        if (res.hasError()) return res;
        config.render.wireframeMode = value;
    }
    if (hasIniValue(data, "render", "depthTest")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "depthTest"), value, "render.depthTest");
        if (res.hasError()) return res;
        config.render.depthTest = value;
    }
    if (hasIniValue(data, "render", "blending")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "blending"), value, "render.blending");
        if (res.hasError()) return res;
        config.render.blending = value;
    }
    if (hasIniValue(data, "render", "dithering")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "dithering"), value, "render.dithering");
        if (res.hasError()) return res;
        config.render.dithering = value;
    }
    if (hasIniValue(data, "render", "spriteBatchSize")) {
        int value;
        auto res = parseInt(getIniValue(data, "render", "spriteBatchSize"), value, "render.spriteBatchSize");
        if (res.hasError()) return res;
        config.render.spriteBatchSize = value;
    }
    if (hasIniValue(data, "render", "maxRenderTargets")) {
        int value;
        auto res = parseInt(getIniValue(data, "render", "maxRenderTargets"), value, "render.maxRenderTargets");
        if (res.hasError()) return res;
        config.render.maxRenderTargets = value;
    }
    if (hasIniValue(data, "render", "allowShaderHotReload")) {
        bool value;
        auto res = parseBool(getIniValue(data, "render", "allowShaderHotReload"), value, "render.allowShaderHotReload");
        if (res.hasError()) return res;
        config.render.allowShaderHotReload = value;
    }
    if (hasIniValue(data, "render", "shaderCachePath")) {
        config.render.shaderCachePath = getIniValue(data, "render", "shaderCachePath");
    }
    
    if (hasIniValue(data, "audio", "enabled")) {
        bool value;
        auto res = parseBool(getIniValue(data, "audio", "enabled"), value, "audio.enabled");
        if (res.hasError()) return res;
        config.audio.enabled = value;
    }
    if (hasIniValue(data, "audio", "masterVolume")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "masterVolume"), value, "audio.masterVolume");
        if (res.hasError()) return res;
        config.audio.masterVolume = value;
    }
    if (hasIniValue(data, "audio", "musicVolume")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "musicVolume"), value, "audio.musicVolume");
        if (res.hasError()) return res;
        config.audio.musicVolume = value;
    }
    if (hasIniValue(data, "audio", "sfxVolume")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "sfxVolume"), value, "audio.sfxVolume");
        if (res.hasError()) return res;
        config.audio.sfxVolume = value;
    }
    if (hasIniValue(data, "audio", "voiceVolume")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "voiceVolume"), value, "audio.voiceVolume");
        if (res.hasError()) return res;
        config.audio.voiceVolume = value;
    }
    if (hasIniValue(data, "audio", "ambientVolume")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "ambientVolume"), value, "audio.ambientVolume");
        if (res.hasError()) return res;
        config.audio.ambientVolume = value;
    }
    if (hasIniValue(data, "audio", "frequency")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "frequency"), value, "audio.frequency");
        if (res.hasError()) return res;
        config.audio.frequency = value;
    }
    if (hasIniValue(data, "audio", "channels")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "channels"), value, "audio.channels");
        if (res.hasError()) return res;
        config.audio.channels = value;
    }
    if (hasIniValue(data, "audio", "chunkSize")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "chunkSize"), value, "audio.chunkSize");
        if (res.hasError()) return res;
        config.audio.chunkSize = value;
    }
    if (hasIniValue(data, "audio", "maxChannels")) {
        int value;
        auto res = parseInt(getIniValue(data, "audio", "maxChannels"), value, "audio.maxChannels");
        if (res.hasError()) return res;
        config.audio.maxChannels = value;
    }
    if (hasIniValue(data, "audio", "spatialAudio")) {
        bool value;
        auto res = parseBool(getIniValue(data, "audio", "spatialAudio"), value, "audio.spatialAudio");
        if (res.hasError()) return res;
        config.audio.spatialAudio = value;
    }
    
    if (hasIniValue(data, "debug", "enabled")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "enabled"), value, "debug.enabled");
        if (res.hasError()) return res;
        config.debug.enabled = value;
    }
    if (hasIniValue(data, "debug", "showFPS")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "showFPS"), value, "debug.showFPS");
        if (res.hasError()) return res;
        config.debug.showFPS = value;
    }
    if (hasIniValue(data, "debug", "showMemoryUsage")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "showMemoryUsage"), value, "debug.showMemoryUsage");
        if (res.hasError()) return res;
        config.debug.showMemoryUsage = value;
    }
    if (hasIniValue(data, "debug", "showRenderStats")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "showRenderStats"), value, "debug.showRenderStats");
        if (res.hasError()) return res;
        config.debug.showRenderStats = value;
    }
    if (hasIniValue(data, "debug", "showColliders")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "showColliders"), value, "debug.showColliders");
        if (res.hasError()) return res;
        config.debug.showColliders = value;
    }
    if (hasIniValue(data, "debug", "showGrid")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "showGrid"), value, "debug.showGrid");
        if (res.hasError()) return res;
        config.debug.showGrid = value;
    }
    if (hasIniValue(data, "debug", "logToFile")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "logToFile"), value, "debug.logToFile");
        if (res.hasError()) return res;
        config.debug.logToFile = value;
    }
    if (hasIniValue(data, "debug", "logToConsole")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "logToConsole"), value, "debug.logToConsole");
        if (res.hasError()) return res;
        config.debug.logToConsole = value;
    }
    if (hasIniValue(data, "debug", "logLevel")) {
        int value;
        auto res = parseInt(getIniValue(data, "debug", "logLevel"), value, "debug.logLevel");
        if (res.hasError()) return res;
        config.debug.logLevel = value;
    }
    if (hasIniValue(data, "debug", "breakOnAssert")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "breakOnAssert"), value, "debug.breakOnAssert");
        if (res.hasError()) return res;
        config.debug.breakOnAssert = value;
    }
    if (hasIniValue(data, "debug", "enableProfiling")) {
        bool value;
        auto res = parseBool(getIniValue(data, "debug", "enableProfiling"), value, "debug.enableProfiling");
        if (res.hasError()) return res;
        config.debug.enableProfiling = value;
    }
    if (hasIniValue(data, "debug", "logFilePath")) {
        config.debug.logFilePath = getIniValue(data, "debug", "logFilePath");
    }
    
    if (hasIniValue(data, "input", "enabled")) {
        bool value;
        auto res = parseBool(getIniValue(data, "input", "enabled"), value, "input.enabled");
        if (res.hasError()) return res;
        config.input.enabled = value;
    }
    if (hasIniValue(data, "input", "rawMouseInput")) {
        bool value;
        auto res = parseBool(getIniValue(data, "input", "rawMouseInput"), value, "input.rawMouseInput");
        if (res.hasError()) return res;
        config.input.rawMouseInput = value;
    }
    if (hasIniValue(data, "input", "mouseSensitivity")) {
        float value;
        auto res = parseFloat(getIniValue(data, "input", "mouseSensitivity"), value, "input.mouseSensitivity");
        if (res.hasError()) return res;
        config.input.mouseSensitivity = value;
    }
    if (hasIniValue(data, "input", "invertMouseY")) {
        bool value;
        auto res = parseBool(getIniValue(data, "input", "invertMouseY"), value, "input.invertMouseY");
        if (res.hasError()) return res;
        config.input.invertMouseY = value;
    }
    if (hasIniValue(data, "input", "invertMouseX")) {
        bool value;
        auto res = parseBool(getIniValue(data, "input", "invertMouseX"), value, "input.invertMouseX");
        if (res.hasError()) return res;
        config.input.invertMouseX = value;
    }
    if (hasIniValue(data, "input", "deadzone")) {
        float value;
        auto res = parseFloat(getIniValue(data, "input", "deadzone"), value, "input.deadzone");
        if (res.hasError()) return res;
        config.input.deadzone = value;
    }
    if (hasIniValue(data, "input", "triggerThreshold")) {
        float value;
        auto res = parseFloat(getIniValue(data, "input", "triggerThreshold"), value, "input.triggerThreshold");
        if (res.hasError()) return res;
        config.input.triggerThreshold = value;
    }
    if (hasIniValue(data, "input", "enableVibration")) {
        bool value;
        auto res = parseBool(getIniValue(data, "input", "enableVibration"), value, "input.enableVibration");
        if (res.hasError()) return res;
        config.input.enableVibration = value;
    }
    if (hasIniValue(data, "input", "maxGamepads")) {
        int value;
        auto res = parseInt(getIniValue(data, "input", "maxGamepads"), value, "input.maxGamepads");
        if (res.hasError()) return res;
        config.input.maxGamepads = value;
    }
    if (hasIniValue(data, "input", "autoConnectGamepads")) {
        bool value;
        auto res = parseBool(getIniValue(data, "input", "autoConnectGamepads"), value, "input.autoConnectGamepads");
        if (res.hasError()) return res;
        config.input.autoConnectGamepads = value;
    }
    if (hasIniValue(data, "input", "gamepadMappingFile")) {
        config.input.gamepadMappingFile = getIniValue(data, "input", "gamepadMappingFile");
    }
    
    if (hasIniValue(data, "resource", "assetRootPath")) {
        config.resource.assetRootPath = getIniValue(data, "resource", "assetRootPath");
    }
    if (hasIniValue(data, "resource", "cachePath")) {
        config.resource.cachePath = getIniValue(data, "resource", "cachePath");
    }
    if (hasIniValue(data, "resource", "savePath")) {
        config.resource.savePath = getIniValue(data, "resource", "savePath");
    }
    if (hasIniValue(data, "resource", "configPath")) {
        config.resource.configPath = getIniValue(data, "resource", "configPath");
    }
    if (hasIniValue(data, "resource", "logPath")) {
        config.resource.logPath = getIniValue(data, "resource", "logPath");
    }
    if (hasIniValue(data, "resource", "useAssetCache")) {
        bool value;
        auto res = parseBool(getIniValue(data, "resource", "useAssetCache"), value, "resource.useAssetCache");
        if (res.hasError()) return res;
        config.resource.useAssetCache = value;
    }
    if (hasIniValue(data, "resource", "maxCacheSize")) {
        int value;
        auto res = parseInt(getIniValue(data, "resource", "maxCacheSize"), value, "resource.maxCacheSize");
        if (res.hasError()) return res;
        config.resource.maxCacheSize = value;
    }
    if (hasIniValue(data, "resource", "hotReloadEnabled")) {
        bool value;
        auto res = parseBool(getIniValue(data, "resource", "hotReloadEnabled"), value, "resource.hotReloadEnabled");
        if (res.hasError()) return res;
        config.resource.hotReloadEnabled = value;
    }
    if (hasIniValue(data, "resource", "hotReloadInterval")) {
        float value;
        auto res = parseFloat(getIniValue(data, "resource", "hotReloadInterval"), value, "resource.hotReloadInterval");
        if (res.hasError()) return res;
        config.resource.hotReloadInterval = value;
    }
    if (hasIniValue(data, "resource", "compressTextures")) {
        bool value;
        auto res = parseBool(getIniValue(data, "resource", "compressTextures"), value, "resource.compressTextures");
        if (res.hasError()) return res;
        config.resource.compressTextures = value;
    }
    if (hasIniValue(data, "resource", "preloadCommonAssets")) {
        bool value;
        auto res = parseBool(getIniValue(data, "resource", "preloadCommonAssets"), value, "resource.preloadCommonAssets");
        if (res.hasError()) return res;
        config.resource.preloadCommonAssets = value;
    }
    
    E2D_LOG_INFO("INI 配置加载成功");
    return ConfigLoadResult::ok();
}

/**
 * @brief 将配置序列化为 INI 字符串
 * @param config 配置对象
 * @return 序列化后的 INI 字符串
 */
std::string IniConfigLoader::saveToString(const AppConfig& config) {
    std::ostringstream oss;
    
    oss << "[app]\n";
    oss << "name=" << config.appName << "\n";
    oss << "version=" << config.appVersion << "\n";
    oss << "organization=" << config.organization << "\n";
    oss << "\n";
    
    oss << "[window]\n";
    oss << "title=" << config.window.title << "\n";
    oss << "width=" << config.window.width << "\n";
    oss << "height=" << config.window.height << "\n";
    oss << "minWidth=" << config.window.minWidth << "\n";
    oss << "minHeight=" << config.window.minHeight << "\n";
    oss << "maxWidth=" << config.window.maxWidth << "\n";
    oss << "maxHeight=" << config.window.maxHeight << "\n";
    oss << "mode=" << windowModeToString(config.window.mode) << "\n";
    oss << "resizable=" << (config.window.resizable ? "true" : "false") << "\n";
    oss << "borderless=" << (config.window.borderless ? "true" : "false") << "\n";
    oss << "alwaysOnTop=" << (config.window.alwaysOnTop ? "true" : "false") << "\n";
    oss << "centered=" << (config.window.centered ? "true" : "false") << "\n";
    oss << "posX=" << config.window.posX << "\n";
    oss << "posY=" << config.window.posY << "\n";
    oss << "hideOnClose=" << (config.window.hideOnClose ? "true" : "false") << "\n";
    oss << "minimizeOnClose=" << (config.window.minimizeOnClose ? "true" : "false") << "\n";
    oss << "opacity=" << config.window.opacity << "\n";
    oss << "transparentFramebuffer=" << (config.window.transparentFramebuffer ? "true" : "false") << "\n";
    oss << "highDPI=" << (config.window.highDPI ? "true" : "false") << "\n";
    oss << "contentScale=" << config.window.contentScale << "\n";
    oss << "\n";
    
    oss << "[render]\n";
    oss << "backend=" << backendTypeToString(config.render.backend) << "\n";
    oss << "targetFPS=" << config.render.targetFPS << "\n";
    oss << "vsync=" << (config.render.vsync ? "true" : "false") << "\n";
    oss << "tripleBuffering=" << (config.render.tripleBuffering ? "true" : "false") << "\n";
    oss << "multisamples=" << config.render.multisamples << "\n";
    oss << "sRGBFramebuffer=" << (config.render.sRGBFramebuffer ? "true" : "false") << "\n";
    oss << "clearColorR=" << config.render.clearColor.r << "\n";
    oss << "clearColorG=" << config.render.clearColor.g << "\n";
    oss << "clearColorB=" << config.render.clearColor.b << "\n";
    oss << "clearColorA=" << config.render.clearColor.a << "\n";
    oss << "maxTextureSize=" << config.render.maxTextureSize << "\n";
    oss << "textureAnisotropy=" << config.render.textureAnisotropy << "\n";
    oss << "wireframeMode=" << (config.render.wireframeMode ? "true" : "false") << "\n";
    oss << "depthTest=" << (config.render.depthTest ? "true" : "false") << "\n";
    oss << "blending=" << (config.render.blending ? "true" : "false") << "\n";
    oss << "dithering=" << (config.render.dithering ? "true" : "false") << "\n";
    oss << "spriteBatchSize=" << config.render.spriteBatchSize << "\n";
    oss << "maxRenderTargets=" << config.render.maxRenderTargets << "\n";
    oss << "allowShaderHotReload=" << (config.render.allowShaderHotReload ? "true" : "false") << "\n";
    oss << "shaderCachePath=" << config.render.shaderCachePath << "\n";
    oss << "\n";
    
    oss << "[audio]\n";
    oss << "enabled=" << (config.audio.enabled ? "true" : "false") << "\n";
    oss << "masterVolume=" << config.audio.masterVolume << "\n";
    oss << "musicVolume=" << config.audio.musicVolume << "\n";
    oss << "sfxVolume=" << config.audio.sfxVolume << "\n";
    oss << "voiceVolume=" << config.audio.voiceVolume << "\n";
    oss << "ambientVolume=" << config.audio.ambientVolume << "\n";
    oss << "frequency=" << config.audio.frequency << "\n";
    oss << "channels=" << config.audio.channels << "\n";
    oss << "chunkSize=" << config.audio.chunkSize << "\n";
    oss << "maxChannels=" << config.audio.maxChannels << "\n";
    oss << "spatialAudio=" << (config.audio.spatialAudio ? "true" : "false") << "\n";
    oss << "\n";
    
    oss << "[debug]\n";
    oss << "enabled=" << (config.debug.enabled ? "true" : "false") << "\n";
    oss << "showFPS=" << (config.debug.showFPS ? "true" : "false") << "\n";
    oss << "showMemoryUsage=" << (config.debug.showMemoryUsage ? "true" : "false") << "\n";
    oss << "showRenderStats=" << (config.debug.showRenderStats ? "true" : "false") << "\n";
    oss << "showColliders=" << (config.debug.showColliders ? "true" : "false") << "\n";
    oss << "showGrid=" << (config.debug.showGrid ? "true" : "false") << "\n";
    oss << "logToFile=" << (config.debug.logToFile ? "true" : "false") << "\n";
    oss << "logToConsole=" << (config.debug.logToConsole ? "true" : "false") << "\n";
    oss << "logLevel=" << config.debug.logLevel << "\n";
    oss << "breakOnAssert=" << (config.debug.breakOnAssert ? "true" : "false") << "\n";
    oss << "enableProfiling=" << (config.debug.enableProfiling ? "true" : "false") << "\n";
    oss << "logFilePath=" << config.debug.logFilePath << "\n";
    oss << "\n";
    
    oss << "[input]\n";
    oss << "enabled=" << (config.input.enabled ? "true" : "false") << "\n";
    oss << "rawMouseInput=" << (config.input.rawMouseInput ? "true" : "false") << "\n";
    oss << "mouseSensitivity=" << config.input.mouseSensitivity << "\n";
    oss << "invertMouseY=" << (config.input.invertMouseY ? "true" : "false") << "\n";
    oss << "invertMouseX=" << (config.input.invertMouseX ? "true" : "false") << "\n";
    oss << "deadzone=" << config.input.deadzone << "\n";
    oss << "triggerThreshold=" << config.input.triggerThreshold << "\n";
    oss << "enableVibration=" << (config.input.enableVibration ? "true" : "false") << "\n";
    oss << "maxGamepads=" << config.input.maxGamepads << "\n";
    oss << "autoConnectGamepads=" << (config.input.autoConnectGamepads ? "true" : "false") << "\n";
    oss << "gamepadMappingFile=" << config.input.gamepadMappingFile << "\n";
    oss << "\n";
    
    oss << "[resource]\n";
    oss << "assetRootPath=" << config.resource.assetRootPath << "\n";
    oss << "cachePath=" << config.resource.cachePath << "\n";
    oss << "savePath=" << config.resource.savePath << "\n";
    oss << "configPath=" << config.resource.configPath << "\n";
    oss << "logPath=" << config.resource.logPath << "\n";
    oss << "useAssetCache=" << (config.resource.useAssetCache ? "true" : "false") << "\n";
    oss << "maxCacheSize=" << config.resource.maxCacheSize << "\n";
    oss << "hotReloadEnabled=" << (config.resource.hotReloadEnabled ? "true" : "false") << "\n";
    oss << "hotReloadInterval=" << config.resource.hotReloadInterval << "\n";
    oss << "compressTextures=" << (config.resource.compressTextures ? "true" : "false") << "\n";
    oss << "preloadCommonAssets=" << (config.resource.preloadCommonAssets ? "true" : "false") << "\n";
    
    return oss.str();
}

/**
 * @brief 检查是否支持指定文件
 * @param filepath 文件路径
 * @return 如果支持返回 true
 */
bool IniConfigLoader::supportsFile(const std::string& filepath) const {
    if (filepath.length() >= 4) {
        std::string ext = filepath.substr(filepath.length() - 4);
        for (char& c : ext) c = static_cast<char>(std::tolower(c));
        return ext == ".ini";
    }
    return false;
}

/**
 * @brief 克隆加载器实例
 * @return 新的加载器实例
 */
UniquePtr<ConfigLoader> IniConfigLoader::clone() const {
    return makeUnique<IniConfigLoader>();
}

/**
 * @brief 生成节键组合字符串
 * @param section 节名
 * @param key 键名
 * @return 组合字符串
 */
std::string IniConfigLoader::sectionKey(const std::string& section, const std::string& key) const {
    return section + "." + key;
}

/**
 * @brief 解析整数
 * @param value 字符串值
 * @param result 输出整数结果
 * @param fieldName 字段名（用于错误报告）
 * @return 解析结果
 */
ConfigLoadResult IniConfigLoader::parseInt(const std::string& value, int& result, const std::string& fieldName) {
    try {
        size_t pos;
        result = std::stoi(value, &pos);
        if (pos != value.length()) {
            return ConfigLoadResult::error("无法解析整数值: " + value, -1, fieldName);
        }
        return ConfigLoadResult::ok();
    } catch (const std::exception& e) {
        return ConfigLoadResult::error(std::string("解析整数失败: ") + e.what(), -1, fieldName);
    }
}

/**
 * @brief 解析浮点数
 * @param value 字符串值
 * @param result 输出浮点数结果
 * @param fieldName 字段名（用于错误报告）
 * @return 解析结果
 */
ConfigLoadResult IniConfigLoader::parseFloat(const std::string& value, float& result, const std::string& fieldName) {
    try {
        size_t pos;
        result = std::stof(value, &pos);
        if (pos != value.length()) {
            return ConfigLoadResult::error("无法解析浮点数值: " + value, -1, fieldName);
        }
        return ConfigLoadResult::ok();
    } catch (const std::exception& e) {
        return ConfigLoadResult::error(std::string("解析浮点数失败: ") + e.what(), -1, fieldName);
    }
}

/**
 * @brief 解析布尔值
 * @param value 字符串值
 * @param result 输出布尔结果
 * @param fieldName 字段名（用于错误报告）
 * @return 解析结果
 */
ConfigLoadResult IniConfigLoader::parseBool(const std::string& value, bool& result, const std::string& fieldName) {
    std::string lower = toLower(value);
    if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
        result = true;
        return ConfigLoadResult::ok();
    }
    if (lower == "false" || lower == "0" || lower == "no" || lower == "off") {
        result = false;
        return ConfigLoadResult::ok();
    }
    return ConfigLoadResult::error("无法解析布尔值: " + value, -1, fieldName);
}

}
