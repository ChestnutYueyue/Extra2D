#pragma once

#include <extra2d/core/service_interface.h>
#include <extra2d/core/service_locator.h>
#include <functional>
#include <vector>
#include <string>

namespace extra2d {

/**
 * @brief 服务注册信息
 */
struct ServiceRegistration {
    std::string name;                   
    ServicePriority priority;           
    std::function<SharedPtr<IService>()> factory; 
    bool enabled = true;                
};

/**
 * @brief 服务注册表
 * 管理服务的注册信息，支持延迟创建和配置
 */
class ServiceRegistry {
public:
    /**
     * @brief 获取单例实例
     * @return 服务注册表实例引用
     */
    static ServiceRegistry& instance();

    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    /**
     * @brief 注册服务
     * @tparam T 服务接口类型
     * @tparam Impl 服务实现类型
     * @param name 服务名称
     * @param priority 服务优先级
     */
    template<typename T, typename Impl>
    void registerService(const std::string& name, ServicePriority priority) {
        static_assert(std::is_base_of_v<IService, T>, 
                      "T must derive from IService");
        static_assert(std::is_base_of_v<T, Impl>, 
                      "Impl must derive from T");

        ServiceRegistration reg;
        reg.name = name;
        reg.priority = priority;
        reg.factory = []() -> SharedPtr<IService> {
            return std::static_pointer_cast<IService>(makeShared<Impl>());
        };
        registrations_.push_back(reg);
    }

    /**
     * @brief 注册服务（带工厂函数）
     * @tparam T 服务接口类型
     * @param name 服务名称
     * @param priority 服务优先级
     * @param factory 工厂函数
     */
    template<typename T>
    void registerServiceWithFactory(
        const std::string& name, 
        ServicePriority priority,
        std::function<SharedPtr<T>()> factory) {
        static_assert(std::is_base_of_v<IService, T>, 
                      "T must derive from IService");

        ServiceRegistration reg;
        reg.name = name;
        reg.priority = priority;
        reg.factory = [factory]() -> SharedPtr<IService> {
            return std::static_pointer_cast<IService>(factory());
        };
        registrations_.push_back(reg);
    }

    /**
     * @brief 启用/禁用服务
     * @param name 服务名称
     * @param enabled 是否启用
     */
    void setServiceEnabled(const std::string& name, bool enabled);

    /**
     * @brief 创建所有已注册的服务
     * 并注册到 ServiceLocator
     */
    void createAllServices();

    /**
     * @brief 获取所有注册信息
     * @return 注册信息列表
     */
    const std::vector<ServiceRegistration>& getRegistrations() const {
        return registrations_;
    }

    /**
     * @brief 清空所有注册
     */
    void clear() {
        registrations_.clear();
    }

private:
    ServiceRegistry() = default;
    ~ServiceRegistry() = default;

    std::vector<ServiceRegistration> registrations_;
};

/**
 * @brief 自动服务注册器
 * 在全局作用域使用，自动注册服务
 */
template<typename Interface, typename Implementation>
class AutoServiceRegistrar {
public:
    AutoServiceRegistrar(const std::string& name, ServicePriority priority) {
        ServiceRegistry::instance().registerService<Interface, Implementation>(
            name, priority);
    }
};

} 

#define E2D_REGISTER_SERVICE_AUTO(Interface, Implementation, Name, Priority) \
    namespace { \
        static ::extra2d::AutoServiceRegistrar<Interface, Implementation> \
            E2D_CONCAT(auto_service_registrar_, __LINE__)(Name, Priority); \
    }
