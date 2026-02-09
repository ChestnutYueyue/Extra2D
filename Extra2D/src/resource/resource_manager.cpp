#include <algorithm>
#include <cstring>
#include <extra2d/audio/audio_engine.h>
#include <extra2d/graphics/opengl/gl_font_atlas.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/resource/resource_manager.h>
#include <extra2d/utils/logger.h>
#include <sys/stat.h>

namespace extra2d {

ResourceManager::ResourceManager() {
#ifdef __SWITCH__
  addSearchPath("romfs:/");
  addSearchPath("sdmc:/");
#endif
}
ResourceManager::~ResourceManager() = default;

ResourceManager &ResourceManager::getInstance() {
  static ResourceManager instance;
  return instance;
}

// ============================================================================
// 搜索路径管理
// ============================================================================

void ResourceManager::addSearchPath(const std::string &path) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  // 避免重复添加
  auto it = std::find(searchPaths_.begin(), searchPaths_.end(), path);
  if (it == searchPaths_.end()) {
    searchPaths_.push_back(path);
    E2D_LOG_DEBUG("ResourceManager: added search path: {}", path);
  }
}

void ResourceManager::removeSearchPath(const std::string &path) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = std::find(searchPaths_.begin(), searchPaths_.end(), path);
  if (it != searchPaths_.end()) {
    searchPaths_.erase(it);
    E2D_LOG_DEBUG("ResourceManager: removed search path: {}", path);
  }
}

void ResourceManager::clearSearchPaths() {
  std::lock_guard<std::mutex> lock(textureMutex_);
  searchPaths_.clear();
  E2D_LOG_DEBUG("ResourceManager: cleared all search paths");
}

// 辅助函数：检查文件是否存在
static bool fileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

// 辅助函数：检查是否是 romfs 路径
static bool isRomfsPath(const std::string &path) {
  return path.find("romfs:/") == 0 || path.find("romfs:\\") == 0;
}

// 辅助函数：拼接路径
static std::string joinPath(const std::string &dir,
                            const std::string &filename) {
  if (dir.empty())
    return filename;
  char lastChar = dir.back();
  if (lastChar == '/' || lastChar == '\\') {
    return dir + filename;
  }
  return dir + "/" + filename;
}

std::string
ResourceManager::findResourcePath(const std::string &filename) const {
  // 首先检查是否是 romfs 路径（Switch 平台）
  if (isRomfsPath(filename)) {
    if (fileExists(filename)) {
      return filename;
    }
    return "";
  }

  // 首先检查是否是绝对路径或相对当前目录存在
  if (fileExists(filename)) {
    return filename;
  }

  // 在搜索路径中查找
  std::lock_guard<std::mutex> lock(textureMutex_);
  for (const auto &path : searchPaths_) {
    std::string fullPath = joinPath(path, filename);
    if (fileExists(fullPath)) {
      return fullPath;
    }
  }

  // 最后尝试在 romfs 中查找（自动添加 romfs:/ 前缀）
  std::string romfsPath = "romfs:/" + filename;
  if (fileExists(romfsPath)) {
    return romfsPath;
  }

  return "";
}

// ============================================================================
// 纹理资源
// ============================================================================

Ptr<Texture> ResourceManager::loadTexture(const std::string &filepath) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  // 检查缓存
  auto it = textureCache_.find(filepath);
  if (it != textureCache_.end()) {
    if (auto texture = it->second.lock()) {
      E2D_LOG_TRACE("ResourceManager: texture cache hit: {}", filepath);
      return texture;
    }
    // 弱引用已失效，移除
    textureCache_.erase(it);
  }

  // 查找完整路径
  std::string fullPath = findResourcePath(filepath);
  if (fullPath.empty()) {
    E2D_LOG_ERROR("ResourceManager: texture file not found: {}", filepath);
    return nullptr;
  }

  // 创建新纹理（根据扩展名自动选择加载路径）
  try {
    auto texture = makePtr<GLTexture>(fullPath);
    if (!texture->isValid()) {
      E2D_LOG_ERROR("ResourceManager: failed to load texture: {}", filepath);
      return nullptr;
    }

    // 存入缓存
    textureCache_[filepath] = texture;
    E2D_LOG_DEBUG("ResourceManager: loaded texture: {}", filepath);
    return texture;
  } catch (...) {
    E2D_LOG_ERROR("ResourceManager: exception loading texture: {}", filepath);
    return nullptr;
  }
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
    if (auto texture = it->second.lock()) {
      GLTexture *glTexture = static_cast<GLTexture *>(texture.get());
      return glTexture->getAlphaMask();
    }
  }
  return nullptr;
}

bool ResourceManager::generateAlphaMask(const std::string &textureKey) {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(textureKey);
  if (it != textureCache_.end()) {
    if (auto texture = it->second.lock()) {
      GLTexture *glTexture = static_cast<GLTexture *>(texture.get());
      if (!glTexture->hasAlphaMask()) {
        glTexture->generateAlphaMask();
      }
      return glTexture->hasAlphaMask();
    }
  }
  return false;
}

bool ResourceManager::hasAlphaMask(const std::string &textureKey) const {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(textureKey);
  if (it != textureCache_.end()) {
    if (auto texture = it->second.lock()) {
      GLTexture *glTexture = static_cast<GLTexture *>(texture.get());
      return glTexture->hasAlphaMask();
    }
  }
  return false;
}

Ptr<Texture> ResourceManager::getTexture(const std::string &key) const {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(key);
  if (it != textureCache_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

bool ResourceManager::hasTexture(const std::string &key) const {
  std::lock_guard<std::mutex> lock(textureMutex_);

  auto it = textureCache_.find(key);
  if (it != textureCache_.end()) {
    return !it->second.expired();
  }
  return false;
}

void ResourceManager::unloadTexture(const std::string &key) {
  std::lock_guard<std::mutex> lock(textureMutex_);
  textureCache_.erase(key);
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

  // 查找完整路径
  std::string fullPath = findResourcePath(filepath);
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
// 多字体后备加载
// ============================================================================

Ptr<FontAtlas> ResourceManager::loadFontWithFallbacks(
    const std::vector<std::string> &fontPaths, int fontSize, bool useSDF) {

  // 尝试加载每一个候选字体
  for (const auto &fontPath : fontPaths) {
    auto font = loadFont(fontPath, fontSize, useSDF);
    if (font) {
      E2D_LOG_INFO("ResourceManager: successfully loaded font from fallback list: {}",
                   fontPath);
      return font;
    }
  }

  E2D_LOG_ERROR("ResourceManager: failed to load any font from fallback list ({} candidates)",
                fontPaths.size());
  return nullptr;
}

Ptr<FontAtlas> ResourceManager::loadFontWithDefaultFallback(
    const std::string &filepath, int fontSize, bool useSDF) {

  // 首先尝试加载用户指定的字体
  auto font = loadFont(filepath, fontSize, useSDF);
  if (font) {
    return font;
  }

  E2D_LOG_WARN("ResourceManager: failed to load font '{}', trying system fallbacks...",
               filepath);

  // 定义系统默认字体候选列表
  std::vector<std::string> fallbackFonts;

#ifdef __SWITCH__
  // Switch 平台默认字体路径
  fallbackFonts = {
      "romfs:/assets/font.ttf",       // 应用自带字体
      "romfs:/assets/default.ttf",    // 默认字体备选
      "romfs:/font.ttf",              // 根目录字体
      "sdmc:/switch/fonts/default.ttf", // SD卡字体目录
      "sdmc:/switch/fonts/font.ttf",
  };
#else
  // PC 平台系统字体路径（Windows/Linux/macOS）
#ifdef _WIN32
  fallbackFonts = {
      "C:/Windows/Fonts/arial.ttf",
      "C:/Windows/Fonts/segoeui.ttf",
      "C:/Windows/Fonts/calibri.ttf",
      "C:/Windows/Fonts/tahoma.ttf",
      "C:/Windows/Fonts/msyh.ttc",  // 微软雅黑
  };
#elif __APPLE__
  fallbackFonts = {
      "/System/Library/Fonts/Helvetica.ttc",
      "/System/Library/Fonts/SFNSDisplay.ttf",
      "/Library/Fonts/Arial.ttf",
  };
#else
  // Linux
  fallbackFonts = {
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
      "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
      "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
  };
#endif
#endif

  // 尝试加载后备字体
  for (const auto &fallbackPath : fallbackFonts) {
    font = loadFont(fallbackPath, fontSize, useSDF);
    if (font) {
      E2D_LOG_INFO("ResourceManager: loaded fallback font: {}", fallbackPath);
      return font;
    }
  }

  E2D_LOG_ERROR("ResourceManager: all font fallbacks exhausted, no font available");
  return nullptr;
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

  // 查找完整路径
  std::string fullPath = findResourcePath(filepath);
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
  // 清理纹理缓存
  {
    std::lock_guard<std::mutex> lock(textureMutex_);
    for (auto it = textureCache_.begin(); it != textureCache_.end();) {
      if (it->second.expired()) {
        E2D_LOG_TRACE("ResourceManager: purging unused texture: {}", it->first);
        it = textureCache_.erase(it);
      } else {
        ++it;
      }
    }
  }

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
  std::lock_guard<std::mutex> lock(textureMutex_);
  size_t count = textureCache_.size();
  textureCache_.clear();
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

size_t ResourceManager::getFontCacheSize() const {
  std::lock_guard<std::mutex> lock(fontMutex_);
  return fontCache_.size();
}

size_t ResourceManager::getSoundCacheSize() const {
  std::lock_guard<std::mutex> lock(soundMutex_);
  return soundCache_.size();
}

} // namespace extra2d
