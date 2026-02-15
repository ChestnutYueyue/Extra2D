#include <extra2d/config/config_loader.h>
#include <extra2d/utils/logger.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace extra2d {

/**
 * @brief 将字符串转换为窗口模式枚举
 * @param modeStr 窗口模式字符串
 * @return 窗口模式枚举值
 */
static WindowMode stringToWindowMode(const std::string& modeStr) {
    if (modeStr == "fullscreen") return WindowMode::Fullscreen;
    if (modeStr == "borderless") return WindowMode::Borderless;
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
    if (backendStr == "opengl") return BackendType::OpenGL;
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

// ============================================================================
// JsonConfigLoader 实现
// ============================================================================

/**
 * @brief 从 JSON 文件加载配置
 * @param filepath 配置文件路径
 * @param config 输出的配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::load(const std::string& filepath, AppConfig& config) {
    E2D_LOG_INFO("正在从 JSON 文件加载配置: {}", filepath);
    
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
 * @brief 保存配置到 JSON 文件
 * @param filepath 配置文件路径
 * @param config 要保存的配置对象
 * @return 保存结果
 */
ConfigSaveResult JsonConfigLoader::save(const std::string& filepath, const AppConfig& config) {
    E2D_LOG_INFO("正在保存配置到 JSON 文件: {}", filepath);
    
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
 * @brief 从 JSON 字符串加载配置
 * @param content JSON 内容字符串
 * @param config 输出的配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::loadFromString(const std::string& content, AppConfig& config) {
    json root;
    
    try {
        root = json::parse(content);
    } catch (const json::parse_error& e) {
        E2D_LOG_ERROR("JSON 解析错误: {}", e.what());
        return ConfigLoadResult::error(std::string("JSON 解析错误: ") + e.what(), 
                                       static_cast<int>(e.byte));
    }
    
    if (root.contains("appName") && root["appName"].is_string()) {
        config.appName = root["appName"].get<std::string>();
    }
    
    if (root.contains("appVersion") && root["appVersion"].is_string()) {
        config.appVersion = root["appVersion"].get<std::string>();
    }
    
    if (root.contains("organization") && root["organization"].is_string()) {
        config.organization = root["organization"].get<std::string>();
    }
    
    if (root.contains("window")) {
        auto result = parseWindowConfig(&root["window"], config.window);
        if (result.hasError()) {
            return result;
        }
    }
    
    if (root.contains("render")) {
        auto result = parseRenderConfig(&root["render"], config.render);
        if (result.hasError()) {
            return result;
        }
    }
    
    if (root.contains("audio")) {
        auto result = parseAudioConfig(&root["audio"], config.audio);
        if (result.hasError()) {
            return result;
        }
    }
    
    if (root.contains("debug")) {
        auto result = parseDebugConfig(&root["debug"], config.debug);
        if (result.hasError()) {
            return result;
        }
    }
    
    if (root.contains("input")) {
        auto result = parseInputConfig(&root["input"], config.input);
        if (result.hasError()) {
            return result;
        }
    }
    
    if (root.contains("resource")) {
        auto result = parseResourceConfig(&root["resource"], config.resource);
        if (result.hasError()) {
            return result;
        }
    }
    
    E2D_LOG_INFO("JSON 配置加载成功");
    return ConfigLoadResult::ok();
}

/**
 * @brief 将配置序列化为 JSON 字符串
 * @param config 配置对象
 * @return 序列化后的 JSON 字符串
 */
std::string JsonConfigLoader::saveToString(const AppConfig& config) {
    json root;
    
    root["appName"] = config.appName;
    root["appVersion"] = config.appVersion;
    root["organization"] = config.organization;
    
    serializeWindowConfig(&root, config.window);
    serializeRenderConfig(&root, config.render);
    serializeAudioConfig(&root, config.audio);
    serializeDebugConfig(&root, config.debug);
    serializeInputConfig(&root, config.input);
    serializeResourceConfig(&root, config.resource);
    
    return root.dump(4);
}

/**
 * @brief 检查是否支持指定文件
 * @param filepath 文件路径
 * @return 如果支持返回 true
 */
bool JsonConfigLoader::supportsFile(const std::string& filepath) const {
    if (filepath.length() >= 5) {
        std::string ext = filepath.substr(filepath.length() - 5);
        for (char& c : ext) c = static_cast<char>(std::tolower(c));
        return ext == ".json";
    }
    return false;
}

/**
 * @brief 克隆加载器实例
 * @return 新的加载器实例
 */
UniquePtr<ConfigLoader> JsonConfigLoader::clone() const {
    return makeUnique<JsonConfigLoader>();
}

/**
 * @brief 解析窗口配置
 * @param jsonValue JSON 值指针
 * @param window 输出的窗口配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::parseWindowConfig(const void* jsonValue, WindowConfigData& window) {
    const json& obj = *static_cast<const json*>(jsonValue);
    
    if (!obj.is_object()) {
        return ConfigLoadResult::error("window 配置必须是一个对象", -1, "window");
    }
    
    if (obj.contains("title") && obj["title"].is_string()) {
        window.title = obj["title"].get<std::string>();
    }
    if (obj.contains("width") && obj["width"].is_number_integer()) {
        window.width = obj["width"].get<int>();
    }
    if (obj.contains("height") && obj["height"].is_number_integer()) {
        window.height = obj["height"].get<int>();
    }
    if (obj.contains("minWidth") && obj["minWidth"].is_number_integer()) {
        window.minWidth = obj["minWidth"].get<int>();
    }
    if (obj.contains("minHeight") && obj["minHeight"].is_number_integer()) {
        window.minHeight = obj["minHeight"].get<int>();
    }
    if (obj.contains("maxWidth") && obj["maxWidth"].is_number_integer()) {
        window.maxWidth = obj["maxWidth"].get<int>();
    }
    if (obj.contains("maxHeight") && obj["maxHeight"].is_number_integer()) {
        window.maxHeight = obj["maxHeight"].get<int>();
    }
    if (obj.contains("mode") && obj["mode"].is_string()) {
        window.mode = stringToWindowMode(obj["mode"].get<std::string>());
    }
    if (obj.contains("resizable") && obj["resizable"].is_boolean()) {
        window.resizable = obj["resizable"].get<bool>();
    }
    if (obj.contains("borderless") && obj["borderless"].is_boolean()) {
        window.borderless = obj["borderless"].get<bool>();
    }
    if (obj.contains("alwaysOnTop") && obj["alwaysOnTop"].is_boolean()) {
        window.alwaysOnTop = obj["alwaysOnTop"].get<bool>();
    }
    if (obj.contains("centered") && obj["centered"].is_boolean()) {
        window.centered = obj["centered"].get<bool>();
    }
    if (obj.contains("posX") && obj["posX"].is_number_integer()) {
        window.posX = obj["posX"].get<int>();
    }
    if (obj.contains("posY") && obj["posY"].is_number_integer()) {
        window.posY = obj["posY"].get<int>();
    }
    if (obj.contains("hideOnClose") && obj["hideOnClose"].is_boolean()) {
        window.hideOnClose = obj["hideOnClose"].get<bool>();
    }
    if (obj.contains("minimizeOnClose") && obj["minimizeOnClose"].is_boolean()) {
        window.minimizeOnClose = obj["minimizeOnClose"].get<bool>();
    }
    if (obj.contains("opacity") && obj["opacity"].is_number()) {
        window.opacity = obj["opacity"].get<float>();
    }
    if (obj.contains("transparentFramebuffer") && obj["transparentFramebuffer"].is_boolean()) {
        window.transparentFramebuffer = obj["transparentFramebuffer"].get<bool>();
    }
    if (obj.contains("highDPI") && obj["highDPI"].is_boolean()) {
        window.highDPI = obj["highDPI"].get<bool>();
    }
    if (obj.contains("contentScale") && obj["contentScale"].is_number()) {
        window.contentScale = obj["contentScale"].get<float>();
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 解析渲染配置
 * @param jsonValue JSON 值指针
 * @param render 输出的渲染配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::parseRenderConfig(const void* jsonValue, RenderConfigData& render) {
    const json& obj = *static_cast<const json*>(jsonValue);
    
    if (!obj.is_object()) {
        return ConfigLoadResult::error("render 配置必须是一个对象", -1, "render");
    }
    
    if (obj.contains("backend") && obj["backend"].is_string()) {
        render.backend = stringToBackendType(obj["backend"].get<std::string>());
    }
    if (obj.contains("targetFPS") && obj["targetFPS"].is_number_integer()) {
        render.targetFPS = obj["targetFPS"].get<int>();
    }
    if (obj.contains("vsync") && obj["vsync"].is_boolean()) {
        render.vsync = obj["vsync"].get<bool>();
    }
    if (obj.contains("tripleBuffering") && obj["tripleBuffering"].is_boolean()) {
        render.tripleBuffering = obj["tripleBuffering"].get<bool>();
    }
    if (obj.contains("multisamples") && obj["multisamples"].is_number_integer()) {
        render.multisamples = obj["multisamples"].get<int>();
    }
    if (obj.contains("sRGBFramebuffer") && obj["sRGBFramebuffer"].is_boolean()) {
        render.sRGBFramebuffer = obj["sRGBFramebuffer"].get<bool>();
    }
    if (obj.contains("clearColor") && obj["clearColor"].is_array() && obj["clearColor"].size() >= 4) {
        render.clearColor.r = obj["clearColor"][0].get<float>();
        render.clearColor.g = obj["clearColor"][1].get<float>();
        render.clearColor.b = obj["clearColor"][2].get<float>();
        render.clearColor.a = obj["clearColor"][3].get<float>();
    }
    if (obj.contains("maxTextureSize") && obj["maxTextureSize"].is_number_integer()) {
        render.maxTextureSize = obj["maxTextureSize"].get<int>();
    }
    if (obj.contains("textureAnisotropy") && obj["textureAnisotropy"].is_number_integer()) {
        render.textureAnisotropy = obj["textureAnisotropy"].get<int>();
    }
    if (obj.contains("wireframeMode") && obj["wireframeMode"].is_boolean()) {
        render.wireframeMode = obj["wireframeMode"].get<bool>();
    }
    if (obj.contains("depthTest") && obj["depthTest"].is_boolean()) {
        render.depthTest = obj["depthTest"].get<bool>();
    }
    if (obj.contains("blending") && obj["blending"].is_boolean()) {
        render.blending = obj["blending"].get<bool>();
    }
    if (obj.contains("dithering") && obj["dithering"].is_boolean()) {
        render.dithering = obj["dithering"].get<bool>();
    }
    if (obj.contains("spriteBatchSize") && obj["spriteBatchSize"].is_number_integer()) {
        render.spriteBatchSize = obj["spriteBatchSize"].get<int>();
    }
    if (obj.contains("maxRenderTargets") && obj["maxRenderTargets"].is_number_integer()) {
        render.maxRenderTargets = obj["maxRenderTargets"].get<int>();
    }
    if (obj.contains("allowShaderHotReload") && obj["allowShaderHotReload"].is_boolean()) {
        render.allowShaderHotReload = obj["allowShaderHotReload"].get<bool>();
    }
    if (obj.contains("shaderCachePath") && obj["shaderCachePath"].is_string()) {
        render.shaderCachePath = obj["shaderCachePath"].get<std::string>();
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 解析音频配置
 * @param jsonValue JSON 值指针
 * @param audio 输出的音频配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::parseAudioConfig(const void* jsonValue, AudioConfigData& audio) {
    const json& obj = *static_cast<const json*>(jsonValue);
    
    if (!obj.is_object()) {
        return ConfigLoadResult::error("audio 配置必须是一个对象", -1, "audio");
    }
    
    if (obj.contains("enabled") && obj["enabled"].is_boolean()) {
        audio.enabled = obj["enabled"].get<bool>();
    }
    if (obj.contains("masterVolume") && obj["masterVolume"].is_number_integer()) {
        audio.masterVolume = obj["masterVolume"].get<int>();
    }
    if (obj.contains("musicVolume") && obj["musicVolume"].is_number_integer()) {
        audio.musicVolume = obj["musicVolume"].get<int>();
    }
    if (obj.contains("sfxVolume") && obj["sfxVolume"].is_number_integer()) {
        audio.sfxVolume = obj["sfxVolume"].get<int>();
    }
    if (obj.contains("voiceVolume") && obj["voiceVolume"].is_number_integer()) {
        audio.voiceVolume = obj["voiceVolume"].get<int>();
    }
    if (obj.contains("ambientVolume") && obj["ambientVolume"].is_number_integer()) {
        audio.ambientVolume = obj["ambientVolume"].get<int>();
    }
    if (obj.contains("frequency") && obj["frequency"].is_number_integer()) {
        audio.frequency = obj["frequency"].get<int>();
    }
    if (obj.contains("channels") && obj["channels"].is_number_integer()) {
        audio.channels = obj["channels"].get<int>();
    }
    if (obj.contains("chunkSize") && obj["chunkSize"].is_number_integer()) {
        audio.chunkSize = obj["chunkSize"].get<int>();
    }
    if (obj.contains("maxChannels") && obj["maxChannels"].is_number_integer()) {
        audio.maxChannels = obj["maxChannels"].get<int>();
    }
    if (obj.contains("spatialAudio") && obj["spatialAudio"].is_boolean()) {
        audio.spatialAudio = obj["spatialAudio"].get<bool>();
    }
    if (obj.contains("listenerPosition") && obj["listenerPosition"].is_array() && obj["listenerPosition"].size() >= 3) {
        audio.listenerPosition[0] = obj["listenerPosition"][0].get<float>();
        audio.listenerPosition[1] = obj["listenerPosition"][1].get<float>();
        audio.listenerPosition[2] = obj["listenerPosition"][2].get<float>();
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 解析调试配置
 * @param jsonValue JSON 值指针
 * @param debug 输出的调试配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::parseDebugConfig(const void* jsonValue, DebugConfigData& debug) {
    const json& obj = *static_cast<const json*>(jsonValue);
    
    if (!obj.is_object()) {
        return ConfigLoadResult::error("debug 配置必须是一个对象", -1, "debug");
    }
    
    if (obj.contains("enabled") && obj["enabled"].is_boolean()) {
        debug.enabled = obj["enabled"].get<bool>();
    }
    if (obj.contains("showFPS") && obj["showFPS"].is_boolean()) {
        debug.showFPS = obj["showFPS"].get<bool>();
    }
    if (obj.contains("showMemoryUsage") && obj["showMemoryUsage"].is_boolean()) {
        debug.showMemoryUsage = obj["showMemoryUsage"].get<bool>();
    }
    if (obj.contains("showRenderStats") && obj["showRenderStats"].is_boolean()) {
        debug.showRenderStats = obj["showRenderStats"].get<bool>();
    }
    if (obj.contains("showColliders") && obj["showColliders"].is_boolean()) {
        debug.showColliders = obj["showColliders"].get<bool>();
    }
    if (obj.contains("showGrid") && obj["showGrid"].is_boolean()) {
        debug.showGrid = obj["showGrid"].get<bool>();
    }
    if (obj.contains("logToFile") && obj["logToFile"].is_boolean()) {
        debug.logToFile = obj["logToFile"].get<bool>();
    }
    if (obj.contains("logToConsole") && obj["logToConsole"].is_boolean()) {
        debug.logToConsole = obj["logToConsole"].get<bool>();
    }
    if (obj.contains("logLevel") && obj["logLevel"].is_number_integer()) {
        debug.logLevel = obj["logLevel"].get<int>();
    }
    if (obj.contains("breakOnAssert") && obj["breakOnAssert"].is_boolean()) {
        debug.breakOnAssert = obj["breakOnAssert"].get<bool>();
    }
    if (obj.contains("enableProfiling") && obj["enableProfiling"].is_boolean()) {
        debug.enableProfiling = obj["enableProfiling"].get<bool>();
    }
    if (obj.contains("logFilePath") && obj["logFilePath"].is_string()) {
        debug.logFilePath = obj["logFilePath"].get<std::string>();
    }
    if (obj.contains("debugFlags") && obj["debugFlags"].is_array()) {
        debug.debugFlags.clear();
        for (const auto& flag : obj["debugFlags"]) {
            if (flag.is_string()) {
                debug.debugFlags.push_back(flag.get<std::string>());
            }
        }
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 解析输入配置
 * @param jsonValue JSON 值指针
 * @param input 输出的输入配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::parseInputConfig(const void* jsonValue, InputConfigData& input) {
    const json& obj = *static_cast<const json*>(jsonValue);
    
    if (!obj.is_object()) {
        return ConfigLoadResult::error("input 配置必须是一个对象", -1, "input");
    }
    
    if (obj.contains("enabled") && obj["enabled"].is_boolean()) {
        input.enabled = obj["enabled"].get<bool>();
    }
    if (obj.contains("rawMouseInput") && obj["rawMouseInput"].is_boolean()) {
        input.rawMouseInput = obj["rawMouseInput"].get<bool>();
    }
    if (obj.contains("mouseSensitivity") && obj["mouseSensitivity"].is_number()) {
        input.mouseSensitivity = obj["mouseSensitivity"].get<float>();
    }
    if (obj.contains("invertMouseY") && obj["invertMouseY"].is_boolean()) {
        input.invertMouseY = obj["invertMouseY"].get<bool>();
    }
    if (obj.contains("invertMouseX") && obj["invertMouseX"].is_boolean()) {
        input.invertMouseX = obj["invertMouseX"].get<bool>();
    }
    if (obj.contains("deadzone") && obj["deadzone"].is_number()) {
        input.deadzone = obj["deadzone"].get<float>();
    }
    if (obj.contains("triggerThreshold") && obj["triggerThreshold"].is_number()) {
        input.triggerThreshold = obj["triggerThreshold"].get<float>();
    }
    if (obj.contains("enableVibration") && obj["enableVibration"].is_boolean()) {
        input.enableVibration = obj["enableVibration"].get<bool>();
    }
    if (obj.contains("maxGamepads") && obj["maxGamepads"].is_number_integer()) {
        input.maxGamepads = obj["maxGamepads"].get<int>();
    }
    if (obj.contains("autoConnectGamepads") && obj["autoConnectGamepads"].is_boolean()) {
        input.autoConnectGamepads = obj["autoConnectGamepads"].get<bool>();
    }
    if (obj.contains("gamepadMappingFile") && obj["gamepadMappingFile"].is_string()) {
        input.gamepadMappingFile = obj["gamepadMappingFile"].get<std::string>();
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 解析资源配置
 * @param jsonValue JSON 值指针
 * @param resource 输出的资源配置对象
 * @return 加载结果
 */
ConfigLoadResult JsonConfigLoader::parseResourceConfig(const void* jsonValue, ResourceConfigData& resource) {
    const json& obj = *static_cast<const json*>(jsonValue);
    
    if (!obj.is_object()) {
        return ConfigLoadResult::error("resource 配置必须是一个对象", -1, "resource");
    }
    
    if (obj.contains("assetRootPath") && obj["assetRootPath"].is_string()) {
        resource.assetRootPath = obj["assetRootPath"].get<std::string>();
    }
    if (obj.contains("cachePath") && obj["cachePath"].is_string()) {
        resource.cachePath = obj["cachePath"].get<std::string>();
    }
    if (obj.contains("savePath") && obj["savePath"].is_string()) {
        resource.savePath = obj["savePath"].get<std::string>();
    }
    if (obj.contains("configPath") && obj["configPath"].is_string()) {
        resource.configPath = obj["configPath"].get<std::string>();
    }
    if (obj.contains("logPath") && obj["logPath"].is_string()) {
        resource.logPath = obj["logPath"].get<std::string>();
    }
    if (obj.contains("useAssetCache") && obj["useAssetCache"].is_boolean()) {
        resource.useAssetCache = obj["useAssetCache"].get<bool>();
    }
    if (obj.contains("maxCacheSize") && obj["maxCacheSize"].is_number_integer()) {
        resource.maxCacheSize = obj["maxCacheSize"].get<int>();
    }
    if (obj.contains("hotReloadEnabled") && obj["hotReloadEnabled"].is_boolean()) {
        resource.hotReloadEnabled = obj["hotReloadEnabled"].get<bool>();
    }
    if (obj.contains("hotReloadInterval") && obj["hotReloadInterval"].is_number()) {
        resource.hotReloadInterval = obj["hotReloadInterval"].get<float>();
    }
    if (obj.contains("compressTextures") && obj["compressTextures"].is_boolean()) {
        resource.compressTextures = obj["compressTextures"].get<bool>();
    }
    if (obj.contains("preloadCommonAssets") && obj["preloadCommonAssets"].is_boolean()) {
        resource.preloadCommonAssets = obj["preloadCommonAssets"].get<bool>();
    }
    if (obj.contains("searchPaths") && obj["searchPaths"].is_array()) {
        resource.searchPaths.clear();
        for (const auto& path : obj["searchPaths"]) {
            if (path.is_string()) {
                resource.searchPaths.push_back(path.get<std::string>());
            }
        }
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 序列化窗口配置到 JSON
 * @param jsonValue JSON 值指针
 * @param window 窗口配置对象
 */
void JsonConfigLoader::serializeWindowConfig(void* jsonValue, const WindowConfigData& window) {
    json& root = *static_cast<json*>(jsonValue);
    json obj;
    
    obj["title"] = window.title;
    obj["width"] = window.width;
    obj["height"] = window.height;
    obj["minWidth"] = window.minWidth;
    obj["minHeight"] = window.minHeight;
    obj["maxWidth"] = window.maxWidth;
    obj["maxHeight"] = window.maxHeight;
    obj["mode"] = windowModeToString(window.mode);
    obj["resizable"] = window.resizable;
    obj["borderless"] = window.borderless;
    obj["alwaysOnTop"] = window.alwaysOnTop;
    obj["centered"] = window.centered;
    obj["posX"] = window.posX;
    obj["posY"] = window.posY;
    obj["hideOnClose"] = window.hideOnClose;
    obj["minimizeOnClose"] = window.minimizeOnClose;
    obj["opacity"] = window.opacity;
    obj["transparentFramebuffer"] = window.transparentFramebuffer;
    obj["highDPI"] = window.highDPI;
    obj["contentScale"] = window.contentScale;
    
    root["window"] = obj;
}

/**
 * @brief 序列化渲染配置到 JSON
 * @param jsonValue JSON 值指针
 * @param render 渲染配置对象
 */
void JsonConfigLoader::serializeRenderConfig(void* jsonValue, const RenderConfigData& render) {
    json& root = *static_cast<json*>(jsonValue);
    json obj;
    
    obj["backend"] = backendTypeToString(render.backend);
    obj["targetFPS"] = render.targetFPS;
    obj["vsync"] = render.vsync;
    obj["tripleBuffering"] = render.tripleBuffering;
    obj["multisamples"] = render.multisamples;
    obj["sRGBFramebuffer"] = render.sRGBFramebuffer;
    obj["clearColor"] = {render.clearColor.r, render.clearColor.g, render.clearColor.b, render.clearColor.a};
    obj["maxTextureSize"] = render.maxTextureSize;
    obj["textureAnisotropy"] = render.textureAnisotropy;
    obj["wireframeMode"] = render.wireframeMode;
    obj["depthTest"] = render.depthTest;
    obj["blending"] = render.blending;
    obj["dithering"] = render.dithering;
    obj["spriteBatchSize"] = render.spriteBatchSize;
    obj["maxRenderTargets"] = render.maxRenderTargets;
    obj["allowShaderHotReload"] = render.allowShaderHotReload;
    obj["shaderCachePath"] = render.shaderCachePath;
    
    root["render"] = obj;
}

/**
 * @brief 序列化音频配置到 JSON
 * @param jsonValue JSON 值指针
 * @param audio 音频配置对象
 */
void JsonConfigLoader::serializeAudioConfig(void* jsonValue, const AudioConfigData& audio) {
    json& root = *static_cast<json*>(jsonValue);
    json obj;
    
    obj["enabled"] = audio.enabled;
    obj["masterVolume"] = audio.masterVolume;
    obj["musicVolume"] = audio.musicVolume;
    obj["sfxVolume"] = audio.sfxVolume;
    obj["voiceVolume"] = audio.voiceVolume;
    obj["ambientVolume"] = audio.ambientVolume;
    obj["frequency"] = audio.frequency;
    obj["channels"] = audio.channels;
    obj["chunkSize"] = audio.chunkSize;
    obj["maxChannels"] = audio.maxChannels;
    obj["spatialAudio"] = audio.spatialAudio;
    obj["listenerPosition"] = {audio.listenerPosition[0], audio.listenerPosition[1], audio.listenerPosition[2]};
    
    root["audio"] = obj;
}

/**
 * @brief 序列化调试配置到 JSON
 * @param jsonValue JSON 值指针
 * @param debug 调试配置对象
 */
void JsonConfigLoader::serializeDebugConfig(void* jsonValue, const DebugConfigData& debug) {
    json& root = *static_cast<json*>(jsonValue);
    json obj;
    
    obj["enabled"] = debug.enabled;
    obj["showFPS"] = debug.showFPS;
    obj["showMemoryUsage"] = debug.showMemoryUsage;
    obj["showRenderStats"] = debug.showRenderStats;
    obj["showColliders"] = debug.showColliders;
    obj["showGrid"] = debug.showGrid;
    obj["logToFile"] = debug.logToFile;
    obj["logToConsole"] = debug.logToConsole;
    obj["logLevel"] = debug.logLevel;
    obj["breakOnAssert"] = debug.breakOnAssert;
    obj["enableProfiling"] = debug.enableProfiling;
    obj["logFilePath"] = debug.logFilePath;
    obj["debugFlags"] = debug.debugFlags;
    
    root["debug"] = obj;
}

/**
 * @brief 序列化输入配置到 JSON
 * @param jsonValue JSON 值指针
 * @param input 输入配置对象
 */
void JsonConfigLoader::serializeInputConfig(void* jsonValue, const InputConfigData& input) {
    json& root = *static_cast<json*>(jsonValue);
    json obj;
    
    obj["enabled"] = input.enabled;
    obj["rawMouseInput"] = input.rawMouseInput;
    obj["mouseSensitivity"] = input.mouseSensitivity;
    obj["invertMouseY"] = input.invertMouseY;
    obj["invertMouseX"] = input.invertMouseX;
    obj["deadzone"] = input.deadzone;
    obj["triggerThreshold"] = input.triggerThreshold;
    obj["enableVibration"] = input.enableVibration;
    obj["maxGamepads"] = input.maxGamepads;
    obj["autoConnectGamepads"] = input.autoConnectGamepads;
    obj["gamepadMappingFile"] = input.gamepadMappingFile;
    
    root["input"] = obj;
}

/**
 * @brief 序列化资源配置到 JSON
 * @param jsonValue JSON 值指针
 * @param resource 资源配置对象
 */
void JsonConfigLoader::serializeResourceConfig(void* jsonValue, const ResourceConfigData& resource) {
    json& root = *static_cast<json*>(jsonValue);
    json obj;
    
    obj["assetRootPath"] = resource.assetRootPath;
    obj["cachePath"] = resource.cachePath;
    obj["savePath"] = resource.savePath;
    obj["configPath"] = resource.configPath;
    obj["logPath"] = resource.logPath;
    obj["useAssetCache"] = resource.useAssetCache;
    obj["maxCacheSize"] = resource.maxCacheSize;
    obj["hotReloadEnabled"] = resource.hotReloadEnabled;
    obj["hotReloadInterval"] = resource.hotReloadInterval;
    obj["compressTextures"] = resource.compressTextures;
    obj["preloadCommonAssets"] = resource.preloadCommonAssets;
    obj["searchPaths"] = resource.searchPaths;
    
    root["resource"] = obj;
}

}
