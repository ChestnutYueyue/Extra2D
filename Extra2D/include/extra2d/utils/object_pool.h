#pragma once

#include <extra2d/core/types.h>
#include <extra2d/utils/logger.h>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <new>
#include <type_traits>
#include <vector>

namespace extra2d {

// ============================================================================
// 对象池 - 自动管理的高性能内存池
// 特性：
// - 自动内存对齐
// - 侵入式空闲链表（零额外内存开销）
// - 线程本地缓存（减少锁竞争）
// - 自动容量管理（自动扩展/收缩）
// - 自动预热
// - 异常安全
// ============================================================================

// 线程本地缓存配置
struct PoolConfig {
    static constexpr size_t DEFAULT_BLOCK_SIZE = 64;
    static constexpr size_t THREAD_CACHE_SIZE = 16;
    static constexpr size_t SHRINK_THRESHOLD_MS = 30000;
    static constexpr double SHRINK_RATIO = 0.5;
};

template <typename T, size_t BlockSize = PoolConfig::DEFAULT_BLOCK_SIZE>
class ObjectPool {
public:
    static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
    static_assert(std::is_destructible_v<T>, "T must be destructible");
    static_assert(BlockSize > 0, "BlockSize must be greater than 0");
    static_assert(alignof(T) <= alignof(std::max_align_t), 
                  "Alignment requirement too high");

    ObjectPool() 
        : freeListHead_(nullptr)
        , blocks_()
        , allocatedCount_(0)
        , totalCapacity_(0)
        , isDestroyed_(false)
        , lastShrinkCheck_(0)
        , prewarmed_(false) {
    }

    ~ObjectPool() {
        clear();
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) noexcept = delete;
    ObjectPool& operator=(ObjectPool&&) noexcept = delete;

    /**
     * @brief 分配一个对象（自动预热、自动扩展）
     * @return 指向分配的对象的指针，失败返回 nullptr
     */
    T* allocate() {
        auto& cache = getThreadCache();
        
        if (T* obj = cache.pop()) {
            new (obj) T();
            allocatedCount_.fetch_add(1, std::memory_order_relaxed);
            return obj;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        if (isDestroyed_) {
            return nullptr;
        }

        if (!prewarmed_) {
            prewarmInternal();
        }

        if (!freeListHead_) {
            growInternal();
        }

        T* obj = popFreeList();
        if (obj) {
            new (obj) T();
            allocatedCount_.fetch_add(1, std::memory_order_relaxed);
        }
        return obj;
    }

    /**
     * @brief 分配并构造一个对象（带参数）
     */
    template <typename... Args>
    T* allocate(Args&&... args) {
        auto& cache = getThreadCache();
        
        if (T* obj = cache.pop()) {
            new (obj) T(std::forward<Args>(args)...);
            allocatedCount_.fetch_add(1, std::memory_order_relaxed);
            return obj;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        if (isDestroyed_) {
            return nullptr;
        }

        if (!prewarmed_) {
            prewarmInternal();
        }

        if (!freeListHead_) {
            growInternal();
        }

        T* obj = popFreeList();
        if (obj) {
            new (obj) T(std::forward<Args>(args)...);
            allocatedCount_.fetch_add(1, std::memory_order_relaxed);
        }
        return obj;
    }

    /**
     * @brief 回收一个对象（自动异常处理）
     * @param obj 要回收的对象指针
     * @return true 如果对象成功回收
     */
    bool deallocate(T* obj) {
        if (!obj) {
            return false;
        }

        try {
            obj->~T();
        } catch (const std::exception& e) {
            Logger::log(LogLevel::Error, "ObjectPool: Exception in destructor: {}", e.what());
        } catch (...) {
            Logger::log(LogLevel::Error, "ObjectPool: Unknown exception in destructor");
        }

        auto& cache = getThreadCache();
        if (cache.push(obj)) {
            allocatedCount_.fetch_sub(1, std::memory_order_relaxed);
            return true;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (!isDestroyed_) {
            pushFreeList(obj);
            allocatedCount_.fetch_sub(1, std::memory_order_relaxed);
            tryAutoShrink();
            return true;
        }
        return false;
    }

    /**
     * @brief 获取当前已分配的对象数量
     */
    size_t allocatedCount() const {
        return allocatedCount_.load(std::memory_order_relaxed);
    }

    /**
     * @brief 获取池中总的对象容量
     */
    size_t capacity() const {
        return totalCapacity_.load(std::memory_order_relaxed);
    }

    /**
     * @brief 获取内存使用量（字节）
     */
    size_t memoryUsage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return blocks_.size() * BlockSize * sizeof(T);
    }

    /**
     * @brief 清空所有内存块
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        isDestroyed_ = true;
        
        for (auto& block : blocks_) {
            alignedFree(block);
        }
        blocks_.clear();
        freeListHead_ = nullptr;
        totalCapacity_.store(0, std::memory_order_relaxed);
        allocatedCount_.store(0, std::memory_order_relaxed);
    }

private:
    struct FreeNode {
        FreeNode* next;
    };

    struct ThreadCache {
        T* objects[PoolConfig::THREAD_CACHE_SIZE];
        size_t count = 0;

        T* pop() {
            if (count > 0) {
                return objects[--count];
            }
            return nullptr;
        }

        bool push(T* obj) {
            if (count < PoolConfig::THREAD_CACHE_SIZE) {
                objects[count++] = obj;
                return true;
            }
            return false;
        }

        void clear() {
            count = 0;
        }
    };

    static ThreadCache& getThreadCache() {
        thread_local ThreadCache cache;
        return cache;
    }

    static constexpr size_t Alignment = alignof(T);
    static constexpr size_t AlignedSize = ((sizeof(T) + Alignment - 1) / Alignment) * Alignment;

    static void* alignedAlloc(size_t size) {
#ifdef _WIN32
        return _aligned_malloc(size, Alignment);
#else
        return std::aligned_alloc(Alignment, size);
#endif
    }

    static void alignedFree(void* ptr) {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        std::free(ptr);
#endif
    }

    void prewarmInternal() {
        if (!freeListHead_) {
            growInternal();
        }
        prewarmed_ = true;
    }

    void growInternal() {
        size_t blockSize = AlignedSize > sizeof(FreeNode) ? AlignedSize : sizeof(FreeNode);
        size_t totalSize = blockSize * BlockSize;
        
        void* block = alignedAlloc(totalSize);
        if (!block) {
            Logger::log(LogLevel::Error, "ObjectPool: Failed to allocate memory block");
            return;
        }

        blocks_.push_back(block);
        totalCapacity_.fetch_add(BlockSize, std::memory_order_relaxed);

        char* ptr = static_cast<char*>(block);
        for (size_t i = 0; i < BlockSize; ++i) {
            pushFreeList(reinterpret_cast<T*>(ptr + i * blockSize));
        }
    }

    void pushFreeList(T* obj) {
        FreeNode* node = reinterpret_cast<FreeNode*>(obj);
        node->next = freeListHead_;
        freeListHead_ = node;
    }

    T* popFreeList() {
        if (!freeListHead_) {
            return nullptr;
        }
        FreeNode* node = freeListHead_;
        freeListHead_ = freeListHead_->next;
        return reinterpret_cast<T*>(node);
    }

    void tryAutoShrink() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        if (elapsed - lastShrinkCheck_ < PoolConfig::SHRINK_THRESHOLD_MS) {
            return;
        }
        lastShrinkCheck_ = elapsed;

        size_t allocated = allocatedCount_.load(std::memory_order_relaxed);
        size_t capacity = totalCapacity_.load(std::memory_order_relaxed);
        
        if (capacity > BlockSize && 
            static_cast<double>(allocated) / capacity < PoolConfig::SHRINK_RATIO) {
            shrinkInternal();
        }
    }

    void shrinkInternal() {
        size_t toFree = 0;
        size_t freeCount = 0;
        
        FreeNode* node = freeListHead_;
        while (node) {
            ++freeCount;
            node = node->next;
        }

        if (freeCount < BlockSize) {
            return;
        }

        size_t blocksToKeep = blocks_.size();
        if (allocatedCount_.load(std::memory_order_relaxed) > 0) {
            blocksToKeep = (allocatedCount_.load() + BlockSize - 1) / BlockSize;
            blocksToKeep = std::max(blocksToKeep, size_t(1));
        }

        if (blocksToKeep >= blocks_.size()) {
            return;
        }

        size_t blocksToRemove = blocks_.size() - blocksToKeep;
        for (size_t i = 0; i < blocksToRemove; ++i) {
            if (blocks_.empty()) break;
            
            void* block = blocks_.back();
            blocks_.pop_back();
            alignedFree(block);
            totalCapacity_.fetch_sub(BlockSize, std::memory_order_relaxed);
        }

        rebuildFreeList();
    }

    void rebuildFreeList() {
        freeListHead_ = nullptr;
        size_t blockSize = AlignedSize > sizeof(FreeNode) ? AlignedSize : sizeof(FreeNode);
        
        for (void* block : blocks_) {
            char* ptr = static_cast<char*>(block);
            for (size_t i = 0; i < BlockSize; ++i) {
                pushFreeList(reinterpret_cast<T*>(ptr + i * blockSize));
            }
        }
    }

    mutable std::mutex mutex_;
    FreeNode* freeListHead_;
    std::vector<void*> blocks_;
    std::atomic<size_t> allocatedCount_;
    std::atomic<size_t> totalCapacity_;
    bool isDestroyed_;
    uint64_t lastShrinkCheck_;
    bool prewarmed_;
};

// ============================================================================
// 智能指针支持的内存池分配器
// ============================================================================

template <typename T, size_t BlockSize = PoolConfig::DEFAULT_BLOCK_SIZE>
class PooledAllocator {
public:
    using PoolType = ObjectPool<T, BlockSize>;

    PooledAllocator() : pool_(std::make_shared<PoolType>()) {}
    explicit PooledAllocator(std::shared_ptr<PoolType> pool) : pool_(pool) {}

    /**
     * @brief 创建一个使用内存池的对象（自动管理）
     */
    template <typename... Args>
    Ptr<T> makeShared(Args&&... args) {
        std::weak_ptr<PoolType> weakPool = pool_;
        T* obj = pool_->allocate(std::forward<Args>(args)...);
        if (!obj) {
            return nullptr;
        }
        return Ptr<T>(obj, Deleter{weakPool});
    }

    /**
     * @brief 获取底层内存池
     */
    std::shared_ptr<PoolType> getPool() const {
        return pool_;
    }

private:
    struct Deleter {
        std::weak_ptr<PoolType> pool;

        void operator()(T* obj) const {
            if (auto sharedPool = pool.lock()) {
                if (!sharedPool->deallocate(obj)) {
                    Logger::log(LogLevel::Warn, "PooledAllocator: Pool destroyed, memory leaked");
                }
            } else {
                Logger::log(LogLevel::Warn, "PooledAllocator: Pool expired during deallocation");
            }
        }
    };

    std::shared_ptr<PoolType> pool_;
};

// ============================================================================
// 全局内存池管理器 - 自动管理所有池的生命周期
// ============================================================================

class ObjectPoolManager {
public:
    static ObjectPoolManager& getInstance() {
        static ObjectPoolManager instance;
        return instance;
    }

    /**
     * @brief 获取指定类型的内存池（自动管理）
     */
    template <typename T, size_t BlockSize = PoolConfig::DEFAULT_BLOCK_SIZE>
    std::shared_ptr<ObjectPool<T, BlockSize>> getPool() {
        static auto pool = std::make_shared<ObjectPool<T, BlockSize>>();
        return pool;
    }

    /**
     * @brief 创建使用内存池的对象（自动管理）
     */
    template <typename T, size_t BlockSize = PoolConfig::DEFAULT_BLOCK_SIZE, typename... Args>
    Ptr<T> makePooled(Args&&... args) {
        auto pool = getPool<T, BlockSize>();
        std::weak_ptr<ObjectPool<T, BlockSize>> weakPool = pool;
        T* obj = pool->allocate(std::forward<Args>(args)...);
        if (!obj) {
            return nullptr;
        }
        return Ptr<T>(obj, [weakPool](T* p) {
            if (auto sharedPool = weakPool.lock()) {
                if (!sharedPool->deallocate(p)) {
                    Logger::log(LogLevel::Warn, "ObjectPoolManager: Pool destroyed during deallocation");
                }
            } else {
                Logger::log(LogLevel::Warn, "ObjectPoolManager: Pool expired");
            }
        });
    }

private:
    ObjectPoolManager() = default;
    ~ObjectPoolManager() = default;
    ObjectPoolManager(const ObjectPoolManager&) = delete;
    ObjectPoolManager& operator=(const ObjectPoolManager&) = delete;
};

// ============================================================================
// 内存池宏定义（便于使用）
// ============================================================================

#define E2D_DECLARE_POOL(T, BlockSize) \
    static extra2d::ObjectPool<T, BlockSize>& getPool() { \
        static extra2d::ObjectPool<T, BlockSize> pool; \
        return pool; \
    }

#define E2D_MAKE_POOLED(T, ...) \
    extra2d::ObjectPoolManager::getInstance().makePooled<T>(__VA_ARGS__)

} // namespace extra2d
