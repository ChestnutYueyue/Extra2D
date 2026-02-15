#include <extra2d/graphics/render_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/platform/iwindow.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace extra2d {

// ============================================================================
// RenderModuleConfig 实现
// ============================================================================

/**
 * @brief 验证渲染配置有效性
 * 
 * 检查渲染配置的各项参数是否在有效范围内：
 * - 目标帧率应在 1-240 之间
 * - 多重采样数应为 0、2、4、8 或 16
 * - 精灵批处理大小应大于 0
 * 
 * @return 如果配置有效返回 true
 */
bool RenderModuleConfig::validate() const {
    if (targetFPS < 1 || targetFPS > 240) {
        E2D_LOG_ERROR("Invalid target FPS: {}, must be between 1 and 240", targetFPS);
        return false;
    }
    
    if (multisamples != 0 && multisamples != 2 && multisamples != 4 && 
        multisamples != 8 && multisamples != 16) {
        E2D_LOG_ERROR("Invalid multisample count: {}, must be 0, 2, 4, 8 or 16", multisamples);
        return false;
    }
    
    if (spriteBatchSize <= 0) {
        E2D_LOG_ERROR("Invalid sprite batch size: {}, must be greater than 0", spriteBatchSize);
        return false;
    }
    
    return true;
}

/**
 * @brief 应用平台约束
 * 
 * 根据不同平台的特性调整渲染配置：
 * - Switch 平台限制 MSAA 最大为 4，禁用 sRGB 帧缓冲
 * - 其他平台保持用户配置
 * 
 * @param platform 目标平台类型
 */
void RenderModuleConfig::applyPlatformConstraints(PlatformType platform) {
    switch (platform) {
        case PlatformType::Switch:
            if (multisamples > 4) {
                E2D_LOG_WARN("Switch platform limits MSAA to 4x, reducing from {}", multisamples);
                multisamples = 4;
            }
            if (sRGBFramebuffer) {
                E2D_LOG_WARN("Switch platform does not support sRGB framebuffer, disabling");
                sRGBFramebuffer = false;
            }
            if (targetFPS > 60) {
                E2D_LOG_WARN("Switch platform target FPS capped at 60");
                targetFPS = 60;
            }
            break;
            
        case PlatformType::Windows:
        case PlatformType::Linux:
        case PlatformType::macOS:
        default:
            break;
    }
}

/**
 * @brief 重置为默认配置
 * 
 * 将所有配置项恢复为默认值：
 * - 后端类型：OpenGL
 * - 垂直同步：启用
 * - 目标帧率：60
 * - 多重采样：禁用
 * - sRGB 帧缓冲：禁用
 * - 精灵批处理大小：1000
 */
void RenderModuleConfig::resetToDefaults() {
    backend = BackendType::OpenGL;
    vsync = true;
    targetFPS = 60;
    multisamples = 0;
    sRGBFramebuffer = false;
    spriteBatchSize = 1000;
}

/**
 * @brief 从 JSON 数据加载配置
 * 
 * 从 JSON 对象中解析渲染配置参数
 * 
 * @param jsonData JSON 数据指针（nlohmann::json 对象指针）
 * @return 加载成功返回 true
 */
bool RenderModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) {
        E2D_LOG_ERROR("Null JSON data provided");
        return false;
    }
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("backend")) {
            std::string backendStr = j["backend"].get<std::string>();
            if (backendStr == "opengl") {
                backend = BackendType::OpenGL;
            } else {
                E2D_LOG_WARN("Unknown backend type: {}, defaulting to OpenGL", backendStr);
                backend = BackendType::OpenGL;
            }
        }
        
        if (j.contains("vsync")) {
            vsync = j["vsync"].get<bool>();
        }
        
        if (j.contains("targetFPS")) {
            targetFPS = j["targetFPS"].get<int>();
        }
        
        if (j.contains("multisamples")) {
            multisamples = j["multisamples"].get<int>();
        }
        
        if (j.contains("sRGBFramebuffer")) {
            sRGBFramebuffer = j["sRGBFramebuffer"].get<bool>();
        }
        
        if (j.contains("spriteBatchSize")) {
            spriteBatchSize = j["spriteBatchSize"].get<int>();
        }
        
        E2D_LOG_INFO("Render config loaded from JSON");
        return true;
    } catch (const json::exception& e) {
        E2D_LOG_ERROR("Failed to parse render config from JSON: {}", e.what());
        return false;
    }
}

/**
 * @brief 保存配置到 JSON 数据
 * 
 * 将当前配置序列化到 JSON 对象
 * 
 * @param jsonData JSON 数据指针（nlohmann::json 对象指针）
 * @return 保存成功返回 true
 */
bool RenderModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) {
        E2D_LOG_ERROR("Null JSON data provided");
        return false;
    }
    
    try {
        json& j = *static_cast<json*>(jsonData);
        
        std::string backendStr = "opengl";
        switch (backend) {
            case BackendType::OpenGL:
                backendStr = "opengl";
                break;
            default:
                backendStr = "opengl";
                break;
        }
        
        j["backend"] = backendStr;
        j["vsync"] = vsync;
        j["targetFPS"] = targetFPS;
        j["multisamples"] = multisamples;
        j["sRGBFramebuffer"] = sRGBFramebuffer;
        j["spriteBatchSize"] = spriteBatchSize;
        
        E2D_LOG_INFO("Render config saved to JSON");
        return true;
    } catch (const json::exception& e) {
        E2D_LOG_ERROR("Failed to save render config to JSON: {}", e.what());
        return false;
    }
}

// ============================================================================
// RenderModuleInitializer 实现
// ============================================================================

/**
 * @brief 构造函数
 * 
 * 初始化渲染模块初始化器的成员变量
 */
RenderModuleInitializer::RenderModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , windowModuleId_(INVALID_MODULE_ID)
    , renderer_(nullptr)
    , initialized_(false) {
}

/**
 * @brief 析构函数
 * 
 * 确保在销毁时关闭模块
 */
RenderModuleInitializer::~RenderModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

/**
 * @brief 获取模块依赖列表
 * 
 * 返回渲染模块依赖的窗口模块标识符列表
 * 
 * @return 依赖模块标识符列表
 */
std::vector<ModuleId> RenderModuleInitializer::getDependencies() const {
    if (windowModuleId_ != INVALID_MODULE_ID) {
        return {windowModuleId_};
    }
    return {};
}

/**
 * @brief 初始化模块
 * 
 * 根据配置创建渲染后端实例并初始化渲染器
 * 
 * @param config 模块配置指针
 * @return 初始化成功返回 true
 */
bool RenderModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) {
        E2D_LOG_WARN("Render module already initialized");
        return true;
    }
    
    if (!config) {
        E2D_LOG_ERROR("Null config provided for render module initialization");
        return false;
    }
    
    const RenderModuleConfig* renderConfig = dynamic_cast<const RenderModuleConfig*>(config);
    if (!renderConfig) {
        E2D_LOG_ERROR("Invalid config type for render module");
        return false;
    }
    
    if (!renderConfig->validate()) {
        E2D_LOG_ERROR("Invalid render module configuration");
        return false;
    }
    
    renderer_ = RenderBackend::create(renderConfig->backend);
    if (!renderer_) {
        E2D_LOG_ERROR("Failed to create render backend");
        return false;
    }
    
    IWindow* window = nullptr;
    if (windowModuleId_ != INVALID_MODULE_ID) {
        ModuleRegistry& registry = ModuleRegistry::instance();
        IModuleConfig* windowConfig = registry.getModuleConfig(windowModuleId_);
        if (windowConfig) {
            E2D_LOG_INFO("Render module found window module dependency");
        }
    }
    
    E2D_LOG_INFO("Render module initialized successfully");
    E2D_LOG_INFO("  Backend: {}", renderConfig->backend == BackendType::OpenGL ? "OpenGL" : "Unknown");
    E2D_LOG_INFO("  VSync: {}", renderConfig->vsync ? "enabled" : "disabled");
    E2D_LOG_INFO("  Target FPS: {}", renderConfig->targetFPS);
    E2D_LOG_INFO("  Multisamples: {}", renderConfig->multisamples);
    E2D_LOG_INFO("  Sprite Batch Size: {}", renderConfig->spriteBatchSize);
    
    initialized_ = true;
    return true;
}

/**
 * @brief 关闭模块
 * 
 * 销毁渲染后端实例并清理资源
 */
void RenderModuleInitializer::shutdown() {
    if (!initialized_) {
        return;
    }
    
    if (renderer_) {
        renderer_->shutdown();
        renderer_.reset();
    }
    
    initialized_ = false;
    E2D_LOG_INFO("Render module shutdown complete");
}

} // namespace extra2d
