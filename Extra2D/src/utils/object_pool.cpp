#include <extra2d/utils/object_pool.h>

namespace extra2d {

// ObjectPoolManager 单例实现
ObjectPoolManager& ObjectPoolManager::getInstance() {
    static ObjectPoolManager instance;
    return instance;
}

void ObjectPoolManager::cleanup() {
    // 静态对象池会在程序退出时自动清理
    // 这个方法用于显式触发清理（如果需要）
    // 由于使用了 weak_ptr，循环引用问题已解决
}

} // namespace extra2d
