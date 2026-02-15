#pragma once

#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

/**
 * @brief 服务优先级枚举
 * 定义服务的初始化顺序，数值越小越先初始化
 */
enum class ServicePriority : int {
    Core = 0,           
    Event = 100,        
    Timer = 200,        
    Scene = 300,        
    Camera = 400,       
    Resource = 500,     
    Audio = 600,        
    User = 1000         
};

/**
 * @brief 服务状态枚举
 */
enum class ServiceState {
    Uninitialized,      
    Initializing,       
    Running,            
    Paused,             
    Stopping,           
    Stopped             
};

/**
 * @brief 服务信息结构体
 */
struct ServiceInfo {
    std::string name;                   
    ServicePriority priority = ServicePriority::User; 
    ServiceState state = ServiceState::Uninitialized; 
    bool enabled = true;                
};

/**
 * @brief 服务接口基类
 * 所有服务必须实现此接口，支持依赖注入和生命周期管理
 */
class IService {
    friend class ServiceLocator;

public:
    virtual ~IService() = default;

    /**
     * @brief 获取服务信息
     * @return 服务信息结构体
     */
    virtual ServiceInfo getServiceInfo() const = 0;

    /**
     * @brief 初始化服务
     * @return 初始化成功返回 true
     */
    virtual bool initialize() = 0;

    /**
     * @brief 关闭服务
     */
    virtual void shutdown() = 0;

    /**
     * @brief 暂停服务
     */
    virtual void pause() { 
        info_.state = ServiceState::Paused; 
    }

    /**
     * @brief 恢复服务
     */
    virtual void resume() { 
        if (info_.state == ServiceState::Paused) {
            info_.state = ServiceState::Running;
        }
    }

    /**
     * @brief 更新服务
     * @param deltaTime 帧间隔时间
     */
    virtual void update(float deltaTime) { }

    /**
     * @brief 检查服务是否已初始化
     * @return 已初始化返回 true
     */
    virtual bool isInitialized() const { 
        return info_.state == ServiceState::Running || 
               info_.state == ServiceState::Paused; 
    }

    /**
     * @brief 获取服务状态
     * @return 当前服务状态
     */
    ServiceState getState() const { return info_.state; }

    /**
     * @brief 获取服务名称
     * @return 服务名称
     */
    const std::string& getName() const { return info_.name; }

protected:
    ServiceInfo info_;  

    /**
     * @brief 设置服务状态
     * @param state 新状态
     */
    void setState(ServiceState state) { info_.state = state; }
};

/**
 * @brief 类型ID生成器
 * 用于为每种服务类型生成唯一ID
 */
using ServiceTypeId = size_t;

namespace detail {
    inline ServiceTypeId nextServiceTypeId() {
        static ServiceTypeId id = 0;
        return ++id;
    }

    template<typename T>
    ServiceTypeId getServiceTypeId() {
        static ServiceTypeId id = nextServiceTypeId();
        return id;
    }
}

} 
