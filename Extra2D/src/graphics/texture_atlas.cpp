#include <extra2d/graphics/texture_atlas.h>
#include <extra2d/utils/logger.h>
#include <algorithm>
#include <cstring>

namespace extra2d {

// ============================================================================
// TextureAtlasPage 实现
// ============================================================================

/**
 * @brief 构造函数
 * @param width 页面宽度
 * @param height 页面高度
 *
 * 创建指定尺寸的纹理图集页面，初始化空白纹理和打包根节点
 */
TextureAtlasPage::TextureAtlasPage(int width, int height)
    : width_(width), height_(height), isFull_(false), usedArea_(0) {
  // 创建空白纹理
  std::vector<uint8_t> emptyData(width * height * 4, 0);
  texture_ = makePtr<GLTexture>(width, height, emptyData.data(), 4);
  
  // 初始化矩形打包根节点
  root_ = std::make_unique<PackNode>(0, 0, width, height);
  
  E2D_LOG_INFO("Created texture atlas page: {}x{}", width, height);
}

/**
 * @brief 析构函数
 *
 * 释放纹理图集页面资源
 */
TextureAtlasPage::~TextureAtlasPage() = default;

/**
 * @brief 尝试添加纹理到图集页面
 * @param name 纹理名称
 * @param texWidth 纹理宽度
 * @param texHeight 纹理高度
 * @param pixels 像素数据
 * @param[out] outUvRect 输出的UV坐标矩形
 * @return 添加成功返回true，失败返回false
 *
 * 尝试将纹理添加到图集页面中，使用矩形打包算法找到合适位置
 */
bool TextureAtlasPage::tryAddTexture(const std::string& name, int texWidth, int texHeight,
                                     const uint8_t* pixels, Rect& outUvRect) {
  if (isFull_) {
    return false;
  }
  
  // 添加边距
  int paddedWidth = texWidth + 2 * PADDING;
  int paddedHeight = texHeight + 2 * PADDING;
  
  // 如果纹理太大，无法放入
  if (paddedWidth > width_ || paddedHeight > height_) {
    return false;
  }
  
  // 尝试插入
  PackNode* node = insert(root_.get(), paddedWidth, paddedHeight);
  if (node == nullptr) {
    // 无法放入，标记为满
    isFull_ = true;
    return false;
  }
  
  // 写入像素数据（跳过边距区域）
  writePixels(node->x + PADDING, node->y + PADDING, texWidth, texHeight, pixels);
  
  // 创建条目
  AtlasEntry entry;
  entry.name = name;
  entry.originalSize = Vec2(static_cast<float>(texWidth), static_cast<float>(texHeight));
  entry.padding = PADDING;
  
  // 计算 UV 坐标（考虑边距）
  float u1 = static_cast<float>(node->x + PADDING) / width_;
  float v1 = static_cast<float>(node->y + PADDING) / height_;
  float u2 = static_cast<float>(node->x + PADDING + texWidth) / width_;
  float v2 = static_cast<float>(node->y + PADDING + texHeight) / height_;
  
  entry.uvRect = Rect(u1, v1, u2 - u1, v2 - v1);
  outUvRect = entry.uvRect;
  
  entries_[name] = std::move(entry);
  usedArea_ += paddedWidth * paddedHeight;
  
  E2D_LOG_DEBUG("Added texture '{}' to atlas: {}x{} at ({}, {})", 
                name, texWidth, texHeight, node->x, node->y);
  
  return true;
}

/**
 * @brief 插入纹理到打包树
 * @param node 当前节点
 * @param width 纹理宽度
 * @param height 纹理高度
 * @return 找到合适的节点返回节点指针，否则返回nullptr
 *
 * 使用二叉树算法递归查找合适的空间位置
 */
TextureAtlasPage::PackNode* TextureAtlasPage::insert(PackNode* node, int width, int height) {
  if (node == nullptr) {
    return nullptr;
  }
  
  // 如果节点已被使用，尝试子节点
  if (node->used) {
    PackNode* result = insert(node->left.get(), width, height);
    if (result != nullptr) {
      return result;
    }
    return insert(node->right.get(), width, height);
  }
  
  // 检查是否适合
  if (width > node->width || height > node->height) {
    return nullptr;
  }
  
  // 如果刚好合适，使用此节点
  if (width == node->width && height == node->height) {
    node->used = true;
    return node;
  }
  
  // 需要分割节点
  int dw = node->width - width;
  int dh = node->height - height;
  
  if (dw > dh) {
    // 水平分割
    node->left = std::make_unique<PackNode>(node->x, node->y, width, node->height);
    node->right = std::make_unique<PackNode>(node->x + width, node->y, dw, node->height);
  } else {
    // 垂直分割
    node->left = std::make_unique<PackNode>(node->x, node->y, node->width, height);
    node->right = std::make_unique<PackNode>(node->x, node->y + height, node->width, dh);
  }
  
  // 递归插入到左子节点
  return insert(node->left.get(), width, height);
}

/**
 * @brief 写入像素数据到纹理
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param w 宽度
 * @param h 高度
 * @param pixels 像素数据
 *
 * 使用glTexSubImage2D更新纹理的指定区域
 */
void TextureAtlasPage::writePixels(int x, int y, int w, int h, const uint8_t* pixels) {
  if (texture_ == nullptr || pixels == nullptr) {
    return;
  }
  
  // 使用 glTexSubImage2D 更新纹理数据
  GLuint texID = static_cast<GLuint>(
      reinterpret_cast<uintptr_t>(texture_->getNativeHandle()));
  
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * @brief 获取图集中的纹理条目信息
 * @param name 纹理名称
 * @return 找到返回条目指针，未找到返回nullptr
 */
const AtlasEntry* TextureAtlasPage::getEntry(const std::string& name) const {
  auto it = entries_.find(name);
  if (it != entries_.end()) {
    return &it->second;
  }
  return nullptr;
}

/**
 * @brief 获取页面使用率
 * @return 使用率（0.0到1.0之间）
 *
 * 计算已使用面积占总面积的比例
 */
float TextureAtlasPage::getUsageRatio() const {
  return static_cast<float>(usedArea_) / (width_ * height_);
}

// ============================================================================
// TextureAtlas 实现
// ============================================================================

/**
 * @brief 默认构造函数
 *
 * 创建一个使用默认页面大小的纹理图集
 */
TextureAtlas::TextureAtlas()
    : pageSize_(TextureAtlasPage::DEFAULT_SIZE),
      sizeThreshold_(256),
      enabled_(true),
      initialized_(false) {
}

/**
 * @brief 析构函数
 *
 * 释放纹理图集资源
 */
TextureAtlas::~TextureAtlas() = default;

/**
 * @brief 初始化纹理图集
 * @param pageSize 页面大小
 *
 * 设置图集页面大小并标记为已初始化
 */
void TextureAtlas::init(int pageSize) {
  pageSize_ = pageSize;
  initialized_ = true;
  E2D_LOG_INFO("TextureAtlas initialized with page size: {}", pageSize);
}

/**
 * @brief 添加纹理到图集
 * @param name 纹理名称
 * @param width 纹理宽度
 * @param height 纹理高度
 * @param pixels 像素数据
 * @return 添加成功返回true，失败返回false
 *
 * 尝试将纹理添加到现有页面，如空间不足则创建新页面
 */
bool TextureAtlas::addTexture(const std::string& name, int width, int height,
                              const uint8_t* pixels) {
  if (!enabled_ || !initialized_) {
    return false;
  }
  
  // 检查是否已存在
  if (contains(name)) {
    return true;
  }
  
  // 检查纹理大小
  if (width > sizeThreshold_ || height > sizeThreshold_) {
    E2D_LOG_DEBUG("Texture '{}' too large for atlas ({}x{} > {}), skipping", 
                  name, width, height, sizeThreshold_);
    return false;
  }
  
  // 尝试添加到现有页面
  Rect uvRect;
  for (auto& page : pages_) {
    if (page->tryAddTexture(name, width, height, pixels, uvRect)) {
      entryToPage_[name] = page.get();
      return true;
    }
  }
  
  // 创建新页面
  auto newPage = std::make_unique<TextureAtlasPage>(pageSize_, pageSize_);
  if (newPage->tryAddTexture(name, width, height, pixels, uvRect)) {
    entryToPage_[name] = newPage.get();
    pages_.push_back(std::move(newPage));
    return true;
  }
  
  E2D_LOG_WARN("Failed to add texture '{}' to atlas", name);
  return false;
}

/**
 * @brief 检查纹理是否已存在于图集中
 * @param name 纹理名称
 * @return 存在返回true，不存在返回false
 */
bool TextureAtlas::contains(const std::string& name) const {
  return entryToPage_.find(name) != entryToPage_.end();
}

/**
 * @brief 获取纹理所在的图集纹理
 * @param name 纹理名称
 * @return 找到返回纹理指针，未找到返回nullptr
 */
const Texture* TextureAtlas::getAtlasTexture(const std::string& name) const {
  auto it = entryToPage_.find(name);
  if (it != entryToPage_.end()) {
    return it->second->getTexture().get();
  }
  return nullptr;
}

/**
 * @brief 获取纹理在图集中的UV坐标矩形
 * @param name 纹理名称
 * @return UV坐标矩形，未找到返回默认值
 */
Rect TextureAtlas::getUVRect(const std::string& name) const {
  auto it = entryToPage_.find(name);
  if (it != entryToPage_.end()) {
    const AtlasEntry* entry = it->second->getEntry(name);
    if (entry != nullptr) {
      return entry->uvRect;
    }
  }
  return Rect(0, 0, 1, 1); // 默认 UV
}

/**
 * @brief 获取纹理的原始尺寸
 * @param name 纹理名称
 * @return 原始尺寸，未找到返回零向量
 */
Vec2 TextureAtlas::getOriginalSize(const std::string& name) const {
  auto it = entryToPage_.find(name);
  if (it != entryToPage_.end()) {
    const AtlasEntry* entry = it->second->getEntry(name);
    if (entry != nullptr) {
      return entry->originalSize;
    }
  }
  return Vec2(0, 0);
}

/**
 * @brief 获取总使用率
 * @return 所有页面的平均使用率
 *
 * 计算所有页面的平均空间使用率
 */
float TextureAtlas::getTotalUsageRatio() const {
  if (pages_.empty()) {
    return 0.0f;
  }
  
  float total = 0.0f;
  for (const auto& page : pages_) {
    total += page->getUsageRatio();
  }
  return total / pages_.size();
}

/**
 * @brief 清空图集
 *
 * 移除所有页面和条目映射
 */
void TextureAtlas::clear() {
  pages_.clear();
  entryToPage_.clear();
  E2D_LOG_INFO("TextureAtlas cleared");
}

// ============================================================================
// TextureAtlasMgr 单例实现
// ============================================================================

/**
 * @brief 获取TextureAtlasMgr单例实例
 * @return TextureAtlasMgr单例的引用
 *
 * 使用静态局部变量实现线程安全的单例模式
 */
TextureAtlasMgr& TextureAtlasMgr::get() {
  static TextureAtlasMgr instance;
  return instance;
}

} // namespace extra2d
