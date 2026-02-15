#pragma once

#include <extra2d/core/service_interface.h>
#include <extra2d/core/types.h>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <functional>
#include <typeindex>
#include <memory>
#include <algorithm>

namespace extra2d {

/**
 * @brief 服务工厂函数类型
 */
template<typename T>
using ServiceFactory = std::function<SharedPtr<T>()>;

/**
 * @brief 服务定位器
 * 实现依赖注入和服务发现模式，解耦模块间依赖
 * 
 * 特性：
 * - 类型安全的服务注册和获取
 * - 支持服务工厂延迟创建
 * - 支持服务依赖声明
 * - 线程安全
 * - 支持 Mock 测试
 */
class ServiceLocator {
public:
    /**
     * @brief 获取单例实例
     * @return 服务定位器实例引用
     */
    static ServiceLocator& instance();

    ServiceLocator(const ServiceLocator&) = delete;
    ServiceLocator& operator=(const ServiceLocator&) = delete;

    /**
     * @brief 注册服务实例
     * @tparam T 服务接口类型
     * @param service 服务实例
     */
    template<typename T>
    void registerService(SharedPtr<T> service) {
        static_assert(std::is_base_of_v<IService, T>, 
                      "T must derive from IService");
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeId = std::type_index(typeid(T));
        services_[typeId] = std::static_pointer_cast<IService>(service);
        orderedServices_.push_back(service);
        sortServices();
    }

    /**
     * @brief 注册服务工厂
     * @tparam T 服务接口类型
     * @param factory 服务工厂函数
     */
    template<typename T>
    void registerFactory(ServiceFactory<T> factory) {
        static_assert(std::is_base_of_v<IService, T>, 
                      "T must derive from IService");
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeId = std::type_index(typeid(T));
        factories_[typeId] = [factory]() -> SharedPtr<IService> {
            return std::static_pointer_cast<IService>(factory());
        };
    }

    /**
     * @brief 获取服务实例
     * @tparam T 服务接口类型
     * @return 服务实例，不存在返回 nullptr
     */
    template<typename T>
    SharedPtr<T> getService() const {
        static_assert(std::is_base_of_v<IService, T>, 
                      "T must derive from IService");
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeId = std::type_index(typeid(T));
        
        auto it = services_.find(typeId);
        if (it != services_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        
        auto factoryIt = factories_.find(typeId);
        if (factoryIt != factories_.end()) {
            auto service = factoryIt->second();
            services_[typeId] = service;
            return std::static_pointer_cast<T>(service);
        }
        
        return nullptr;
    }

    /**
     * @brief 尝试获取服务实例（不创建）
     * @tparam T 服务接口类型
     * @return 服务实例，不存在返回 nullptr
     */
    template<typename T>
    SharedPtr<T> tryGetService() const {
        static_assert(std::is_base_of_v<IService, T>, 
                      "T must derive from IService");
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeId = std::type_index(typeid(T));
        auto it = services_.find(typeId);
        if (it != services_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }

    /**
     * @brief 检查服务是否已注册
     * @tparam T 服务接口类型
     * @return 已注册返回 true
     */
    template<typename T>
    bool hasService() const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeId = std::type_index(typeid(T));
        return services_.find(typeId) != services_.end() ||
               factories_.find(typeId) != factories_.end();
    }

    /**
     * @brief 注销服务
     * @tparam T 服务接口类型
     */
    template<typename T>
    void unregisterService() {
        std::lock_guard<std::mutex> lock(mutex_);
        auto typeId = std::type_index(typeid(T));
        
        auto it = services_.find(typeId);
        if (it != services_.end()) {
            auto service = it->second;
            services_.erase(it);
            
            auto orderIt = std::find(orderedServices_.begin(), 
                                     orderedServices_.end(), service);
            if (orderIt != orderedServices_.end()) {
                orderedServices_.erase(orderIt);
            }
        }
        
        factories_.erase(typeId);
    }

    /**
     * @brief 初始化所有已注册的服务
     * @return 所有服务初始化成功返回 true
     */
    bool initializeAll();

    /**
     * @brief 关闭所有服务
     */
    void shutdownAll();

    /**
     * @brief 更新所有服务
     * @param deltaTime 帧间隔时间
     */
    void updateAll(float deltaTime);

    /**
     * @brief 暂停所有服务
     */
    void pauseAll();

    /**
     * @brief 恢复所有服务
     */
    void resumeAll();

    /**
     * @brief 获取所有服务（按优先级排序）
     * @return 服务列表
     */
    std::vector<SharedPtr<IService>> getAllServices() const;

    /**
     * @brief 清空所有服务和工厂
     */
    void clear();

    /**
     * @brief 获取已注册服务数量
     * @return 服务数量
     */
    size_t size() const { return services_.size(); }

private:
    ServiceLocator() = default;
    ~ServiceLocator() = default;

    /**
     * @brief 按优先级排序服务
     */
    void sortServices();

    mutable std::unordered_map<std::type_index, SharedPtr<IService>> services_;
    std::unordered_map<std::type_index, std::function<SharedPtr<IService>()>> factories_;
    std::vector<SharedPtr<IService>> orderedServices_;
    mutable std::mutex mutex_;
};

/**
 * @brief 服务注册器
 * 用于静态注册服务
 */
template<typename Interface, typename Implementation>
class ServiceRegistrar {
public:
    explicit ServiceRegistrar(ServiceFactory<Interface> factory = nullptr) {
        if (factory) {
            ServiceLocator::instance().registerFactory<Interface>(factory);
        } else {
            ServiceLocator::instance().registerFactory<Interface>(
                []() -> SharedPtr<Interface> {
                    return makeShared<Implementation>();
                }
            );
        }
    }
};

} 

#define E2D_REGISTER_SERVICE(Interface, Implementation) \
    namespace { \
        static ::extra2d::ServiceRegistrar<Interface, Implementation> \
            E2D_CONCAT(service_registrar_, __LINE__); \
    }

#define E2D_REGISTER_SERVICE_FACTORY(Interface, Factory) \
    namespace { \
        static ::extra2d::ServiceRegistrar<Interface, Interface> \
            E2D_CONCAT(service_factory_registrar_, __LINE__)(Factory); \
    }
