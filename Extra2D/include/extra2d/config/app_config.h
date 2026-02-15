#pragma once

#include <extra2d/config/platform_config.h>
#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

/**
 * @file app_config.h
 * @brief 应用级别配置
 * 
 * 本文件仅包含应用级别的配置项，不包含任何模块特定配置。
 * 各模块应该在自己的模块文件中定义配置结构，并实现 IModuleConfig 接口。
 * 
 * 模块配置通过 ModuleRegistry 注册，由 ConfigManager 统一管理。
 * 这种设计遵循开闭原则，新增模块无需修改引擎核心代码。
 */

/**
 * @brief 应用配置结构体
 * 仅包含应用级别的配置项，模块配置由各模块自行管理
 */
struct AppConfig {
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
};

} 
