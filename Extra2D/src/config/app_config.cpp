#include <extra2d/config/app_config.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/utils/logger.h>
#include <algorithm>

namespace extra2d {

/**
 * @brief 检查是否存在指定的调试标志
 * @param flag 要检查的标志名称
 * @return 如果存在返回 true
 */
bool DebugConfigData::hasDebugFlag(const std::string& flag) const {
    return std::find(debugFlags.begin(), debugFlags.end(), flag) != debugFlags.end();
}

/**
 * @brief 添加调试标志
 * @param flag 要添加的标志名称
 */
void DebugConfigData::addDebugFlag(const std::string& flag) {
    if (!hasDebugFlag(flag)) {
        debugFlags.push_back(flag);
    }
}

/**
 * @brief 移除调试标志
 * @param flag 要移除的标志名称
 */
void DebugConfigData::removeDebugFlag(const std::string& flag) {
    auto it = std::find(debugFlags.begin(), debugFlags.end(), flag);
    if (it != debugFlags.end()) {
        debugFlags.erase(it);
    }
}

/**
 * @brief 添加资源搜索路径
 * @param path 要添加的搜索路径
 */
void ResourceConfigData::addSearchPath(const std::string& path) {
    if (!hasSearchPath(path)) {
        searchPaths.push_back(path);
    }
}

/**
 * @brief 移除资源搜索路径
 * @param path 要移除的搜索路径
 */
void ResourceConfigData::removeSearchPath(const std::string& path) {
    auto it = std::find(searchPaths.begin(), searchPaths.end(), path);
    if (it != searchPaths.end()) {
        searchPaths.erase(it);
    }
}

/**
 * @brief 检查是否存在指定的搜索路径
 * @param path 要检查的路径
 * @return 如果存在返回 true
 */
bool ResourceConfigData::hasSearchPath(const std::string& path) const {
    return std::find(searchPaths.begin(), searchPaths.end(), path) != searchPaths.end();
}

/**
 * @brief 创建默认配置
 * 返回一个包含所有默认值的应用配置实例
 * @return 默认的应用配置实例
 */
AppConfig AppConfig::createDefault() {
    AppConfig config;
    
    config.window.title = "Extra2D Application";
    config.window.width = 1280;
    config.window.height = 720;
    config.window.minWidth = 320;
    config.window.minHeight = 240;
    config.window.maxWidth = 0;
    config.window.maxHeight = 0;
    config.window.mode = WindowMode::Windowed;
    config.window.resizable = true;
    config.window.borderless = false;
    config.window.alwaysOnTop = false;
    config.window.centered = true;
    config.window.posX = -1;
    config.window.posY = -1;
    config.window.hideOnClose = false;
    config.window.minimizeOnClose = true;
    config.window.opacity = 1.0f;
    config.window.transparentFramebuffer = false;
    config.window.highDPI = true;
    config.window.contentScale = 1.0f;
    
    config.render.backend = BackendType::OpenGL;
    config.render.targetFPS = 60;
    config.render.vsync = true;
    config.render.tripleBuffering = false;
    config.render.multisamples = 0;
    config.render.sRGBFramebuffer = false;
    config.render.clearColor = Color{0.0f, 0.0f, 0.0f, 1.0f};
    config.render.maxTextureSize = 0;
    config.render.textureAnisotropy = 1;
    config.render.wireframeMode = false;
    config.render.depthTest = false;
    config.render.blending = true;
    config.render.dithering = false;
    config.render.spriteBatchSize = 1000;
    config.render.maxRenderTargets = 1;
    config.render.allowShaderHotReload = false;
    
    config.audio.enabled = true;
    config.audio.masterVolume = 100;
    config.audio.musicVolume = 100;
    config.audio.sfxVolume = 100;
    config.audio.voiceVolume = 100;
    config.audio.ambientVolume = 100;
    config.audio.frequency = 44100;
    config.audio.channels = 2;
    config.audio.chunkSize = 2048;
    config.audio.maxChannels = 16;
    config.audio.spatialAudio = false;
    config.audio.listenerPosition[0] = 0.0f;
    config.audio.listenerPosition[1] = 0.0f;
    config.audio.listenerPosition[2] = 0.0f;
    
    config.debug.enabled = false;
    config.debug.showFPS = false;
    config.debug.showMemoryUsage = false;
    config.debug.showRenderStats = false;
    config.debug.showColliders = false;
    config.debug.showGrid = false;
    config.debug.logToFile = false;
    config.debug.logToConsole = true;
    config.debug.logLevel = 2;
    config.debug.breakOnAssert = true;
    config.debug.enableProfiling = false;
    config.debug.debugFlags.clear();
    
    config.input.enabled = true;
    config.input.rawMouseInput = false;
    config.input.mouseSensitivity = 1.0f;
    config.input.invertMouseY = false;
    config.input.invertMouseX = false;
    config.input.deadzone = 0.15f;
    config.input.triggerThreshold = 0.5f;
    config.input.enableVibration = true;
    config.input.maxGamepads = 4;
    config.input.autoConnectGamepads = true;
    
    config.resource.assetRootPath = "assets";
    config.resource.cachePath = "cache";
    config.resource.savePath = "saves";
    config.resource.configPath = "config";
    config.resource.logPath = "logs";
    config.resource.useAssetCache = true;
    config.resource.maxCacheSize = 512;
    config.resource.hotReloadEnabled = false;
    config.resource.hotReloadInterval = 1.0f;
    config.resource.compressTextures = false;
    config.resource.preloadCommonAssets = true;
    config.resource.searchPaths.clear();
    
    config.appName = "Extra2D App";
    config.appVersion = "1.0.0";
    config.organization = "";
    config.configFile = "config.json";
    config.targetPlatform = PlatformType::Auto;
    
    return config;
}

/**
 * @brief 验证配置的有效性
 * 检查所有配置项是否在有效范围内
 * @return 如果配置有效返回 true，否则返回 false
 */
bool AppConfig::validate() const {
    if (window.width <= 0) {
        E2D_LOG_ERROR("Config validation failed: window width must be positive");
        return false;
    }
    if (window.height <= 0) {
        E2D_LOG_ERROR("Config validation failed: window height must be positive");
        return false;
    }
    if (window.minWidth <= 0) {
        E2D_LOG_ERROR("Config validation failed: minimum window width must be positive");
        return false;
    }
    if (window.minHeight <= 0) {
        E2D_LOG_ERROR("Config validation failed: minimum window height must be positive");
        return false;
    }
    if (window.maxWidth > 0 && window.maxWidth < window.minWidth) {
        E2D_LOG_ERROR("Config validation failed: maximum width less than minimum width");
        return false;
    }
    if (window.maxHeight > 0 && window.maxHeight < window.minHeight) {
        E2D_LOG_ERROR("Config validation failed: maximum height less than minimum height");
        return false;
    }
    if (window.width < window.minWidth) {
        E2D_LOG_ERROR("Config validation failed: window width less than minimum");
        return false;
    }
    if (window.height < window.minHeight) {
        E2D_LOG_ERROR("Config validation failed: window height less than minimum");
        return false;
    }
    if (window.maxWidth > 0 && window.width > window.maxWidth) {
        E2D_LOG_ERROR("Config validation failed: window width exceeds maximum");
        return false;
    }
    if (window.maxHeight > 0 && window.height > window.maxHeight) {
        E2D_LOG_ERROR("Config validation failed: window height exceeds maximum");
        return false;
    }
    if (window.opacity < 0.0f || window.opacity > 1.0f) {
        E2D_LOG_ERROR("Config validation failed: window opacity must be between 0 and 1");
        return false;
    }
    if (window.contentScale <= 0.0f) {
        E2D_LOG_ERROR("Config validation failed: content scale must be positive");
        return false;
    }
    
    if (render.targetFPS < 0) {
        E2D_LOG_ERROR("Config validation failed: target FPS cannot be negative");
        return false;
    }
    if (render.multisamples < 0 || render.multisamples > 16) {
        E2D_LOG_ERROR("Config validation failed: multisamples must be between 0 and 16");
        return false;
    }
    if (render.textureAnisotropy < 1 || render.textureAnisotropy > 16) {
        E2D_LOG_ERROR("Config validation failed: texture anisotropy must be between 1 and 16");
        return false;
    }
    if (render.spriteBatchSize <= 0) {
        E2D_LOG_ERROR("Config validation failed: sprite batch size must be positive");
        return false;
    }
    if (render.maxRenderTargets <= 0) {
        E2D_LOG_ERROR("Config validation failed: max render targets must be positive");
        return false;
    }
    
    if (!audio.isValidVolume(audio.masterVolume)) {
        E2D_LOG_ERROR("Config validation failed: master volume must be between 0 and 100");
        return false;
    }
    if (!audio.isValidVolume(audio.musicVolume)) {
        E2D_LOG_ERROR("Config validation failed: music volume must be between 0 and 100");
        return false;
    }
    if (!audio.isValidVolume(audio.sfxVolume)) {
        E2D_LOG_ERROR("Config validation failed: SFX volume must be between 0 and 100");
        return false;
    }
    if (!audio.isValidVolume(audio.voiceVolume)) {
        E2D_LOG_ERROR("Config validation failed: voice volume must be between 0 and 100");
        return false;
    }
    if (!audio.isValidVolume(audio.ambientVolume)) {
        E2D_LOG_ERROR("Config validation failed: ambient volume must be between 0 and 100");
        return false;
    }
    if (audio.frequency <= 0) {
        E2D_LOG_ERROR("Config validation failed: audio frequency must be positive");
        return false;
    }
    if (audio.channels <= 0) {
        E2D_LOG_ERROR("Config validation failed: audio channels must be positive");
        return false;
    }
    if (audio.chunkSize <= 0) {
        E2D_LOG_ERROR("Config validation failed: audio chunk size must be positive");
        return false;
    }
    if (audio.maxChannels <= 0) {
        E2D_LOG_ERROR("Config validation failed: max audio channels must be positive");
        return false;
    }
    
    if (debug.logLevel < 0 || debug.logLevel > 5) {
        E2D_LOG_ERROR("Config validation failed: log level must be between 0 and 5");
        return false;
    }
    
    if (!input.isDeadzoneValid()) {
        E2D_LOG_ERROR("Config validation failed: deadzone must be between 0 and 1");
        return false;
    }
    if (input.mouseSensitivity <= 0.0f) {
        E2D_LOG_ERROR("Config validation failed: mouse sensitivity must be positive");
        return false;
    }
    if (input.triggerThreshold < 0.0f || input.triggerThreshold > 1.0f) {
        E2D_LOG_ERROR("Config validation failed: trigger threshold must be between 0 and 1");
        return false;
    }
    if (input.maxGamepads <= 0) {
        E2D_LOG_ERROR("Config validation failed: max gamepads must be positive");
        return false;
    }
    
    if (resource.maxCacheSize <= 0) {
        E2D_LOG_ERROR("Config validation failed: max cache size must be positive");
        return false;
    }
    if (resource.hotReloadInterval <= 0.0f) {
        E2D_LOG_ERROR("Config validation failed: hot reload interval must be positive");
        return false;
    }
    
    if (appName.empty()) {
        E2D_LOG_ERROR("Config validation failed: app name cannot be empty");
        return false;
    }
    if (appVersion.empty()) {
        E2D_LOG_ERROR("Config validation failed: app version cannot be empty");
        return false;
    }
    if (configFile.empty()) {
        E2D_LOG_ERROR("Config validation failed: config file cannot be empty");
        return false;
    }
    
    return true;
}

/**
 * @brief 应用平台约束
 * 根据平台特性调整配置参数
 * @param platform 平台配置接口
 */
void AppConfig::applyPlatformConstraints(const PlatformConfig& platform) {
    const PlatformCapabilities& caps = platform.capabilities();
    
    if (!caps.supportsWindowed && window.mode == WindowMode::Windowed) {
        E2D_LOG_WARN("Platform does not support windowed mode, switching to fullscreen");
        window.mode = WindowMode::Fullscreen;
    }
    if (!caps.supportsBorderless && window.mode == WindowMode::Borderless) {
        E2D_LOG_WARN("Platform does not support borderless mode, switching to fullscreen");
        window.mode = WindowMode::Fullscreen;
    }
    if (!caps.supportsFullscreen && window.mode == WindowMode::Fullscreen) {
        E2D_LOG_WARN("Platform does not support fullscreen mode, switching to windowed");
        window.mode = WindowMode::Windowed;
    }
    
    if (!caps.supportsResize) {
        window.resizable = false;
    }
    if (!caps.supportsHighDPI) {
        window.highDPI = false;
    }
    if (!caps.supportsCursor) {
        window.borderless = false;
    }
    
    if (!caps.supportsVSync) {
        render.vsync = false;
    }
    
    if (caps.maxTextureSize > 0) {
        if (render.maxTextureSize == 0 || render.maxTextureSize > caps.maxTextureSize) {
            render.maxTextureSize = caps.maxTextureSize;
        }
    }
    
    if (!caps.supportsGamepad) {
        input.maxGamepads = 0;
        input.enableVibration = false;
    }
    if (!caps.supportsKeyboard) {
        input.enabled = false;
    }
    if (!caps.supportsMouse) {
        input.rawMouseInput = false;
    }
    
    platform.applyConstraints(*this);
    
    E2D_LOG_INFO("Applied platform constraints for: {}", platform.platformName());
}

/**
 * @brief 重置为默认值
 * 将所有配置项恢复为默认值
 */
void AppConfig::reset() {
    *this = createDefault();
    E2D_LOG_INFO("App config reset to defaults");
}

/**
 * @brief 合并另一个配置
 * 将 other 中的非默认值覆盖到当前配置
 * @param other 要合并的配置
 */
void AppConfig::merge(const AppConfig& other) {
    if (other.window.title != "Extra2D Application") {
        window.title = other.window.title;
    }
    if (other.window.width != 1280) {
        window.width = other.window.width;
    }
    if (other.window.height != 720) {
        window.height = other.window.height;
    }
    if (other.window.minWidth != 320) {
        window.minWidth = other.window.minWidth;
    }
    if (other.window.minHeight != 240) {
        window.minHeight = other.window.minHeight;
    }
    if (other.window.maxWidth != 0) {
        window.maxWidth = other.window.maxWidth;
    }
    if (other.window.maxHeight != 0) {
        window.maxHeight = other.window.maxHeight;
    }
    if (other.window.mode != WindowMode::Windowed) {
        window.mode = other.window.mode;
    }
    if (other.window.resizable != true) {
        window.resizable = other.window.resizable;
    }
    if (other.window.borderless != false) {
        window.borderless = other.window.borderless;
    }
    if (other.window.alwaysOnTop != false) {
        window.alwaysOnTop = other.window.alwaysOnTop;
    }
    if (other.window.centered != true) {
        window.centered = other.window.centered;
    }
    if (other.window.posX >= 0) {
        window.posX = other.window.posX;
    }
    if (other.window.posY >= 0) {
        window.posY = other.window.posY;
    }
    if (other.window.hideOnClose != false) {
        window.hideOnClose = other.window.hideOnClose;
    }
    if (other.window.minimizeOnClose != true) {
        window.minimizeOnClose = other.window.minimizeOnClose;
    }
    if (other.window.opacity != 1.0f) {
        window.opacity = other.window.opacity;
    }
    if (other.window.transparentFramebuffer != false) {
        window.transparentFramebuffer = other.window.transparentFramebuffer;
    }
    if (other.window.highDPI != true) {
        window.highDPI = other.window.highDPI;
    }
    if (other.window.contentScale != 1.0f) {
        window.contentScale = other.window.contentScale;
    }
    
    if (other.render.backend != BackendType::OpenGL) {
        render.backend = other.render.backend;
    }
    if (other.render.targetFPS != 60) {
        render.targetFPS = other.render.targetFPS;
    }
    if (other.render.vsync != true) {
        render.vsync = other.render.vsync;
    }
    if (other.render.tripleBuffering != false) {
        render.tripleBuffering = other.render.tripleBuffering;
    }
    if (other.render.multisamples != 0) {
        render.multisamples = other.render.multisamples;
    }
    if (other.render.sRGBFramebuffer != false) {
        render.sRGBFramebuffer = other.render.sRGBFramebuffer;
    }
    if (other.render.maxTextureSize != 0) {
        render.maxTextureSize = other.render.maxTextureSize;
    }
    if (other.render.textureAnisotropy != 1) {
        render.textureAnisotropy = other.render.textureAnisotropy;
    }
    if (other.render.wireframeMode != false) {
        render.wireframeMode = other.render.wireframeMode;
    }
    if (other.render.depthTest != false) {
        render.depthTest = other.render.depthTest;
    }
    if (other.render.blending != true) {
        render.blending = other.render.blending;
    }
    if (other.render.dithering != false) {
        render.dithering = other.render.dithering;
    }
    if (other.render.spriteBatchSize != 1000) {
        render.spriteBatchSize = other.render.spriteBatchSize;
    }
    if (other.render.maxRenderTargets != 1) {
        render.maxRenderTargets = other.render.maxRenderTargets;
    }
    if (other.render.allowShaderHotReload != false) {
        render.allowShaderHotReload = other.render.allowShaderHotReload;
    }
    if (!other.render.shaderCachePath.empty()) {
        render.shaderCachePath = other.render.shaderCachePath;
    }
    
    if (other.audio.enabled != true) {
        audio.enabled = other.audio.enabled;
    }
    if (other.audio.masterVolume != 100) {
        audio.masterVolume = other.audio.masterVolume;
    }
    if (other.audio.musicVolume != 100) {
        audio.musicVolume = other.audio.musicVolume;
    }
    if (other.audio.sfxVolume != 100) {
        audio.sfxVolume = other.audio.sfxVolume;
    }
    if (other.audio.voiceVolume != 100) {
        audio.voiceVolume = other.audio.voiceVolume;
    }
    if (other.audio.ambientVolume != 100) {
        audio.ambientVolume = other.audio.ambientVolume;
    }
    if (other.audio.frequency != 44100) {
        audio.frequency = other.audio.frequency;
    }
    if (other.audio.channels != 2) {
        audio.channels = other.audio.channels;
    }
    if (other.audio.chunkSize != 2048) {
        audio.chunkSize = other.audio.chunkSize;
    }
    if (other.audio.maxChannels != 16) {
        audio.maxChannels = other.audio.maxChannels;
    }
    if (other.audio.spatialAudio != false) {
        audio.spatialAudio = other.audio.spatialAudio;
    }
    
    if (other.debug.enabled != false) {
        debug.enabled = other.debug.enabled;
    }
    if (other.debug.showFPS != false) {
        debug.showFPS = other.debug.showFPS;
    }
    if (other.debug.showMemoryUsage != false) {
        debug.showMemoryUsage = other.debug.showMemoryUsage;
    }
    if (other.debug.showRenderStats != false) {
        debug.showRenderStats = other.debug.showRenderStats;
    }
    if (other.debug.showColliders != false) {
        debug.showColliders = other.debug.showColliders;
    }
    if (other.debug.showGrid != false) {
        debug.showGrid = other.debug.showGrid;
    }
    if (other.debug.logToFile != false) {
        debug.logToFile = other.debug.logToFile;
    }
    if (other.debug.logToConsole != true) {
        debug.logToConsole = other.debug.logToConsole;
    }
    if (other.debug.logLevel != 2) {
        debug.logLevel = other.debug.logLevel;
    }
    if (other.debug.breakOnAssert != true) {
        debug.breakOnAssert = other.debug.breakOnAssert;
    }
    if (other.debug.enableProfiling != false) {
        debug.enableProfiling = other.debug.enableProfiling;
    }
    if (!other.debug.logFilePath.empty()) {
        debug.logFilePath = other.debug.logFilePath;
    }
    for (const auto& flag : other.debug.debugFlags) {
        debug.addDebugFlag(flag);
    }
    
    if (other.input.enabled != true) {
        input.enabled = other.input.enabled;
    }
    if (other.input.rawMouseInput != false) {
        input.rawMouseInput = other.input.rawMouseInput;
    }
    if (other.input.mouseSensitivity != 1.0f) {
        input.mouseSensitivity = other.input.mouseSensitivity;
    }
    if (other.input.invertMouseY != false) {
        input.invertMouseY = other.input.invertMouseY;
    }
    if (other.input.invertMouseX != false) {
        input.invertMouseX = other.input.invertMouseX;
    }
    if (other.input.deadzone != 0.15f) {
        input.deadzone = other.input.deadzone;
    }
    if (other.input.triggerThreshold != 0.5f) {
        input.triggerThreshold = other.input.triggerThreshold;
    }
    if (other.input.enableVibration != true) {
        input.enableVibration = other.input.enableVibration;
    }
    if (other.input.maxGamepads != 4) {
        input.maxGamepads = other.input.maxGamepads;
    }
    if (other.input.autoConnectGamepads != true) {
        input.autoConnectGamepads = other.input.autoConnectGamepads;
    }
    if (!other.input.gamepadMappingFile.empty()) {
        input.gamepadMappingFile = other.input.gamepadMappingFile;
    }
    
    if (other.resource.assetRootPath != "assets") {
        resource.assetRootPath = other.resource.assetRootPath;
    }
    if (other.resource.cachePath != "cache") {
        resource.cachePath = other.resource.cachePath;
    }
    if (other.resource.savePath != "saves") {
        resource.savePath = other.resource.savePath;
    }
    if (other.resource.configPath != "config") {
        resource.configPath = other.resource.configPath;
    }
    if (other.resource.logPath != "logs") {
        resource.logPath = other.resource.logPath;
    }
    if (other.resource.useAssetCache != true) {
        resource.useAssetCache = other.resource.useAssetCache;
    }
    if (other.resource.maxCacheSize != 512) {
        resource.maxCacheSize = other.resource.maxCacheSize;
    }
    if (other.resource.hotReloadEnabled != false) {
        resource.hotReloadEnabled = other.resource.hotReloadEnabled;
    }
    if (other.resource.hotReloadInterval != 1.0f) {
        resource.hotReloadInterval = other.resource.hotReloadInterval;
    }
    if (other.resource.compressTextures != false) {
        resource.compressTextures = other.resource.compressTextures;
    }
    if (other.resource.preloadCommonAssets != true) {
        resource.preloadCommonAssets = other.resource.preloadCommonAssets;
    }
    for (const auto& path : other.resource.searchPaths) {
        resource.addSearchPath(path);
    }
    
    if (other.appName != "Extra2D App") {
        appName = other.appName;
    }
    if (other.appVersion != "1.0.0") {
        appVersion = other.appVersion;
    }
    if (!other.organization.empty()) {
        organization = other.organization;
    }
    if (other.configFile != "config.json") {
        configFile = other.configFile;
    }
    if (other.targetPlatform != PlatformType::Auto) {
        targetPlatform = other.targetPlatform;
    }
    
    E2D_LOG_INFO("Merged app config");
}

}
