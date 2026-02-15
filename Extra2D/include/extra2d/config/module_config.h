#pragma once

#include <extra2d/config/platform_config.h>
#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

/**
 * @brief 模块标识符类型
 */
using ModuleId = uint32_t;

/**
 * @brief 无效模块标识符常量
 */
constexpr ModuleId INVALID_MODULE_ID = 0;

/**
 * @brief 模块优先级枚举
 * 定义模块的初始化顺序，数值越小越先初始化
 */
enum class ModulePriority : int {
    Core = 0,           ///< 核心模块（最先初始化）
    Platform = 100,     ///< 平台相关模块
    Graphics = 200,     ///< 图形渲染模块
    Audio = 300,        ///< 音频模块
    Input = 400,        ///< 输入模块
    Resource = 500,     ///< 资源管理模块
    Game = 1000,        ///< 游戏逻辑模块
    User = 2000         ///< 用户自定义模块
};

/**
 * @brief 模块信息结构体
 * 包含模块的基本信息
 */
struct ModuleInfo {
    ModuleId id = INVALID_MODULE_ID;            ///< 模块唯一标识符
    std::string name;                           ///< 模块名称
    std::string version;                        ///< 模块版本号
    ModulePriority priority = ModulePriority::User; ///< 模块优先级
    bool enabled = true;                        ///< 是否启用
};

/**
 * @brief 模块配置接口
 * 所有模块配置类必须实现此接口
 */
class IModuleConfig {
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IModuleConfig() = default;
    
    /**
     * @brief 获取模块信息
     * @return 模块信息结构体
     */
    virtual ModuleInfo getModuleInfo() const = 0;
    
    /**
     * @brief 获取配置节名称
     * 用于配置文件中的节名
     * @return 配置节名称字符串
     */
    virtual std::string getConfigSectionName() const = 0;
    
    /**
     * @brief 验证配置有效性
     * @return 如果配置有效返回 true
     */
    virtual bool validate() const { return true; }
    
    /**
     * @brief 应用平台约束
     * 根据平台特性调整配置
     * @param platform 目标平台类型
     */
    virtual void applyPlatformConstraints(PlatformType platform) { }
    
    /**
     * @brief 重置为默认配置
     */
    virtual void resetToDefaults() = 0;
    
    /**
     * @brief 从 JSON 数据加载配置
     * @param jsonData JSON 数据指针
     * @return 加载成功返回 true
     */
    virtual bool loadFromJson(const void* jsonData) { return true; }
    
    /**
     * @brief 保存配置到 JSON 数据
     * @param jsonData JSON 数据指针
     * @return 保存成功返回 true
     */
    virtual bool saveToJson(void* jsonData) const { return true; }
};

} // namespace extra2d
