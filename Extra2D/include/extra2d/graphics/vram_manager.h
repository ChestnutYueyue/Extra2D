#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>

namespace extra2d {

// ============================================================================
// VRAM 管理器 - 跟踪显存使用情况
// ============================================================================
class VRAMManager {
public:
    static VRAMManager& getInstance();

    // 纹理显存跟踪
    void allocTexture(size_t size);
    void freeTexture(size_t size);

    // VBO/FBO 显存跟踪
    void allocBuffer(size_t size);
    void freeBuffer(size_t size);

    // 查询显存使用情况
    size_t getUsedVRAM() const;
    size_t getTextureVRAM() const;
    size_t getBufferVRAM() const;
    size_t getAvailableVRAM() const;

    // 显存预算管理
    void setVRAMBudget(size_t budget);
    size_t getVRAMBudget() const;
    bool isOverBudget() const;

    // 统计信息
    void printStats() const;

    // 重置计数器
    void reset();

private:
    VRAMManager();
    ~VRAMManager() = default;
    VRAMManager(const VRAMManager&) = delete;
    VRAMManager& operator=(const VRAMManager&) = delete;

    mutable std::mutex mutex_;

    size_t textureVRAM_;
    size_t bufferVRAM_;
    size_t vramBudget_;

    // 统计
    uint32_t textureAllocCount_;
    uint32_t textureFreeCount_;
    uint32_t bufferAllocCount_;
    uint32_t bufferFreeCount_;
    size_t peakTextureVRAM_;
    size_t peakBufferVRAM_;
};

} // namespace extra2d
