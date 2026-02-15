#include <extra2d/graphics/shader_cache.h>
#include <extra2d/utils/logger.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace extra2d {

namespace fs = std::filesystem;

/**
 * @brief 获取单例实例
 * @return 缓存管理器实例引用
 */
ShaderCache& ShaderCache::getInstance() {
    static ShaderCache instance;
    return instance;
}

/**
 * @brief 初始化缓存系统
 * @param cacheDir 缓存目录路径
 * @return 初始化成功返回true，失败返回false
 */
bool ShaderCache::init(const std::string& cacheDir) {
    cacheDir_ = cacheDir;

    if (!ensureCacheDirectory()) {
        E2D_LOG_ERROR("Failed to create cache directory: {}", cacheDir);
        return false;
    }

    if (!loadCacheIndex()) {
        E2D_LOG_WARN("Failed to load cache index, starting fresh");
    }

    initialized_ = true;
    E2D_LOG_INFO("Shader cache initialized at: {}", cacheDir);
    return true;
}

/**
 * @brief 关闭缓存系统
 */
void ShaderCache::shutdown() {
    if (!initialized_) {
        return;
    }

    saveCacheIndex();
    cacheMap_.clear();
    initialized_ = false;
    E2D_LOG_INFO("Shader cache shutdown");
}

/**
 * @brief 检查缓存是否有效
 * @param name Shader名称
 * @param sourceHash 源码哈希值
 * @return 缓存有效返回true，否则返回false
 */
bool ShaderCache::hasValidCache(const std::string& name, const std::string& sourceHash) {
    auto it = cacheMap_.find(name);
    if (it == cacheMap_.end()) {
        return false;
    }

    return it->second.sourceHash == sourceHash;
}

/**
 * @brief 加载缓存的二进制数据
 * @param name Shader名称
 * @return 缓存条目指针，不存在返回nullptr
 */
Ptr<ShaderCacheEntry> ShaderCache::loadCache(const std::string& name) {
    auto it = cacheMap_.find(name);
    if (it == cacheMap_.end()) {
        return nullptr;
    }

    std::string cachePath = getCachePath(name);
    std::ifstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        E2D_LOG_WARN("Failed to open cache file: {}", cachePath);
        return nullptr;
    }

    auto entry = std::make_shared<ShaderCacheEntry>(it->second);
    entry->binary.clear();

    file.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    entry->binary.resize(fileSize);
    file.read(reinterpret_cast<char*>(entry->binary.data()), fileSize);

    return entry;
}

/**
 * @brief 保存编译结果到缓存
 * @param entry 缓存条目
 * @return 保存成功返回true，失败返回false
 */
bool ShaderCache::saveCache(const ShaderCacheEntry& entry) {
    if (!initialized_) {
        return false;
    }

    std::string cachePath = getCachePath(entry.name);
    std::ofstream file(cachePath, std::ios::binary);
    if (!file.is_open()) {
        E2D_LOG_ERROR("Failed to create cache file: {}", cachePath);
        return false;
    }

    file.write(reinterpret_cast<const char*>(entry.binary.data()), entry.binary.size());
    file.close();

    cacheMap_[entry.name] = entry;
    saveCacheIndex();

    E2D_LOG_DEBUG("Shader cache saved: {}", entry.name);
    return true;
}

/**
 * @brief 使缓存失效
 * @param name Shader名称
 */
void ShaderCache::invalidate(const std::string& name) {
    auto it = cacheMap_.find(name);
    if (it == cacheMap_.end()) {
        return;
    }

    std::string cachePath = getCachePath(name);
    fs::remove(cachePath);

    cacheMap_.erase(it);
    saveCacheIndex();

    E2D_LOG_DEBUG("Shader cache invalidated: {}", name);
}

/**
 * @brief 清除所有缓存
 */
void ShaderCache::clearAll() {
    for (const auto& pair : cacheMap_) {
        std::string cachePath = getCachePath(pair.first);
        fs::remove(cachePath);
    }

    cacheMap_.clear();
    saveCacheIndex();

    E2D_LOG_INFO("All shader caches cleared");
}

/**
 * @brief 计算源码哈希值
 * @param vertSource 顶点着色器源码
 * @param fragSource 片段着色器源码
 * @return 哈希值字符串
 */
std::string ShaderCache::computeHash(const std::string& vertSource,
                                     const std::string& fragSource) {
    std::string combined = vertSource + fragSource;

    uint32_t hash = 5381;
    for (char c : combined) {
        hash = ((hash << 5) + hash) + static_cast<uint32_t>(c);
    }

    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

/**
 * @brief 加载缓存索引
 * @return 加载成功返回true，失败返回false
 */
bool ShaderCache::loadCacheIndex() {
    std::string indexPath = cacheDir_ + "/.cache_index";
    
    if (!fs::exists(indexPath)) {
        return true;
    }

    std::ifstream file(indexPath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string name = line.substr(0, pos);
        std::string hash = line.substr(pos + 1);

        std::string cachePath = getCachePath(name);
        if (fs::exists(cachePath)) {
            ShaderCacheEntry entry;
            entry.name = name;
            entry.sourceHash = hash;
            entry.compileTime = static_cast<uint64_t>(
                std::chrono::system_clock::now().time_since_epoch().count());
            cacheMap_[name] = entry;
        }
    }

    return true;
}

/**
 * @brief 保存缓存索引
 * @return 保存成功返回true，失败返回false
 */
bool ShaderCache::saveCacheIndex() {
    std::string indexPath = cacheDir_ + "/.cache_index";

    std::ofstream file(indexPath);
    if (!file.is_open()) {
        return false;
    }

    file << "# Extra2D Shader Cache Index\n";
    file << "# Format: name=hash\n";

    for (const auto& pair : cacheMap_) {
        file << pair.first << "=" << pair.second.sourceHash << "\n";
    }

    return true;
}

/**
 * @brief 获取缓存文件路径
 * @param name Shader名称
 * @return 缓存文件完整路径
 */
std::string ShaderCache::getCachePath(const std::string& name) const {
    return cacheDir_ + "/" + name + ".cache";
}

/**
 * @brief 确保缓存目录存在
 * @return 目录存在或创建成功返回true，否则返回false
 */
bool ShaderCache::ensureCacheDirectory() {
    if (cacheDir_.empty()) {
        return false;
    }

    std::error_code ec;
    if (!fs::exists(cacheDir_)) {
        if (!fs::create_directories(cacheDir_, ec)) {
            return false;
        }
    }

    return true;
}

} // namespace extra2d
