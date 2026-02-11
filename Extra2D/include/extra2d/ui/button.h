#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/graphics/texture.h>
#include <extra2d/platform/window.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

// 图片缩放模式
enum class ImageScaleMode {
  Original, // 使用原图大小
  Stretch,  // 拉伸填充
  ScaleFit, // 等比缩放，保持完整显示
  ScaleFill // 等比缩放，填充整个区域（可能裁剪）
};

// ============================================================================
// 基础按钮类
// ============================================================================
class Button : public Widget {
public:
  Button();
  explicit Button(const std::string &text);
  ~Button() override = default;

  // ------------------------------------------------------------------------
  // 静态创建方法
  // ------------------------------------------------------------------------
  static Ptr<Button> create();
  static Ptr<Button> create(const std::string &text);
  static Ptr<Button> create(const std::string &text, Ptr<FontAtlas> font);

  // ------------------------------------------------------------------------
  // 链式调用构建器方法
  // ------------------------------------------------------------------------
  Button *withPosition(float x, float y);
  Button *withPosition(const Vec2 &pos);
  Button *withAnchor(float x, float y);
  Button *withAnchor(const Vec2 &anchor);
  Button *withText(const std::string &text);
  Button *withFont(Ptr<FontAtlas> font);
  Button *withTextColor(const Color &color);
  Button *withBackgroundColor(const Color &normal, const Color &hover,
                              const Color &pressed);
  Button *withSize(float width, float height);
  Button *withPadding(const Vec2 &padding);
  Button *withPadding(float x, float y);
  Button *withBorder(const Color &color, float width);
  Button *withCornerRadius(float radius);
  Button *withRoundedCornersEnabled(bool enabled);
  Button *withHoverCursor(CursorShape cursor);

  // ------------------------------------------------------------------------
  // 链式调用 - 坐标空间设置
  // ------------------------------------------------------------------------
  Button *withCoordinateSpace(CoordinateSpace space);
  Button *withScreenPosition(float x, float y);
  Button *withScreenPosition(const Vec2 &pos);
  Button *withCameraOffset(float x, float y);
  Button *withCameraOffset(const Vec2 &offset);

  // ------------------------------------------------------------------------
  // 文字内容
  // ------------------------------------------------------------------------
  void setText(const std::string &text);
  const std::string &getText() const { return text_; }

  // ------------------------------------------------------------------------
  // 字体
  // ------------------------------------------------------------------------
  void setFont(Ptr<FontAtlas> font);
  Ptr<FontAtlas> getFont() const { return font_; }

  // ------------------------------------------------------------------------
  // 内边距
  // ------------------------------------------------------------------------
  void setPadding(const Vec2 &padding);
  Vec2 getPadding() const { return padding_; }

  // ------------------------------------------------------------------------
  // 文字颜色
  // ------------------------------------------------------------------------
  void setTextColor(const Color &color);
  Color getTextColor() const { return textColor_; }

  // ------------------------------------------------------------------------
  // 纯色背景设置
  // ------------------------------------------------------------------------
  void setBackgroundColor(const Color &normal, const Color &hover,
                          const Color &pressed);

  // ------------------------------------------------------------------------
  // 边框设置
  // ------------------------------------------------------------------------
  void setBorder(const Color &color, float width);

  // ------------------------------------------------------------------------
  // 图片背景设置
  // ------------------------------------------------------------------------
  void setBackgroundImage(Ptr<Texture> normal, Ptr<Texture> hover = nullptr,
                          Ptr<Texture> pressed = nullptr);
  void setBackgroundImageScaleMode(ImageScaleMode mode);
  void setCustomSize(const Vec2 &size);
  void setCustomSize(float width, float height);

  // ------------------------------------------------------------------------
  // 圆角矩形设置
  // ------------------------------------------------------------------------
  void setCornerRadius(float radius);
  float getCornerRadius() const { return cornerRadius_; }
  void setRoundedCornersEnabled(bool enabled);
  bool isRoundedCornersEnabled() const { return roundedCornersEnabled_; }

  // ------------------------------------------------------------------------
  // 鼠标光标设置
  // ------------------------------------------------------------------------
  void setHoverCursor(CursorShape cursor);
  CursorShape getHoverCursor() const { return hoverCursor_; }

  // ------------------------------------------------------------------------
  // Alpha遮罩点击检测
  // ------------------------------------------------------------------------
  void setUseAlphaMaskForHitTest(bool enabled);
  bool isUseAlphaMaskForHitTest() const { return useAlphaMaskForHitTest_; }

  // ------------------------------------------------------------------------
  // 点击回调
  // ------------------------------------------------------------------------
  void setOnClick(Function<void()> callback);

  Rect getBoundingBox() const override;

protected:
  void onDrawWidget(RenderBackend &renderer) override;
  void drawBackgroundImage(RenderBackend &renderer, const Rect &rect);
  void drawRoundedRect(RenderBackend &renderer, const Rect &rect,
                       const Color &color, float radius);
  void fillRoundedRect(RenderBackend &renderer, const Rect &rect,
                       const Color &color, float radius);
  Vec2 calculateImageSize(const Vec2 &buttonSize, const Vec2 &imageSize);

  // 状态访问（供子类使用）
  bool isHovered() const { return hovered_; }
  bool isPressed() const { return pressed_; }

private:
  std::string text_;
  Ptr<FontAtlas> font_;
  Vec2 padding_ = Vec2(10.0f, 6.0f);

  // 文字颜色
  Color textColor_ = Colors::White;

  // 纯色背景
  Color bgNormal_ = Color(0.2f, 0.2f, 0.2f, 1.0f);
  Color bgHover_ = Color(0.28f, 0.28f, 0.28f, 1.0f);
  Color bgPressed_ = Color(0.15f, 0.15f, 0.15f, 1.0f);

  // 图片背景
  Ptr<Texture> imgNormal_;
  Ptr<Texture> imgHover_;
  Ptr<Texture> imgPressed_;
  ImageScaleMode scaleMode_ = ImageScaleMode::Original;
  bool useImageBackground_ = false;

  // 边框
  Color borderColor_ = Color(0.6f, 0.6f, 0.6f, 1.0f);
  float borderWidth_ = 1.0f;

  // 圆角矩形
  float cornerRadius_ = 8.0f;
  bool roundedCornersEnabled_ = false;

  // 鼠标光标
  CursorShape hoverCursor_ = CursorShape::Hand;
  bool cursorChanged_ = false;

  // Alpha遮罩点击检测
  bool useAlphaMaskForHitTest_ = false;

  bool hovered_ = false;
  bool pressed_ = false;

  Function<void()> onClick_;
};

// ============================================================================
// 切换按钮 - 点击切换两种状态（支持图片和文字）
// ============================================================================
class ToggleImageButton : public Button {
public:
  ToggleImageButton();
  ~ToggleImageButton() override = default;

  static Ptr<ToggleImageButton> create();

  // 设置两种状态的图片
  void setStateImages(Ptr<Texture> stateOffNormal, Ptr<Texture> stateOnNormal,
                      Ptr<Texture> stateOffHover = nullptr,
                      Ptr<Texture> stateOnHover = nullptr,
                      Ptr<Texture> stateOffPressed = nullptr,
                      Ptr<Texture> stateOnPressed = nullptr);

  // 设置两种状态的文字
  void setStateText(const std::string &textOff, const std::string &textOn);

  // 设置两种状态的文字颜色
  void setStateTextColor(const Color &colorOff, const Color &colorOn);

  // 获取/设置当前状态
  bool isOn() const { return isOn_; }
  void setOn(bool on);
  void toggle();

  // 设置状态改变回调
  void setOnStateChange(Function<void(bool)> callback);

protected:
  void onDrawWidget(RenderBackend &renderer) override;

private:
  // 状态图片
  Ptr<Texture> imgOffNormal_, imgOnNormal_;
  Ptr<Texture> imgOffHover_, imgOnHover_;
  Ptr<Texture> imgOffPressed_, imgOnPressed_;

  // 状态文字
  std::string textOff_, textOn_;
  bool useStateText_ = false;

  // 状态文字颜色
  Color textColorOff_ = Colors::White;
  Color textColorOn_ = Colors::White;
  bool useStateTextColor_ = false;

  bool isOn_ = false;
  Function<void(bool)> onStateChange_;
};

} // namespace extra2d
