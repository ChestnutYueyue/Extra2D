#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/input/input_config.h>
#include <extra2d/platform/iinput.h>
#include <extra2d/core/types.h>

namespace extra2d {

/**
 * @file input_module.h
 * @brief 输入模块
 * 
 * 输入模块管理键盘、鼠标、手柄和触摸输入。
 * 通过事件系统分发输入事件。
 */

/**
 * @brief 输入模块配置
 * 实现 IModuleConfig 接口
 */
class InputModuleConfig : public IModuleConfig {
public:
    InputConfigData inputConfig;

    /**
     * @brief 获取模块信息
     * @return 模块信息结构体
     */
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
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
     * @param platform 目标平台类型
     */
    void applyPlatformConstraints(PlatformType platform) override;
    
    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults() override {
        inputConfig = InputConfigData{};
    }
    
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
 */
class InputModuleInitializer : public IModuleInitializer {
public:
    InputModuleInitializer();
    ~InputModuleInitializer() override;
    
    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Input; }
    std::vector<ModuleId> getDependencies() const override;
    bool initialize(const IModuleConfig* config) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }
    
    void setModuleId(ModuleId id) { moduleId_ = id; }
    
    /**
     * @brief 获取输入接口
     * @return 输入接口指针
     */
    IInput* getInput() const { return input_; }
    
    /**
     * @brief 更新输入状态
     * 每帧调用，更新输入状态并分发事件
     */
    void update();

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    IInput* input_ = nullptr;
    bool initialized_ = false;
    InputConfigData config_;
};

/**
 * @brief 获取输入模块标识符
 * @return 输入模块标识符
 */
ModuleId get_input_module_id();

/**
 * @brief 注册输入模块
 */
void register_input_module();

} 
