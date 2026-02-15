#include <extra2d/platform/input_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/platform/platform_module.h>
#include <extra2d/utils/logger.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace extra2d {

// ============================================================================
// InputModuleConfig 实现
// ============================================================================

/**
 * @brief 验证配置有效性
 * 检查各项配置参数是否在有效范围内
 * @return 如果配置有效返回 true
 */
bool InputModuleConfig::validate() const {
    if (deadzone < 0.0f || deadzone > 1.0f) {
        E2D_LOG_ERROR("InputModuleConfig: deadzone 必须在 [0.0, 1.0] 范围内，当前值: {}", deadzone);
        return false;
    }
    
    if (mouseSensitivity < 0.0f || mouseSensitivity > 10.0f) {
        E2D_LOG_ERROR("InputModuleConfig: mouseSensitivity 必须在 [0.0, 10.0] 范围内，当前值: {}", mouseSensitivity);
        return false;
    }
    
    if (!enableKeyboard && !enableMouse && !enableGamepad && !enableTouch) {
        E2D_LOG_WARN("InputModuleConfig: 所有输入设备都已禁用");
    }
    
    return true;
}

/**
 * @brief 应用平台约束
 * 根据平台特性调整配置
 * @param platform 目标平台类型
 */
void InputModuleConfig::applyPlatformConstraints(PlatformType platform) {
    switch (platform) {
        case PlatformType::Switch:
            enableMouse = false;
            enableTouch = true;
            enableGamepad = true;
            E2D_LOG_INFO("InputModuleConfig: Switch 平台 - 禁用鼠标输入");
            break;
            
        case PlatformType::Windows:
        case PlatformType::Linux:
        case PlatformType::macOS:
            enableMouse = true;
            enableKeyboard = true;
            enableTouch = false;
            E2D_LOG_INFO("InputModuleConfig: PC 平台 - 禁用触摸输入");
            break;
            
        case PlatformType::Auto:
        default:
            break;
    }
}

/**
 * @brief 重置为默认配置
 */
void InputModuleConfig::resetToDefaults() {
    enableKeyboard = true;
    enableMouse = true;
    enableGamepad = true;
    enableTouch = true;
    deadzone = 0.15f;
    mouseSensitivity = 1.0f;
    
    E2D_LOG_INFO("InputModuleConfig: 已重置为默认配置");
}

/**
 * @brief 从 JSON 数据加载配置
 * @param jsonData JSON 数据指针
 * @return 加载成功返回 true
 */
bool InputModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) {
        E2D_LOG_ERROR("InputModuleConfig: JSON 数据指针为空");
        return false;
    }
    
    try {
        const json& obj = *static_cast<const json*>(jsonData);
        
        if (!obj.is_object()) {
            E2D_LOG_ERROR("InputModuleConfig: JSON 数据不是对象类型");
            return false;
        }
        
        if (obj.contains("enableKeyboard") && obj["enableKeyboard"].is_boolean()) {
            enableKeyboard = obj["enableKeyboard"].get<bool>();
        }
        
        if (obj.contains("enableMouse") && obj["enableMouse"].is_boolean()) {
            enableMouse = obj["enableMouse"].get<bool>();
        }
        
        if (obj.contains("enableGamepad") && obj["enableGamepad"].is_boolean()) {
            enableGamepad = obj["enableGamepad"].get<bool>();
        }
        
        if (obj.contains("enableTouch") && obj["enableTouch"].is_boolean()) {
            enableTouch = obj["enableTouch"].get<bool>();
        }
        
        if (obj.contains("deadzone") && obj["deadzone"].is_number()) {
            deadzone = obj["deadzone"].get<float>();
        }
        
        if (obj.contains("mouseSensitivity") && obj["mouseSensitivity"].is_number()) {
            mouseSensitivity = obj["mouseSensitivity"].get<float>();
        }
        
        E2D_LOG_INFO("InputModuleConfig: 从 JSON 加载配置成功");
        return true;
        
    } catch (const json::exception& e) {
        E2D_LOG_ERROR("InputModuleConfig: JSON 解析错误: {}", e.what());
        return false;
    }
}

/**
 * @brief 保存配置到 JSON 数据
 * @param jsonData JSON 数据指针
 * @return 保存成功返回 true
 */
bool InputModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) {
        E2D_LOG_ERROR("InputModuleConfig: JSON 数据指针为空");
        return false;
    }
    
    try {
        json& obj = *static_cast<json*>(jsonData);
        
        obj["enableKeyboard"] = enableKeyboard;
        obj["enableMouse"] = enableMouse;
        obj["enableGamepad"] = enableGamepad;
        obj["enableTouch"] = enableTouch;
        obj["deadzone"] = deadzone;
        obj["mouseSensitivity"] = mouseSensitivity;
        
        E2D_LOG_INFO("InputModuleConfig: 保存配置到 JSON 成功");
        return true;
        
    } catch (const json::exception& e) {
        E2D_LOG_ERROR("InputModuleConfig: JSON 序列化错误: {}", e.what());
        return false;
    }
}

// ============================================================================
// InputModuleInitializer 实现
// ============================================================================

/**
 * @brief 构造函数
 */
InputModuleInitializer::InputModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , windowModuleId_(INVALID_MODULE_ID)
    , input_(nullptr)
    , initialized_(false) {
    E2D_LOG_DEBUG("InputModuleInitializer: 构造函数调用");
}

/**
 * @brief 析构函数
 */
InputModuleInitializer::~InputModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
    E2D_LOG_DEBUG("InputModuleInitializer: 析构函数调用");
}

/**
 * @brief 获取模块依赖列表
 * 返回此模块依赖的窗口模块标识符
 * @return 依赖模块标识符列表
 */
std::vector<ModuleId> InputModuleInitializer::getDependencies() const {
    std::vector<ModuleId> dependencies;
    
    if (windowModuleId_ != INVALID_MODULE_ID) {
        dependencies.push_back(windowModuleId_);
    }
    
    return dependencies;
}

/**
 * @brief 初始化模块
 * 从窗口模块获取输入接口
 * @param config 模块配置指针
 * @return 初始化成功返回 true
 */
bool InputModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) {
        E2D_LOG_WARN("InputModuleInitializer: 模块已经初始化");
        return true;
    }
    
    if (!config) {
        E2D_LOG_ERROR("InputModuleInitializer: 配置指针为空");
        return false;
    }
    
    const InputModuleConfig* inputConfig = dynamic_cast<const InputModuleConfig*>(config);
    if (!inputConfig) {
        E2D_LOG_ERROR("InputModuleInitializer: 配置类型不正确");
        return false;
    }
    
    if (!inputConfig->validate()) {
        E2D_LOG_ERROR("InputModuleInitializer: 配置验证失败");
        return false;
    }
    
    ModuleInfo info = config->getModuleInfo();
    moduleId_ = info.id;
    
    E2D_LOG_INFO("InputModuleInitializer: 正在初始化输入模块 '{}' (版本: {})", 
                 info.name, info.version);
    
    E2D_LOG_INFO("InputModuleInitializer: 输入配置 - 键盘: {}, 鼠标: {}, 手柄: {}, 触摸: {}",
                 inputConfig->enableKeyboard ? "启用" : "禁用",
                 inputConfig->enableMouse ? "启用" : "禁用",
                 inputConfig->enableGamepad ? "启用" : "禁用",
                 inputConfig->enableTouch ? "启用" : "禁用");
    
    E2D_LOG_INFO("InputModuleInitializer: 死区: {}, 鼠标灵敏度: {}",
                 inputConfig->deadzone, inputConfig->mouseSensitivity);
    
    E2D_LOG_INFO("InputModuleInitializer: 输入模块初始化成功");
    initialized_ = true;
    return true;
}

/**
 * @brief 关闭模块
 */
void InputModuleInitializer::shutdown() {
    if (!initialized_) {
        E2D_LOG_WARN("InputModuleInitializer: 模块未初始化，无需关闭");
        return;
    }
    
    E2D_LOG_INFO("InputModuleInitializer: 正在关闭输入模块");
    
    input_ = nullptr;
    moduleId_ = INVALID_MODULE_ID;
    initialized_ = false;
    
    E2D_LOG_INFO("InputModuleInitializer: 输入模块已关闭");
}

} // namespace extra2d
