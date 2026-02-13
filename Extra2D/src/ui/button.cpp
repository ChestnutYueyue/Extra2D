#include <algorithm>
#include <cmath>
#include <extra2d/app/application.h>
#include <extra2d/core/string.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/ui/button.h>

namespace extra2d {

// ============================================================================
// Button 实现
// ============================================================================

/**
 * @brief 默认构造函数
 */
Button::Button() {
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
    if (toggleMode_) {
      toggle();
    }
    if (onClick_) {
      onClick_();
    }
  });
}

/**
 * @brief 带文本的构造函数
 * @param text 按钮文本
 */
Button::Button(const std::string &text) : Button() { text_ = text; }

// ------------------------------------------------------------------------
// 静态创建方法
// ------------------------------------------------------------------------
/**
 * @brief 创建空按钮对象
 * @return 按钮对象指针
 */
Ptr<Button> Button::create() { return makePtr<Button>(); }

/**
 * @brief 创建带文本的按钮对象
 * @param text 按钮文本
 * @return 按钮对象指针
 */
Ptr<Button> Button::create(const std::string &text) {
  return makePtr<Button>(text);
}

/**
 * @brief 创建带文本和字体的按钮对象
 * @param text 按钮文本
 * @param font 字体图集
 * @return 按钮对象指针
 */
Ptr<Button> Button::create(const std::string &text, Ptr<FontAtlas> font) {
  auto btn = makePtr<Button>(text);
  btn->setFont(font);
  return btn;
}

// ------------------------------------------------------------------------
// 普通设置方法
// ------------------------------------------------------------------------
/**
 * @brief 设置按钮文本
 * @param text 文本内容
 */
void Button::setText(const std::string &text) {
  text_ = text;
  if (font_ && getSize().empty()) {
    Vec2 textSize = font_->measureText(text_);
    setSize(textSize.x + padding_.x * 2.0f, textSize.y + padding_.y * 2.0f);
  }
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void Button::setFont(Ptr<FontAtlas> font) {
  font_ = font;
  if (font_ && getSize().empty() && !text_.empty()) {
    Vec2 textSize = font_->measureText(text_);
    setSize(textSize.x + padding_.x * 2.0f, textSize.y + padding_.y * 2.0f);
  }
}

/**
 * @brief 设置内边距（Vec2版本）
 * @param padding 内边距向量
 */
void Button::setPadding(const Vec2 &padding) {
  padding_ = padding;
  if (font_ && getSize().empty() && !text_.empty()) {
    Vec2 textSize = font_->measureText(text_);
    setSize(textSize.x + padding_.x * 2.0f, textSize.y + padding_.y * 2.0f);
  }
}

/**
 * @brief 设置内边距（分量版本）
 * @param x X方向内边距
 * @param y Y方向内边距
 */
void Button::setPadding(float x, float y) {
  setPadding(Vec2(x, y));
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void Button::setTextColor(const Color &color) { textColor_ = color; }

/**
 * @brief 设置背景颜色
 * @param normal 普通状态颜色
 * @param hover 悬停状态颜色
 * @param pressed 按下状态颜色
 */
void Button::setBackgroundColor(const Color &normal, const Color &hover,
                                const Color &pressed) {
  bgNormal_ = normal;
  bgHover_ = hover;
  bgPressed_ = pressed;
}

/**
 * @brief 设置边框
 * @param color 边框颜色
 * @param width 边框宽度
 */
void Button::setBorder(const Color &color, float width) {
  borderColor_ = color;
  borderWidth_ = width;
}

/**
 * @brief 设置圆角半径
 * @param radius 圆角半径
 */
void Button::setCornerRadius(float radius) {
  cornerRadius_ = std::max(0.0f, radius);
}

/**
 * @brief 设置是否启用圆角
 * @param enabled 是否启用
 */
void Button::setRoundedCornersEnabled(bool enabled) {
  roundedCornersEnabled_ = enabled;
}

/**
 * @brief 设置是否使用Alpha遮罩进行点击检测
 * @param enabled 是否启用
 */
void Button::setUseAlphaMaskForHitTest(bool enabled) {
  useAlphaMaskForHitTest_ = enabled;
}

/**
 * @brief 设置点击回调
 * @param callback 回调函数
 */
void Button::setOnClick(Function<void()> callback) {
  onClick_ = std::move(callback);
}

/**
 * @brief 设置是否为切换模式
 * @param enabled 是否启用
 */
void Button::setToggleMode(bool enabled) { toggleMode_ = enabled; }

/**
 * @brief 设置开关状态
 * @param on 是否开启
 */
void Button::setOn(bool on) {
  if (isOn_ != on) {
    isOn_ = on;
    if (onStateChange_) {
      onStateChange_(isOn_);
    }
  }
}

/**
 * @brief 切换状态
 */
void Button::toggle() { setOn(!isOn_); }

/**
 * @brief 设置状态改变回调
 * @param callback 回调函数
 */
void Button::setOnStateChange(Function<void(bool)> callback) {
  onStateChange_ = std::move(callback);
}

/**
 * @brief 设置状态文字
 * @param textOff 关闭状态文字
 * @param textOn 开启状态文字
 */
void Button::setStateText(const std::string &textOff,
                          const std::string &textOn) {
  textOff_ = textOff;
  textOn_ = textOn;
  useStateText_ = true;
}

/**
 * @brief 设置状态文字颜色
 * @param colorOff 关闭状态颜色
 * @param colorOn 开启状态颜色
 */
void Button::setStateTextColor(const Color &colorOff, const Color &colorOn) {
  textColorOff_ = colorOff;
  textColorOn_ = colorOn;
  useStateTextColor_ = true;
}

/**
 * @brief 设置悬停光标
 * @param cursor 光标形状
 */
void Button::setHoverCursor(CursorShape cursor) { hoverCursor_ = cursor; }

/**
 * @brief 设置背景图片
 * @param normal 普通状态图片
 * @param hover 悬停状态图片
 * @param pressed 按下状态图片
 */
void Button::setBackgroundImage(Ptr<Texture> normal, Ptr<Texture> hover,
                                Ptr<Texture> pressed) {
  imgNormal_ = normal;
  imgHover_ = hover ? hover : normal;
  imgPressed_ = pressed ? pressed : (hover ? hover : normal);
  useImageBackground_ = (normal != nullptr);
  useTextureRect_ = false;

  if (useImageBackground_ && scaleMode_ == ImageScaleMode::Original && normal) {
    setSize(static_cast<float>(normal->getWidth()),
            static_cast<float>(normal->getHeight()));
  }
}

/**
 * @brief 设置背景图片（带矩形区域）
 * @param texture 纹理
 * @param rect 矩形区域
 */
void Button::setBackgroundImage(Ptr<Texture> texture, const Rect &rect) {
  imgNormal_ = texture;
  imgHover_ = texture;
  imgPressed_ = texture;
  imgNormalRect_ = rect;
  imgHoverRect_ = rect;
  imgPressedRect_ = rect;
  useImageBackground_ = (texture != nullptr);
  useTextureRect_ = true;
  useStateImages_ = false;

  if (useImageBackground_ && scaleMode_ == ImageScaleMode::Original) {
    setSize(rect.size.width, rect.size.height);
  }
}

/**
 * @brief 设置状态背景图片
 * @param offNormal 关闭状态普通图片
 * @param onNormal 开启状态普通图片
 * @param offHover 关闭状态悬停图片
 * @param onHover 开启状态悬停图片
 * @param offPressed 关闭状态按下图片
 * @param onPressed 开启状态按下图片
 */
void Button::setStateBackgroundImage(
    Ptr<Texture> offNormal, Ptr<Texture> onNormal, Ptr<Texture> offHover,
    Ptr<Texture> onHover, Ptr<Texture> offPressed, Ptr<Texture> onPressed) {
  imgOffNormal_ = offNormal;
  imgOnNormal_ = onNormal;
  imgOffHover_ = offHover ? offHover : offNormal;
  imgOnHover_ = onHover ? onHover : onNormal;
  imgOffPressed_ = offPressed ? offPressed : offNormal;
  imgOnPressed_ = onPressed ? onPressed : onNormal;

  useStateImages_ = (offNormal != nullptr && onNormal != nullptr);
  useImageBackground_ = useStateImages_;
  useTextureRect_ = false;

  if (useStateImages_ && scaleMode_ == ImageScaleMode::Original && offNormal) {
    setSize(static_cast<float>(offNormal->getWidth()),
            static_cast<float>(offNormal->getHeight()));
  }
}

/**
 * @brief 设置背景图片缩放模式
 * @param mode 缩放模式
 */
void Button::setBackgroundImageScaleMode(ImageScaleMode mode) {
  scaleMode_ = mode;
  if (useImageBackground_ && scaleMode_ == ImageScaleMode::Original &&
      imgNormal_) {
    setSize(static_cast<float>(imgNormal_->getWidth()),
            static_cast<float>(imgNormal_->getHeight()));
  }
}

/**
 * @brief 设置自定义尺寸（Vec2版本）
 * @param size 尺寸向量
 */
void Button::setCustomSize(const Vec2 &size) { setSize(size.x, size.y); }

/**
 * @brief 设置自定义尺寸（分量版本）
 * @param width 宽度
 * @param height 高度
 */
void Button::setCustomSize(float width, float height) {
  setSize(width, height);
}

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
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

/**
 * @brief 计算图片尺寸
 * @param buttonSize 按钮尺寸
 * @param imageSize 图片尺寸
 * @return 计算后的图片尺寸
 */
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

/**
 * @brief 绘制背景图片
 * @param renderer 渲染后端
 * @param rect 绘制区域
 */
void Button::drawBackgroundImage(RenderBackend &renderer, const Rect &rect) {
  Texture *texture = nullptr;
  Rect srcRect;

  if (useStateImages_) {
    if (isOn_) {
      if (pressed_ && imgOnPressed_) {
        texture = imgOnPressed_.get();
      } else if (hovered_ && imgOnHover_) {
        texture = imgOnHover_.get();
      } else {
        texture = imgOnNormal_.get();
      }
    } else {
      if (pressed_ && imgOffPressed_) {
        texture = imgOffPressed_.get();
      } else if (hovered_ && imgOffHover_) {
        texture = imgOffHover_.get();
      } else {
        texture = imgOffNormal_.get();
      }
    }
    if (texture) {
      srcRect = Rect(0, 0, static_cast<float>(texture->getWidth()),
                     static_cast<float>(texture->getHeight()));
    }
  } else {
    if (pressed_ && imgPressed_) {
      texture = imgPressed_.get();
      srcRect = useTextureRect_
                    ? imgPressedRect_
                    : Rect(0, 0, static_cast<float>(imgPressed_->getWidth()),
                           static_cast<float>(imgPressed_->getHeight()));
    } else if (hovered_ && imgHover_) {
      texture = imgHover_.get();
      srcRect = useTextureRect_
                    ? imgHoverRect_
                    : Rect(0, 0, static_cast<float>(imgHover_->getWidth()),
                           static_cast<float>(imgHover_->getHeight()));
    } else if (imgNormal_) {
      texture = imgNormal_.get();
      srcRect = useTextureRect_
                    ? imgNormalRect_
                    : Rect(0, 0, static_cast<float>(imgNormal_->getWidth()),
                           static_cast<float>(imgNormal_->getHeight()));
    }
  }

  if (!texture)
    return;

  Vec2 imageSize(srcRect.size.width, srcRect.size.height);
  Vec2 buttonSize(rect.size.width, rect.size.height);
  Vec2 drawSize = calculateImageSize(buttonSize, imageSize);

  Vec2 drawPos(rect.origin.x + (rect.size.width - drawSize.x) * 0.5f,
               rect.origin.y + (rect.size.height - drawSize.y) * 0.5f);

  Rect destRect(drawPos.x, drawPos.y, drawSize.x, drawSize.y);

  renderer.drawSprite(*texture, destRect, srcRect, Colors::White, 0.0f,
                      Vec2::Zero());
}

/**
 * @brief 绘制圆角矩形边框
 * @param renderer 渲染后端
 * @param rect 矩形区域
 * @param color 颜色
 * @param radius 圆角半径
 */
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
  renderer.drawLine(Vec2(x + r, y + h), Vec2(x + w - r, y + h), color,
                    borderWidth_);
  renderer.drawLine(Vec2(x, y + r), Vec2(x, y + h - r), color, borderWidth_);
  renderer.drawLine(Vec2(x + w, y + r), Vec2(x + w, y + h - r), color,
                    borderWidth_);

  for (int i = 0; i < segments; i++) {
    float angle1 = 3.14159f * 0.5f * (float)i / segments + 3.14159f;
    float angle2 = 3.14159f * 0.5f * (float)(i + 1) / segments + 3.14159f;
    Vec2 p1(x + r + r * cosf(angle1), y + r + r * sinf(angle1));
    Vec2 p2(x + r + r * cosf(angle2), y + r + r * sinf(angle2));
    renderer.drawLine(p1, p2, color, borderWidth_);
  }
  for (int i = 0; i < segments; i++) {
    float angle1 = 3.14159f * 0.5f * (float)i / segments + 3.14159f * 1.5f;
    float angle2 =
        3.14159f * 0.5f * (float)(i + 1) / segments + 3.14159f * 1.5f;
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
    float angle2 =
        3.14159f * 0.5f * (float)(i + 1) / segments + 3.14159f * 0.5f;
    Vec2 p1(x + r + r * cosf(angle1), y + h - r + r * sinf(angle1));
    Vec2 p2(x + r + r * cosf(angle2), y + h - r + r * sinf(angle2));
    renderer.drawLine(p1, p2, color, borderWidth_);
  }
}

/**
 * @brief 填充圆角矩形
 * @param renderer 渲染后端
 * @param rect 矩形区域
 * @param color 颜色
 * @param radius 圆角半径
 */
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

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
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

    renderer.endSpriteBatch();

    if (borderWidth_ > 0.0f) {
      if (roundedCornersEnabled_) {
        drawRoundedRect(renderer, rect, borderColor_, cornerRadius_);
      } else {
        renderer.drawRect(rect, borderColor_, borderWidth_);
      }
    }

    renderer.beginSpriteBatch();
  }

  if (font_) {
    std::string textToDraw;
    if (useStateText_) {
      textToDraw = isOn_ ? textOn_ : textOff_;
    } else {
      textToDraw = text_;
    }

    Color colorToUse;
    if (useStateTextColor_) {
      colorToUse = isOn_ ? textColorOn_ : textColorOff_;
    } else {
      colorToUse = textColor_;
    }

    if (!textToDraw.empty()) {
      Vec2 textSize = font_->measureText(textToDraw);

      Vec2 textPos(rect.center().x - textSize.x * 0.5f,
                   rect.center().y - textSize.y * 0.5f);

      float minX = rect.left() + padding_.x;
      float minY = rect.top() + padding_.y;
      float maxX = rect.right() - padding_.x - textSize.x;
      float maxY = rect.bottom() - padding_.y - textSize.y;

      textPos.x = std::max(minX, std::min(textPos.x, maxX));
      textPos.y = std::max(minY, std::min(textPos.y, maxY));

      colorToUse.a = 1.0f;

      renderer.drawText(*font_, textToDraw, textPos, colorToUse);
    }
  }
}

} // namespace extra2d
