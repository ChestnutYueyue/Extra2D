#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace extra2d {

/**
 * @brief 模块注册表条目结构体
 * 存储模块的配置和初始化器工厂
 */
struct ModuleEntry {
    ModuleId id;                                ///< 模块标识符
    UniquePtr<IModuleConfig> config;            ///< 模块配置
    ModuleInitializerFactory initializerFactory;///< 初始化器工厂函数
    bool initialized = false;                   ///< 是否已初始化
};

/**
 * @brief 模块注册表类
 * 单例模式，管理所有模块的注册、查询和初始化顺序
 */
class ModuleRegistry {
public:
    /**
     * @brief 获取单例实例
     * @return 模块注册表实例引用
     */
    static ModuleRegistry& instance();
    
    /**
     * @brief 禁止拷贝构造
     */
    ModuleRegistry(const ModuleRegistry&) = delete;
    
    /**
     * @brief 禁止赋值操作
     */
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;
    
    /**
     * @brief 注册模块
     * @param config 模块配置
     * @param initializerFactory 初始化器工厂函数（可选）
     * @return 分配的模块标识符
     */
    ModuleId registerModule(
        UniquePtr<IModuleConfig> config,
        ModuleInitializerFactory initializerFactory = nullptr
    );
    
    /**
     * @brief 注销模块
     * @param id 模块标识符
     * @return 注销成功返回 true
     */
    bool unregisterModule(ModuleId id);
    
    /**
     * @brief 获取模块配置
     * @param id 模块标识符
     * @return 模块配置指针，不存在返回 nullptr
     */
    IModuleConfig* getModuleConfig(ModuleId id) const;
    
    /**
     * @brief 根据名称获取模块配置
     * @param name 模块名称
     * @return 模块配置指针，不存在返回 nullptr
     */
    IModuleConfig* getModuleConfigByName(const std::string& name) const;
    
    /**
     * @brief 创建模块初始化器
     * @param id 模块标识符
     * @return 初始化器实例，不存在返回 nullptr
     */
    UniquePtr<IModuleInitializer> createInitializer(ModuleId id) const;
    
    /**
     * @brief 获取所有已注册模块标识符
     * @return 模块标识符列表
     */
    std::vector<ModuleId> getAllModules() const;
    
    /**
     * @brief 获取模块初始化顺序
     * 根据优先级和依赖关系计算初始化顺序
     * @return 按初始化顺序排列的模块标识符列表
     */
    std::vector<ModuleId> getInitializationOrder() const;
    
    /**
     * @brief 检查模块是否存在
     * @param id 模块标识符
     * @return 存在返回 true
     */
    bool hasModule(ModuleId id) const;
    
    /**
     * @brief 清空所有注册的模块
     */
    void clear();
    
    /**
     * @brief 获取已注册模块数量
     * @return 模块数量
     */
    size_t size() const { return modules_.size(); }
    
private:
    /**
     * @brief 私有构造函数
     */
    ModuleRegistry() = default;
    
    /**
     * @brief 私有析构函数
     */
    ~ModuleRegistry() = default;
    
    /**
     * @brief 生成新的模块标识符
     * @return 新的模块标识符
     */
    ModuleId generateId();
    
    std::unordered_map<ModuleId, ModuleEntry> modules_;  ///< 模块注册表
    std::unordered_map<std::string, ModuleId> nameToId_; ///< 名称到标识符映射
    mutable std::mutex mutex_;                           ///< 线程安全互斥锁
    ModuleId nextId_ = 1;                                ///< 下一个可用标识符
};

/**
 * @brief 模块注册宏
 * 在全局作用域使用此宏注册模块
 * 
 * @example
 * E2D_REGISTER_MODULE(MyModuleConfig, MyModuleInitializer)
 */
#define E2D_REGISTER_MODULE(ConfigClass, InitializerClass) \
    namespace { \
        static const ::extra2d::ModuleId E2D_ANONYMOUS_VAR(module_id_) = \
            ::extra2d::ModuleRegistry::instance().registerModule( \
                ::extra2d::makeUnique<ConfigClass>(), \
                []() -> ::extra2d::UniquePtr<::extra2d::IModuleInitializer> { \
                    return ::extra2d::makeUnique<InitializerClass>(); \
                } \
            ); \
    }

} // namespace extra2d
