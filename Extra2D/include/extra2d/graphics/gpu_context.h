#pragma once

#include <atomic>

namespace extra2d {

// ============================================================================
// GPU 上下文状态管理器
// 用于跟踪 OpenGL/Vulkan 等 GPU 上下文的生命周期状态
// 确保在 GPU 资源析构时能安全地检查上下文是否有效
// ============================================================================

class GPUContext {
public:
    /// 获取单例实例
    static GPUContext& getInstance();

    /// 标记 GPU 上下文为有效（在初始化完成后调用）
    void markValid();

    /// 标记 GPU 上下文为无效（在销毁前调用）
    void markInvalid();

    /// 检查 GPU 上下文是否有效
    bool isValid() const;

private:
    GPUContext() = default;
    ~GPUContext() = default;
    GPUContext(const GPUContext&) = delete;
    GPUContext& operator=(const GPUContext&) = delete;

    std::atomic<bool> valid_{false};
};

} // namespace extra2d
