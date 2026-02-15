#pragma once

#include <extra2d/core/service_interface.h>
#include <extra2d/utils/timer.h>

namespace extra2d {

/**
 * @brief 计时器服务接口
 */
class ITimerService : public IService {
public:
    virtual ~ITimerService() = default;

    virtual uint32 addTimer(float delay, Timer::Callback callback) = 0;
    virtual uint32 addRepeatingTimer(float interval, Timer::Callback callback) = 0;
    virtual void cancelTimer(uint32 timerId) = 0;
    virtual void pauseTimer(uint32 timerId) = 0;
    virtual void resumeTimer(uint32 timerId) = 0;
    virtual void clear() = 0;
    virtual size_t getTimerCount() const = 0;
};

/**
 * @brief 计时器服务实现
 */
class TimerService : public ITimerService {
public:
    TimerService();
    ~TimerService() override = default;

    ServiceInfo getServiceInfo() const override;

    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;

    uint32 addTimer(float delay, Timer::Callback callback) override;
    uint32 addRepeatingTimer(float interval, Timer::Callback callback) override;
    void cancelTimer(uint32 timerId) override;
    void pauseTimer(uint32 timerId) override;
    void resumeTimer(uint32 timerId) override;
    void clear() override;
    size_t getTimerCount() const override;

    TimerManager& getManager() { return manager_; }
    const TimerManager& getManager() const { return manager_; }

private:
    TimerManager manager_;
};

} 
