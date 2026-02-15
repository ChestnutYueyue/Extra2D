#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/platform/window_config.h>
#include <extra2d/platform/iwindow.h>
#include <string>

namespace extra2d {

/**
 * @file window_module.h
 * @brief 窗口模块
 * 
 * 窗口模块使用 SDL2 作为唯一后端，支持以下平台：
 * - Windows
 * - Linux
 * - macOS
 * - Nintendo Switch
 */

/**
 * @brief 窗口模块配置
 * 实现 IModuleConfig 接口
 */
class WindowModuleConfig : public IModuleConfig {
public:
    WindowConfigData windowConfig;

    /**
     * @brief 获取模块信息
     * @return 模块信息结构体
     */
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
        info.name = "Window";
        info.version = "1.0.0";
        info.priority = ModulePriority::Core;
        info.enabled = true;
        return info;
    }

    /**
     * @brief 获取配置节名称
     * @return 配置节名称字符串
     */
    std::string getConfigSectionName() const override {
        return "window";
    }

    /**
     * @brief 验证配置有效性
     * @return 如果配置有效返回 true
     */
    bool validate() const override {
        return windowConfig.width > 0 && windowConfig.height > 0;
    }

    /**
     * @brief 应用平台约束
     * @param platform 目标平台类型
     */
    void applyPlatformConstraints(PlatformType platform) override;

    /**
     * @brief 重置为默认配置
     */
    void resetToDefaults() override {
        windowConfig = WindowConfigData{};
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
 * @brief 窗口模块初始化器
 * 实现 IModuleInitializer 接口
 */
class WindowModuleInitializer : public IModuleInitializer {
public:
    /**
     * @brief 构造函数
     */
    WindowModuleInitializer();

    /**
     * @brief 析构函数
     */
    ~WindowModuleInitializer() override;

    /**
     * @brief 获取模块标识符
     * @return 模块唯一标识符
     */
    ModuleId getModuleId() const override { return moduleId_; }

    /**
     * @brief 获取模块优先级
     * @return 模块优先级
     */
    ModulePriority getPriority() const override { return ModulePriority::Core; }

    /**
     * @brief 获取模块依赖列表
     * @return 依赖模块标识符列表（窗口模块无依赖）
     */
    std::vector<ModuleId> getDependencies() const override { return {}; }

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
     * @brief 设置模块标识符
     * @param id 模块标识符
     */
    void setModuleId(ModuleId id) { moduleId_ = id; }

    /**
     * @brief 设置窗口配置
     * @param config 窗口配置数据
     */
    void setWindowConfig(const WindowConfigData& config) { windowConfig_ = config; }

    /**
     * @brief 获取窗口接口
     * @return 窗口接口指针
     */
    IWindow* getWindow() const { return window_.get(); }

private:
    /**
     * @brief 初始化 SDL2 后端
     * @return 初始化成功返回 true
     */
    bool initSDL2();

    /**
     * @brief 关闭 SDL2 后端
     */
    void shutdownSDL2();

    /**
     * @brief 创建窗口
     * @param config 窗口配置数据
     * @return 创建成功返回 true
     */
    bool createWindow(const WindowConfigData& config);

    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
    bool sdl2Initialized_ = false;
    WindowConfigData windowConfig_;
    UniquePtr<IWindow> window_;
};

/**
 * @brief 获取窗口模块标识符
 * @return 窗口模块标识符
 */
ModuleId get_window_module_id();

/**
 * @brief 注册窗口模块
 */
void register_window_module();

} 
