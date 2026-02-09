#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/graphics/texture_pool.h>
#include <extra2d/utils/logger.h>
#include <stb/stb_image.h>

namespace extra2d {

// ============================================================================
// TexturePool实现
// ============================================================================

TexturePool &TexturePool::getInstance() {
  static TexturePool instance;
  return instance;
}

bool TexturePool::init(const TexturePoolConfig &config) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (initialized_) {
    E2D_WARN("纹理池已经初始化");
    return true;
  }

  config_ = config;

  E2D_INFO("初始化纹理池...");
  E2D_INFO("  最大缓存大小: {} MB", config_.maxCacheSize / (1024 * 1024));
  E2D_INFO("  最大纹理数量: {}", config_.maxTextureCount);
  E2D_INFO("  自动清理间隔: {} 秒", config_.unloadInterval);
  E2D_INFO("  异步加载: {}", config_.enableAsyncLoad ? "启用" : "禁用");

  initialized_ = true;
  return true;
}

void TexturePool::shutdown() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!initialized_) {
    return;
  }

  E2D_INFO("关闭纹理池...");

  printStats();

  clear();

  initialized_ = false;
}

Ptr<Texture> TexturePool::get(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!initialized_) {
    E2D_ERROR("纹理池未初始化");
    return nullptr;
  }

  // 检查缓存
  auto it = cache_.find(filepath);
  if (it != cache_.end()) {
    // 缓存命中
    touch(filepath);
    hitCount_++;
    it->second.accessCount++;
    return it->second.texture;
  }

  // 缓存未命中，加载纹理
  missCount_++;

  auto texture = loadTexture(filepath);
  if (!texture) {
    return nullptr;
  }

  // 添加到缓存
  add(filepath, texture);

  return texture;
}

void TexturePool::getAsync(const std::string &filepath,
                           std::function<void(Ptr<Texture>)> callback) {
  if (!initialized_) {
    E2D_ERROR("纹理池未初始化");
    if (callback) {
      callback(nullptr);
    }
    return;
  }

  if (!config_.enableAsyncLoad) {
    // 异步加载禁用，直接同步加载
    auto texture = get(filepath);
    if (callback) {
      callback(texture);
    }
    return;
  }

  // 添加到异步任务队列
  std::lock_guard<std::mutex> lock(mutex_);
  asyncTasks_.push_back({filepath, callback});
}

Ptr<Texture> TexturePool::createFromData(const std::string &name,
                                         const uint8_t *data, int width,
                                         int height, PixelFormat format) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!initialized_) {
    E2D_ERROR("纹理池未初始化");
    return nullptr;
  }

  // 检查缓存
  auto it = cache_.find(name);
  if (it != cache_.end()) {
    touch(name);
    return it->second.texture;
  }

  // 创建纹理
  int channels = 4;
  switch (format) {
  case PixelFormat::R8:
    channels = 1;
    break;
  case PixelFormat::RG8:
    channels = 2;
    break;
  case PixelFormat::RGB8:
    channels = 3;
    break;
  case PixelFormat::RGBA8:
    channels = 4;
    break;
  default:
    channels = 4;
    break;
  }
  auto texture = makePtr<GLTexture>(width, height, data, channels);
  if (!texture || !texture->isValid()) {
    E2D_ERROR("创建纹理失败: {}", name);
    return nullptr;
  }

  // 添加到缓存
  add(name, texture);

  E2D_INFO("从数据创建纹理: {} ({}x{})", name, width, height);

  return texture;
}

void TexturePool::add(const std::string &key, Ptr<Texture> texture) {
  if (!texture || !texture->isValid()) {
    return;
  }

  // 检查是否需要清理
  while (totalSize_ >= config_.maxCacheSize ||
         cache_.size() >= config_.maxTextureCount) {
    evict();
  }

  // 计算纹理大小
  size_t size = calculateTextureSize(texture->getWidth(), texture->getHeight(),
                                     texture->getFormat());

  // 创建缓存项
  CacheEntry entry;
  entry.texture = texture;
  entry.size = size;
  entry.lastAccessTime = 0.0f; // 将在touch中更新
  entry.accessCount = 0;

  // 添加到缓存
  cache_[key] = entry;

  // 添加到LRU列表头部
  lruList_.push_front(key);
  lruMap_[key] = lruList_.begin();

  totalSize_ += size;

  E2D_DEBUG_LOG("纹理添加到缓存: {} ({} KB)", key, size / 1024);
}

void TexturePool::remove(const std::string &key) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = cache_.find(key);
  if (it == cache_.end()) {
    return;
  }

  // 从LRU列表移除
  auto lruIt = lruMap_.find(key);
  if (lruIt != lruMap_.end()) {
    lruList_.erase(lruIt->second);
    lruMap_.erase(lruIt);
  }

  // 更新总大小
  totalSize_ -= it->second.size;

  // 从缓存移除
  cache_.erase(it);

  E2D_DEBUG_LOG("纹理从缓存移除: {}", key);
}

bool TexturePool::has(const std::string &key) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return cache_.find(key) != cache_.end();
}

void TexturePool::clear() {
  std::lock_guard<std::mutex> lock(mutex_);

  cache_.clear();
  lruMap_.clear();
  lruList_.clear();
  totalSize_ = 0;

  E2D_INFO("纹理缓存已清空");
}

void TexturePool::trim(size_t targetSize) {
  std::lock_guard<std::mutex> lock(mutex_);

  while (totalSize_ > targetSize && !lruList_.empty()) {
    evict();
  }

  E2D_INFO("纹理缓存已清理到 {} MB", totalSize_ / (1024 * 1024));
}

size_t TexturePool::getTextureCount() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return cache_.size();
}

size_t TexturePool::getCacheSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return totalSize_;
}

float TexturePool::getHitRate() const {
  std::lock_guard<std::mutex> lock(mutex_);

  uint64_t total = hitCount_ + missCount_;
  if (total == 0) {
    return 0.0f;
  }

  return static_cast<float>(hitCount_) / static_cast<float>(total);
}

void TexturePool::printStats() const {
  std::lock_guard<std::mutex> lock(mutex_);

  E2D_INFO("纹理池统计:");
  E2D_INFO("  缓存纹理数: {}/{}", cache_.size(), config_.maxTextureCount);
  E2D_INFO("  缓存大小: {} / {} MB", totalSize_ / (1024 * 1024),
           config_.maxCacheSize / (1024 * 1024));
  E2D_INFO("  缓存命中: {}", hitCount_);
  E2D_INFO("  缓存未命中: {}", missCount_);
  E2D_INFO("  命中率: {:.1f}%", getHitRate() * 100.0f);
}

void TexturePool::update(float dt) {
  if (!initialized_) {
    return;
  }

  // 处理异步任务
  if (config_.enableAsyncLoad) {
    processAsyncTasks();
  }

  // 自动清理
  autoUnloadTimer_ += dt;
  if (autoUnloadTimer_ >= config_.unloadInterval) {
    autoUnloadTimer_ = 0.0f;

    // 如果缓存超过80%，清理到50%
    if (totalSize_ > config_.maxCacheSize * 0.8f) {
      trim(config_.maxCacheSize * 0.5f);
    }
  }
}

void TexturePool::setAutoUnloadInterval(float interval) {
  config_.unloadInterval = interval;
}

// ============================================================================
// 内部方法
// ============================================================================

void TexturePool::touch(const std::string &key) {
  auto it = lruMap_.find(key);
  if (it == lruMap_.end()) {
    return;
  }

  // 移动到LRU列表头部
  lruList_.splice(lruList_.begin(), lruList_, it->second);

  // 更新时间戳
  auto cacheIt = cache_.find(key);
  if (cacheIt != cache_.end()) {
    cacheIt->second.lastAccessTime = 0.0f;
  }
}

void TexturePool::evict() {
  if (lruList_.empty()) {
    return;
  }

  // 移除LRU列表末尾的项（最久未使用）
  auto key = lruList_.back();

  auto it = cache_.find(key);
  if (it != cache_.end()) {
    totalSize_ -= it->second.size;
    cache_.erase(it);
  }

  lruMap_.erase(key);
  lruList_.pop_back();

  E2D_DEBUG_LOG("纹理被清理出缓存: {}", key);
}

Ptr<Texture> TexturePool::loadTexture(const std::string &filepath) {
  // 使用stb_image加载图像
  // 不翻转图片，保持原始方向
  // OpenGL纹理坐标原点在左下角，图片数据原点在左上角
  // 在渲染时通过纹理坐标翻转来处理
  stbi_set_flip_vertically_on_load(false);
  int width, height, channels;
  unsigned char *pixels =
      stbi_load(filepath.c_str(), &width, &height, &channels, 4);
  if (!pixels) {
    E2D_ERROR("stbi_load 失败: {} - {}", filepath, stbi_failure_reason());
    return nullptr;
  }

  // 创建GLTexture（直接使用像素数据）
  auto texture = std::make_shared<GLTexture>(width, height, pixels, 4);
  if (!texture || !texture->isValid()) {
    stbi_image_free(pixels);
    E2D_ERROR("创建纹理失败: {}", filepath);
    return nullptr;
  }

  stbi_image_free(pixels);

  E2D_INFO("加载纹理: {} ({}x{}, {} channels)", filepath, width, height,
           channels);

  return texture;
}

size_t TexturePool::calculateTextureSize(int width, int height,
                                         PixelFormat format) {
  // 压缩纹理格式使用块大小计算
  switch (format) {
  case PixelFormat::ETC2_RGB8: {
    size_t blocksWide = (width + 3) / 4;
    size_t blocksHigh = (height + 3) / 4;
    return blocksWide * blocksHigh * 8;
  }
  case PixelFormat::ETC2_RGBA8: {
    size_t blocksWide = (width + 3) / 4;
    size_t blocksHigh = (height + 3) / 4;
    return blocksWide * blocksHigh * 16;
  }
  case PixelFormat::ASTC_4x4: {
    size_t blocksWide = (width + 3) / 4;
    size_t blocksHigh = (height + 3) / 4;
    return blocksWide * blocksHigh * 16;
  }
  case PixelFormat::ASTC_6x6: {
    size_t blocksWide = (width + 5) / 6;
    size_t blocksHigh = (height + 5) / 6;
    return blocksWide * blocksHigh * 16;
  }
  case PixelFormat::ASTC_8x8: {
    size_t blocksWide = (width + 7) / 8;
    size_t blocksHigh = (height + 7) / 8;
    return blocksWide * blocksHigh * 16;
  }
  default:
    break;
  }

  // 非压缩格式
  int bytesPerPixel = 4;
  switch (format) {
  case PixelFormat::R8:
    bytesPerPixel = 1;
    break;
  case PixelFormat::RG8:
    bytesPerPixel = 2;
    break;
  case PixelFormat::RGB8:
    bytesPerPixel = 3;
    break;
  case PixelFormat::RGBA8:
    bytesPerPixel = 4;
    break;
  default:
    bytesPerPixel = 4;
    break;
  }

  return static_cast<size_t>(width * height * bytesPerPixel);
}

void TexturePool::processAsyncTasks() {
  std::vector<AsyncLoadTask> tasks;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks = std::move(asyncTasks_);
    asyncTasks_.clear();
  }

  for (auto &task : tasks) {
    auto texture = get(task.filepath);
    if (task.callback) {
      task.callback(texture);
    }
  }
}

} // namespace extra2d
