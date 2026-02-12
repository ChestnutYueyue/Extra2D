#pragma once

#include <extra2d/animation/sprite_frame.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/resource/resource_manager.h>
#include <extra2d/utils/logger.h>
#include <mutex>
#include <string>
#include <unordered_map>

namespace extra2d {

// ============================================================================
// SpriteFrameCache - 精灵帧全局缓存（借鉴 Cocos SpriteFrameCache）
// 全局单例管理所有精灵帧，避免重复创建，支持图集自动切割
// ============================================================================
class SpriteFrameCache {
public:
  static SpriteFrameCache &getInstance() {
    static SpriteFrameCache instance;
    return instance;
  }

  // ------ 添加帧 ------

  /// 添加单个精灵帧
  void addSpriteFrame(Ptr<SpriteFrame> frame, const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);
    frames_[name] = std::move(frame);
  }

  /// 从纹理和矩形区域创建并添加帧
  void addSpriteFrameFromTexture(Ptr<Texture> texture, const Rect &rect,
                                 const std::string &name) {
    auto frame = SpriteFrame::create(std::move(texture), rect);
    frame->setName(name);
    addSpriteFrame(std::move(frame), name);
  }

  /// 从纹理图集批量切割添加（等宽等高网格）
  void addSpriteFramesFromGrid(const std::string &texturePath, int frameWidth,
                               int frameHeight, int frameCount = -1,
                               int spacing = 0, int margin = 0) {
    auto texture = loadTextureFromFile(texturePath);
    if (!texture)
      return;
    addSpriteFramesFromGrid(texture, texturePath, frameWidth, frameHeight,
                            frameCount, spacing, margin);
  }

  /// 从纹理对象批量切割添加（等宽等高网格，无需走 TexturePool）
  void addSpriteFramesFromGrid(Ptr<Texture> texture,
                               const std::string &keyPrefix, int frameWidth,
                               int frameHeight, int frameCount = -1,
                               int spacing = 0, int margin = 0) {
    if (!texture)
      return;

    int texW = texture->getWidth();
    int texH = texture->getHeight();
    int usableW = texW - 2 * margin;
    int usableH = texH - 2 * margin;
    int cols = (usableW + spacing) / (frameWidth + spacing);
    int rows = (usableH + spacing) / (frameHeight + spacing);
    int total = (frameCount > 0) ? frameCount : cols * rows;

    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < total; ++i) {
      int col = i % cols;
      int row = i / cols;
      if (row >= rows)
        break;

      Rect rect(static_cast<float>(margin + col * (frameWidth + spacing)),
                static_cast<float>(margin + row * (frameHeight + spacing)),
                static_cast<float>(frameWidth),
                static_cast<float>(frameHeight));

      std::string name = keyPrefix + "#" + std::to_string(i);
      auto frame = SpriteFrame::create(texture, rect);
      frame->setName(name);
      frames_[name] = std::move(frame);
    }
  }

  // ------ 获取帧 ------

  /// 按名称获取
  Ptr<SpriteFrame> getSpriteFrame(const std::string &name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = frames_.find(name);
    if (it != frames_.end())
      return it->second;
    return nullptr;
  }

  /// 通过路径+索引获取或创建（ANI 格式的定位方式）
  Ptr<SpriteFrame> getOrCreateFromFile(const std::string &texturePath,
                                       int index = 0) {
    std::string key = texturePath + "#" + std::to_string(index);

    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = frames_.find(key);
      if (it != frames_.end())
        return it->second;
    }

    // 缓存未命中，加载纹理并创建 SpriteFrame
    auto texture = loadTextureFromFile(texturePath);
    if (!texture)
      return nullptr;

    // 默认整张纹理作为一帧（index=0），或用整张纹理
    Rect rect(0.0f, 0.0f, static_cast<float>(texture->getWidth()),
              static_cast<float>(texture->getHeight()));

    auto frame = SpriteFrame::create(texture, rect);
    frame->setName(key);

    std::lock_guard<std::mutex> lock(mutex_);
    frames_[key] = frame;
    return frame;
  }

  // ------ 缓存管理 ------

  bool has(const std::string &name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_.find(name) != frames_.end();
  }

  void removeSpriteFrame(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);
    frames_.erase(name);
  }

  /// 移除未被外部引用的精灵帧（use_count == 1 表示仅缓存自身持有）
  void removeUnusedSpriteFrames() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = frames_.begin(); it != frames_.end();) {
      if (it->second.use_count() == 1) {
        it = frames_.erase(it);
      } else {
        ++it;
      }
    }
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    frames_.clear();
  }

  size_t count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_.size();
  }

private:
  SpriteFrameCache() = default;
  ~SpriteFrameCache() = default;
  SpriteFrameCache(const SpriteFrameCache &) = delete;
  SpriteFrameCache &operator=(const SpriteFrameCache &) = delete;

  /**
   * @brief 从文件加载纹理（使用 ResourceManager 的 LRU 缓存）
   * @param filepath 纹理文件路径
   * @return 纹理对象，失败返回nullptr
   */
  Ptr<Texture> loadTextureFromFile(const std::string &filepath) {
    // 使用 ResourceManager 的纹理缓存机制
    // 这样可以享受 LRU 缓存、自动清理和缓存统计等功能
    auto &resources = ResourceManager::getInstance();
    
    // 先检查缓存中是否已有该纹理
    auto texture = resources.getTexture(filepath);
    if (texture) {
      E2D_TRACE("SpriteFrameCache: 使用缓存纹理: {}", filepath);
      return texture;
    }
    
    // 缓存未命中，通过 ResourceManager 加载
    texture = resources.loadTexture(filepath);
    if (!texture) {
      E2D_ERROR("SpriteFrameCache: 加载纹理失败: {}", filepath);
      return nullptr;
    }
    
    E2D_TRACE("SpriteFrameCache: 加载新纹理: {}", filepath);
    return texture;
  }

  mutable std::mutex mutex_;
  std::unordered_map<std::string, Ptr<SpriteFrame>> frames_;
};

// 便捷宏
#define E2D_SPRITE_FRAME_CACHE() ::extra2d::SpriteFrameCache::getInstance()

} // namespace extra2d
