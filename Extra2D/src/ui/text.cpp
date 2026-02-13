#include <cstdarg>
#include <cstdio>
#include <extra2d/core/string.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/ui/text.h>


namespace extra2d {

/**
 * @brief 默认构造函数
 */
Text::Text() { setAnchor(0.0f, 0.0f); }

/**
 * @brief 带文本的构造函数
 * @param text 初始文本内容
 */
Text::Text(const std::string &text) : text_(text) {
  sizeDirty_ = true;
  setAnchor(0.0f, 0.0f);
}

/**
 * @brief 设置文本内容
 * @param text 新的文本内容
 */
void Text::setText(const std::string &text) {
  text_ = text;
  sizeDirty_ = true;
  updateSpatialIndex();
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void Text::setFont(Ptr<FontAtlas> font) {
  font_ = font;
  sizeDirty_ = true;
  updateSpatialIndex();
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void Text::setTextColor(const Color &color) { color_ = color; }

/**
 * @brief 设置字体大小
 * @param size 字体大小
 */
void Text::setFontSize(int size) {
  fontSize_ = size;
  sizeDirty_ = true;
  updateSpatialIndex();
}

/**
 * @brief 设置水平对齐方式
 * @param align 对齐方式
 */
void Text::setAlignment(Alignment align) {
  alignment_ = align;
  updateSpatialIndex();
}

/**
 * @brief 设置垂直对齐方式
 * @param align 垂直对齐方式
 */
void Text::setVerticalAlignment(VerticalAlignment align) {
  verticalAlignment_ = align;
  updateSpatialIndex();
}

/**
 * @brief 获取文本尺寸
 * @return 文本的宽度和高度
 */
Vec2 Text::getTextSize() const {
  updateCache();
  return cachedSize_;
}

/**
 * @brief 获取行高
 * @return 行高值
 */
float Text::getLineHeight() const {
  if (font_) {
    return font_->getLineHeight();
  }
  return static_cast<float>(fontSize_);
}

/**
 * @brief 更新缓存
 */
void Text::updateCache() const {
  if (!sizeDirty_ || !font_) {
    return;
  }

  cachedSize_ = font_->measureText(text_);
  sizeDirty_ = false;
}

/**
 * @brief 计算绘制位置
 * @return 绘制位置坐标
 */
Vec2 Text::calculateDrawPosition() const {
  Vec2 pos = getPosition();
  Vec2 textSize = getTextSize();
  Size widgetSize = getSize();
  Vec2 anchor = getAnchor();

  float refWidth = widgetSize.empty() ? textSize.x : widgetSize.width;
  float refHeight = widgetSize.empty() ? textSize.y : widgetSize.height;

  pos.x -= textSize.x * anchor.x;
  pos.y -= textSize.y * anchor.y;

  if (!widgetSize.empty()) {
    switch (alignment_) {
    case Alignment::Center:
      pos.x += (refWidth - textSize.x) * 0.5f;
      break;
    case Alignment::Right:
      pos.x += refWidth - textSize.x;
      break;
    case Alignment::Left:
    default:
      break;
    }
  }

  if (!widgetSize.empty()) {
    switch (verticalAlignment_) {
    case VerticalAlignment::Middle:
      pos.y += (refHeight - textSize.y) * 0.5f;
      break;
    case VerticalAlignment::Bottom:
      pos.y += refHeight - textSize.y;
      break;
    case VerticalAlignment::Top:
    default:
      break;
    }
  }

  return pos;
}

/**
 * @brief 创建空文本对象
 * @return 文本对象指针
 */
Ptr<Text> Text::create() { return makePtr<Text>(); }

/**
 * @brief 创建带文本的对象
 * @param text 文本内容
 * @return 文本对象指针
 */
Ptr<Text> Text::create(const std::string &text) { return makePtr<Text>(text); }

/**
 * @brief 创建带文本和字体的对象
 * @param text 文本内容
 * @param font 字体图集
 * @return 文本对象指针
 */
Ptr<Text> Text::create(const std::string &text, Ptr<FontAtlas> font) {
  auto t = makePtr<Text>(text);
  t->setFont(font);
  return t;
}

/**
 * @brief 格式化创建文本
 * @param fmt 格式字符串
 * @param ... 可变参数
 * @return 文本对象指针
 */
Ptr<Text> Text::createFormat(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return makePtr<Text>(buffer);
}

/**
 * @brief 格式化创建文本（带字体）
 * @param font 字体图集
 * @param fmt 格式字符串
 * @param ... 可变参数
 * @return 文本对象指针
 */
Ptr<Text> Text::createFormat(Ptr<FontAtlas> font, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  auto t = makePtr<Text>(buffer);
  t->setFont(font);
  return t;
}

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
Rect Text::getBoundingBox() const {
  if (!font_ || text_.empty()) {
    return Rect();
  }

  updateCache();
  Vec2 size = cachedSize_;
  if (size.x <= 0.0f || size.y <= 0.0f) {
    return Rect();
  }

  Vec2 pos = calculateDrawPosition();
  return Rect(pos.x, pos.y, size.x, size.y);
}

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
void Text::onDrawWidget(RenderBackend &renderer) {
  if (!font_ || text_.empty()) {
    return;
  }

  Vec2 pos = calculateDrawPosition();
  renderer.drawText(*font_, text_, pos, color_);
}

} // namespace extra2d
