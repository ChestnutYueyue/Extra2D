#include <extra2d/graphics/texture_atlas.h>
#include <extra2d/utils/logger.h>
#include <algorithm>
#include <cstring>

namespace extra2d {

// ============================================================================
// TextureAtlasPage 实现
// ============================================================================

TextureAtlasPage::TextureAtlasPage(int width, int height)
    : width_(width), height_(height), isFull_(false), usedArea_(0) {
  // 创建空白纹理
  std::vector<uint8_t> emptyData(width * height * 4, 0);
  texture_ = makePtr<GLTexture>(width, height, emptyData.data(), 4);
  
  // 初始化矩形打包根节点
  root_ = std::make_unique<PackNode>(0, 0, width, height);
  
  E2D_LOG_INFO("Created texture atlas page: {}x{}", width, height);
}

TextureAtlasPage::~TextureAtlasPage() = default;

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

const AtlasEntry* TextureAtlasPage::getEntry(const std::string& name) const {
  auto it = entries_.find(name);
  if (it != entries_.end()) {
    return &it->second;
  }
  return nullptr;
}

float TextureAtlasPage::getUsageRatio() const {
  return static_cast<float>(usedArea_) / (width_ * height_);
}

// ============================================================================
// TextureAtlas 实现
// ============================================================================

TextureAtlas::TextureAtlas()
    : pageSize_(TextureAtlasPage::DEFAULT_SIZE),
      sizeThreshold_(256),
      enabled_(true),
      initialized_(false) {
}

TextureAtlas::~TextureAtlas() = default;

void TextureAtlas::init(int pageSize) {
  pageSize_ = pageSize;
  initialized_ = true;
  E2D_LOG_INFO("TextureAtlas initialized with page size: {}", pageSize);
}

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

bool TextureAtlas::contains(const std::string& name) const {
  return entryToPage_.find(name) != entryToPage_.end();
}

const Texture* TextureAtlas::getAtlasTexture(const std::string& name) const {
  auto it = entryToPage_.find(name);
  if (it != entryToPage_.end()) {
    return it->second->getTexture().get();
  }
  return nullptr;
}

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

void TextureAtlas::clear() {
  pages_.clear();
  entryToPage_.clear();
  E2D_LOG_INFO("TextureAtlas cleared");
}

// ============================================================================
// TextureAtlasManager 单例实现
// ============================================================================

TextureAtlasManager& TextureAtlasManager::getInstance() {
  static TextureAtlasManager instance;
  return instance;
}

} // namespace extra2d
