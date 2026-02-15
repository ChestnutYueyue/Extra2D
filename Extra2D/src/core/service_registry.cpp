#include <extra2d/core/service_registry.h>

namespace extra2d {

ServiceRegistry& ServiceRegistry::instance() {
    static ServiceRegistry instance;
    return instance;
}

void ServiceRegistry::setServiceEnabled(const std::string& name, bool enabled) {
    for (auto& reg : registrations_) {
        if (reg.name == name) {
            reg.enabled = enabled;
            break;
        }
    }
}

void ServiceRegistry::createAllServices() {
    std::sort(registrations_.begin(), registrations_.end(),
        [](const ServiceRegistration& a, const ServiceRegistration& b) {
            return static_cast<int>(a.priority) < static_cast<int>(b.priority);
        });

    for (const auto& reg : registrations_) {
        if (!reg.enabled) {
            continue;
        }

        auto service = reg.factory();
        if (service) {
            ServiceLocator::instance().registerService<IService>(service);
        }
    }
}

} 
