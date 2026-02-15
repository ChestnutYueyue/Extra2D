#include <extra2d/platform/window_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/platform/platform_module.h>
#include <extra2d/utils/logger.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace extra2d {

// ============================================================================
// WindowModuleConfig 实现
// ============================================================================

/**
 * @brief 验证窗口配置有效性
 * 检查窗口尺寸、标题等配置是否合法
 * @return 如果配置有效返回 true
 */
bool WindowModuleConfig::validate() const {
    if (windowConfig.width <= 0) {
        E2D_LOG_ERROR("Window width must be positive, got: {}", windowConfig.width);
        return false;
    }
    
    if (windowConfig.height <= 0) {
        E2D_LOG_ERROR("Window height must be positive, got: {}", windowConfig.height);
        return false;
    }
    
    if (windowConfig.title.empty()) {
        E2D_LOG_WARN("Window title is empty, using default title");
    }
    
    if (windowConfig.multisamples < 0) {
        E2D_LOG_ERROR("MSAA samples cannot be negative, got: {}", windowConfig.multisamples);
        return false;
    }
    
    if (windowConfig.multisamples != 0 && 
        windowConfig.multisamples != 2 && 
        windowConfig.multisamples != 4 && 
        windowConfig.multisamples != 8 && 
        windowConfig.multisamples != 16) {
        E2D_LOG_WARN("MSAA samples should be 0, 2, 4, 8, or 16, got: {}", windowConfig.multisamples);
    }
    
    if (backend.empty()) {
        E2D_LOG_ERROR("Backend name cannot be empty");
        return false;
    }
    
    return true;
}

/**
 * @brief 应用平台约束
 * 根据目标平台特性调整窗口配置
 * @param platform 目标平台类型
 */
void WindowModuleConfig::applyPlatformConstraints(PlatformType platform) {
    switch (platform) {
        case PlatformType::Switch:
            E2D_LOG_INFO("Applying Nintendo Switch platform constraints");
            windowConfig.mode = WindowMode::Fullscreen;
            windowConfig.resizable = false;
            windowConfig.centered = false;
            windowConfig.width = 1920;
            windowConfig.height = 1080;
            backend = "switch";
            break;
            
        case PlatformType::Windows:
        case PlatformType::Linux:
        case PlatformType::macOS:
            E2D_LOG_INFO("Applying desktop platform constraints");
            if (windowConfig.width <= 0) {
                windowConfig.width = 1280;
            }
            if (windowConfig.height <= 0) {
                windowConfig.height = 720;
            }
            break;
            
        case PlatformType::Auto:
        default:
            E2D_LOG_INFO("Auto-detecting platform constraints");
            break;
    }
}

/**
 * @brief 重置为默认配置
 * 将所有配置项恢复为默认值
 */
void WindowModuleConfig::resetToDefaults() {
    windowConfig = WindowConfigData{};
    backend = "sdl2";
    
    E2D_LOG_INFO("Window module config reset to defaults");
}

/**
 * @brief 从 JSON 数据加载配置
 * 解析 JSON 对象并填充配置数据
 * @param jsonData JSON 数据指针
 * @return 加载成功返回 true
 */
bool WindowModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) {
        E2D_LOG_ERROR("JSON data is null");
        return false;
    }
    
    const json& obj = *static_cast<const json*>(jsonData);
    
    if (!obj.is_object()) {
        E2D_LOG_ERROR("JSON data must be an object");
        return false;
    }
    
    if (obj.contains("title") && obj["title"].is_string()) {
        windowConfig.title = obj["title"].get<std::string>();
    }
    
    if (obj.contains("width") && obj["width"].is_number_integer()) {
        windowConfig.width = obj["width"].get<int>();
    }
    
    if (obj.contains("height") && obj["height"].is_number_integer()) {
        windowConfig.height = obj["height"].get<int>();
    }
    
    if (obj.contains("fullscreen") && obj["fullscreen"].is_boolean()) {
        windowConfig.mode = obj["fullscreen"].get<bool>() ? WindowMode::Fullscreen : WindowMode::Windowed;
    }
    
    if (obj.contains("mode") && obj["mode"].is_string()) {
        std::string modeStr = obj["mode"].get<std::string>();
        if (modeStr == "fullscreen") {
            windowConfig.mode = WindowMode::Fullscreen;
        } else if (modeStr == "borderless") {
            windowConfig.mode = WindowMode::Borderless;
        } else {
            windowConfig.mode = WindowMode::Windowed;
        }
    }
    
    if (obj.contains("resizable") && obj["resizable"].is_boolean()) {
        windowConfig.resizable = obj["resizable"].get<bool>();
    }
    
    if (obj.contains("vsync") && obj["vsync"].is_boolean()) {
        windowConfig.vsync = obj["vsync"].get<bool>();
    }
    
    if (obj.contains("multisamples") && obj["multisamples"].is_number_integer()) {
        windowConfig.multisamples = obj["multisamples"].get<int>();
    }
    
    if (obj.contains("msaaSamples") && obj["msaaSamples"].is_number_integer()) {
        windowConfig.multisamples = obj["msaaSamples"].get<int>();
    }
    
    if (obj.contains("centered") && obj["centered"].is_boolean()) {
        windowConfig.centered = obj["centered"].get<bool>();
    }
    
    if (obj.contains("centerWindow") && obj["centerWindow"].is_boolean()) {
        windowConfig.centered = obj["centerWindow"].get<bool>();
    }
    
    if (obj.contains("visible") && obj["visible"].is_boolean()) {
        windowConfig.visible = obj["visible"].get<bool>();
    }
    
    if (obj.contains("decorated") && obj["decorated"].is_boolean()) {
        windowConfig.decorated = obj["decorated"].get<bool>();
    }
    
    if (obj.contains("backend") && obj["backend"].is_string()) {
        backend = obj["backend"].get<std::string>();
    }
    
    E2D_LOG_INFO("Window module config loaded from JSON");
    return true;
}

/**
 * @brief 保存配置到 JSON 数据
 * 将配置数据序列化为 JSON 对象
 * @param jsonData JSON 数据指针
 * @return 保存成功返回 true
 */
bool WindowModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) {
        E2D_LOG_ERROR("JSON data pointer is null");
        return false;
    }
    
    json& obj = *static_cast<json*>(jsonData);
    
    obj["title"] = windowConfig.title;
    obj["width"] = windowConfig.width;
    obj["height"] = windowConfig.height;
    
    std::string modeStr = "windowed";
    if (windowConfig.mode == WindowMode::Fullscreen) {
        modeStr = "fullscreen";
    } else if (windowConfig.mode == WindowMode::Borderless) {
        modeStr = "borderless";
    }
    obj["mode"] = modeStr;
    
    obj["resizable"] = windowConfig.resizable;
    obj["vsync"] = windowConfig.vsync;
    obj["multisamples"] = windowConfig.multisamples;
    obj["centered"] = windowConfig.centered;
    obj["visible"] = windowConfig.visible;
    obj["decorated"] = windowConfig.decorated;
    obj["backend"] = backend;
    
    E2D_LOG_INFO("Window module config saved to JSON");
    return true;
}

// ============================================================================
// WindowModuleInitializer 实现
// ============================================================================

/**
 * @brief 构造函数
 * 初始化窗口模块初始化器
 */
WindowModuleInitializer::WindowModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , window_(nullptr)
    , initialized_(false) {
    E2D_LOG_DEBUG("WindowModuleInitializer constructed");
}

/**
 * @brief 析构函数
 * 确保模块正确关闭
 */
WindowModuleInitializer::~WindowModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
    E2D_LOG_DEBUG("WindowModuleInitializer destructed");
}

/**
 * @brief 初始化模块
 * 使用 BackendFactory 创建窗口实例
 * @param config 模块配置指针
 * @return 初始化成功返回 true
 */
bool WindowModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) {
        E2D_LOG_WARN("Window module already initialized");
        return true;
    }
    
    if (!config) {
        E2D_LOG_ERROR("Window module config is null");
        return false;
    }
    
    const WindowModuleConfig* windowConfig = dynamic_cast<const WindowModuleConfig*>(config);
    if (!windowConfig) {
        E2D_LOG_ERROR("Invalid config type for window module");
        return false;
    }
    
    ModuleInfo info = config->getModuleInfo();
    moduleId_ = info.id;
    
    const std::string& backend = windowConfig->backend;
    
    if (!BackendFactory::has(backend)) {
        E2D_LOG_ERROR("Backend '{}' not available", backend);
        auto backends = BackendFactory::backends();
        if (backends.empty()) {
            E2D_LOG_ERROR("No backends registered!");
            return false;
        }
        std::string backendList;
        for (const auto& b : backends) {
            if (!backendList.empty()) backendList += ", ";
            backendList += b;
        }
        E2D_LOG_WARN("Available backends: {}", backendList);
        return false;
    }
    
    window_ = BackendFactory::createWindow(backend);
    if (!window_) {
        E2D_LOG_ERROR("Failed to create window for backend: {}", backend);
        return false;
    }
    
    if (!window_->create(windowConfig->windowConfig)) {
        E2D_LOG_ERROR("Failed to create window with given config");
        window_.reset();
        return false;
    }
    
    initialized_ = true;
    E2D_LOG_INFO("Window module initialized successfully (backend: {}, {}x{})", 
                 backend, 
                 windowConfig->windowConfig.width, 
                 windowConfig->windowConfig.height);
    
    return true;
}

/**
 * @brief 关闭模块
 * 销毁窗口实例并重置状态
 */
void WindowModuleInitializer::shutdown() {
    if (!initialized_) {
        E2D_LOG_WARN("Window module not initialized, nothing to shutdown");
        return;
    }
    
    if (window_) {
        window_->destroy();
        window_.reset();
    }
    
    initialized_ = false;
    moduleId_ = INVALID_MODULE_ID;
    
    E2D_LOG_INFO("Window module shutdown complete");
}

} // namespace extra2d
