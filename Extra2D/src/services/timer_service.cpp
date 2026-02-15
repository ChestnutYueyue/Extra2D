#include <extra2d/services/timer_service.h>

namespace extra2d {

TimerService::TimerService() {
    info_.name = "TimerService";
    info_.priority = ServicePriority::Timer;
    info_.enabled = true;
}

ServiceInfo TimerService::getServiceInfo() const {
    return info_;
}

bool TimerService::initialize() {
    setState(ServiceState::Running);
    return true;
}

void TimerService::shutdown() {
    manager_.clear();
    setState(ServiceState::Stopped);
}

void TimerService::update(float deltaTime) {
    if (getState() == ServiceState::Running) {
        manager_.update(deltaTime);
    }
}

uint32 TimerService::addTimer(float delay, Timer::Callback callback) {
    return manager_.addTimer(delay, callback);
}

uint32 TimerService::addRepeatingTimer(float interval, Timer::Callback callback) {
    return manager_.addRepeatingTimer(interval, callback);
}

void TimerService::cancelTimer(uint32 timerId) {
    manager_.cancelTimer(timerId);
}

void TimerService::pauseTimer(uint32 timerId) {
    manager_.pauseTimer(timerId);
}

void TimerService::resumeTimer(uint32 timerId) {
    manager_.resumeTimer(timerId);
}

void TimerService::clear() {
    manager_.clear();
}

size_t TimerService::getTimerCount() const {
    return manager_.getTimerCount();
}

} 
