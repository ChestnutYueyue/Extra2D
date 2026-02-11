#include <extra2d/ui/text.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>
#include <cstdarg>
#include <cstdio>

namespace extra2d {

Text::Text() {
  // 文字默认锚点为左上角，这样setPosition(0, 0)会在左上角显示
  setAnchor(0.0f, 0.0f);
}

Text::Text(const std::string &text) : text_(text) {
  sizeDirty_ = true;
  // 文字默认锚点为左上角，这样setPosition(0, 0)会在左上角显示
  setAnchor(0.0f, 0.0f);
}

// ------------------------------------------------------------------------
// 链式调用构建器方法
// ------------------------------------------------------------------------
Text *Text::withPosition(float x, float y) {
  setPosition(x, y);
  return this;
}

Text *Text::withPosition(const Vec2 &pos) {
  setPosition(pos);
  return this;
}

Text *Text::withAnchor(float x, float y) {
  setAnchor(x, y);
  return this;
}

Text *Text::withAnchor(const Vec2 &anchor) {
  setAnchor(anchor);
  return this;
}

Text *Text::withTextColor(const Color &color) {
  setTextColor(color);
  return this;
}

Text *Text::withFont(Ptr<FontAtlas> font) {
  setFont(font);
  return this;
}

Text *Text::withFontSize(int size) {
  setFontSize(size);
  return this;
}

Text *Text::withAlignment(Alignment align) {
  setAlignment(align);
  return this;
}

Text *Text::withVerticalAlignment(VerticalAlignment align) {
  setVerticalAlignment(align);
  return this;
}

// ------------------------------------------------------------------------
// 链式调用 - 坐标空间设置
// ------------------------------------------------------------------------
Text *Text::withCoordinateSpace(CoordinateSpace space) {
  setCoordinateSpace(space);
  return this;
}

Text *Text::withScreenPosition(float x, float y) {
  setScreenPosition(x, y);
  return this;
}

Text *Text::withScreenPosition(const Vec2 &pos) {
  setScreenPosition(pos);
  return this;
}

Text *Text::withCameraOffset(float x, float y) {
  setCameraOffset(x, y);
  return this;
}

Text *Text::withCameraOffset(const Vec2 &offset) {
  setCameraOffset(offset);
  return this;
}

// ------------------------------------------------------------------------
// 普通设置方法
// ------------------------------------------------------------------------
void Text::setText(const std::string &text) {
  text_ = text;
  sizeDirty_ = true;
  updateSpatialIndex();
}

void Text::setFont(Ptr<FontAtlas> font) {
  font_ = font;
  sizeDirty_ = true;
  updateSpatialIndex();
}

void Text::setTextColor(const Color &color) { color_ = color; }

void Text::setFontSize(int size) {
  fontSize_ = size;
  sizeDirty_ = true;
  updateSpatialIndex();
}

void Text::setAlignment(Alignment align) {
  alignment_ = align;
  updateSpatialIndex();
}

void Text::setVerticalAlignment(VerticalAlignment align) {
  verticalAlignment_ = align;
  updateSpatialIndex();
}

Vec2 Text::getTextSize() const {
  updateCache();
  return cachedSize_;
}

float Text::getLineHeight() const {
  if (font_) {
    return font_->getLineHeight();
  }
  return static_cast<float>(fontSize_);
}

void Text::updateCache() const {
  if (!sizeDirty_ || !font_) {
    return;
  }

  cachedSize_ = font_->measureText(text_);
  sizeDirty_ = false;
}

Vec2 Text::calculateDrawPosition() const {
  Vec2 pos = getPosition();
  Vec2 textSize = getTextSize();
  Size widgetSize = getSize();
  Vec2 anchor = getAnchor();

  // 如果设置了控件大小，使用控件大小作为对齐参考
  float refWidth = widgetSize.empty() ? textSize.x : widgetSize.width;
  float refHeight = widgetSize.empty() ? textSize.y : widgetSize.height;

  // 锚点调整：锚点(0.5, 0.5)表示文本中心在pos位置
  // 需要将文本向左上方偏移锚点比例 * 文本大小
  pos.x -= textSize.x * anchor.x;
  pos.y -= textSize.y * anchor.y;

  // 水平对齐（仅在设置了控件大小时生效）
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

  // 垂直对齐（仅在设置了控件大小时生效）
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

Ptr<Text> Text::create() { return makePtr<Text>(); }

Ptr<Text> Text::create(const std::string &text) { return makePtr<Text>(text); }

Ptr<Text> Text::create(const std::string &text, Ptr<FontAtlas> font) {
  auto t = makePtr<Text>(text);
  t->setFont(font);
  return t;
}

// ------------------------------------------------------------------------
// 格式化创建方法
// ------------------------------------------------------------------------
Ptr<Text> Text::createFormat(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  return makePtr<Text>(buffer);
}

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

void Text::onDrawWidget(RenderBackend &renderer) {
  if (!font_ || text_.empty()) {
    return;
  }

  Vec2 pos = calculateDrawPosition();
  renderer.drawText(*font_, text_, pos, color_);
}

} // namespace extra2d
