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
  void setPadding(float x, float y);
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
  float getBorderWidth() const { return borderWidth_; }
  Color getBorderColor() const { return borderColor_; }

  // ------------------------------------------------------------------------
  // 图片背景设置
  // ------------------------------------------------------------------------
  void setBackgroundImage(Ptr<Texture> normal, Ptr<Texture> hover = nullptr,
                          Ptr<Texture> pressed = nullptr);
  void setBackgroundImage(Ptr<Texture> texture, const Rect &rect);

  /**
   * @brief 为切换按钮的两种状态设置图片背景
   * @param offNormal 关闭状态的普通图片
   * @param onNormal 开启状态的普通图片
   * @param offHover 关闭状态的悬停图片（可选）
   * @param onHover 开启状态的悬停图片（可选）
   * @param offPressed 关闭状态的按下图片（可选）
   * @param onPressed 开启状态的按下图片（可选）
   */
  void setStateBackgroundImage(Ptr<Texture> offNormal, Ptr<Texture> onNormal,
                               Ptr<Texture> offHover = nullptr,
                               Ptr<Texture> onHover = nullptr,
                               Ptr<Texture> offPressed = nullptr,
                               Ptr<Texture> onPressed = nullptr);

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

  // ------------------------------------------------------------------------
  // 切换模式支持（Toggle Button）
  // ------------------------------------------------------------------------

  /**
   * @brief 设置是否为切换模式
   * @param enabled true 表示启用切换模式，点击时自动切换 on/off 状态
   */
  void setToggleMode(bool enabled);

  /**
   * @brief 获取当前是否为切换模式
   * @return true 表示处于切换模式
   */
  bool isToggleMode() const { return toggleMode_; }

  /**
   * @brief 设置当前状态（仅切换模式有效）
   * @param on true 表示开启状态，false 表示关闭状态
   */
  void setOn(bool on);

  /**
   * @brief 获取当前状态
   * @return true 表示开启状态，false 表示关闭状态
   */
  bool isOn() const { return isOn_; }

  /**
   * @brief 切换当前状态
   */
  void toggle();

  /**
   * @brief 设置状态改变回调
   * @param callback 状态改变时调用的回调函数，参数为新状态
   */
  void setOnStateChange(Function<void(bool)> callback);

  // ------------------------------------------------------------------------
  // 状态文字设置（用于切换按钮）
  // ------------------------------------------------------------------------

  /**
   * @brief 为两种状态设置不同的文字
   * @param textOff 关闭状态显示的文字
   * @param textOn 开启状态显示的文字
   */
  void setStateText(const std::string &textOff, const std::string &textOn);

  /**
   * @brief 为两种状态设置不同的文字颜色
   * @param colorOff 关闭状态的文字颜色
   * @param colorOn 开启状态的文字颜色
   */
  void setStateTextColor(const Color &colorOff, const Color &colorOn);

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
  Rect imgNormalRect_;
  Rect imgHoverRect_;
  Rect imgPressedRect_;
  ImageScaleMode scaleMode_ = ImageScaleMode::Original;
  bool useImageBackground_ = false;
  bool useTextureRect_ = false;

  // 切换按钮状态图片
  Ptr<Texture> imgOffNormal_, imgOnNormal_;
  Ptr<Texture> imgOffHover_, imgOnHover_;
  Ptr<Texture> imgOffPressed_, imgOnPressed_;
  bool useStateImages_ = false;

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

  // 切换模式相关
  bool toggleMode_ = false;
  bool isOn_ = false;
  Function<void(bool)> onStateChange_;

  // 状态文字
  std::string textOff_, textOn_;
  bool useStateText_ = false;

  // 状态文字颜色
  Color textColorOff_ = Colors::White;
  Color textColorOn_ = Colors::White;
  bool useStateTextColor_ = false;

  Function<void()> onClick_;
};

} // namespace extra2d
