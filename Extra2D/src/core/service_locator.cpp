#include <extra2d/core/service_locator.h>
#include <algorithm>

namespace extra2d {

ServiceLocator& ServiceLocator::instance() {
    static ServiceLocator instance;
    return instance;
}

bool ServiceLocator::initializeAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& service : orderedServices_) {
        if (!service) continue;
        
        auto info = service->getServiceInfo();
        if (!info.enabled) continue;
        
        if (!service->isInitialized()) {
            service->setState(ServiceState::Initializing);
            if (!service->initialize()) {
                service->setState(ServiceState::Stopped);
                return false;
            }
            service->setState(ServiceState::Running);
        }
    }
    
    return true;
}

void ServiceLocator::shutdownAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto it = orderedServices_.rbegin(); 
         it != orderedServices_.rend(); ++it) {
        if (*it && (*it)->isInitialized()) {
            (*it)->setState(ServiceState::Stopping);
            (*it)->shutdown();
            (*it)->setState(ServiceState::Stopped);
        }
    }
}

void ServiceLocator::updateAll(float deltaTime) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& service : orderedServices_) {
        if (service && service->isInitialized()) {
            auto state = service->getState();
            if (state == ServiceState::Running) {
                service->update(deltaTime);
            }
        }
    }
}

void ServiceLocator::pauseAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& service : orderedServices_) {
        if (service && service->isInitialized()) {
            service->pause();
        }
    }
}

void ServiceLocator::resumeAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& service : orderedServices_) {
        if (service && service->isInitialized()) {
            service->resume();
        }
    }
}

std::vector<SharedPtr<IService>> ServiceLocator::getAllServices() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return orderedServices_;
}

void ServiceLocator::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto it = orderedServices_.rbegin(); 
         it != orderedServices_.rend(); ++it) {
        if (*it && (*it)->isInitialized()) {
            (*it)->setState(ServiceState::Stopping);
            (*it)->shutdown();
            (*it)->setState(ServiceState::Stopped);
        }
    }
    
    services_.clear();
    factories_.clear();
    orderedServices_.clear();
}

void ServiceLocator::sortServices() {
    std::stable_sort(orderedServices_.begin(), orderedServices_.end(),
        [](const SharedPtr<IService>& a, const SharedPtr<IService>& b) {
            if (!a || !b) return false;
            return static_cast<int>(a->getServiceInfo().priority) < 
                   static_cast<int>(b->getServiceInfo().priority);
        });
}

} 
