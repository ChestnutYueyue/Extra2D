#include <algorithm>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

// Switch 推荐 VRAM 预算 ~400MB
static constexpr size_t DEFAULT_VRAM_BUDGET = 400 * 1024 * 1024;

VRAMManager::VRAMManager()
    : textureVRAM_(0), bufferVRAM_(0), vramBudget_(DEFAULT_VRAM_BUDGET),
      textureAllocCount_(0), textureFreeCount_(0), bufferAllocCount_(0),
      bufferFreeCount_(0), peakTextureVRAM_(0), peakBufferVRAM_(0) {}

VRAMManager &VRAMManager::getInstance() {
  static VRAMManager instance;
  return instance;
}

void VRAMManager::allocTexture(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  textureVRAM_ += size;
  textureAllocCount_++;
  peakTextureVRAM_ = std::max(peakTextureVRAM_, textureVRAM_);

  if (isOverBudget()) {
    E2D_LOG_WARN("VRAM over budget! Used: {} MB / Budget: {} MB",
                 getUsedVRAM() / (1024 * 1024), vramBudget_ / (1024 * 1024));
  }
}

void VRAMManager::freeTexture(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (size <= textureVRAM_) {
    textureVRAM_ -= size;
  } else {
    textureVRAM_ = 0;
  }
  textureFreeCount_++;
}

void VRAMManager::allocBuffer(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  bufferVRAM_ += size;
  bufferAllocCount_++;
  peakBufferVRAM_ = std::max(peakBufferVRAM_, bufferVRAM_);

  if (isOverBudget()) {
    E2D_LOG_WARN("VRAM over budget! Used: {} MB / Budget: {} MB",
                 getUsedVRAM() / (1024 * 1024), vramBudget_ / (1024 * 1024));
  }
}

void VRAMManager::freeBuffer(size_t size) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (size <= bufferVRAM_) {
    bufferVRAM_ -= size;
  } else {
    bufferVRAM_ = 0;
  }
  bufferFreeCount_++;
}

size_t VRAMManager::getUsedVRAM() const { return textureVRAM_ + bufferVRAM_; }

size_t VRAMManager::getTextureVRAM() const { return textureVRAM_; }

size_t VRAMManager::getBufferVRAM() const { return bufferVRAM_; }

size_t VRAMManager::getAvailableVRAM() const {
  size_t used = getUsedVRAM();
  return (used < vramBudget_) ? (vramBudget_ - used) : 0;
}

void VRAMManager::setVRAMBudget(size_t budget) {
  std::lock_guard<std::mutex> lock(mutex_);
  vramBudget_ = budget;
  E2D_LOG_INFO("VRAM budget set to {} MB", budget / (1024 * 1024));
}

size_t VRAMManager::getVRAMBudget() const { return vramBudget_; }

bool VRAMManager::isOverBudget() const { return getUsedVRAM() > vramBudget_; }

void VRAMManager::printStats() const {
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

void VRAMManager::reset() {
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
