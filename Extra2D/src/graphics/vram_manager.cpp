#include <algorithm>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

// Switch 推荐 VRAM 预算 ~400MB
static constexpr size_t DEFAULT_VRAM_BUDGET = 400 * 1024 * 1024;

/**
 * @brief 默认构造函数
 *
 * 初始化VRAM管理器，设置默认预算为400MB
 */
VRAMMgr::VRAMMgr()
    : textureVRAM_(0), bufferVRAM_(0), vramBudget_(DEFAULT_VRAM_BUDGET),
      textureAllocCount_(0), textureFreeCount_(0), bufferAllocCount_(0),
      bufferFreeCount_(0), peakTextureVRAM_(0), peakBufferVRAM_(0) {}

/**
 * @brief 获取VRAMMgr单例实例
 * @return VRAMMgr单例的引用
 *
 * 使用静态局部变量实现线程安全的单例模式
 */
VRAMMgr &VRAMMgr::get() {
  static VRAMMgr instance;
  return instance;
}

/**
 * @brief 分配纹理VRAM
 * @param size 分配的字节数
 *
 * 增加纹理VRAM使用量并更新峰值，如果超出预算则输出警告
 */
void VRAMMgr::allocTexture(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  textureVRAM_ += size;
  textureAllocCount_++;
  peakTextureVRAM_ = std::max(peakTextureVRAM_, textureVRAM_);

  if (isOverBudget()) {
    E2D_LOG_WARN("VRAM over budget! Used: {} MB / Budget: {} MB",
                 getUsedVRAM() / (1024 * 1024), vramBudget_ / (1024 * 1024));
  }
}

/**
 * @brief 释放纹理VRAM
 * @param size 释放的字节数
 *
 * 减少纹理VRAM使用量
 */
void VRAMMgr::freeTexture(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (size <= textureVRAM_) {
    textureVRAM_ -= size;
  } else {
    textureVRAM_ = 0;
  }
  textureFreeCount_++;
}

/**
 * @brief 分配缓冲VRAM
 * @param size 分配的字节数
 *
 * 增加缓冲VRAM使用量并更新峰值，如果超出预算则输出警告
 */
void VRAMMgr::allocBuffer(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  bufferVRAM_ += size;
  bufferAllocCount_++;
  peakBufferVRAM_ = std::max(peakBufferVRAM_, bufferVRAM_);

  if (isOverBudget()) {
    E2D_LOG_WARN("VRAM over budget! Used: {} MB / Budget: {} MB",
                 getUsedVRAM() / (1024 * 1024), vramBudget_ / (1024 * 1024));
  }
}

/**
 * @brief 释放缓冲VRAM
 * @param size 释放的字节数
 *
 * 减少缓冲VRAM使用量
 */
void VRAMMgr::freeBuffer(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (size <= bufferVRAM_) {
    bufferVRAM_ -= size;
  } else {
    bufferVRAM_ = 0;
  }
  bufferFreeCount_++;
}

/**
 * @brief 获取总VRAM使用量
 * @return 总使用字节数
 *
 * 返回纹理和缓冲VRAM使用量的总和
 */
size_t VRAMMgr::getUsedVRAM() const { return textureVRAM_ + bufferVRAM_; }

/**
 * @brief 获取纹理VRAM使用量
 * @return 纹理使用字节数
 */
size_t VRAMMgr::getTextureVRAM() const { return textureVRAM_; }

/**
 * @brief 获取缓冲VRAM使用量
 * @return 缓冲使用字节数
 */
size_t VRAMMgr::getBufferVRAM() const { return bufferVRAM_; }

/**
 * @brief 获取可用VRAM
 * @return 可用字节数
 *
 * 返回预算内剩余的VRAM空间
 */
size_t VRAMMgr::getAvailableVRAM() const {
  size_t used = getUsedVRAM();
  return (used < vramBudget_) ? (vramBudget_ - used) : 0;
}

/**
 * @brief 设置VRAM预算
 * @param budget 预算字节数
 *
 * 设置VRAM使用上限
 */
void VRAMMgr::setVRAMBudget(size_t budget) {
  std::lock_guard<std::mutex> lock(mutex_);
  vramBudget_ = budget;
  E2D_LOG_INFO("VRAM budget set to {} MB", budget / (1024 * 1024));
}

/**
 * @brief 获取VRAM预算
 * @return 预算字节数
 */
size_t VRAMMgr::getVRAMBudget() const { return vramBudget_; }

/**
 * @brief 检查是否超出预算
 * @return 超出预算返回true，否则返回false
 */
bool VRAMMgr::isOverBudget() const { return getUsedVRAM() > vramBudget_; }

/**
 * @brief 打印VRAM统计信息
 *
 * 输出纹理和缓冲的VRAM使用情况、峰值和分配/释放次数
 */
void VRAMMgr::printStats() const {
  std::lock_guard<std::mutex> lock(mutex_);
  E2D_LOG_INFO("=== VRAM Stats ===");
  E2D_LOG_INFO("  Texture VRAM: {} MB (peak: {} MB)",
               textureVRAM_ / (1024 * 1024), peakTextureVRAM_ / (1024 * 1024));
  E2D_LOG_INFO("  Buffer VRAM:  {} MB (peak: {} MB)",
               bufferVRAM_ / (1024 * 1024), peakBufferVRAM_ / (1024 * 1024));
  E2D_LOG_INFO("  Total Used:   {} MB / {} MB budget",
               (textureVRAM_ + bufferVRAM_) / (1024 * 1024),
               vramBudget_ / (1024 * 1024));
  E2D_LOG_INFO("  Texture allocs/frees: {} / {}", textureAllocCount_,
               textureFreeCount_);
  E2D_LOG_INFO("  Buffer allocs/frees:  {} / {}", bufferAllocCount_,
               bufferFreeCount_);
}

/**
 * @brief 重置所有统计信息
 *
 * 清零所有VRAM计数器和峰值记录
 */
void VRAMMgr::reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  textureVRAM_ = 0;
  bufferVRAM_ = 0;
  textureAllocCount_ = 0;
  textureFreeCount_ = 0;
  bufferAllocCount_ = 0;
  bufferFreeCount_ = 0;
  peakTextureVRAM_ = 0;
  peakBufferVRAM_ = 0;
}

} // namespace extra2d
