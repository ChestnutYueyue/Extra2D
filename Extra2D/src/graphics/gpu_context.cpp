#include <extra2d/graphics/gpu_context.h>

namespace extra2d {

GPUContext& GPUContext::getInstance() {
    static GPUContext instance;
    return instance;
}

void GPUContext::markValid() {
    valid_.store(true, std::memory_order_release);
}

void GPUContext::markInvalid() {
    valid_.store(false, std::memory_order_release);
}

bool GPUContext::isValid() const {
    return valid_.load(std::memory_order_acquire);
}

} // namespace extra2d
