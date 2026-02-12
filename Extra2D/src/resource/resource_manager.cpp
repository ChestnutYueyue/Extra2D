#include <algorithm>
#include <cstring>
#include <extra2d/audio/audio_engine.h>
#include <extra2d/graphics/opengl/gl_font_atlas.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/resource/resource_manager.h>
#include <extra2d/utils/logger.h>
#include <sys/stat.h>

// Windows 平台需要包含的头文件
#ifdef _WIN32
#include <windows.h>
#endif

namespace extra2d {

// 辅助函数：检查文件是否存在
static bool fileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

// 辅助函数：检查是否是 romfs 路径
static bool isRomfsPath(const std::string &path) {
  return path.find("romfs:/") == 0 || path.find("romfs:\\") == 0;
}

// 辅助函数：获取可执行文件所在目录（Windows 平台）
#ifdef _WIN32
static std::string getExecutableDirectory() {
  char buffer[MAX_PATH];
  DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
  if (len > 0 && len < MAX_PATH) {
    std::string exePath(buffer, len);
    size_t lastSlash = exePath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
      return exePath.substr(0, lastSlash);
    }
  }
  return "";
}
#endif

// 解析资源路径（优先尝试 romfs:/ 前缀，然后 sdmc:/，最后尝试相对于可执行文件的路径）
static std::string resolveResourcePath(const std::string &filepath) {
  // 如果已经是 romfs 或 sdmc 路径，直接返回
  if (isRomfsPath(filepath) || filepath.find("sdmc:/") == 0) {
    return filepath;
  }

  // 优先尝试 romfs:/ 前缀的路径（Switch 平台）
  std::string romfsPath = "romfs:/" + filepath;
  if (fileExists(romfsPath)) {
    return romfsPath;
  }

  // 尝试 sdmc:/ 前缀的路径（Switch SD卡）
  std::string sdmcPath = "sdmc:/" + filepath;
  if (fileExists(sdmcPath)) {
    return sdmcPath;
  }

  // 如果都不存在，尝试原路径
  if (fileExists(filepath)) {
    return filepath;
  }

  // Windows 平台：尝试相对于可执行文件的路径
#ifdef _WIN32
  std::string exeDir = getExecutableDirectory();
  if (!exeDir.empty()) {
    std::string exeRelativePath = exeDir + "/" + filepath;
    if (fileExists(exeRelativePath)) {
      return exeRelativePath;
    }
  }
#endif

  return "";
}

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() {
  shutdownAsyncLoader();
}

ResourceManager &ResourceManager::getInstance() {
  static ResourceManager instance;
  return instance;
}

// ============================================================================
// 异步加载系统
// ============================================================================

void ResourceManager::initAsyncLoader() {
  if (asyncRunning_) {
    return;
  }
  
  asyncRunning_ = true;
  asyncThread_ = std::make_unique<std::thread>(&ResourceManager::asyncLoadLoop, this);
  E2D_LOG_INFO("ResourceManager: async loader initialized");
}

void ResourceManager::shutdownAsyncLoader() {
  if (!asyncRunning_) {
    return;
  }
  
  asyncRunning_ = false;
  asyncCondition_.notify_all();
  
  if (asyncThread_ && asyncThread_->joinable()) {
    asyncThread_->join();
  }
  
  E2D_LOG_INFO("ResourceManager: async loader shutdown");
}

void ResourceManager::waitForAsyncLoads() {
  while (pendingAsyncLoads_ > 0) {
    std::this_thread::yield();
  }
}

bool ResourceManager::hasPendingAsyncLoads() const {
  return pendingAsyncLoads_ > 0;
}

void ResourceManager::asyncLoadLoop() {
  while (asyncRunning_) {
    AsyncLoadTask task;
    
    {
      std::unique_lock<std::mutex> lock(asyncQueueMutex_);
      asyncCondition_.wait(lock, [this] { return !asyncTaskQueue_.empty() || !asyncRunning_; });
      
      if (!asyncRunning_) {
        break;
      }
      
      if (asyncTaskQueue_.empty()) {
        continue;
      }
      
      task = std::move(asyncTaskQueue_.front());
      asyncTaskQueue_.pop();
    }
    
    // 执行加载
    auto texture = loadTextureInternal(task.filepath, task.format);
    
    // 回调
    if (task.callback) {
      task.callback(texture);
    }
    
    // 设置 promise
    task.promise.set_value(texture);
    
    pendingAsyncLoads_--;
  }
}

// ============================================================================
// 纹理资源 - 同步加载
// ============================================================================

Ptr<Texture> ResourceManager::loadTexture(const std::string &filepath) {
  return loadTexture(filepath, false, TextureFormat::Auto);
}

Ptr<Texture> ResourceManager::loadTexture(const std::string &filepath, bool async) {
  return loadTexture(filepath, async, TextureFormat::Auto);
}

Ptr<Texture> ResourceManager::loadTexture(const std::string &filepath, bool async, TextureFormat format) {
  if (async) {
    // 异步加载：返回空指针，实际纹理通过回调获取
    loadTextureAsync(filepath, format, nullptr);
    return nullptr;
  }
  
  return loadTextureInternal(filepath, format);
}

void ResourceManager::loadTextureAsync(const std::string &filepath, TextureLoadCallback callback) {
  loadTextureAsync(filepath, TextureFormat::Auto, callback);
}

void ResourceManager::loadTextureAsync(const std::string &filepath, TextureFormat format, TextureLoadCallback callback) {
  // 确保异步加载系统已启动
  if (!asyncRunning_) {
    initAsyncLoader();
  }

  // 检查缓存
  {
    std::lock_guard<std::mutex> lock(textureMutex_);
    auto it = textureCache_.find(filepath);
    if (it != textureCache_.end()) {
      // 缓存命中，更新LRU并立即回调
      touchTexture(filepath);
      textureHitCount_++;
      it->second.accessCount++;
      if (callback) {
        callback(it->second.texture);
      }
      return;
    }
  }

  // 添加到异步任务队列
  AsyncLoadTask task;
  task.filepath = filepath;
  task.format = format;
  task.callback = callback;

  {
    std::lock_guard<std::mutex> lock(asyncQueueMutex_);
    asyncTaskQueue_.push(std::move(task));
  }

  pendingAsyncLoads_++;
  asyncCondition_.notify_one();

  E2D_LOG_DEBUG("ResourceManager: queued async texture load: {}", filepath);
}

Ptr<Texture> ResourceManager::loadTextureInternal(const std::string &filepath, TextureFormat format) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  // 检查缓存
  auto it = textureCache_.find(filepath);
  if (it != textureCache_.end()) {
    // 缓存命中
    touchTexture(filepath);
    textureHitCount_++;
    it->second.accessCount++;
    E2D_LOG_TRACE("ResourceManager: texture cache hit: {}", filepath);
    return it->second.texture;
  }

  // 缓存未命中
  textureMissCount_++;

  // 解析资源路径（优先尝试 romfs:/ 前缀）
  std::string fullPath = resolveResourcePath(filepath);
  if (fullPath.empty()) {
    E2D_LOG_ERROR("ResourceManager: texture file not found: {}", filepath);
    return nullptr;
  }

  // 创建新纹理
  try {
    auto texture = makePtr<GLTexture>(fullPath);
    if (!texture->isValid()) {
      E2D_LOG_ERROR("ResourceManager: failed to load texture: {}", filepath);
      return nullptr;
    }

    // 如果需要压缩，处理纹理格式
    if (format != TextureFormat::Auto && format != TextureFormat::RGBA8) {
      // 注意：实际压缩需要在纹理创建时处理
      // 这里仅记录日志，实际实现需要在 GLTexture 中支持
      E2D_LOG_DEBUG("ResourceManager: texture format {} requested for {}",
                    static_cast<int>(format), filepath);
    }

    // 检查是否需要清理缓存
    evictTexturesIfNeeded();

    // 计算纹理大小
    size_t textureSize = calculateTextureSize(texture->getWidth(), texture->getHeight(), texture->getFormat());

    // 分配LRU节点
    uint32_t lruIndex = allocateLRUNode(filepath);

    // 创建缓存项
    TextureCacheEntry entry;
    entry.texture = texture;
    entry.size = textureSize;
    entry.lastAccessTime = 0.0f;
    entry.accessCount = 1;

    // 添加到缓存
    textureCache_[filepath] = entry;
    totalTextureSize_ += textureSize;

    // 添加到LRU链表头部
    moveToFront(lruIndex);

    E2D_LOG_DEBUG("ResourceManager: loaded texture: {} ({} KB)", filepath, textureSize / 1024);
    return texture;
  } catch (...) {
    E2D_LOG_ERROR("ResourceManager: exception loading texture: {}", filepath);
    return nullptr;
  }
}

TextureFormat ResourceManager::selectBestFormat(TextureFormat requested) const {
  if (requested != TextureFormat::Auto) {
    return requested;
  }
  
  // 自动选择最佳格式
  // 检查支持的扩展
  // 这里简化处理，实际应该查询 OpenGL 扩展
  
  // 桌面平台优先 DXT
  // 移动平台优先 ETC2 或 ASTC
  
  // 默认返回 RGBA8
  return TextureFormat::RGBA8;
}

std::vector<uint8_t> ResourceManager::compressTexture(const uint8_t* data, int width, int height, 
                                                       int channels, TextureFormat format) {
  // 纹理压缩实现
  // 这里需要根据格式使用相应的压缩库
  // 如：squish (DXT), etcpack (ETC2), astc-encoder (ASTC)
  
  // 目前返回原始数据
  std::vector<uint8_t> result(data, data + width * height * channels);
  return result;
}

Ptr<Texture>
ResourceManager::loadTextureWithAlphaMask(const std::string &filepath) {
  // 先加载纹理
  auto texture = loadTexture(filepath);
  if (!texture) {
    return nullptr;
  }

  // 生成Alpha遮罩
  generateAlphaMask(filepath);

  return texture;
}

const AlphaMask *
ResourceManager::getAlphaMask(const std::string &textureKey) const {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(textureKey);
  if (it != textureCache_.end()) {
    GLTexture *glTexture = static_cast<GLTexture *>(it->second.texture.get());
    return glTexture->getAlphaMask();
  }
  return nullptr;
}

bool ResourceManager::generateAlphaMask(const std::string &textureKey) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(textureKey);
  if (it != textureCache_.end()) {
    GLTexture *glTexture = static_cast<GLTexture *>(it->second.texture.get());
    if (!glTexture->hasAlphaMask()) {
      glTexture->generateAlphaMask();
    }
    return glTexture->hasAlphaMask();
  }
  return false;
}

bool ResourceManager::hasAlphaMask(const std::string &textureKey) const {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(textureKey);
  if (it != textureCache_.end()) {
    GLTexture *glTexture = static_cast<GLTexture *>(it->second.texture.get());
    return glTexture->hasAlphaMask();
  }
  return false;
}

Ptr<Texture> ResourceManager::getTexture(const std::string &key) const {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(key);
  if (it != textureCache_.end()) {
    const_cast<ResourceManager*>(this)->touchTexture(key);
    const_cast<ResourceManager*>(this)->textureHitCount_++;
    const_cast<uint32_t&>(it->second.accessCount)++;
    return it->second.texture;
  }
  return nullptr;
}

bool ResourceManager::hasTexture(const std::string &key) const {
  std::lock_guard<std::mutex> lock(textureMutex_);
  return textureCache_.find(key) != textureCache_.end();
}

void ResourceManager::unloadTexture(const std::string &key) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(key);
  if (it != textureCache_.end()) {
    // 从LRU链表中移除
    auto nodeIt = std::find_if(lruNodes_.begin(), lruNodes_.end(),
                               [&key](const LRUNode &node) { return node.valid && node.key == key; });
    if (nodeIt != lruNodes_.end()) {
      removeFromList(static_cast<uint32_t>(nodeIt - lruNodes_.begin()) + 1);
      freeLRUNode(static_cast<uint32_t>(nodeIt - lruNodes_.begin()) + 1);
    }

    // 更新总大小
    totalTextureSize_ -= it->second.size;

    // 从缓存移除
    textureCache_.erase(it);
    E2D_LOG_DEBUG("ResourceManager: unloaded texture: {}", key);
  }
}

// ============================================================================
// 字体图集资源
// ============================================================================

std::string ResourceManager::makeFontKey(const std::string &filepath,
                                         int fontSize, bool useSDF) const {
  return filepath + "#" + std::to_string(fontSize) + (useSDF ? "#sdf" : "");
}

Ptr<FontAtlas> ResourceManager::loadFont(const std::string &filepath,
                                         int fontSize, bool useSDF) {
  std::lock_guard<std::mutex> lock(fontMutex_);

  std::string key = makeFontKey(filepath, fontSize, useSDF);

  // 检查缓存
  auto it = fontCache_.find(key);
  if (it != fontCache_.end()) {
    if (auto font = it->second.lock()) {
      E2D_LOG_TRACE("ResourceManager: font cache hit: {}", key);
      return font;
    }
    // 弱引用已失效，移除
    fontCache_.erase(it);
  }

  // 解析资源路径（优先尝试 romfs:/ 前缀）
  std::string fullPath = resolveResourcePath(filepath);
  if (fullPath.empty()) {
    E2D_LOG_ERROR("ResourceManager: font file not found: {}", filepath);
    return nullptr;
  }

  // 创建新字体图集
  try {
    auto font = makePtr<GLFontAtlas>(fullPath, fontSize, useSDF);
    if (!font->getTexture() || !font->getTexture()->isValid()) {
      E2D_LOG_ERROR("ResourceManager: failed to load font: {}", filepath);
      return nullptr;
    }

    // 存入缓存
    fontCache_[key] = font;
    E2D_LOG_DEBUG("ResourceManager: loaded font: {} (size={}, sdf={})",
                  filepath, fontSize, useSDF);
    return font;
  } catch (...) {
    E2D_LOG_ERROR("ResourceManager: exception loading font: {}", filepath);
    return nullptr;
  }
}

Ptr<FontAtlas> ResourceManager::getFont(const std::string &key) const {
  std::lock_guard<std::mutex> lock(fontMutex_);

  auto it = fontCache_.find(key);
  if (it != fontCache_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

bool ResourceManager::hasFont(const std::string &key) const {
  std::lock_guard<std::mutex> lock(fontMutex_);

  auto it = fontCache_.find(key);
  if (it != fontCache_.end()) {
    return !it->second.expired();
  }
  return false;
}

void ResourceManager::unloadFont(const std::string &key) {
  std::lock_guard<std::mutex> lock(fontMutex_);
  fontCache_.erase(key);
  E2D_LOG_DEBUG("ResourceManager: unloaded font: {}", key);
}

// ============================================================================
// 音效资源
// ============================================================================

Ptr<Sound> ResourceManager::loadSound(const std::string &filepath) {
  return loadSound(filepath, filepath);
}

Ptr<Sound> ResourceManager::loadSound(const std::string &name,
                                      const std::string &filepath) {
  std::lock_guard<std::mutex> lock(soundMutex_);

  // 检查缓存
  auto it = soundCache_.find(name);
  if (it != soundCache_.end()) {
    if (auto sound = it->second.lock()) {
      E2D_LOG_TRACE("ResourceManager: sound cache hit: {}", name);
      return sound;
    }
    // 弱引用已失效，移除
    soundCache_.erase(it);
  }

  // 解析资源路径（优先尝试 romfs:/ 前缀）
  std::string fullPath = resolveResourcePath(filepath);
  if (fullPath.empty()) {
    E2D_LOG_ERROR("ResourceManager: sound file not found: {}", filepath);
    return nullptr;
  }

  // 使用 AudioEngine 加载音效
  auto sound = AudioEngine::getInstance().loadSound(name, fullPath);
  if (!sound) {
    E2D_LOG_ERROR("ResourceManager: failed to load sound: {}", filepath);
    return nullptr;
  }

  // 存入缓存
  soundCache_[name] = sound;
  E2D_LOG_DEBUG("ResourceManager: loaded sound: {}", filepath);
  return sound;
}

Ptr<Sound> ResourceManager::getSound(const std::string &key) const {
  std::lock_guard<std::mutex> lock(soundMutex_);

  auto it = soundCache_.find(key);
  if (it != soundCache_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

bool ResourceManager::hasSound(const std::string &key) const {
  std::lock_guard<std::mutex> lock(soundMutex_);

  auto it = soundCache_.find(key);
  if (it != soundCache_.end()) {
    return !it->second.expired();
  }
  return false;
}

void ResourceManager::unloadSound(const std::string &key) {
  std::lock_guard<std::mutex> lock(soundMutex_);

  // 从 AudioEngine 也卸载
  AudioEngine::getInstance().unloadSound(key);
  soundCache_.erase(key);
  E2D_LOG_DEBUG("ResourceManager: unloaded sound: {}", key);
}

// ============================================================================
// 缓存清理
// ============================================================================

void ResourceManager::purgeUnused() {
  // 纹理缓存使用LRU策略，不需要清理失效引用
  // 字体缓存
  {
    std::lock_guard<std::mutex> lock(fontMutex_);
    for (auto it = fontCache_.begin(); it != fontCache_.end();) {
      if (it->second.expired()) {
        E2D_LOG_TRACE("ResourceManager: purging unused font: {}", it->first);
        it = fontCache_.erase(it);
      } else {
        ++it;
      }
    }
  }

  // 音效缓存
  {
    std::lock_guard<std::mutex> lock(soundMutex_);
    for (auto it = soundCache_.begin(); it != soundCache_.end();) {
      if (it->second.expired()) {
        E2D_LOG_TRACE("ResourceManager: purging unused sound: {}", it->first);
        it = soundCache_.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void ResourceManager::clearTextureCache() {
  std::lock_guard<std::mutex> lock(textureMutex_);
  size_t count = textureCache_.size();
  textureCache_.clear();
  lruNodes_.clear();
  lruHead_ = 0;
  lruTail_ = 0;
  freeList_ = 0;
  totalTextureSize_ = 0;
  E2D_LOG_INFO("ResourceManager: cleared {} textures from cache", count);
}

void ResourceManager::clearFontCache() {
  std::lock_guard<std::mutex> lock(fontMutex_);
  size_t count = fontCache_.size();
  fontCache_.clear();
  E2D_LOG_INFO("ResourceManager: cleared {} fonts from cache", count);
}

void ResourceManager::clearSoundCache() {
  std::lock_guard<std::mutex> lock(soundMutex_);
  size_t count = soundCache_.size();

  // 同时清理 AudioEngine 中的缓存
  for (const auto &pair : soundCache_) {
    AudioEngine::getInstance().unloadSound(pair.first);
  }
  soundCache_.clear();
  E2D_LOG_INFO("ResourceManager: cleared {} sounds from cache", count);
}

void ResourceManager::clearAllCaches() {
  clearTextureCache();
  clearFontCache();
  clearSoundCache();
  E2D_LOG_INFO("ResourceManager: all caches cleared");
}

size_t ResourceManager::getTextureCacheSize() const {
  std::lock_guard<std::mutex> lock(textureMutex_);
  return textureCache_.size();
}

// ============================================================================
// LRU 缓存管理
// ============================================================================

void ResourceManager::setTextureCache(size_t maxCacheSize, size_t maxTextureCount,
                                      float unloadInterval) {
  std::lock_guard<std::mutex> lock(textureMutex_);
  maxCacheSize_ = maxCacheSize;
  maxTextureCount_ = maxTextureCount;
  unloadInterval_ = unloadInterval;
}

size_t ResourceManager::getTextureCacheMemoryUsage() const {
  std::lock_guard<std::mutex> lock(textureMutex_);
  return totalTextureSize_;
}

float ResourceManager::getTextureCacheHitRate() const {
  std::lock_guard<std::mutex> lock(textureMutex_);
  uint64_t total = textureHitCount_ + textureMissCount_;
  if (total == 0) {
    return 0.0f;
  }
  return static_cast<float>(textureHitCount_) / static_cast<float>(total);
}

void ResourceManager::printTextureCacheStats() const {
  std::lock_guard<std::mutex> lock(textureMutex_);
  E2D_LOG_INFO("纹理缓存统计:");
  E2D_LOG_INFO("  缓存纹理数: {}/{}", textureCache_.size(), maxTextureCount_);
  E2D_LOG_INFO("  缓存大小: {} / {} MB", totalTextureSize_ / (1024 * 1024),
               maxCacheSize_ / (1024 * 1024));
  E2D_LOG_INFO("  缓存命中: {}", textureHitCount_);
  E2D_LOG_INFO("  缓存未命中: {}", textureMissCount_);
  E2D_LOG_INFO("  命中率: {:.1f}%", getTextureCacheHitRate() * 100.0f);
}

void ResourceManager::update(float dt) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  // 自动清理
  autoUnloadTimer_ += dt;
  if (autoUnloadTimer_ >= unloadInterval_) {
    autoUnloadTimer_ = 0.0f;

    // 如果缓存超过80%，清理到50%
    if (totalTextureSize_ > maxCacheSize_ * 0.8f) {
      size_t targetSize = static_cast<size_t>(maxCacheSize_ * 0.5f);
      while (totalTextureSize_ > targetSize && lruTail_ != 0) {
        std::string key = evictLRU();
        if (key.empty()) break;

        auto it = textureCache_.find(key);
        if (it != textureCache_.end()) {
          totalTextureSize_ -= it->second.size;
          textureCache_.erase(it);
          E2D_LOG_DEBUG("ResourceManager: auto-evicted texture: {}", key);
        }
      }
      E2D_LOG_INFO("ResourceManager: texture cache trimmed to {} MB", totalTextureSize_ / (1024 * 1024));
    }
  }
}

// ============================================================================
// LRU 缓存内部方法
// ============================================================================

uint32_t ResourceManager::allocateLRUNode(const std::string &key) {
  uint32_t index;
  if (freeList_ != 0) {
    // 复用空闲节点
    index = freeList_;
    freeList_ = lruNodes_[index - 1].next;
  } else {
    // 分配新节点
    lruNodes_.emplace_back();
    index = static_cast<uint32_t>(lruNodes_.size());
  }

  LRUNode &node = lruNodes_[index - 1];
  node.key = key;
  node.prev = 0;
  node.next = 0;
  node.valid = true;
  return index;
}

void ResourceManager::freeLRUNode(uint32_t index) {
  if (index == 0 || index > lruNodes_.size()) return;

  LRUNode &node = lruNodes_[index - 1];
  node.valid = false;
  node.key.clear();
  node.prev = 0;
  node.next = freeList_;
  freeList_ = index;
}

void ResourceManager::moveToFront(uint32_t index) {
  if (index == 0 || index > lruNodes_.size() || index == lruHead_) return;

  removeFromList(index);

  LRUNode &node = lruNodes_[index - 1];
  node.prev = 0;
  node.next = lruHead_;

  if (lruHead_ != 0) {
    lruNodes_[lruHead_ - 1].prev = index;
  }
  lruHead_ = index;

  if (lruTail_ == 0) {
    lruTail_ = index;
  }
}

void ResourceManager::removeFromList(uint32_t index) {
  if (index == 0 || index > lruNodes_.size()) return;

  LRUNode &node = lruNodes_[index - 1];

  if (node.prev != 0) {
    lruNodes_[node.prev - 1].next = node.next;
  } else {
    lruHead_ = node.next;
  }

  if (node.next != 0) {
    lruNodes_[node.next - 1].prev = node.prev;
  } else {
    lruTail_ = node.prev;
  }
}

std::string ResourceManager::evictLRU() {
  if (lruTail_ == 0) return "";

  uint32_t index = lruTail_;
  std::string key = lruNodes_[index - 1].key;

  removeFromList(index);
  freeLRUNode(index);

  return key;
}

void ResourceManager::touchTexture(const std::string &key) {
  // 查找对应的LRU节点
  auto nodeIt = std::find_if(lruNodes_.begin(), lruNodes_.end(),
                             [&key](const LRUNode &node) { return node.valid && node.key == key; });
  if (nodeIt != lruNodes_.end()) {
    uint32_t index = static_cast<uint32_t>(nodeIt - lruNodes_.begin()) + 1;
    moveToFront(index);
  }

  // 更新时间戳
  auto it = textureCache_.find(key);
  if (it != textureCache_.end()) {
    it->second.lastAccessTime = 0.0f;
  }
}

void ResourceManager::evictTexturesIfNeeded() {
  while ((totalTextureSize_ >= maxCacheSize_ ||
          textureCache_.size() >= maxTextureCount_) &&
         lruTail_ != 0) {
    std::string key = evictLRU();
    if (key.empty()) break;

    auto it = textureCache_.find(key);
    if (it != textureCache_.end()) {
      totalTextureSize_ -= it->second.size;
      textureCache_.erase(it);
      E2D_LOG_DEBUG("ResourceManager: evicted texture: {}", key);
    }
  }
}

size_t ResourceManager::calculateTextureSize(int width, int height, PixelFormat format) const {
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

size_t ResourceManager::getFontCacheSize() const {
  std::lock_guard<std::mutex> lock(fontMutex_);
  return fontCache_.size();
}

size_t ResourceManager::getSoundCacheSize() const {
  std::lock_guard<std::mutex> lock(soundMutex_);
  return soundCache_.size();
}

} // namespace extra2d
