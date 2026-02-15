#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/platform/iinput.h>
#include <extra2d/core/types.h>

namespace extra2d {

/**
 * @brief 输入模块配置
 * 实现 IModuleConfig 接口
 */
class InputModuleConfig : public IModuleConfig {
public:
    bool enableKeyboard = true;
    bool enableMouse = true;
    bool enableGamepad = true;
    bool enableTouch = true;
    float deadzone = 0.15f;
    float mouseSensitivity = 1.0f;
    
    /**
     * @brief 获取模块信息
     * @return 模块信息结构体
     */
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.name = "Input";
        info.version = "1.0.0";
        info.priority = ModulePriority::Input;
        info.enabled = true;
        return info;
    }
    
    /**
     * @brief 获取配置节名称
     * @return 配置节名称字符串
     */
    std::string getConfigSectionName() const override { return "input"; }
    
    /**
     * @brief 验证配置有效性
     * @return 如果配置有效返回 true
     */
    bool validate() const override;
    
    /**
     * @brief 应用平台约束
     * 根据平台特性调整配置
     * @param platform 目标平台类型
     */
    void applyPlatformConstraints(PlatformType platform) override;
    
    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults() override;
    
    /**
     * @brief 从 JSON 数据加载配置
     * @param jsonData JSON 数据指针
     * @return 加载成功返回 true
     */
    bool loadFromJson(const void* jsonData) override;
    
    /**
     * @brief 保存配置到 JSON 数据
     * @param jsonData JSON 数据指针
     * @return 保存成功返回 true
     */
    bool saveToJson(void* jsonData) const override;
};

/**
 * @brief 输入模块初始化器
 * 实现 IModuleInitializer 接口
 * 依赖窗口模块
 */
class InputModuleInitializer : public IModuleInitializer {
public:
    /**
     * @brief 构造函数
     */
    InputModuleInitializer();
    
    /**
     * @brief 析构函数
     */
    ~InputModuleInitializer() override;
    
    /**
     * @brief 获取模块标识符
     * @return 模块唯一标识符
     */
    ModuleId getModuleId() const override { return moduleId_; }
    
    /**
     * @brief 获取模块优先级
     * @return 模块优先级
     */
    ModulePriority getPriority() const override { return ModulePriority::Input; }
    
    /**
     * @brief 获取模块依赖列表
     * 返回此模块依赖的其他模块标识符
     * @return 依赖模块标识符列表
     */
    std::vector<ModuleId> getDependencies() const override;
    
    /**
     * @brief 初始化模块
     * @param config 模块配置指针
     * @return 初始化成功返回 true
     */
    bool initialize(const IModuleConfig* config) override;
    
    /**
     * @brief 关闭模块
     */
    void shutdown() override;
    
    /**
     * @brief 检查模块是否已初始化
     * @return 已初始化返回 true
     */
    bool isInitialized() const override { return initialized_; }
    
    /**
     * @brief 获取输入接口
     * @return 输入接口指针
     */
    IInput* getInput() const { return input_; }
    
    /**
     * @brief 设置窗口模块标识符
     * @param windowModuleId 窗口模块标识符
     */
    void setWindowModuleId(ModuleId windowModuleId) { windowModuleId_ = windowModuleId; }
    
private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    ModuleId windowModuleId_ = INVALID_MODULE_ID;
    IInput* input_ = nullptr;
    bool initialized_ = false;
};

} // namespace extra2d
