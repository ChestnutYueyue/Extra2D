#include <extra2d/graphics/texture_pool.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/scene/scene.h>

#include <algorithm>
#include <cstring>

namespace extra2d {

// ============================================================================
// TexturePool 实现
// ============================================================================

/**
 * @brief 默认构造函数
 *
 * 创建一个未初始化的纹理池
 */
TexturePool::TexturePool()
    : scene_(nullptr)
    , maxMemoryUsage_(0)
    , currentMemoryUsage_(0)
    , cacheHits_(0)
    , cacheMisses_(0)
    , evictionCount_(0) {
}

/**
 * @brief 构造函数
 * @param scene 场景指针
 * @param maxMemoryUsage 最大内存使用量（0 表示无限制）
 *
 * 创建一个指定场景和内存限制的纹理池
 */
TexturePool::TexturePool(Scene* scene, size_t maxMemoryUsage)
    : scene_(scene)
    , maxMemoryUsage_(maxMemoryUsage)
    , currentMemoryUsage_(0)
    , cacheHits_(0)
    , cacheMisses_(0)
    , evictionCount_(0) {
    E2D_LOG_INFO("TexturePool created with max memory: {} bytes", maxMemoryUsage);
}

/**
 * @brief 初始化纹理池
 * @param scene 场景指针
 * @param maxMemoryUsage 最大内存使用量（0 表示无限制）
 *
 * 设置纹理池的场景和内存限制
 */
void TexturePool::init(Scene* scene, size_t maxMemoryUsage) {
    scene_ = scene;
    maxMemoryUsage_ = maxMemoryUsage;
    E2D_LOG_INFO("TexturePool initialized with max memory: {} bytes", maxMemoryUsage);
}

/**
 * @brief 析构函数
 *
 * 清理纹理池并释放所有资源
 */
TexturePool::~TexturePool() {
    clear();
    E2D_LOG_INFO("TexturePool destroyed");
}

// ============================================================================
// 纹理加载
// ============================================================================

/**
 * @brief 从文件加载纹理
 * @param path 文件路径
 * @param options 加载选项
 * @return 纹理引用
 *
 * 加载完整纹理文件到纹理池
 */
TextureRef TexturePool::load(const std::string& path, const TextureLoadOptions& options) {
    return load(path, Rect::Zero(), options);
}

/**
 * @brief 从文件加载纹理区域
 * @param path 文件路径
 * @param region 纹理区域
 * @param options 加载选项
 * @return 纹理引用
 *
 * 加载纹理文件的指定区域到纹理池
 */
TextureRef TexturePool::load(const std::string& path, const Rect& region,
                              const TextureLoadOptions& options) {
    TextureKey key(path, region);

    std::lock_guard<std::mutex> lock(mutex_);

    // 检查缓存
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        // 缓存命中
        it->second.touch();
        it->second.refCount.fetch_add(1, std::memory_order_relaxed);
        cacheHits_.fetch_add(1, std::memory_order_relaxed);

        E2D_LOG_DEBUG("Texture cache hit: {}", path);
        return TextureRef(it->second.texture, &it->second, &mutex_);
    }

    // 缓存未命中
    cacheMisses_.fetch_add(1, std::memory_order_relaxed);

    // 获取渲染后端
    RenderBackend* backend = nullptr;
    if (scene_) {
        // 假设 Scene 有获取 RenderBackend 的方法
        // 这里需要根据实际接口调整
        backend = nullptr;  // TODO: 从 Scene 获取 RenderBackend
    }

    if (!backend) {
        E2D_LOG_ERROR("TexturePool: RenderBackend not available");
        return TextureRef();
    }

    // 加载纹理
    Ptr<Texture> texture = backend->loadTexture(path);
    if (!texture) {
        E2D_LOG_ERROR("TexturePool: Failed to load texture: {}", path);
        return TextureRef();
    }

    // 计算内存大小
    size_t memorySize = calculateTextureMemory(texture.get());

    // 检查内存限制
    if (maxMemoryUsage_ > 0 && currentMemoryUsage_ + memorySize > maxMemoryUsage_) {
        // 尝试淘汰
        evictLRU(currentMemoryUsage_ + memorySize - maxMemoryUsage_);

        // 再次检查
        if (currentMemoryUsage_ + memorySize > maxMemoryUsage_) {
            E2D_LOG_WARN("TexturePool: Memory limit exceeded, cannot load texture: {}", path);
            return TextureRef();
        }
    }

    // 创建缓存条目
    auto result = cache_.emplace(key, TexturePoolEntry(nullptr, key, 0));
    if (result.second) {
        result.first->second.texture = texture;
        result.first->second.memorySize = memorySize;
        result.first->second.refCount.store(1, std::memory_order_relaxed);
        result.first->second.touch();
        currentMemoryUsage_ += memorySize;
        E2D_LOG_INFO("TexturePool: Loaded texture: {} ({} bytes)", path, memorySize);
        return TextureRef(texture, &result.first->second, &mutex_);
    }

    return TextureRef();
}

/**
 * @brief 从内存加载纹理
 * @param data 像素数据
 * @param width 宽度
 * @param height 高度
 * @param channels 通道数
 * @param key 缓存键
 * @return 纹理引用
 *
 * 从内存中的像素数据创建纹理并加入纹理池
 */
TextureRef TexturePool::loadFromMemory(const uint8_t* data, int width, int height,
                                        int channels, const std::string& key) {
    TextureKey textureKey(key);

    std::lock_guard<std::mutex> lock(mutex_);

    // 检查缓存
    auto it = cache_.find(textureKey);
    if (it != cache_.end()) {
        it->second.touch();
        it->second.refCount.fetch_add(1, std::memory_order_relaxed);
        cacheHits_.fetch_add(1, std::memory_order_relaxed);
        return TextureRef(it->second.texture, &it->second, &mutex_);
    }

    cacheMisses_.fetch_add(1, std::memory_order_relaxed);

    // 获取渲染后端
    RenderBackend* backend = nullptr;
    if (scene_) {
        backend = nullptr;  // TODO: 从 Scene 获取 RenderBackend
    }

    if (!backend) {
        E2D_LOG_ERROR("TexturePool: RenderBackend not available");
        return TextureRef();
    }

    // 创建纹理
    Ptr<Texture> texture = backend->createTexture(width, height, data, channels);
    if (!texture) {
        E2D_LOG_ERROR("TexturePool: Failed to create texture from memory");
        return TextureRef();
    }

    // 计算内存大小
    size_t memorySize = calculateTextureMemory(texture.get());

    // 检查内存限制
    if (maxMemoryUsage_ > 0 && currentMemoryUsage_ + memorySize > maxMemoryUsage_) {
        evictLRU(currentMemoryUsage_ + memorySize - maxMemoryUsage_);

        if (currentMemoryUsage_ + memorySize > maxMemoryUsage_) {
            E2D_LOG_WARN("TexturePool: Memory limit exceeded");
            return TextureRef();
        }
    }

    // 创建缓存条目
    auto result = cache_.emplace(textureKey, TexturePoolEntry(nullptr, textureKey, 0));
    if (result.second) {
        result.first->second.texture = texture;
        result.first->second.memorySize = memorySize;
        result.first->second.refCount.store(1, std::memory_order_relaxed);
        result.first->second.touch();
        currentMemoryUsage_ += memorySize;
        E2D_LOG_INFO("TexturePool: Created texture from memory ({} bytes)", memorySize);
        return TextureRef(texture, &result.first->second, &mutex_);
    }

    return TextureRef();
}

/**
 * @brief 获取或加载纹理
 * @param path 文件路径
 * @param options 加载选项
 * @return 纹理引用
 *
 * 如果纹理已缓存则返回缓存，否则加载纹理
 */
TextureRef TexturePool::getOrLoad(const std::string& path, const TextureLoadOptions& options) {
    return getOrLoad(path, Rect::Zero(), options);
}

/**
 * @brief 获取或加载纹理区域
 * @param path 文件路径
 * @param region 纹理区域
 * @param options 加载选项
 * @return 纹理引用
 *
 * 如果纹理区域已缓存则返回缓存，否则加载纹理区域
 */
TextureRef TexturePool::getOrLoad(const std::string& path, const Rect& region,
                                   const TextureLoadOptions& options) {
    TextureKey key(path, region);

    std::lock_guard<std::mutex> lock(mutex_);

    // 检查缓存
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        it->second.touch();
        it->second.refCount.fetch_add(1, std::memory_order_relaxed);
        cacheHits_.fetch_add(1, std::memory_order_relaxed);
        return TextureRef(it->second.texture, &it->second, &mutex_);
    }

    // 释放锁后调用 load
    // 注意：这里需要重新设计以避免死锁
    // 简化处理：直接在这里加载

    cacheMisses_.fetch_add(1, std::memory_order_relaxed);

    RenderBackend* backend = nullptr;
    if (scene_) {
        backend = nullptr;  // TODO: 从 Scene 获取 RenderBackend
    }

    if (!backend) {
        E2D_LOG_ERROR("TexturePool: RenderBackend not available");
        return TextureRef();
    }

    Ptr<Texture> texture = backend->loadTexture(path);
    if (!texture) {
        E2D_LOG_ERROR("TexturePool: Failed to load texture: {}", path);
        return TextureRef();
    }

    size_t memorySize = calculateTextureMemory(texture.get());

    if (maxMemoryUsage_ > 0 && currentMemoryUsage_ + memorySize > maxMemoryUsage_) {
        evictLRU(currentMemoryUsage_ + memorySize - maxMemoryUsage_);

        if (currentMemoryUsage_ + memorySize > maxMemoryUsage_) {
            E2D_LOG_WARN("TexturePool: Memory limit exceeded");
            return TextureRef();
        }
    }

    auto result = cache_.emplace(key, TexturePoolEntry(nullptr, key, 0));
    if (result.second) {
        result.first->second.texture = texture;
        result.first->second.memorySize = memorySize;
        result.first->second.refCount.store(1, std::memory_order_relaxed);
        result.first->second.touch();
        currentMemoryUsage_ += memorySize;
        return TextureRef(texture, &result.first->second, &mutex_);
    }

    return TextureRef();
}

// ============================================================================
// 引用计数管理
// ============================================================================

/**
 * @brief 增加引用计数
 * @param key 纹理键
 * @return 是否成功
 *
 * 增加指定纹理的引用计数
 */
bool TexturePool::addRef(const TextureKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        it->second.touch();
        it->second.refCount.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    return false;
}

/**
 * @brief 减少引用计数
 * @param key 纹理键
 * @return 减少后的引用计数
 *
 * 减少指定纹理的引用计数并返回新值
 */
uint32_t TexturePool::release(const TextureKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        uint32_t count = it->second.refCount.fetch_sub(1, std::memory_order_relaxed);
        return count > 0 ? count - 1 : 0;
    }
    return 0;
}

/**
 * @brief 获取引用计数
 * @param key 纹理键
 * @return 引用计数
 *
 * 获取指定纹理的当前引用计数
 */
uint32_t TexturePool::getRefCount(const TextureKey& key) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second.refCount.load(std::memory_order_relaxed);
    }
    return 0;
}

// ============================================================================
// 缓存管理
// ============================================================================

/**
 * @brief 检查纹理是否已缓存
 * @param key 纹理键
 * @return 是否已缓存
 *
 * 检查指定纹理是否存在于缓存中
 */
bool TexturePool::isCached(const TextureKey& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.find(key) != cache_.end();
}

/**
 * @brief 从缓存中移除纹理
 * @param key 纹理键
 * @return 是否成功
 *
 * 从缓存中移除指定的纹理
 */
bool TexturePool::removeFromCache(const TextureKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        currentMemoryUsage_ -= it->second.memorySize;
        cache_.erase(it);
        E2D_LOG_DEBUG("TexturePool: Removed texture from cache");
        return true;
    }
    return false;
}

/**
 * @brief 垃圾回收（移除引用计数为 0 的纹理）
 * @return 移除的纹理数量
 *
 * 清理所有引用计数为0的纹理，释放内存
 */
size_t TexturePool::collectGarbage() {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t removed = 0;
    for (auto it = cache_.begin(); it != cache_.end(); ) {
        if (it->second.refCount.load(std::memory_order_relaxed) == 0) {
            currentMemoryUsage_ -= it->second.memorySize;
            it = cache_.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }

    if (removed > 0) {
        E2D_LOG_INFO("TexturePool: Garbage collected {} textures", removed);
    }

    return removed;
}

/**
 * @brief 清空所有缓存
 *
 * 移除纹理池中的所有纹理
 */
void TexturePool::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    cache_.clear();
    currentMemoryUsage_ = 0;

    E2D_LOG_INFO("TexturePool: Cleared all textures");
}

// ============================================================================
// 内存管理
// ============================================================================

/**
 * @brief 获取当前内存使用量
 * @return 内存使用量（字节）
 *
 * 返回纹理池当前的内存使用量
 */
size_t TexturePool::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentMemoryUsage_;
}

/**
 * @brief 设置最大内存使用量
 * @param maxMemory 最大内存使用量（0 表示无限制）
 *
 * 设置纹理池的内存上限，如果当前使用量超过新上限则执行淘汰
 */
void TexturePool::setMaxMemoryUsage(size_t maxMemory) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxMemoryUsage_ = maxMemory;

    // 如果当前内存超过新的限制，执行淘汰
    if (maxMemoryUsage_ > 0 && currentMemoryUsage_ > maxMemoryUsage_) {
        evictLRU(maxMemoryUsage_);
    }

    E2D_LOG_INFO("TexturePool: Max memory set to {} bytes", maxMemory);
}

/**
 * @brief 执行 LRU 淘汰
 * @param targetMemory 目标内存使用量
 * @return 淘汰的纹理数量
 *
 * 根据LRU算法淘汰最少使用的纹理以达到目标内存使用量
 */
size_t TexturePool::evictLRU(size_t targetMemory) {
    // 注意：调用者应该已持有锁

    if (cache_.empty()) {
        return 0;
    }

    // 收集所有条目并按最后访问时间排序
    std::vector<std::pair<TextureKey, uint64_t>> entries;
    entries.reserve(cache_.size());

    for (const auto& pair : cache_) {
        // 只淘汰引用计数为 0 的纹理
        if (pair.second.refCount.load(std::memory_order_relaxed) == 0) {
            entries.emplace_back(pair.first, pair.second.lastAccessTime);
        }
    }

    // 按访问时间升序排序（最旧的在前）
    std::sort(entries.begin(), entries.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    size_t evicted = 0;
    size_t target = targetMemory > 0 ? targetMemory : 0;

    for (const auto& entry : entries) {
        if (targetMemory > 0 && currentMemoryUsage_ <= target) {
            break;
        }

        auto it = cache_.find(entry.first);
        if (it != cache_.end()) {
            currentMemoryUsage_ -= it->second.memorySize;
            cache_.erase(it);
            ++evicted;
        }
    }

    if (evicted > 0) {
        evictionCount_.fetch_add(evicted, std::memory_order_relaxed);
        E2D_LOG_INFO("TexturePool: LRU evicted {} textures", evicted);
    }

    return evicted;
}

// ============================================================================
// 统计信息
// ============================================================================

/**
 * @brief 获取统计信息
 * @return 统计信息结构体
 *
 * 返回纹理池的统计信息，包括纹理数量、内存使用、缓存命中率等
 */
TexturePool::Stats TexturePool::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);

    Stats stats;
    stats.textureCount = cache_.size();
    stats.memoryUsage = currentMemoryUsage_;
    stats.maxMemoryUsage = maxMemoryUsage_;
    stats.cacheHits = cacheHits_.load(std::memory_order_relaxed);
    stats.cacheMisses = cacheMisses_.load(std::memory_order_relaxed);
    stats.evictionCount = evictionCount_.load(std::memory_order_relaxed);

    return stats;
}

/**
 * @brief 重置统计信息
 *
 * 清零缓存命中、未命中和淘汰计数
 */
void TexturePool::resetStats() {
    cacheHits_.store(0, std::memory_order_relaxed);
    cacheMisses_.store(0, std::memory_order_relaxed);
    evictionCount_.store(0, std::memory_order_relaxed);
}

// ============================================================================
// 私有方法
// ============================================================================

/**
 * @brief 计算纹理内存大小
 * @param texture 纹理对象
 * @return 内存大小（字节）
 *
 * 根据纹理的尺寸、通道数和像素格式计算内存占用
 */
size_t TexturePool::calculateTextureMemory(const Texture* texture) {
    if (!texture) {
        return 0;
    }

    int width = texture->getWidth();
    int height = texture->getHeight();
    int channels = texture->getChannels();

    // 基础内存计算
    size_t baseSize = static_cast<size_t>(width) * height * channels;

    // 根据像素格式调整
    PixelFormat format = texture->getFormat();
    switch (format) {
        case PixelFormat::RGB16F:
        case PixelFormat::RGBA16F:
            baseSize *= 2;  // 半精度浮点
            break;
        case PixelFormat::RGB32F:
        case PixelFormat::RGBA32F:
            baseSize *= 4;  // 全精度浮点
            break;
        case PixelFormat::Depth16:
            baseSize = static_cast<size_t>(width) * height * 2;
            break;
        case PixelFormat::Depth24:
        case PixelFormat::Depth24Stencil8:
            baseSize = static_cast<size_t>(width) * height * 4;
            break;
        case PixelFormat::Depth32F:
            baseSize = static_cast<size_t>(width) * height * 4;
            break;
        default:
            break;
    }

    // 考虑 Mipmaps（大约增加 33% 内存）
    // 注意：这里假设生成了 mipmaps，实际应该根据 TextureLoadOptions 判断
    // baseSize = baseSize * 4 / 3;

    return baseSize;
}

/**
 * @brief 检查是否需要淘汰
 * @return 是否需要淘汰
 *
 * 检查当前内存使用量是否超过限制
 */
bool TexturePool::needsEviction() const {
    return maxMemoryUsage_ > 0 && currentMemoryUsage_ > maxMemoryUsage_;
}

/**
 * @brief 尝试自动淘汰
 *
 * 如果内存使用量超过限制，执行LRU淘汰
 */
void TexturePool::tryAutoEvict() {
    if (needsEviction()) {
        evictLRU(maxMemoryUsage_);
    }
}

}  // namespace extra2d
