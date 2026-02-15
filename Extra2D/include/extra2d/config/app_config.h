#pragma once

#include <extra2d/config/platform_config.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/render_backend.h>
#include <string>
#include <vector>

namespace extra2d {

// ============================================================================
// 窗口模式枚举
// ============================================================================
enum class WindowMode {
    Windowed,       
    Fullscreen,     
    Borderless      
};

// ============================================================================
// 窗口配置数据
// ============================================================================
struct WindowConfigData {
    std::string title = "Extra2D Application";
    int width = 1280;
    int height = 720;
    int minWidth = 320;
    int minHeight = 240;
    int maxWidth = 0;  
    int maxHeight = 0;  
    WindowMode mode = WindowMode::Windowed;
    bool resizable = true;
    bool borderless = false;
    bool alwaysOnTop = false;
    bool centered = true;
    int posX = -1;     
    int posY = -1;     
    bool hideOnClose = false;
    bool minimizeOnClose = true;
    float opacity = 1.0f;
    bool transparentFramebuffer = false;
    bool highDPI = true;
    float contentScale = 1.0f;
    bool vsync = true;
    int multisamples = 0;
    bool visible = true;
    bool decorated = true;

    bool isSizeValid() const { return width > 0 && height > 0; }
    bool hasPosition() const { return posX >= 0 && posY >= 0; }
    float aspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }
    bool isFullscreen() const { return mode == WindowMode::Fullscreen; }
    bool isBorderless() const { return mode == WindowMode::Borderless || borderless; }
};

// ============================================================================
// 渲染配置数据
// ============================================================================
struct RenderConfigData {
    BackendType backend = BackendType::OpenGL;
    int targetFPS = 60;
    bool vsync = true;
    bool tripleBuffering = false;
    int multisamples = 0;  
    bool sRGBFramebuffer = false;
    Color clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    int maxTextureSize = 0;  
    int textureAnisotropy = 1;
    bool wireframeMode = false;
    bool depthTest = false;
    bool blending = true;
    bool dithering = false;
    int spriteBatchSize = 1000;
    int maxRenderTargets = 1;
    bool allowShaderHotReload = false;
    std::string shaderCachePath;

    bool isMultisampleEnabled() const { return multisamples > 0; }
    bool isFPSCapped() const { return targetFPS > 0; }
};

// ============================================================================
// 音频配置数据
// ============================================================================
struct AudioConfigData {
    bool enabled = true;
    int masterVolume = 100;     
    int musicVolume = 100;      
    int sfxVolume = 100;        
    int voiceVolume = 100;      
    int ambientVolume = 100;    
    int frequency = 44100;      
    int channels = 2;           
    int chunkSize = 2048;       
    int maxChannels = 16;       
    bool spatialAudio = false;  
    float listenerPosition[3] = {0.0f, 0.0f, 0.0f};

    bool isValidVolume(int volume) const { return volume >= 0 && volume <= 100; }
    float volumeToFloat(int volume) const { return static_cast<float>(volume) / 100.0f; }
};

// ============================================================================
// 调试配置数据
// ============================================================================
struct DebugConfigData {
    bool enabled = false;
    bool showFPS = false;
    bool showMemoryUsage = false;
    bool showRenderStats = false;
    bool showColliders = false;
    bool showGrid = false;
    bool logToFile = false;
    bool logToConsole = true;
    int logLevel = 2;  
    bool breakOnAssert = true;
    bool enableProfiling = false;
    std::string logFilePath;
    std::vector<std::string> debugFlags;

    bool hasDebugFlag(const std::string& flag) const;
    void addDebugFlag(const std::string& flag);
    void removeDebugFlag(const std::string& flag);
};

// ============================================================================
// 输入配置数据
// ============================================================================
struct InputConfigData {
    bool enabled = true;
    bool rawMouseInput = false;
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    bool invertMouseX = false;
    float deadzone = 0.15f;  
    float triggerThreshold = 0.5f;
    bool enableVibration = true;
    int maxGamepads = 4;
    bool autoConnectGamepads = true;
    std::string gamepadMappingFile;

    bool isDeadzoneValid() const { return deadzone >= 0.0f && deadzone <= 1.0f; }
};

// ============================================================================
// 资源配置数据
// ============================================================================
struct ResourceConfigData {
    std::string assetRootPath = "assets";
    std::string cachePath = "cache";
    std::string savePath = "saves";
    std::string configPath = "config";
    std::string logPath = "logs";
    bool useAssetCache = true;
    int maxCacheSize = 512;  
    bool hotReloadEnabled = false;
    float hotReloadInterval = 1.0f;  
    bool compressTextures = false;
    bool preloadCommonAssets = true;
    std::vector<std::string> searchPaths;

    void addSearchPath(const std::string& path);
    void removeSearchPath(const std::string& path);
    bool hasSearchPath(const std::string& path) const;
};

// ============================================================================
// 应用统一配置
// ============================================================================
struct AppConfig {
    WindowConfigData window;
    RenderConfigData render;
    AudioConfigData audio;
    DebugConfigData debug;
    InputConfigData input;
    ResourceConfigData resource;
    std::string appName = "Extra2D App";
    std::string appVersion = "1.0.0";
    std::string organization = "";
    std::string configFile = "config.json";
    PlatformType targetPlatform = PlatformType::Auto;

    /**
     * @brief 创建默认配置
     * @return 默认的应用配置实例
     */
    static AppConfig createDefault();

    /**
     * @brief 验证配置的有效性
     * @return 如果配置有效返回 true，否则返回 false
     */
    bool validate() const;

    /**
     * @brief 应用平台约束
     * @param platform 平台配置接口
     */
    void applyPlatformConstraints(const PlatformConfig& platform);

    /**
     * @brief 重置为默认值
     */
    void reset();

    /**
     * @brief 合并另一个配置（非默认值覆盖当前值）
     * @param other 要合并的配置
     */
    void merge(const AppConfig& other);

    /**
     * @brief 检查配置是否有效
     * @return 如果所有必要字段都有效返回 true
     */
    bool isValid() const { return validate(); }

    /**
     * @brief 获取窗口宽高比
     * @return 窗口的宽高比
     */
    float aspectRatio() const { return window.aspectRatio(); }
};

} 
