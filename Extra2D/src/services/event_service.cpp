#include <extra2d/services/event_service.h>

namespace extra2d {

EventService::EventService() {
    info_.name = "EventService";
    info_.priority = ServicePriority::Event;
    info_.enabled = true;
}

ServiceInfo EventService::getServiceInfo() const {
    return info_;
}

bool EventService::initialize() {
    setState(ServiceState::Running);
    return true;
}

void EventService::shutdown() {
    queue_.clear();
    dispatcher_.removeAllListeners();
    setState(ServiceState::Stopped);
}

void EventService::update(float deltaTime) {
    if (getState() == ServiceState::Running) {
        processQueue();
    }
}

void EventService::pushEvent(const Event& event) {
    queue_.push(event);
}

void EventService::pushEvent(Event&& event) {
    queue_.push(std::move(event));
}

bool EventService::pollEvent(Event& event) {
    return queue_.poll(event);
}

ListenerId EventService::addListener(EventType type, EventDispatcher::EventCallback callback) {
    return dispatcher_.addListener(type, callback);
}

void EventService::removeListener(ListenerId id) {
    dispatcher_.removeListener(id);
}

void EventService::removeAllListeners(EventType type) {
    dispatcher_.removeAllListeners(type);
}

void EventService::removeAllListeners() {
    dispatcher_.removeAllListeners();
}

void EventService::dispatch(Event& event) {
    dispatcher_.dispatch(event);
}

void EventService::processQueue() {
    dispatcher_.processQueue(queue_);
}

size_t EventService::getListenerCount(EventType type) const {
    return dispatcher_.getListenerCount(type);
}

size_t EventService::getTotalListenerCount() const {
    return dispatcher_.getTotalListenerCount();
}

size_t EventService::getQueueSize() const {
    return queue_.size();
}

} 
