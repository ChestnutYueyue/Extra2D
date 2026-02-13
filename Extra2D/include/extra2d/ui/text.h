#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/ui/widget.h>
#include <cstdarg>
#include <cstdio>
#include <string>

namespace extra2d {

// ============================================================================
// 文本组件 - 继承自 Widget 的 UI 组件
// ============================================================================
class Text : public Widget {
public:
  // ------------------------------------------------------------------------
  // 对齐方式枚举
  // ------------------------------------------------------------------------
  enum class Alignment { Left, Center, Right };
  enum class VerticalAlignment { Top, Middle, Bottom };

  Text();
  explicit Text(const std::string &text);
  ~Text() override = default;

  // ------------------------------------------------------------------------
  // 静态创建方法
  // ------------------------------------------------------------------------
  static Ptr<Text> create();
  static Ptr<Text> create(const std::string &text);
  static Ptr<Text> create(const std::string &text, Ptr<FontAtlas> font);

  // ------------------------------------------------------------------------
  // 格式化创建方法（类似 printf）
  // 使用示例：
  //   auto text = Text::createFormat("FPS: %d", 60);
  //   auto text = Text::createFormat(font, "得分: %d", score);
  // ------------------------------------------------------------------------
  static Ptr<Text> createFormat(const char *fmt, ...);
  static Ptr<Text> createFormat(Ptr<FontAtlas> font, const char *fmt, ...);

  // ------------------------------------------------------------------------
  // 文字内容
  // ------------------------------------------------------------------------
  void setText(const std::string &text);
  const std::string &getText() const { return text_; }

  // ------------------------------------------------------------------------
  // 格式化文本设置（类似 printf）
  // 使用示例：
  //   text->setFormat("FPS: %d", 60);
  //   text->setFormat("位置: (%.1f, %.1f)", x, y);
  //   text->setFormat("生命值: %d/100", hp);
  // ------------------------------------------------------------------------
  void setFormat(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    text_ = buffer;
    sizeDirty_ = true;
    updateSpatialIndex();
  }

  // ------------------------------------------------------------------------
  // 字体
  // ------------------------------------------------------------------------
  void setFont(Ptr<FontAtlas> font);
  Ptr<FontAtlas> getFont() const { return font_; }

  // ------------------------------------------------------------------------
  // 文字属性
  // ------------------------------------------------------------------------
  void setTextColor(const Color &color);
  Color getTextColor() const { return color_; }

  void setFontSize(int size);
  int getFontSize() const { return fontSize_; }

  // ------------------------------------------------------------------------
  // 对齐方式
  // ------------------------------------------------------------------------
  void setAlignment(Alignment align);
  Alignment getAlignment() const { return alignment_; }

  // ------------------------------------------------------------------------
  // 垂直对齐方式
  // ------------------------------------------------------------------------
  void setVerticalAlignment(VerticalAlignment align);
  VerticalAlignment getVerticalAlignment() const { return verticalAlignment_; }

  // ------------------------------------------------------------------------
  // 尺寸计算
  // ------------------------------------------------------------------------
  Vec2 getTextSize() const;
  float getLineHeight() const;

  Rect getBoundingBox() const override;

protected:
  void onDrawWidget(RenderBackend &renderer) override;

private:
  std::string text_;
  Ptr<FontAtlas> font_;
  Color color_ = Colors::White;
  int fontSize_ = 16;
  Alignment alignment_ = Alignment::Left;
  VerticalAlignment verticalAlignment_ = VerticalAlignment::Top;

  mutable Vec2 cachedSize_ = Vec2::Zero();
  mutable bool sizeDirty_ = true;

  void updateCache() const;
  Vec2 calculateDrawPosition() const;
};

} // namespace extra2d
