#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace extra2d {

// ============================================================================
// 纹理图集 - 自动将小纹理合并到大图集以减少 DrawCall
// ============================================================================

/**
 * @brief 图集中的单个纹理条目
 */
struct AtlasEntry {
  std::string name;           // 原始纹理名称/路径
  Rect uvRect;                // 在图集中的 UV 坐标范围
  Vec2 originalSize;          // 原始纹理尺寸
  uint32_t padding;           // 边距（用于避免纹理 bleeding）
  
  AtlasEntry() : uvRect(), originalSize(), padding(2) {}
};

/**
 * @brief 纹理图集页面
 * 当单个图集放不下时，创建多个页面
 */
class TextureAtlasPage {
public:
  static constexpr int DEFAULT_SIZE = 2048;
  static constexpr int MAX_SIZE = 4096;
  static constexpr int MIN_TEXTURE_SIZE = 32;  // 小于此大小的纹理才考虑合并
  static constexpr int PADDING = 2;            // 纹理间边距
  
  TextureAtlasPage(int width = DEFAULT_SIZE, int height = DEFAULT_SIZE);
  ~TextureAtlasPage();
  
  // 尝试添加纹理到图集
  // 返回是否成功，如果成功则输出 uvRect
  bool tryAddTexture(const std::string& name, int texWidth, int texHeight, 
                     const uint8_t* pixels, Rect& outUvRect);
  
  // 获取图集纹理
  Ptr<Texture> getTexture() const { return texture_; }
  
  // 获取条目
  const AtlasEntry* getEntry(const std::string& name) const;
  
  // 获取使用率
  float getUsageRatio() const;
  
  // 获取尺寸
  int getWidth() const { return width_; }
  int getHeight() const { return height_; }
  
  // 是否已满
  bool isFull() const { return isFull_; }
  
private:
  int width_, height_;
  Ptr<Texture> texture_;
  std::unordered_map<std::string, AtlasEntry> entries_;
  
  // 矩形打包数据
  struct PackNode {
    int x, y, width, height;
    bool used;
    std::unique_ptr<PackNode> left;
    std::unique_ptr<PackNode> right;
    
    PackNode(int x_, int y_, int w, int h) 
      : x(x_), y(y_), width(w), height(h), used(false) {}
  };
  
  std::unique_ptr<PackNode> root_;
  bool isFull_;
  int usedArea_;
  
  // 递归插入
  PackNode* insert(PackNode* node, int width, int height);
  void writePixels(int x, int y, int w, int h, const uint8_t* pixels);
};

/**
 * @brief 纹理图集管理器
 * 自动管理多个图集页面，提供统一的纹理查询接口
 */
class TextureAtlas {
public:
  TextureAtlas();
  ~TextureAtlas();
  
  // 初始化
  void init(int pageSize = TextureAtlasPage::DEFAULT_SIZE);
  
  // 添加纹理到图集
  // 如果纹理太大，返回 false，应该作为独立纹理加载
  bool addTexture(const std::string& name, int width, int height, 
                  const uint8_t* pixels);
  
  // 查询纹理是否在图集中
  bool contains(const std::string& name) const;
  
  // 获取纹理在图集中的信息
  // 返回图集纹理和 UV 坐标
  const Texture* getAtlasTexture(const std::string& name) const;
  Rect getUVRect(const std::string& name) const;
  
  // 获取原始纹理尺寸
  Vec2 getOriginalSize(const std::string& name) const;
  
  // 获取所有图集页面
  const std::vector<std::unique_ptr<TextureAtlasPage>>& getPages() const { return pages_; }
  
  // 获取总使用率
  float getTotalUsageRatio() const;
  
  // 清空所有图集
  void clear();
  
  // 设置是否启用自动图集
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }
  
  // 设置纹理大小阈值（小于此大小的纹理才进入图集）
  void setSizeThreshold(int threshold) { sizeThreshold_ = threshold; }
  int getSizeThreshold() const { return sizeThreshold_; }

private:
  std::vector<std::unique_ptr<TextureAtlasPage>> pages_;
  std::unordered_map<std::string, TextureAtlasPage*> entryToPage_;
  
  int pageSize_;
  int sizeThreshold_;
  bool enabled_;
  bool initialized_;
};

/**
 * @brief 全局图集管理器（单例）
 */
class TextureAtlasMgr {
public:
  static TextureAtlasMgr& get();
  
  // 获取主图集
  TextureAtlas& getAtlas() { return atlas_; }
  
  // 快捷方法
  bool addTexture(const std::string& name, int width, int height, 
                  const uint8_t* pixels) {
    return atlas_.addTexture(name, width, height, pixels);
  }
  
  bool contains(const std::string& name) const {
    return atlas_.contains(name);
  }
  
  const Texture* getAtlasTexture(const std::string& name) const {
    return atlas_.getAtlasTexture(name);
  }
  
  Rect getUVRect(const std::string& name) const {
    return atlas_.getUVRect(name);
  }

private:
  TextureAtlasMgr() = default;
  ~TextureAtlasMgr() = default;
  
  TextureAtlasMgr(const TextureAtlasMgr&) = delete;
  TextureAtlasMgr& operator=(const TextureAtlasMgr&) = delete;
  
  TextureAtlas atlas_;
};

} // namespace extra2d
