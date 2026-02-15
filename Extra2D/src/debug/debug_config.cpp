#include <extra2d/debug/debug_config.h>
#include <algorithm>

namespace extra2d {

bool DebugConfigData::hasDebugFlag(const std::string& flag) const {
    return std::find(debugFlags.begin(), debugFlags.end(), flag) != debugFlags.end();
}

void DebugConfigData::addDebugFlag(const std::string& flag) {
    if (!hasDebugFlag(flag)) {
        debugFlags.push_back(flag);
    }
}

void DebugConfigData::removeDebugFlag(const std::string& flag) {
    auto it = std::find(debugFlags.begin(), debugFlags.end(), flag);
    if (it != debugFlags.end()) {
        debugFlags.erase(it);
    }
}

} 
