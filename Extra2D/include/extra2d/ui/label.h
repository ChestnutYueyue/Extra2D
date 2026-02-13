#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

// ============================================================================
// 文本标签组件 - 用于显示静态文本
// 支持多行、对齐、阴影、描边等游戏常用效果
// ============================================================================
class Label : public Widget {
public:
    Label();
    explicit Label(const std::string &text);
    ~Label() override = default;

    // ------------------------------------------------------------------------
    // 静态创建方法
    // ------------------------------------------------------------------------
    static Ptr<Label> create();
    static Ptr<Label> create(const std::string &text);
    static Ptr<Label> create(const std::string &text, Ptr<FontAtlas> font);

    // ------------------------------------------------------------------------
    // 文本内容
    // ------------------------------------------------------------------------
    void setText(const std::string &text);
    const std::string &getText() const { return text_; }

    // ------------------------------------------------------------------------
    // 字体设置
    // ------------------------------------------------------------------------
    void setFont(Ptr<FontAtlas> font);
    Ptr<FontAtlas> getFont() const { return font_; }

    // ------------------------------------------------------------------------
    // 文字颜色
    // ------------------------------------------------------------------------
    void setTextColor(const Color &color);
    Color getTextColor() const { return textColor_; }

    // ------------------------------------------------------------------------
    // 字体大小
    // ------------------------------------------------------------------------
    void setFontSize(int size);
    int getFontSize() const { return fontSize_; }

    // ------------------------------------------------------------------------
    // 水平对齐方式
    // ------------------------------------------------------------------------
    enum class HorizontalAlign { Left, Center, Right };
    
    void setHorizontalAlign(HorizontalAlign align);
    HorizontalAlign getHorizontalAlign() const { return hAlign_; }

    // ------------------------------------------------------------------------
    // 垂直对齐方式
    // ------------------------------------------------------------------------
    enum class VerticalAlign { Top, Middle, Bottom };
    
    void setVerticalAlign(VerticalAlign align);
    VerticalAlign getVerticalAlign() const { return vAlign_; }

    // ------------------------------------------------------------------------
    // 阴影效果
    // ------------------------------------------------------------------------
    void setShadowEnabled(bool enabled);
    bool isShadowEnabled() const { return shadowEnabled_; }
    
    void setShadowColor(const Color &color);
    Color getShadowColor() const { return shadowColor_; }
    
    void setShadowOffset(const Vec2 &offset);
    Vec2 getShadowOffset() const { return shadowOffset_; }

    // ------------------------------------------------------------------------
    // 描边效果
    // ------------------------------------------------------------------------
    void setOutlineEnabled(bool enabled);
    bool isOutlineEnabled() const { return outlineEnabled_; }
    
    void setOutlineColor(const Color &color);
    Color getOutlineColor() const { return outlineColor_; }
    
    void setOutlineWidth(float width);
    float getOutlineWidth() const { return outlineWidth_; }

    // ------------------------------------------------------------------------
    // 多行文本
    // ------------------------------------------------------------------------
    void setMultiLine(bool multiLine);
    bool isMultiLine() const { return multiLine_; }
    
    void setLineSpacing(float spacing);
    float getLineSpacing() const { return lineSpacing_; }

    // ------------------------------------------------------------------------
    // 最大宽度（用于自动换行）
    // ------------------------------------------------------------------------
    void setMaxWidth(float maxWidth);
    float getMaxWidth() const { return maxWidth_; }

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
    Color textColor_ = Colors::White;
    int fontSize_ = 16;
    
    HorizontalAlign hAlign_ = HorizontalAlign::Left;
    VerticalAlign vAlign_ = VerticalAlign::Top;
    
    bool shadowEnabled_ = false;
    Color shadowColor_ = Color(0.0f, 0.0f, 0.0f, 0.5f);
    Vec2 shadowOffset_ = Vec2(2.0f, 2.0f);
    
    bool outlineEnabled_ = false;
    Color outlineColor_ = Colors::Black;
    float outlineWidth_ = 1.0f;
    
    bool multiLine_ = false;
    float lineSpacing_ = 1.0f;
    float maxWidth_ = 0.0f;
    
    mutable Vec2 cachedSize_ = Vec2::Zero();
    mutable bool sizeDirty_ = true;

    void updateCache() const;
    void drawText(RenderBackend &renderer, const Vec2 &position, const Color &color);
    Vec2 calculateDrawPosition() const;
    std::vector<std::string> splitLines() const;
};

} // namespace extra2d
