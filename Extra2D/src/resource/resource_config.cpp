#include <extra2d/resource/resource_config.h>
#include <algorithm>

namespace extra2d {

void ResourceConfigData::addSearchPath(const std::string& path) {
    if (!hasSearchPath(path)) {
        searchPaths.push_back(path);
    }
}

void ResourceConfigData::removeSearchPath(const std::string& path) {
    auto it = std::find(searchPaths.begin(), searchPaths.end(), path);
    if (it != searchPaths.end()) {
        searchPaths.erase(it);
    }
}

bool ResourceConfigData::hasSearchPath(const std::string& path) const {
    return std::find(searchPaths.begin(), searchPaths.end(), path) != searchPaths.end();
}

} 
