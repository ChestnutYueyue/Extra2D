#pragma once

#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/utils/logger.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace extra2d {

// 前向声明
class Scene;
class RenderBackend;

// ============================================================================
// 纹理加载选项
// ============================================================================
struct TextureLoadOptions {
    bool generateMipmaps = true;      // 是否生成 mipmaps
    bool sRGB = true;                 // 是否使用 sRGB 色彩空间
    bool premultiplyAlpha = false;    // 是否预乘 Alpha
    PixelFormat preferredFormat = PixelFormat::RGBA8;  // 首选像素格式
};

// ============================================================================
// 纹理键 - 用于唯一标识纹理缓存条目
// ============================================================================
struct TextureKey {
    std::string path;    // 纹理文件路径
    Rect region;         // 纹理区域（用于纹理图集）

    /**
     * @brief 默认构造函数
     */
    TextureKey() = default;

    /**
     * @brief 构造函数（仅路径）
     * @param p 纹理文件路径
     */
    explicit TextureKey(const std::string& p) : path(p), region(Rect::Zero()) {}

    /**
     * @brief 构造函数（路径 + 区域）
     * @param p 纹理文件路径
     * @param r 纹理区域
     */
    TextureKey(const std::string& p, const Rect& r) : path(p), region(r) {}

    /**
     * @brief 相等比较运算符
     * @param other 另一个 TextureKey
     * @return 是否相等
     */
    bool operator==(const TextureKey& other) const {
        return path == other.path && region == other.region;
    }

    /**
     * @brief 不等比较运算符
     * @param other 另一个 TextureKey
     * @return 是否不等
     */
    bool operator!=(const TextureKey& other) const {
        return !(*this == other);
    }
};

// ============================================================================
// TextureKey 哈希函子
// ============================================================================
struct TextureKeyHash {
    /**
     * @brief 计算 TextureKey 的哈希值
     * @param key 纹理键
     * @return 哈希值
     */
    size_t operator()(const TextureKey& key) const {
        size_t h1 = std::hash<std::string>{}(key.path);
        size_t h2 = std::hash<float>{}(key.region.origin.x);
        size_t h3 = std::hash<float>{}(key.region.origin.y);
        size_t h4 = std::hash<float>{}(key.region.size.width);
        size_t h5 = std::hash<float>{}(key.region.size.height);

        // 组合哈希值
        size_t result = h1;
        result ^= h2 + 0x9e3779b9 + (result << 6) + (result >> 2);
        result ^= h3 + 0x9e3779b9 + (result << 6) + (result >> 2);
        result ^= h4 + 0x9e3779b9 + (result << 6) + (result >> 2);
        result ^= h5 + 0x9e3779b9 + (result << 6) + (result >> 2);
        return result;
    }
};

// ============================================================================
// 纹理池条目
// ============================================================================
struct TexturePoolEntry {
    Ptr<Texture> texture;              // 纹理对象
    mutable std::atomic<uint32_t> refCount;    // 引用计数
    TextureKey key;                    // 纹理键
    size_t memorySize;                 // 内存占用（字节）
    mutable uint64_t lastAccessTime;   // 最后访问时间戳

    /**
     * @brief 默认构造函数
     */
    TexturePoolEntry()
        : texture(nullptr)
        , refCount(0)
        , key()
        , memorySize(0)
        , lastAccessTime(0) {}

    /**
     * @brief 构造函数
     * @param tex 纹理对象
     * @param k 纹理键
     * @param memSize 内存占用
     */
    TexturePoolEntry(Ptr<Texture> tex, const TextureKey& k, size_t memSize)
        : texture(tex)
        , refCount(1)
        , key(k)
        , memorySize(memSize)
        , lastAccessTime(getCurrentTime()) {}

    /**
     * @brief 移动构造函数
     * @param other 另一个条目
     */
    TexturePoolEntry(TexturePoolEntry&& other) noexcept
        : texture(std::move(other.texture))
        , refCount(other.refCount.load(std::memory_order_relaxed))
        , key(std::move(other.key))
        , memorySize(other.memorySize)
        , lastAccessTime(other.lastAccessTime) {}

    /**
     * @brief 移动赋值运算符
     * @param other 另一个条目
     * @return 引用
     */
    TexturePoolEntry& operator=(TexturePoolEntry&& other) noexcept {
        if (this != &other) {
            texture = std::move(other.texture);
            refCount.store(other.refCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
            key = std::move(other.key);
            memorySize = other.memorySize;
            lastAccessTime = other.lastAccessTime;
        }
        return *this;
    }

    // 禁止拷贝
    TexturePoolEntry(const TexturePoolEntry&) = delete;
    TexturePoolEntry& operator=(const TexturePoolEntry&) = delete;

    /**
     * @brief 更新最后访问时间
     */
    void touch() const { lastAccessTime = getCurrentTime(); }

    /**
     * @brief 获取当前时间戳
     * @return 时间戳（毫秒）
     */
    static uint64_t getCurrentTime() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch());
        return static_cast<uint64_t>(duration.count());
    }
};

// ============================================================================
// 纹理引用智能指针 - 自动管理纹理池引用计数
// ============================================================================
class TextureRef {
public:
    /**
     * @brief 默认构造函数
     */
    TextureRef() : texture_(nullptr), entry_(nullptr), mutex_(nullptr) {}

    /**
     * @brief 构造函数
     * @param texture 纹理对象
     * @param entry 纹理池条目
     * @param mutex 互斥锁
     */
    TextureRef(Ptr<Texture> texture, TexturePoolEntry* entry, std::mutex* mutex)
        : texture_(texture), entry_(entry), mutex_(mutex) {}

    /**
     * @brief 创建独立的纹理引用（不管理引用计数）
     * @param texture 纹理对象
     * @return 独立的纹理引用
     */
    static TextureRef fromTexture(Ptr<Texture> texture) {
        return TextureRef(texture, nullptr, nullptr);
    }

    /**
     * @brief 拷贝构造函数
     * @param other 另一个 TextureRef
     */
    TextureRef(const TextureRef& other)
        : texture_(other.texture_), entry_(other.entry_), mutex_(other.mutex_) {
        if (entry_ && entry_->refCount.load(std::memory_order_relaxed) > 0) {
            entry_->refCount.fetch_add(1, std::memory_order_relaxed);
        }
    }

    /**
     * @brief 移动构造函数
     * @param other 另一个 TextureRef
     */
    TextureRef(TextureRef&& other) noexcept
        : texture_(std::move(other.texture_))
        , entry_(other.entry_)
        , mutex_(other.mutex_) {
        other.entry_ = nullptr;
        other.mutex_ = nullptr;
    }

    /**
     * @brief 析构函数
     */
    ~TextureRef() { reset(); }

    /**
     * @brief 拷贝赋值运算符
     * @param other 另一个 TextureRef
     * @return 引用
     */
    TextureRef& operator=(const TextureRef& other) {
        if (this != &other) {
            reset();
            texture_ = other.texture_;
            entry_ = other.entry_;
            mutex_ = other.mutex_;
            if (entry_ && entry_->refCount.load(std::memory_order_relaxed) > 0) {
                entry_->refCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    /**
     * @brief 移动赋值运算符
     * @param other 另一个 TextureRef
     * @return 引用
     */
    TextureRef& operator=(TextureRef&& other) noexcept {
        if (this != &other) {
            reset();
            texture_ = std::move(other.texture_);
            entry_ = other.entry_;
            mutex_ = other.mutex_;
            other.entry_ = nullptr;
            other.mutex_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief 重置引用
     */
    void reset() {
        if (entry_ && mutex_) {
            std::lock_guard<std::mutex> lock(*mutex_);
            if (entry_->refCount.load(std::memory_order_relaxed) > 0) {
                entry_->refCount.fetch_sub(1, std::memory_order_relaxed);
            }
        }
        texture_.reset();
        entry_ = nullptr;
        mutex_ = nullptr;
    }

    /**
     * @brief 获取纹理对象
     * @return 纹理对象指针
     */
    Texture* get() const { return texture_.get(); }

    /**
     * @brief 获取纹理对象（智能指针）
     * @return 纹理对象智能指针
     */
    Ptr<Texture> getPtr() const { return texture_; }

    /**
     * @brief 检查是否有效
     * @return 是否有效
     */
    bool valid() const { return texture_ != nullptr; }

    /**
     * @brief 布尔转换运算符
     */
    explicit operator bool() const { return valid(); }

    /**
     * @brief 箭头运算符
     */
    Texture* operator->() const { return texture_.get(); }

    /**
     * @brief 解引用运算符
     */
    Texture& operator*() const { return *texture_; }

private:
    Ptr<Texture> texture_;
    TexturePoolEntry* entry_;
    std::mutex* mutex_;
};

// ============================================================================
// 纹理池 - 纹理缓存和内存管理系统
// 特性：
// - 纹理缓存和复用
// - 引用计数管理
// - 内存使用限制
// - LRU 淘汰策略
// - 线程安全
// ============================================================================
class TexturePool {
public:
    // ========================================================================
    // 统计信息
    // ========================================================================
    struct Stats {
        size_t textureCount = 0;       // 纹理数量
        size_t memoryUsage = 0;        // 内存使用量（字节）
        size_t maxMemoryUsage = 0;     // 最大内存使用量
        size_t cacheHits = 0;          // 缓存命中次数
        size_t cacheMisses = 0;        // 缓存未命中次数
        size_t evictionCount = 0;      // 淘汰次数
    };

    // ========================================================================
    // 构造和析构
    // ========================================================================

    /**
     * @brief 默认构造函数
     */
    TexturePool();

    /**
     * @brief 构造函数
     * @param scene 场景指针
     * @param maxMemoryUsage 最大内存使用量（0 表示无限制）
     */
    explicit TexturePool(Scene* scene, size_t maxMemoryUsage = 0);

    /**
     * @brief 析构函数
     */
    ~TexturePool();

    // 禁止拷贝
    TexturePool(const TexturePool&) = delete;
    TexturePool& operator=(const TexturePool&) = delete;

    /**
     * @brief 初始化纹理池
     * @param scene 场景指针
     * @param maxMemoryUsage 最大内存使用量（0 表示无限制）
     */
    void init(Scene* scene, size_t maxMemoryUsage = 0);

    // ========================================================================
    // 纹理加载
    // ========================================================================

    /**
     * @brief 从文件加载纹理
     * @param path 文件路径
     * @param options 加载选项
     * @return 纹理引用
     */
    TextureRef load(const std::string& path,
                    const TextureLoadOptions& options = TextureLoadOptions());

    /**
     * @brief 从文件加载纹理区域
     * @param path 文件路径
     * @param region 纹理区域
     * @param options 加载选项
     * @return 纹理引用
     */
    TextureRef load(const std::string& path, const Rect& region,
                    const TextureLoadOptions& options = TextureLoadOptions());

    /**
     * @brief 从内存加载纹理
     * @param data 像素数据
     * @param width 宽度
     * @param height 高度
     * @param channels 通道数
     * @param key 缓存键
     * @return 纹理引用
     */
    TextureRef loadFromMemory(const uint8_t* data, int width, int height,
                              int channels, const std::string& key);

    /**
     * @brief 获取或加载纹理
     * @param path 文件路径
     * @param options 加载选项
     * @return 纹理引用
     */
    TextureRef getOrLoad(const std::string& path,
                         const TextureLoadOptions& options = TextureLoadOptions());

    /**
     * @brief 获取或加载纹理区域
     * @param path 文件路径
     * @param region 纹理区域
     * @param options 加载选项
     * @return 纹理引用
     */
    TextureRef getOrLoad(const std::string& path, const Rect& region,
                         const TextureLoadOptions& options = TextureLoadOptions());

    // ========================================================================
    // 引用计数管理
    // ========================================================================

    /**
     * @brief 增加引用计数
     * @param key 纹理键
     * @return 是否成功
     */
    bool addRef(const TextureKey& key);

    /**
     * @brief 减少引用计数
     * @param key 纹理键
     * @return 减少后的引用计数
     */
    uint32_t release(const TextureKey& key);

    /**
     * @brief 获取引用计数
     * @param key 纹理键
     * @return 引用计数
     */
    uint32_t getRefCount(const TextureKey& key) const;

    // ========================================================================
    // 缓存管理
    // ========================================================================

    /**
     * @brief 检查纹理是否已缓存
     * @param key 纹理键
     * @return 是否已缓存
     */
    bool isCached(const TextureKey& key) const;

    /**
     * @brief 从缓存中移除纹理
     * @param key 纹理键
     * @return 是否成功
     */
    bool removeFromCache(const TextureKey& key);

    /**
     * @brief 垃圾回收（移除引用计数为 0 的纹理）
     * @return 移除的纹理数量
     */
    size_t collectGarbage();

    /**
     * @brief 清空所有缓存
     */
    void clear();

    // ========================================================================
    // 内存管理
    // ========================================================================

    /**
     * @brief 获取当前内存使用量
     * @return 内存使用量（字节）
     */
    size_t getMemoryUsage() const;

    /**
     * @brief 设置最大内存使用量
     * @param maxMemory 最大内存使用量（0 表示无限制）
     */
    void setMaxMemoryUsage(size_t maxMemory);

    /**
     * @brief 获取最大内存使用量
     * @return 最大内存使用量
     */
    size_t getMaxMemoryUsage() const { return maxMemoryUsage_; }

    /**
     * @brief 执行 LRU 淘汰
     * @param targetMemory 目标内存使用量
     * @return 淘汰的纹理数量
     */
    size_t evictLRU(size_t targetMemory = 0);

    // ========================================================================
    // 统计信息
    // ========================================================================

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    Stats getStats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStats();

private:
    /**
     * @brief 计算纹理内存大小
     * @param texture 纹理对象
     * @return 内存大小（字节）
     */
    static size_t calculateTextureMemory(const Texture* texture);

    /**
     * @brief 检查是否需要淘汰
     * @return 是否需要淘汰
     */
    bool needsEviction() const;

    /**
     * @brief 尝试自动淘汰
     */
    void tryAutoEvict();

    Scene* scene_;    // 场景指针
    mutable std::mutex mutex_;    // 互斥锁
    std::unordered_map<TextureKey, TexturePoolEntry, TextureKeyHash> cache_;  // 纹理缓存

    size_t maxMemoryUsage_;    // 最大内存使用量
    size_t currentMemoryUsage_;    // 当前内存使用量

    // 统计信息
    mutable std::atomic<size_t> cacheHits_;
    mutable std::atomic<size_t> cacheMisses_;
    mutable std::atomic<size_t> evictionCount_;
};

}  // namespace extra2d
