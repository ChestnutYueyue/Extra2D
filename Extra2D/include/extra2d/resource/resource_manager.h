#pragma once

#include <extra2d/audio/sound.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/alpha_mask.h>
#include <extra2d/graphics/font.h>
#include <extra2d/graphics/texture.h>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <queue>
#include <thread>
#include <atomic>
#include <list>
#include <vector>

namespace extra2d {

// ============================================================================
// 资源管理器 - 统一管理纹理、字体、音效等资源
// 支持异步加载和纹理压缩
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

// ============================================================================
// 纹理LRU缓存项
// ============================================================================
struct TextureCacheEntry {
  Ptr<Texture> texture;
  size_t size = 0;                          // 纹理大小（字节）
  float lastAccessTime = 0.0f;              // 最后访问时间
  uint32_t accessCount = 0;                 // 访问次数
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
  // 文本文件资源
  // ------------------------------------------------------------------------

  /// 加载文本文件（带缓存）
  /// @param filepath 文件路径，支持 romfs:/ 前缀
  /// @return 文件内容字符串，加载失败返回空字符串
  std::string loadTextFile(const std::string &filepath);

  /// 加载文本文件（指定编码）
  /// @param filepath 文件路径
  /// @param encoding 文件编码（默认 UTF-8）
  /// @return 文件内容字符串
  std::string loadTextFile(const std::string &filepath, const std::string &encoding);

  /// 通过key获取已缓存的文本内容
  std::string getTextFile(const std::string &key) const;

  /// 检查文本文件是否已缓存
  bool hasTextFile(const std::string &key) const;

  /// 卸载指定文本文件
  void unloadTextFile(const std::string &key);

  /// 清理所有文本文件缓存
  void clearTextFileCache();

  // ------------------------------------------------------------------------
  // JSON 文件资源
  // ------------------------------------------------------------------------

  /// 加载并解析 JSON 文件
  /// @param filepath 文件路径，支持 romfs:/ 前缀
  /// @return JSON 字符串内容，加载或解析失败返回空字符串
  /// @note 返回的是原始 JSON 字符串，需要自行解析
  std::string loadJsonFile(const std::string &filepath);

  /// 通过key获取已缓存的 JSON 内容
  std::string getJsonFile(const std::string &key) const;

  /// 检查 JSON 文件是否已缓存
  bool hasJsonFile(const std::string &key) const;

  /// 卸载指定 JSON 文件
  void unloadJsonFile(const std::string &key);

  /// 清理所有 JSON 文件缓存
  void clearJsonFileCache();

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
  size_t getTextFileCacheSize() const;
  size_t getJsonFileCacheSize() const;

  // ------------------------------------------------------------------------
  // LRU 缓存管理
  // ------------------------------------------------------------------------

  /// 设置纹理缓存参数
  void setTextureCache(size_t maxCacheSize, size_t maxTextureCount,
                       float unloadInterval);

  /// 获取当前缓存的总大小（字节）
  size_t getTextureCacheMemoryUsage() const;

  /// 获取缓存命中率
  float getTextureCacheHitRate() const;

  /// 打印缓存统计信息
  void printTextureCacheStats() const;

  /// 更新缓存（在主循环中调用，用于自动清理）
  void update(float dt);

  // ------------------------------------------------------------------------
  // 异步加载控制
  // ------------------------------------------------------------------------

  /// 初始化异步加载系统（可选，自动在首次异步加载时初始化）
  void initAsyncLoader();

  /// 关闭异步加载系统
  void shutdownAsyncLoader();

  /// 等待所有异步加载完成
  void waitForAsyncLoads();

  /// 检查是否有正在进行的异步加载
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

  // 互斥锁保护缓存
  mutable std::mutex textureMutex_;
  mutable std::mutex fontMutex_;
  mutable std::mutex soundMutex_;
  mutable std::mutex textFileMutex_;
  mutable std::mutex jsonFileMutex_;

  // 资源缓存 - 使用弱指针实现自动清理
  std::unordered_map<std::string, WeakPtr<FontAtlas>> fontCache_;
  std::unordered_map<std::string, WeakPtr<Sound>> soundCache_;

  // 文本文件缓存 - 使用强引用（字符串值类型）
  std::unordered_map<std::string, std::string> textFileCache_;
  std::unordered_map<std::string, std::string> jsonFileCache_;

  // ============================================================================
  // 纹理LRU缓存
  // ============================================================================

  // LRU链表节点
  struct LRUNode {
    std::string key;
    uint32_t prev = 0;   // 数组索引，0表示无效
    uint32_t next = 0;   // 数组索引，0表示无效
    bool valid = false;
  };

  // 纹理缓存配置
  size_t maxCacheSize_ = 64 * 1024 * 1024;  // 最大缓存大小 (64MB)
  size_t maxTextureCount_ = 256;             // 最大纹理数量
  float unloadInterval_ = 30.0f;             // 自动清理间隔 (秒)

  // 纹理缓存 - 使用强指针保持引用
  std::unordered_map<std::string, TextureCacheEntry> textureCache_;

  // 侵入式LRU链表 - 使用数组索引代替指针，提高缓存局部性
  std::vector<LRUNode> lruNodes_;
  uint32_t lruHead_ = 0;   // 最近使用
  uint32_t lruTail_ = 0;   // 最久未使用
  uint32_t freeList_ = 0;  // 空闲节点链表

  // 统计
  size_t totalTextureSize_ = 0;
  uint64_t textureHitCount_ = 0;
  uint64_t textureMissCount_ = 0;
  float autoUnloadTimer_ = 0.0f;
  
  // 异步加载相关
  struct AsyncLoadTask {
    std::string filepath;
    TextureFormat format;
    TextureLoadCallback callback;
    std::promise<Ptr<Texture>> promise;
  };
  
  std::queue<AsyncLoadTask> asyncTaskQueue_;
  std::mutex asyncQueueMutex_;
  std::condition_variable asyncCondition_;
  std::unique_ptr<std::thread> asyncThread_;
  std::atomic<bool> asyncRunning_{false};
  std::atomic<int> pendingAsyncLoads_{0};
  
  void asyncLoadLoop();

  // ============================================================================
  // LRU 缓存内部方法
  // ============================================================================

  /// 分配LRU节点
  uint32_t allocateLRUNode(const std::string &key);

  /// 释放LRU节点
  void freeLRUNode(uint32_t index);

  /// 将节点移到链表头部（最近使用）
  void moveToFront(uint32_t index);

  /// 从链表中移除节点
  void removeFromList(uint32_t index);

  /// 驱逐最久未使用的纹理
  std::string evictLRU();

  /// 访问纹理（更新LRU位置）
  void touchTexture(const std::string &key);

  /// 驱逐纹理直到满足大小限制
  void evictTexturesIfNeeded();

  /// 计算纹理大小
  size_t calculateTextureSize(int width, int height, PixelFormat format) const;
};

} // namespace extra2d
