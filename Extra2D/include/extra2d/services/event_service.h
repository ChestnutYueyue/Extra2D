#pragma once

#include <extra2d/core/service_interface.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>

namespace extra2d {

/**
 * @brief 事件服务接口
 */
class IEventService : public IService {
public:
    virtual ~IEventService() = default;

    virtual void pushEvent(const Event& event) = 0;
    virtual void pushEvent(Event&& event) = 0;
    virtual bool pollEvent(Event& event) = 0;

    virtual ListenerId addListener(EventType type, EventDispatcher::EventCallback callback) = 0;
    virtual void removeListener(ListenerId id) = 0;
    virtual void removeAllListeners(EventType type) = 0;
    virtual void removeAllListeners() = 0;

    virtual void dispatch(Event& event) = 0;
    virtual void processQueue() = 0;

    virtual size_t getListenerCount(EventType type) const = 0;
    virtual size_t getTotalListenerCount() const = 0;
    virtual size_t getQueueSize() const = 0;
};

/**
 * @brief 事件服务实现
 */
class EventService : public IEventService {
public:
    EventService();
    ~EventService() override = default;

    ServiceInfo getServiceInfo() const override;

    bool initialize() override;
    void shutdown() override;
    void update(float deltaTime) override;

    void pushEvent(const Event& event) override;
    void pushEvent(Event&& event) override;
    bool pollEvent(Event& event) override;

    ListenerId addListener(EventType type, EventDispatcher::EventCallback callback) override;
    void removeListener(ListenerId id) override;
    void removeAllListeners(EventType type) override;
    void removeAllListeners() override;

    void dispatch(Event& event) override;
    void processQueue() override;

    size_t getListenerCount(EventType type) const override;
    size_t getTotalListenerCount() const override;
    size_t getQueueSize() const override;

    EventQueue& getQueue() { return queue_; }
    const EventQueue& getQueue() const { return queue_; }
    EventDispatcher& getDispatcher() { return dispatcher_; }
    const EventDispatcher& getDispatcher() const { return dispatcher_; }

private:
    EventQueue queue_;
    EventDispatcher dispatcher_;
};

} 
