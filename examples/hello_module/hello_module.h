#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <string>

namespace extra2d {

/**
 * @brief Hello模块配置数据结构
 */
struct HelloModuleConfigData {
    std::string greeting = "Hello, Extra2D!";
    int repeatCount = 1;
    bool enableLogging = true;
};

/**
 * @brief Hello模块配置类
 * 
 * 这是一个简单的自定义模块示例，展示如何：
 * 1. 定义模块配置数据结构
 * 2. 实现IModuleConfig接口
 * 3. 支持JSON配置加载/保存
 */
class HelloModuleConfig : public IModuleConfig {
public:
    HelloModuleConfigData config;

    /**
     * @brief 获取模块信息
     */
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
        info.name = "HelloModule";
        info.version = "1.0.0";
        info.priority = ModulePriority::User;
        info.enabled = true;
        return info;
    }

    /**
     * @brief 获取配置节名称
     */
    std::string getConfigSectionName() const override {
        return "hello";
    }

    /**
     * @brief 验证配置有效性
     */
    bool validate() const override {
        return !config.greeting.empty() && config.repeatCount > 0;
    }

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults() override {
        config = HelloModuleConfigData{};
    }

    /**
     * @brief 应用平台约束
     */
    void applyPlatformConstraints(PlatformType platform) override {
        (void)platform;
    }

    /**
     * @brief 从JSON加载配置
     */
    bool loadFromJson(const void* jsonData) override;

    /**
     * @brief 保存配置到JSON
     */
    bool saveToJson(void* jsonData) const override;
};

/**
 * @brief Hello模块初始化器
 * 
 * 负责模块的生命周期管理
 */
class HelloModuleInitializer : public IModuleInitializer {
public:
    HelloModuleInitializer();
    ~HelloModuleInitializer() override;

    /**
     * @brief 获取模块标识符
     */
    ModuleId getModuleId() const override { return moduleId_; }

    /**
     * @brief 获取模块优先级
     */
    ModulePriority getPriority() const override { return ModulePriority::User; }

    /**
     * @brief 获取模块依赖列表
     */
    std::vector<ModuleId> getDependencies() const override;

    /**
     * @brief 初始化模块
     */
    bool initialize(const IModuleConfig* config) override;

    /**
     * @brief 关闭模块
     */
    void shutdown() override;

    /**
     * @brief 检查是否已初始化
     */
    bool isInitialized() const override { return initialized_; }

    /**
     * @brief 设置模块标识符
     */
    void setModuleId(ModuleId id) { moduleId_ = id; }

    /**
     * @brief 执行问候操作
     */
    void sayHello() const;

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
    HelloModuleConfigData config_;
};

/**
 * @brief 获取Hello模块标识符
 */
ModuleId get_hello_module_id();

/**
 * @brief 注册Hello模块
 */
void register_hello_module();

} // namespace extra2d
