#include <extra2d/graphics/gpu_context.h>

namespace extra2d {

/**
 * @brief 获取GPUContext单例实例
 * @return GPUContext单例的引用
 *
 * 使用静态局部变量实现线程安全的单例模式
 */
GPUContext& GPUContext::get() {
    static GPUContext instance;
    return instance;
}

/**
 * @brief 标记GPU上下文为有效状态
 *
 * 使用原子操作设置有效标志为true，使用release内存序
 */
void GPUContext::markValid() {
    valid_.store(true, std::memory_order_release);
}

/**
 * @brief 标记GPU上下文为无效状态
 *
 * 使用原子操作设置有效标志为false，使用release内存序
 */
void GPUContext::markInvalid() {
    valid_.store(false, std::memory_order_release);
}

/**
 * @brief 检查GPU上下文是否有效
 * @return 如果GPU上下文有效返回true，否则返回false
 *
 * 使用原子操作读取有效标志，使用acquire内存序
 */
bool GPUContext::isValid() const {
    return valid_.load(std::memory_order_acquire);
}

} // namespace extra2d
