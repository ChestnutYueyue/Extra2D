#include <algorithm>
#include <cstring>
#include <extra2d/audio/audio_engine.h>
#include <extra2d/graphics/opengl/gl_font_atlas.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/graphics/texture_pool.h>
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

ResourceManager::ResourceManager() {
  // 初始化纹理池
  TexturePoolConfig config;
  config.maxCacheSize = 128 * 1024 * 1024;  // 128MB
  config.maxTextureCount = 512;
  config.enableAsyncLoad = true;
  TexturePool::getInstance().init(config);
}

ResourceManager::~ResourceManager() {
  TexturePool::getInstance().shutdown();
}

ResourceManager &ResourceManager::getInstance() {
  static ResourceManager instance;
  return instance;
}

// ============================================================================
// 更新（在主循环中调用）
// ============================================================================

void ResourceManager::update(float dt) {
  TexturePool::getInstance().update(dt);
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
  // 解析资源路径
  std::string fullPath = resolveResourcePath(filepath);
  if (fullPath.empty()) {
    E2D_LOG_ERROR("ResourceManager: texture file not found: {}", filepath);
    if (callback) {
      callback(nullptr);
    }
    return;
  }
  
  // 使用纹理池的异步加载
  TexturePool::getInstance().getAsync(fullPath, 
    [callback, filepath](Ptr<Texture> texture) {
      if (texture) {
        E2D_LOG_DEBUG("ResourceManager: async texture loaded: {}", filepath);
      } else {
        E2D_LOG_ERROR("ResourceManager: failed to async load texture: {}", filepath);
      }
      if (callback) {
        callback(texture);
      }
    });
}

Ptr<Texture> ResourceManager::loadTextureInternal(const std::string &filepath, TextureFormat format) {
  // 解析资源路径（优先尝试 romfs:/ 前缀）
  std::string fullPath = resolveResourcePath(filepath);
  if (fullPath.empty()) {
    E2D_LOG_ERROR("ResourceManager: texture file not found: {}", filepath);
    return nullptr;
  }

  // 使用纹理池获取纹理
  auto texture = TexturePool::getInstance().get(fullPath);
  
  if (!texture) {
    E2D_LOG_ERROR("ResourceManager: failed to load texture: {}", filepath);
    return nullptr;
  }

  // 如果需要压缩，处理纹理格式（记录日志，实际压缩需要在纹理创建时处理）
  if (format != TextureFormat::Auto && format != TextureFormat::RGBA8) {
    E2D_LOG_DEBUG("ResourceManager: texture format {} requested for {}", 
                  static_cast<int>(format), filepath);
  }

  E2D_LOG_DEBUG("ResourceManager: loaded texture: {}", filepath);
  return texture;
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
  // 解析路径获取完整路径
  std::string fullPath = resolveResourcePath(textureKey);
  if (fullPath.empty()) {
    return nullptr;
  }

  // 从纹理池获取纹理
  auto texture = TexturePool::getInstance().get(fullPath);
  if (!texture) {
    return nullptr;
  }

  GLTexture *glTexture = static_cast<GLTexture *>(texture.get());
  return glTexture->getAlphaMask();
}

bool ResourceManager::generateAlphaMask(const std::string &textureKey) {
  // 解析路径获取完整路径
  std::string fullPath = resolveResourcePath(textureKey);
  if (fullPath.empty()) {
    return false;
  }

  // 从纹理池获取纹理
  auto texture = TexturePool::getInstance().get(fullPath);
  if (!texture) {
    return false;
  }

  GLTexture *glTexture = static_cast<GLTexture *>(texture.get());
  if (!glTexture->hasAlphaMask()) {
    glTexture->generateAlphaMask();
  }
  return glTexture->hasAlphaMask();
}

bool ResourceManager::hasAlphaMask(const std::string &textureKey) const {
  // 解析路径获取完整路径
  std::string fullPath = resolveResourcePath(textureKey);
  if (fullPath.empty()) {
    return false;
  }

  // 从纹理池获取纹理
  auto texture = TexturePool::getInstance().get(fullPath);
  if (!texture) {
    return false;
  }

  GLTexture *glTexture = static_cast<GLTexture *>(texture.get());
  return glTexture->hasAlphaMask();
}

Ptr<Texture> ResourceManager::getTexture(const std::string &key) const {
  // 解析路径获取完整路径
  std::string fullPath = resolveResourcePath(key);
  if (fullPath.empty()) {
    return nullptr;
  }

  // 从纹理池获取纹理
  return TexturePool::getInstance().get(fullPath);
}

bool ResourceManager::hasTexture(const std::string &key) const {
  // 解析路径获取完整路径
  std::string fullPath = resolveResourcePath(key);
  if (fullPath.empty()) {
    return false;
  }

  // 检查纹理池是否有此纹理
  return TexturePool::getInstance().has(fullPath);
}

void ResourceManager::unloadTexture(const std::string &key) {
  // 解析路径获取完整路径
  std::string fullPath = resolveResourcePath(key);
  if (fullPath.empty()) {
    return;
  }

  // 从纹理池移除
  TexturePool::getInstance().remove(fullPath);
  E2D_LOG_DEBUG("ResourceManager: unloaded texture: {}", key);
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
  // 纹理缓存由 TexturePool 自动管理，无需手动清理
  // 但我们可以触发 TexturePool 的清理
  TexturePool::getInstance().trim(TexturePool::getInstance().getCacheSize() * 0.8f);

  // 清理字体缓存
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

  // 清理音效缓存
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
  TexturePool::getInstance().clear();
  E2D_LOG_INFO("ResourceManager: texture cache cleared via TexturePool");
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
  return TexturePool::getInstance().getTextureCount();
}

size_t ResourceManager::getFontCacheSize() const {
  std::lock_guard<std::mutex> lock(fontMutex_);
  return fontCache_.size();
}

size_t ResourceManager::getSoundCacheSize() const {
  std::lock_guard<std::mutex> lock(soundMutex_);
  return soundCache_.size();
}

// ============================================================================
// 异步加载控制（已弃用，保留接口兼容性）
// ============================================================================

void ResourceManager::initAsyncLoader() {
  // 纹理池已内置异步加载，无需额外初始化
  E2D_LOG_DEBUG("ResourceManager: async loader is handled by TexturePool");
}

void ResourceManager::shutdownAsyncLoader() {
  // 纹理池在 shutdown 时处理
}

void ResourceManager::waitForAsyncLoads() {
  // 纹理池异步加载通过回调处理，无需等待
  // 如需等待，可通过回调机制实现
}

bool ResourceManager::hasPendingAsyncLoads() const {
  // 纹理池异步加载通过回调处理，无法直接查询
  return false;
}

} // namespace extra2d
