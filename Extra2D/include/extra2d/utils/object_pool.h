#pragma once

#include <extra2d/core/types.h>
#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>
#include <iostream>

namespace extra2d {

// ============================================================================
// 对象池 - 用于高效分配和回收小对象
// 减少频繁的内存分配/释放开销
// ============================================================================

template <typename T, size_t BlockSize = 64>
class ObjectPool {
public:
    static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
    static_assert(std::is_destructible_v<T>, "T must be destructible");

    ObjectPool() = default;
    ~ObjectPool() {
        // 确保所有对象都已归还后再销毁
        clear();
    }

    // 禁止拷贝
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    // 禁止移动（避免地址失效问题）
    ObjectPool(ObjectPool&& other) noexcept = delete;
    ObjectPool& operator=(ObjectPool&& other) noexcept = delete;

    /**
     * @brief 分配一个对象
     * @return 指向分配的对象的指针
     */
    T* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);

        // 检查对象池是否已销毁
        if (isDestroyed_) {
            return nullptr;
        }

        if (freeList_.empty()) {
            grow();
        }

        T* obj = freeList_.back();
        freeList_.pop_back();

        // 在原地构造对象
        new (obj) T();

        allocatedCount_++;
        return obj;
    }

    /**
     * @brief 分配并构造一个对象（带参数）
     */
    template <typename... Args>
    T* allocate(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);

        // 检查对象池是否已销毁
        if (isDestroyed_) {
            return nullptr;
        }

        if (freeList_.empty()) {
            grow();
        }

        T* obj = freeList_.back();
        freeList_.pop_back();

        // 在原地构造对象
        new (obj) T(std::forward<Args>(args)...);

        allocatedCount_++;
        return obj;
    }

    /**
     * @brief 回收一个对象
     * @param obj 要回收的对象指针
     * @return true 如果对象成功回收到池中，false 如果池已销毁或对象无效
     */
    bool deallocate(T* obj) {
        if (obj == nullptr) {
            return false;
        }

        // 调用析构函数
        obj->~T();

        std::lock_guard<std::mutex> lock(mutex_);
        // 检查对象池是否还在运行（避免在对象池销毁后访问）
        if (!isDestroyed_) {
            freeList_.push_back(obj);
            allocatedCount_--;
            return true;
        }
        return false;
    }

    /**
     * @brief 获取当前已分配的对象数量
     */
    size_t allocatedCount() const {
        return allocatedCount_.load();
    }

    /**
     * @brief 获取池中可用的对象数量
     */
    size_t availableCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return freeList_.size();
    }

    /**
     * @brief 获取池中总的对象容量
     */
    size_t capacity() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return blocks_.size() * BlockSize;
    }

    /**
     * @brief 清空所有内存块
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);

        isDestroyed_ = true;

        // 释放所有内存块
        for (auto& block : blocks_) {
            // 使用与分配时匹配的释放方式
            ::operator delete[](block);
        }
        blocks_.clear();
        freeList_.clear();
        allocatedCount_ = 0;
    }

private:
    /**
     * @brief 扩展池容量
     */
    void grow() {
        // 分配新的内存块（使用标准对齐）
        T* block = static_cast<T*>(::operator new[](sizeof(T) * BlockSize));

        blocks_.push_back(block);

        // 将新块中的对象添加到空闲列表
        for (size_t i = 0; i < BlockSize; ++i) {
            freeList_.push_back(&block[i]);
        }
    }

    mutable std::mutex mutex_;
    std::vector<T*> blocks_;        // 内存块列表
    std::vector<T*> freeList_;      // 空闲对象列表
    std::atomic<size_t> allocatedCount_{0};
    bool isDestroyed_ = false;      // 标记对象池是否已销毁
};

// ============================================================================
// 智能指针支持的内存池分配器
// ============================================================================

template <typename T, size_t BlockSize = 64>
class PooledAllocator {
public:
    using PoolType = ObjectPool<T, BlockSize>;

    PooledAllocator() : pool_(std::make_shared<PoolType>()) {}
    explicit PooledAllocator(std::shared_ptr<PoolType> pool) : pool_(pool) {}

    /**
     * @brief 创建一个使用内存池的对象
     */
    template <typename... Args>
    Ptr<T> makeShared(Args&&... args) {
        // 使用 weak_ptr 避免循环引用
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
            // 尝试获取 pool
            if (auto sharedPool = pool.lock()) {
                // 尝试归还给对象池
                if (!sharedPool->deallocate(obj)) {
                    // 对象池已销毁，手动释放内存
                    ::operator delete[](obj);
                }
            } else {
                // Pool 已被销毁，释放内存
                ::operator delete[](obj);
            }
        }
    };

    std::shared_ptr<PoolType> pool_;
};

// ============================================================================
// 全局内存池管理器
// ============================================================================

class ObjectPoolManager {
public:
    static ObjectPoolManager& getInstance();

    /**
     * @brief 获取指定类型的内存池
     */
    template <typename T, size_t BlockSize = 64>
    std::shared_ptr<ObjectPool<T, BlockSize>> getPool() {
        // 使用静态局部变量确保延迟初始化和正确析构顺序
        static auto pool = std::make_shared<ObjectPool<T, BlockSize>>();
        return pool;
    }

    /**
     * @brief 创建使用内存池的对象
     * 注意：返回的对象必须在程序退出前手动释放，否则可能导致悬挂引用
     */
    template <typename T, size_t BlockSize = 64, typename... Args>
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
                    // 对象池已销毁
                    ::operator delete[](p);
                }
            } else {
                // Pool 已被销毁
                ::operator delete[](p);
            }
        });
    }

    /**
     * @brief 清理所有未使用的内存池资源
     * 在程序退出前调用，确保资源正确释放
     */
    void cleanup();

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

#define E2D_MAKE_POOSED(T, ...) \
    extra2d::ObjectPoolManager::getInstance().makePooled<T>(__VA_ARGS__)

} // namespace extra2d
