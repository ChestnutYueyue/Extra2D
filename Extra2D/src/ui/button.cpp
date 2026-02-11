#include <algorithm>
#include <cmath>
#include <extra2d/app/application.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/ui/button.h>
#include <extra2d/core/string.h>

namespace extra2d {

// ============================================================================
// Button 实现
// ============================================================================

Button::Button() {
  // 按钮默认锚点为左上角，这样setPosition(0, 0)会在左上角显示
  setAnchor(0.0f, 0.0f);
  setSpatialIndexed(false);
  
  auto &dispatcher = getEventDispatcher();
  dispatcher.addListener(EventType::UIHoverEnter, [this](Event &) {
    hovered_ = true;
    auto &app = Application::instance();
    app.window().setCursor(hoverCursor_);
    cursorChanged_ = true;
  });
  dispatcher.addListener(EventType::UIHoverExit, [this](Event &) {
    hovered_ = false;
    pressed_ = false;
    if (cursorChanged_) {
      auto &app = Application::instance();
      app.window().resetCursor();
      cursorChanged_ = false;
    }
  });
  dispatcher.addListener(EventType::UIPressed,
                         [this](Event &) { pressed_ = true; });
  dispatcher.addListener(EventType::UIReleased,
                         [this](Event &) { pressed_ = false; });
  dispatcher.addListener(EventType::UIClicked, [this](Event &) {
    if (onClick_)
      onClick_();
  });
}

Button::Button(const std::string &text) : Button() {
  text_ = text;
}

// ------------------------------------------------------------------------
// 静态创建方法
// ------------------------------------------------------------------------
Ptr<Button> Button::create() { 
  return makePtr<Button>(); 
}

Ptr<Button> Button::create(const std::string &text) { 
  return makePtr<Button>(text); 
}

Ptr<Button> Button::create(const std::string &text, Ptr<FontAtlas> font) {
  auto btn = makePtr<Button>(text);
  btn->setFont(font);
  return btn;
}

// ------------------------------------------------------------------------
// 链式调用构建器方法
// ------------------------------------------------------------------------
Button *Button::withPosition(float x, float y) {
  setPosition(x, y);
  return this;
}

Button *Button::withPosition(const Vec2 &pos) {
  setPosition(pos);
  return this;
}

Button *Button::withAnchor(float x, float y) {
  setAnchor(x, y);
  return this;
}

Button *Button::withAnchor(const Vec2 &anchor) {
  setAnchor(anchor);
  return this;
}

Button *Button::withText(const std::string &text) {
  setText(text);
  return this;
}

Button *Button::withFont(Ptr<FontAtlas> font) {
  setFont(font);
  return this;
}

Button *Button::withTextColor(const Color &color) {
  setTextColor(color);
  return this;
}

Button *Button::withBackgroundColor(const Color &normal, const Color &hover,
                                    const Color &pressed) {
  setBackgroundColor(normal, hover, pressed);
  return this;
}

Button *Button::withSize(float width, float height) {
  setSize(width, height);
  return this;
}

Button *Button::withPadding(const Vec2 &padding) {
  setPadding(padding);
  return this;
}

Button *Button::withPadding(float x, float y) {
  setPadding(Vec2(x, y));
  return this;
}

Button *Button::withBorder(const Color &color, float width) {
  setBorder(color, width);
  return this;
}

Button *Button::withCornerRadius(float radius) {
  setCornerRadius(radius);
  return this;
}

Button *Button::withRoundedCornersEnabled(bool enabled) {
  setRoundedCornersEnabled(enabled);
  return this;
}

Button *Button::withHoverCursor(CursorShape cursor) {
  setHoverCursor(cursor);
  return this;
}

// ------------------------------------------------------------------------
// 链式调用 - 坐标空间设置
// ------------------------------------------------------------------------
Button *Button::withCoordinateSpace(CoordinateSpace space) {
  setCoordinateSpace(space);
  return this;
}

Button *Button::withScreenPosition(float x, float y) {
  setScreenPosition(x, y);
  return this;
}

Button *Button::withScreenPosition(const Vec2 &pos) {
  setScreenPosition(pos);
  return this;
}

Button *Button::withCameraOffset(float x, float y) {
  setCameraOffset(x, y);
  return this;
}

Button *Button::withCameraOffset(const Vec2 &offset) {
  setCameraOffset(offset);
  return this;
}

// ------------------------------------------------------------------------
// 普通设置方法
// ------------------------------------------------------------------------
void Button::setText(const std::string &text) {
  text_ = text;
  if (font_ && getSize().empty()) {
    Vec2 textSize = font_->measureText(text_);
    setSize(textSize.x + padding_.x * 2.0f, textSize.y + padding_.y * 2.0f);
  }
}

void Button::setFont(Ptr<FontAtlas> font) {
  font_ = font;
  if (font_ && getSize().empty() && !text_.empty()) {
    Vec2 textSize = font_->measureText(text_);
    setSize(textSize.x + padding_.x * 2.0f, textSize.y + padding_.y * 2.0f);
  }
}

void Button::setPadding(const Vec2 &padding) {
  padding_ = padding;
  if (font_ && getSize().empty() && !text_.empty()) {
    Vec2 textSize = font_->measureText(text_);
    setSize(textSize.x + padding_.x * 2.0f, textSize.y + padding_.y * 2.0f);
  }
}

void Button::setTextColor(const Color &color) { 
  textColor_ = color; 
}

void Button::setBackgroundColor(const Color &normal, const Color &hover,
                                const Color &pressed) {
  bgNormal_ = normal;
  bgHover_ = hover;
  bgPressed_ = pressed;
}

void Button::setBorder(const Color &color, float width) {
  borderColor_ = color;
  borderWidth_ = width;
}

void Button::setCornerRadius(float radius) {
  cornerRadius_ = std::max(0.0f, radius);
}

void Button::setRoundedCornersEnabled(bool enabled) {
  roundedCornersEnabled_ = enabled;
}

void Button::setUseAlphaMaskForHitTest(bool enabled) {
  useAlphaMaskForHitTest_ = enabled;
}

void Button::setOnClick(Function<void()> callback) {
  onClick_ = std::move(callback);
}

void Button::setHoverCursor(CursorShape cursor) { 
  hoverCursor_ = cursor; 
}

void Button::setBackgroundImage(Ptr<Texture> normal, Ptr<Texture> hover,
                                Ptr<Texture> pressed) {
  imgNormal_ = normal;
  imgHover_ = hover ? hover : normal;
  imgPressed_ = pressed ? pressed : (hover ? hover : normal);
  useImageBackground_ = (normal != nullptr);

  if (useImageBackground_ && scaleMode_ == ImageScaleMode::Original && normal) {
    setSize(static_cast<float>(normal->getWidth()),
            static_cast<float>(normal->getHeight()));
  }
}

void Button::setBackgroundImageScaleMode(ImageScaleMode mode) {
  scaleMode_ = mode;
  if (useImageBackground_ && scaleMode_ == ImageScaleMode::Original &&
      imgNormal_) {
    setSize(static_cast<float>(imgNormal_->getWidth()),
            static_cast<float>(imgNormal_->getHeight()));
  }
}

void Button::setCustomSize(const Vec2 &size) { 
  setSize(size.x, size.y); 
}

void Button::setCustomSize(float width, float height) {
  setSize(width, height);
}

Rect Button::getBoundingBox() const {
  auto pos = getRenderPosition();
  auto anchor = getAnchor();
  auto scale = getScale();
  auto size = getSize();

  if (size.empty()) {
    return Rect();
  }

  float w = size.width * scale.x;
  float h = size.height * scale.y;
  float x0 = pos.x - size.width * anchor.x * scale.x;
  float y0 = pos.y - size.height * anchor.y * scale.y;

  return Rect(x0, y0, w, h);
}

Vec2 Button::calculateImageSize(const Vec2 &buttonSize, const Vec2 &imageSize) {
  switch (scaleMode_) {
  case ImageScaleMode::Original:
    return imageSize;

  case ImageScaleMode::Stretch:
    return buttonSize;

  case ImageScaleMode::ScaleFit: {
    float scaleX = buttonSize.x / imageSize.x;
    float scaleY = buttonSize.y / imageSize.y;
    float scale = std::min(scaleX, scaleY);
    return Vec2(imageSize.x * scale, imageSize.y * scale);
  }

  case ImageScaleMode::ScaleFill: {
    float scaleX = buttonSize.x / imageSize.x;
    float scaleY = buttonSize.y / imageSize.y;
    float scale = std::max(scaleX, scaleY);
    return Vec2(imageSize.x * scale, imageSize.y * scale);
  }
  }
  return imageSize;
}

void Button::drawBackgroundImage(RenderBackend &renderer, const Rect &rect) {
  Texture *texture = nullptr;
  if (pressed_ && imgPressed_) {
    texture = imgPressed_.get();
  } else if (hovered_ && imgHover_) {
    texture = imgHover_.get();
  } else if (imgNormal_) {
    texture = imgNormal_.get();
  }

  if (!texture)
    return;

  Vec2 imageSize(static_cast<float>(texture->getWidth()),
                 static_cast<float>(texture->getHeight()));
  Vec2 buttonSize(rect.size.width, rect.size.height);
  Vec2 drawSize = calculateImageSize(buttonSize, imageSize);

  Vec2 drawPos(rect.origin.x + (rect.size.width - drawSize.x) * 0.5f,
               rect.origin.y + (rect.size.height - drawSize.y) * 0.5f);

  Rect destRect(drawPos.x, drawPos.y, drawSize.x, drawSize.y);

  renderer.drawSprite(*texture, destRect, Rect(0, 0, imageSize.x, imageSize.y),
                      Colors::White, 0.0f, Vec2::Zero());
}

void Button::drawRoundedRect(RenderBackend &renderer, const Rect &rect,
                             const Color &color, float radius) {
  float maxRadius = std::min(rect.size.width, rect.size.height) * 0.5f;
  radius = std::min(radius, maxRadius);

  if (radius <= 0.0f) {
    renderer.drawRect(rect, color, borderWidth_);
    return;
  }

  const int segments = 8;
  float x = rect.origin.x;
  float y = rect.origin.y;
  float w = rect.size.width;
  float h = rect.size.height;
  float r = radius;

  renderer.drawLine(Vec2(x + r, y), Vec2(x + w - r, y), color, borderWidth_);
  renderer.drawLine(Vec2(x + r, y + h), Vec2(x + w - r, y + h), color, borderWidth_);
  renderer.drawLine(Vec2(x, y + r), Vec2(x, y + h - r), color, borderWidth_);
  renderer.drawLine(Vec2(x + w, y + r), Vec2(x + w, y + h - r), color, borderWidth_);

  for (int i = 0; i < segments; i++) {
    float angle1 = 3.14159f * 0.5f * (float)i / segments + 3.14159f;
    float angle2 = 3.14159f * 0.5f * (float)(i + 1) / segments + 3.14159f;
    Vec2 p1(x + r + r * cosf(angle1), y + r + r * sinf(angle1));
    Vec2 p2(x + r + r * cosf(angle2), y + r + r * sinf(angle2));
    renderer.drawLine(p1, p2, color, borderWidth_);
  }
  for (int i = 0; i < segments; i++) {
    float angle1 = 3.14159f * 0.5f * (float)i / segments + 3.14159f * 1.5f;
    float angle2 = 3.14159f * 0.5f * (float)(i + 1) / segments + 3.14159f * 1.5f;
    Vec2 p1(x + w - r + r * cosf(angle1), y + r + r * sinf(angle1));
    Vec2 p2(x + w - r + r * cosf(angle2), y + r + r * sinf(angle2));
    renderer.drawLine(p1, p2, color, borderWidth_);
  }
  for (int i = 0; i < segments; i++) {
    float angle1 = 3.14159f * 0.5f * (float)i / segments;
    float angle2 = 3.14159f * 0.5f * (float)(i + 1) / segments;
    Vec2 p1(x + w - r + r * cosf(angle1), y + h - r + r * sinf(angle1));
    Vec2 p2(x + w - r + r * cosf(angle2), y + h - r + r * sinf(angle2));
    renderer.drawLine(p1, p2, color, borderWidth_);
  }
  for (int i = 0; i < segments; i++) {
    float angle1 = 3.14159f * 0.5f * (float)i / segments + 3.14159f * 0.5f;
    float angle2 = 3.14159f * 0.5f * (float)(i + 1) / segments + 3.14159f * 0.5f;
    Vec2 p1(x + r + r * cosf(angle1), y + h - r + r * sinf(angle1));
    Vec2 p2(x + r + r * cosf(angle2), y + h - r + r * sinf(angle2));
    renderer.drawLine(p1, p2, color, borderWidth_);
  }
}

void Button::fillRoundedRect(RenderBackend &renderer, const Rect &rect,
                             const Color &color, float radius) {
  float maxRadius = std::min(rect.size.width, rect.size.height) * 0.5f;
  radius = std::min(radius, maxRadius);

  if (radius <= 0.0f) {
    renderer.fillRect(rect, color);
    return;
  }

  const int segments = 8;
  float x = rect.origin.x;
  float y = rect.origin.y;
  float w = rect.size.width;
  float h = rect.size.height;
  float r = radius;

  std::vector<Vec2> vertices;

  vertices.push_back(Vec2(x + r, y + r));
  vertices.push_back(Vec2(x + w - r, y + r));
  vertices.push_back(Vec2(x + w - r, y + h - r));
  vertices.push_back(Vec2(x + r, y + h - r));

  renderer.fillPolygon(vertices, color);

  renderer.fillRect(Rect(x + r, y, w - 2 * r, r), color);
  renderer.fillRect(Rect(x + r, y + h - r, w - 2 * r, r), color);
  renderer.fillRect(Rect(x, y + r, r, h - 2 * r), color);
  renderer.fillRect(Rect(x + w - r, y + r, r, h - 2 * r), color);

  vertices.clear();
  vertices.push_back(Vec2(x + r, y + r));
  for (int i = 0; i <= segments; i++) {
    float angle = 3.14159f + 3.14159f * 0.5f * (float)i / segments;
    vertices.push_back(Vec2(x + r + r * cosf(angle), y + r + r * sinf(angle)));
  }
  renderer.fillPolygon(vertices, color);

  vertices.clear();
  vertices.push_back(Vec2(x + w - r, y + r));
  for (int i = 0; i <= segments; i++) {
    float angle = 3.14159f * 1.5f + 3.14159f * 0.5f * (float)i / segments;
    vertices.push_back(
        Vec2(x + w - r + r * cosf(angle), y + r + r * sinf(angle)));
  }
  renderer.fillPolygon(vertices, color);

  vertices.clear();
  vertices.push_back(Vec2(x + w - r, y + h - r));
  for (int i = 0; i <= segments; i++) {
    float angle = 0 + 3.14159f * 0.5f * (float)i / segments;
    vertices.push_back(
        Vec2(x + w - r + r * cosf(angle), y + h - r + r * sinf(angle)));
  }
  renderer.fillPolygon(vertices, color);

  vertices.clear();
  vertices.push_back(Vec2(x + r, y + h - r));
  for (int i = 0; i <= segments; i++) {
    float angle = 3.14159f * 0.5f + 3.14159f * 0.5f * (float)i / segments;
    vertices.push_back(
        Vec2(x + r + r * cosf(angle), y + h - r + r * sinf(angle)));
  }
  renderer.fillPolygon(vertices, color);
}

void Button::onDrawWidget(RenderBackend &renderer) {
  Rect rect = getBoundingBox();
  if (rect.empty()) {
    return;
  }

  if (useImageBackground_) {
    drawBackgroundImage(renderer, rect);
  } else {
    renderer.endSpriteBatch();

    Color bg = bgNormal_;
    if (pressed_) {
      bg = bgPressed_;
    } else if (hovered_) {
      bg = bgHover_;
    }

    if (roundedCornersEnabled_) {
      fillRoundedRect(renderer, rect, bg, cornerRadius_);
    } else {
      renderer.fillRect(rect, bg);
    }

    renderer.beginSpriteBatch();
  }

  renderer.endSpriteBatch();

  if (borderWidth_ > 0.0f) {
    if (roundedCornersEnabled_) {
      drawRoundedRect(renderer, rect, borderColor_, cornerRadius_);
    } else {
      renderer.drawRect(rect, borderColor_, borderWidth_);
    }
  }

  renderer.beginSpriteBatch();

  if (font_ && !text_.empty()) {
    Vec2 textSize = font_->measureText(text_);

    Vec2 textPos(rect.center().x - textSize.x * 0.5f,
                 rect.center().y - textSize.y * 0.5f);

    float minX = rect.left() + padding_.x;
    float minY = rect.top() + padding_.y;
    float maxX = rect.right() - padding_.x - textSize.x;
    float maxY = rect.bottom() - padding_.y - textSize.y;

    textPos.x = std::max(minX, std::min(textPos.x, maxX));
    textPos.y = std::max(minY, std::min(textPos.y, maxY));

    Color finalTextColor = textColor_;
    finalTextColor.a = 1.0f;

    renderer.drawText(*font_, text_, textPos, finalTextColor);
  }
}

// ============================================================================
// ToggleImageButton 实现
// ============================================================================

ToggleImageButton::ToggleImageButton() {
  setOnClick([this]() { toggle(); });
}

Ptr<ToggleImageButton> ToggleImageButton::create() {
  return makePtr<ToggleImageButton>();
}

void ToggleImageButton::setStateImages(Ptr<Texture> stateOffNormal,
                                       Ptr<Texture> stateOnNormal,
                                       Ptr<Texture> stateOffHover,
                                       Ptr<Texture> stateOnHover,
                                       Ptr<Texture> stateOffPressed,
                                       Ptr<Texture> stateOnPressed) {
  imgOffNormal_ = stateOffNormal;
  imgOnNormal_ = stateOnNormal;
  imgOffHover_ = stateOffHover ? stateOffHover : stateOffNormal;
  imgOnHover_ = stateOnHover ? stateOnHover : stateOnNormal;
  imgOffPressed_ = stateOffPressed ? stateOffPressed : stateOffNormal;
  imgOnPressed_ = stateOnPressed ? stateOnPressed : stateOnNormal;

  if (imgOffNormal_) {
    setSize(static_cast<float>(imgOffNormal_->getWidth()),
            static_cast<float>(imgOffNormal_->getHeight()));
  }
}

void ToggleImageButton::setOn(bool on) {
  if (isOn_ != on) {
    isOn_ = on;
    if (onStateChange_) {
      onStateChange_(isOn_);
    }
  }
}

void ToggleImageButton::toggle() { 
  setOn(!isOn_); 
}

void ToggleImageButton::setOnStateChange(Function<void(bool)> callback) {
  onStateChange_ = std::move(callback);
}

void ToggleImageButton::setStateText(const std::string &textOff,
                                     const std::string &textOn) {
  textOff_ = textOff;
  textOn_ = textOn;
  useStateText_ = true;
}

void ToggleImageButton::setStateTextColor(const Color &colorOff,
                                          const Color &colorOn) {
  textColorOff_ = colorOff;
  textColorOn_ = colorOn;
  useStateTextColor_ = true;
}

void ToggleImageButton::onDrawWidget(RenderBackend &renderer) {
  Rect rect = getBoundingBox();
  if (rect.empty()) {
    return;
  }

  Ptr<Texture> texture = nullptr;

  if (isOn_) {
    if (isPressed() && imgOnPressed_) {
      texture = imgOnPressed_;
    } else if (isHovered() && imgOnHover_) {
      texture = imgOnHover_;
    } else {
      texture = imgOnNormal_;
    }
  } else {
    if (isPressed() && imgOffPressed_) {
      texture = imgOffPressed_;
    } else if (isHovered() && imgOffHover_) {
      texture = imgOffHover_;
    } else {
      texture = imgOffNormal_;
    }
  }

  if (texture) {
    Vec2 imageSize(static_cast<float>(texture->getWidth()),
                   static_cast<float>(texture->getHeight()));
    Vec2 buttonSize(rect.size.width, rect.size.height);
    Vec2 drawSize = imageSize;

    Vec2 drawPos(rect.origin.x + (buttonSize.x - drawSize.x) * 0.5f,
                 rect.origin.y + (buttonSize.y - drawSize.y) * 0.5f);

    Rect destRect(drawPos.x, drawPos.y, drawSize.x, drawSize.y);
    renderer.drawSprite(*texture, destRect,
                        Rect(0, 0, imageSize.x, imageSize.y), Colors::White,
                        0.0f, Vec2::Zero());
  }

  renderer.endSpriteBatch();

  float borderWidth = 1.0f;
  Color borderColor =
      isOn_ ? Color(0.0f, 1.0f, 0.0f, 0.8f) : Color(0.6f, 0.6f, 0.6f, 1.0f);
  if (borderWidth > 0.0f) {
    if (isRoundedCornersEnabled()) {
      drawRoundedRect(renderer, rect, borderColor, getCornerRadius());
    } else {
      renderer.drawRect(rect, borderColor, borderWidth);
    }
  }

  renderer.beginSpriteBatch();

  auto font = getFont();
  if (font) {
    std::string textToDraw;
    if (useStateText_) {
      textToDraw = isOn_ ? textOn_ : textOff_;
    } else {
      textToDraw = getText();
    }

    Color colorToUse;
    if (useStateTextColor_) {
      colorToUse = isOn_ ? textColorOn_ : textColorOff_;
    } else {
      colorToUse = getTextColor();
    }

    if (!textToDraw.empty()) {
      Vec2 textSize = font->measureText(textToDraw);
      Vec2 textPos(rect.center().x - textSize.x * 0.5f,
                   rect.center().y - textSize.y * 0.5f);

      colorToUse.a = 1.0f;

      renderer.drawText(*font, textToDraw, textPos, colorToUse);
    }
  }
}

} // namespace extra2d
