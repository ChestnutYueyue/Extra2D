#pragma once

#include <extra2d/audio/sound.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/alpha_mask.h>
#include <extra2d/graphics/font.h>
#include <extra2d/graphics/texture.h>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace extra2d {

// ============================================================================
// 资源管理器 - 统一管理纹理、字体、音效等资源
// 使用 TexturePool 作为纹理管理后端
// ============================================================================

// 纹理格式枚举
enum class TextureFormat {
  Auto = 0,     // 自动选择最佳格式
  RGBA8,        // 32位 RGBA
  RGB8,         // 24位 RGB
  DXT1,         // BC1/DXT1 压缩（1 bit alpha）
  DXT5,         // BC3/DXT5 压缩（完整 alpha）
  ETC2,         // ETC2 压缩（移动平台）
  ASTC4x4,      // ASTC 4x4 压缩（高质量）
  ASTC8x8,      // ASTC 8x8 压缩（高压缩率）
};

// 异步加载回调类型
using TextureLoadCallback = std::function<void(Ptr<Texture>)>;

class ResourceManager {
public:
  // ------------------------------------------------------------------------
  // 单例访问
  // ------------------------------------------------------------------------
  static ResourceManager &getInstance();

  // ------------------------------------------------------------------------
  // 更新（在主循环中调用）
  // ------------------------------------------------------------------------
  
  /// 更新资源管理器，触发纹理池自动清理等
  void update(float dt);

  // ------------------------------------------------------------------------
  // 纹理资源 - 同步加载
  // ------------------------------------------------------------------------

  /// 加载纹理（带缓存）
  Ptr<Texture> loadTexture(const std::string &filepath);
  
  /// 加载纹理（指定是否异步）
  Ptr<Texture> loadTexture(const std::string &filepath, bool async);
  
  /// 加载纹理（完整参数：异步 + 压缩格式）
  Ptr<Texture> loadTexture(const std::string &filepath, bool async, TextureFormat format);
  
  /// 异步加载纹理（带回调）
  void loadTextureAsync(const std::string &filepath, TextureLoadCallback callback);
  
  /// 异步加载纹理（指定格式 + 回调）
  void loadTextureAsync(const std::string &filepath, TextureFormat format, TextureLoadCallback callback);

  /// 加载纹理并生成Alpha遮罩（用于不规则形状图片）
  Ptr<Texture> loadTextureWithAlphaMask(const std::string &filepath);

  /// 通过key获取已缓存的纹理
  Ptr<Texture> getTexture(const std::string &key) const;

  /// 检查纹理是否已缓存
  bool hasTexture(const std::string &key) const;

  /// 卸载指定纹理
  void unloadTexture(const std::string &key);

  // ------------------------------------------------------------------------
  // Alpha遮罩资源
  // ------------------------------------------------------------------------

  /// 获取纹理的Alpha遮罩（如果已生成）
  const AlphaMask *getAlphaMask(const std::string &textureKey) const;

  /// 为已加载的纹理生成Alpha遮罩
  bool generateAlphaMask(const std::string &textureKey);

  /// 检查纹理是否有Alpha遮罩
  bool hasAlphaMask(const std::string &textureKey) const;

  // ------------------------------------------------------------------------
  // 字体图集资源
  // ------------------------------------------------------------------------

  /// 加载字体图集（带缓存）
  Ptr<FontAtlas> loadFont(const std::string &filepath, int fontSize,
                          bool useSDF = false);

  /// 通过key获取已缓存的字体图集
  Ptr<FontAtlas> getFont(const std::string &key) const;

  /// 检查字体是否已缓存
  bool hasFont(const std::string &key) const;

  /// 卸载指定字体
  void unloadFont(const std::string &key);

  // ------------------------------------------------------------------------
  // 音效资源
  // ------------------------------------------------------------------------

  /// 加载音效（带缓存）
  Ptr<Sound> loadSound(const std::string &filepath);
  Ptr<Sound> loadSound(const std::string &name, const std::string &filepath);

  /// 通过key获取已缓存的音效
  Ptr<Sound> getSound(const std::string &key) const;

  /// 检查音效是否已缓存
  bool hasSound(const std::string &key) const;

  /// 卸载指定音效
  void unloadSound(const std::string &key);

  // ------------------------------------------------------------------------
  // 缓存清理
  // ------------------------------------------------------------------------

  /// 清理所有失效的弱引用（自动清理已释放的资源）
  void purgeUnused();

  /// 清理指定类型的所有缓存
  void clearTextureCache();
  void clearFontCache();
  void clearSoundCache();

  /// 清理所有资源缓存
  void clearAllCaches();

  /// 获取各类资源的缓存数量
  size_t getTextureCacheSize() const;
  size_t getFontCacheSize() const;
  size_t getSoundCacheSize() const;

  // ------------------------------------------------------------------------
  // 异步加载控制（已弃用，保留接口兼容性）
  // 纹理异步加载由 TexturePool 内部处理
  // ------------------------------------------------------------------------
  
  /// 初始化异步加载系统（可选，自动在首次异步加载时初始化）
  /// @deprecated 纹理池已内置异步加载，无需调用
  void initAsyncLoader();
  
  /// 关闭异步加载系统
  /// @deprecated 纹理池自动管理生命周期，无需调用
  void shutdownAsyncLoader();
  
  /// 等待所有异步加载完成
  /// @deprecated 使用回调机制处理异步加载结果
  void waitForAsyncLoads();
  
  /// 检查是否有正在进行的异步加载
  /// @deprecated 始终返回 false
  bool hasPendingAsyncLoads() const;

  ResourceManager();
  ~ResourceManager();
  ResourceManager(const ResourceManager &) = delete;
  ResourceManager &operator=(const ResourceManager &) = delete;

private:
  // 生成字体缓存key
  std::string makeFontKey(const std::string &filepath, int fontSize,
                          bool useSDF) const;
  
  // 内部加载实现
  Ptr<Texture> loadTextureInternal(const std::string &filepath, TextureFormat format);
  
  // 选择最佳纹理格式
  TextureFormat selectBestFormat(TextureFormat requested) const;
  
  // 压缩纹理数据
  std::vector<uint8_t> compressTexture(const uint8_t* data, int width, int height, 
                                       int channels, TextureFormat format);

  // 互斥锁保护缓存（字体和音效缓存仍需锁保护）
  mutable std::mutex fontMutex_;
  mutable std::mutex soundMutex_;

  // 资源缓存 - 使用弱指针实现自动清理
  // 纹理缓存已移至 TexturePool，此处仅保留字体和音效缓存
  std::unordered_map<std::string, WeakPtr<FontAtlas>> fontCache_;
  std::unordered_map<std::string, WeakPtr<Sound>> soundCache_;
};

} // namespace extra2d
