#include <extra2d/utils/object_pool.h>

namespace extra2d {

// ObjectPoolManager 单例实现
ObjectPoolManager& ObjectPoolManager::getInstance() {
    static ObjectPoolManager instance;
    return instance;
}

} // namespace extra2d
