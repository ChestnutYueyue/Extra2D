#pragma once

#include <easy2d/core/types.h>
#include <easy2d/graphics/texture.h>
#include <string>
#include <unordered_map>
#include <list>
#include <functional>
#include <mutex>

namespace easy2d {

// ============================================================================
// 纹理池配置
// ============================================================================
struct TexturePoolConfig {
    size_t maxCacheSize = 64 * 1024 * 1024;  // 最大缓存大小 (64MB)
    size_t maxTextureCount = 256;             // 最大纹理数量
    float unloadInterval = 30.0f;             // 自动清理间隔 (秒)
    bool enableAsyncLoad = true;              // 启用异步加载
};

// ============================================================================
// 纹理池 - 使用LRU缓存策略管理纹理复用
// ============================================================================
class TexturePool {
public:
    // ------------------------------------------------------------------------
    // 单例访问
    // ------------------------------------------------------------------------
    static TexturePool& getInstance();

    // ------------------------------------------------------------------------
    // 初始化和关闭
    // ------------------------------------------------------------------------
    bool init(const TexturePoolConfig& config = TexturePoolConfig{});
    void shutdown();

    // ------------------------------------------------------------------------
    // 纹理获取
    // ------------------------------------------------------------------------
    
    /**
     * @brief 从文件获取纹理（带缓存）
     * @param filepath 纹理文件路径
     * @return 纹理对象，失败返回nullptr
     */
    Ptr<Texture> get(const std::string& filepath);
    
    /**
     * @brief 异步加载纹理
     * @param filepath 纹理文件路径
     * @param callback 加载完成回调
     */
    void getAsync(const std::string& filepath, 
                  std::function<void(Ptr<Texture>)> callback);
    
    /**
     * @brief 从内存数据创建纹理（自动缓存）
     * @param name 纹理名称（用于缓存键）
     * @param data 图像数据
     * @param width 宽度
     * @param height 高度
     * @param format 像素格式
     * @return 纹理对象
     */
    Ptr<Texture> createFromData(const std::string& name,
                                const uint8_t* data,
                                int width,
                                int height,
                                PixelFormat format = PixelFormat::RGBA8);

    // ------------------------------------------------------------------------
    // 缓存管理
    // ------------------------------------------------------------------------
    
    /**
     * @brief 手动添加纹理到缓存
     */
    void add(const std::string& key, Ptr<Texture> texture);
    
    /**
     * @brief 从缓存移除纹理
     */
    void remove(const std::string& key);
    
    /**
     * @brief 检查纹理是否在缓存中
     */
    bool has(const std::string& key) const;
    
    /**
     * @brief 清空所有缓存
     */
    void clear();
    
    /**
     * @brief 清理未使用的纹理（LRU策略）
     * @param targetSize 目标缓存大小
     */
    void trim(size_t targetSize);

    // ------------------------------------------------------------------------
    // 统计信息
    // ------------------------------------------------------------------------
    
    /**
     * @brief 获取当前缓存的纹理数量
     */
    size_t getTextureCount() const;
    
    /**
     * @brief 获取当前缓存的总大小（字节）
     */
    size_t getCacheSize() const;
    
    /**
     * @brief 获取缓存命中率
     */
    float getHitRate() const;
    
    /**
     * @brief 打印统计信息
     */
    void printStats() const;

    // ------------------------------------------------------------------------
    // 自动清理
    // ------------------------------------------------------------------------
    
    /**
     * @brief 更新（在主循环中调用，用于自动清理）
     */
    void update(float dt);
    
    /**
     * @brief 设置自动清理间隔
     */
    void setAutoUnloadInterval(float interval);

private:
    TexturePool() = default;
    ~TexturePool() = default;
    TexturePool(const TexturePool&) = delete;
    TexturePool& operator=(const TexturePool&) = delete;

    // 缓存项
    struct CacheEntry {
        Ptr<Texture> texture;
        size_t size;           // 纹理大小（字节）
        float lastAccessTime;  // 最后访问时间
        uint32_t accessCount;  // 访问次数
    };

    // LRU列表（最近使用的在前面）
    using LRUList = std::list<std::string>;
    using LRUIterator = LRUList::iterator;

    mutable std::mutex mutex_;
    TexturePoolConfig config_;
    
    // 缓存存储
    std::unordered_map<std::string, CacheEntry> cache_;
    std::unordered_map<std::string, LRUIterator> lruMap_;
    LRUList lruList_;
    
    // 统计
    size_t totalSize_ = 0;
    uint64_t hitCount_ = 0;
    uint64_t missCount_ = 0;
    float autoUnloadTimer_ = 0.0f;
    bool initialized_ = false;

    // 异步加载队列
    struct AsyncLoadTask {
        std::string filepath;
        std::function<void(Ptr<Texture>)> callback;
    };
    std::vector<AsyncLoadTask> asyncTasks_;

    // 内部方法
    void touch(const std::string& key);
    void evict();
    Ptr<Texture> loadTexture(const std::string& filepath);
    size_t calculateTextureSize(int width, int height, PixelFormat format);
    void processAsyncTasks();
};

// ============================================================================
// 便捷宏
// ============================================================================
#define E2D_TEXTURE_POOL() ::easy2d::TexturePool::getInstance()

} // namespace easy2d
