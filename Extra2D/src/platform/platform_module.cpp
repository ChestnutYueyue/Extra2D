#include <extra2d/platform/platform_module.h>

namespace extra2d {

std::unordered_map<std::string, BackendFactory::BackendEntry>& BackendFactory::registry() {
    static std::unordered_map<std::string, BackendEntry> reg;
    return reg;
}

void BackendFactory::reg(const std::string& name, WindowFn win, InputFn in) {
    registry()[name] = {win, in};
}

UniquePtr<IWindow> BackendFactory::createWindow(const std::string& name) {
    auto& reg = registry();
    auto it = reg.find(name);
    if (it != reg.end() && it->second.windowFn) {
        return it->second.windowFn();
    }
    return nullptr;
}

UniquePtr<IInput> BackendFactory::createInput(const std::string& name) {
    auto& reg = registry();
    auto it = reg.find(name);
    if (it != reg.end() && it->second.inputFn) {
        return it->second.inputFn();
    }
    return nullptr;
}

std::vector<std::string> BackendFactory::backends() {
    std::vector<std::string> result;
    for (const auto& pair : registry()) {
        result.push_back(pair.first);
    }
    return result;
}

bool BackendFactory::has(const std::string& name) {
    return registry().find(name) != registry().end();
}

} // namespace extra2d
