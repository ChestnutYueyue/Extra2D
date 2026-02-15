#pragma once

#include <extra2d/config/module_config.h>
#include <functional>
#include <vector>

namespace extra2d {

/**
 * @brief 模块初始化器接口
 * 所有模块初始化器必须实现此接口
 */
class IModuleInitializer {
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IModuleInitializer() = default;
    
    /**
     * @brief 获取模块标识符
     * @return 模块唯一标识符
     */
    virtual ModuleId getModuleId() const = 0;
    
    /**
     * @brief 获取模块优先级
     * @return 模块优先级
     */
    virtual ModulePriority getPriority() const = 0;
    
    /**
     * @brief 获取模块依赖列表
     * 返回此模块依赖的其他模块标识符
     * @return 依赖模块标识符列表
     */
    virtual std::vector<ModuleId> getDependencies() const { return {}; }
    
    /**
     * @brief 初始化模块
     * @param config 模块配置指针
     * @return 初始化成功返回 true
     */
    virtual bool initialize(const IModuleConfig* config) = 0;
    
    /**
     * @brief 关闭模块
     */
    virtual void shutdown() = 0;
    
    /**
     * @brief 检查模块是否已初始化
     * @return 已初始化返回 true
     */
    virtual bool isInitialized() const = 0;
};

/**
 * @brief 模块初始化器工厂函数类型
 */
using ModuleInitializerFactory = std::function<UniquePtr<IModuleInitializer>()>;

} // namespace extra2d
