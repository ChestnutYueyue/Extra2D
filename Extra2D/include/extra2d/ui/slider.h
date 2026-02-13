#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

// ============================================================================
// 滑动条组件
// ============================================================================
class Slider : public Widget {
public:
    Slider();
    ~Slider() override = default;

    static Ptr<Slider> create();
    static Ptr<Slider> create(float min, float max, float value);

    // ------------------------------------------------------------------------
    // 数值范围
    // ------------------------------------------------------------------------
    void setRange(float min, float max);
    float getMin() const { return min_; }
    float getMax() const { return max_; }

    // ------------------------------------------------------------------------
    // 当前值
    // ------------------------------------------------------------------------
    void setValue(float value);
    float getValue() const { return value_; }

    // ------------------------------------------------------------------------
    // 步进值
    // ------------------------------------------------------------------------
    void setStep(float step);
    float getStep() const { return step_; }

    // ------------------------------------------------------------------------
    // 方向
    // ------------------------------------------------------------------------
    void setVertical(bool vertical);
    bool isVertical() const { return vertical_; }

    // ------------------------------------------------------------------------
    // 轨道尺寸
    // ------------------------------------------------------------------------
    void setTrackSize(float size);
    float getTrackSize() const { return trackSize_; }

    // ------------------------------------------------------------------------
    // 滑块尺寸
    // ------------------------------------------------------------------------
    void setThumbSize(float size);
    float getThumbSize() const { return thumbSize_; }

    // ------------------------------------------------------------------------
    // 颜色设置
    // ------------------------------------------------------------------------
    void setTrackColor(const Color &color);
    Color getTrackColor() const { return trackColor_; }

    void setFillColor(const Color &color);
    Color getFillColor() const { return fillColor_; }

    void setThumbColor(const Color &color);
    Color getThumbColor() const { return thumbColor_; }

    void setThumbHoverColor(const Color &color);
    Color getThumbHoverColor() const { return thumbHoverColor_; }

    void setThumbPressedColor(const Color &color);
    Color getThumbPressedColor() const { return thumbPressedColor_; }

    // ------------------------------------------------------------------------
    // 显示设置
    // ------------------------------------------------------------------------
    void setShowThumb(bool show);
    bool isShowThumb() const { return showThumb_; }

    void setShowFill(bool show);
    bool isShowFill() const { return showFill_; }

    // ------------------------------------------------------------------------
    // 文本显示
    // ------------------------------------------------------------------------
    void setTextEnabled(bool enabled);
    bool isTextEnabled() const { return textEnabled_; }

    void setFont(Ptr<FontAtlas> font);
    Ptr<FontAtlas> getFont() const { return font_; }

    void setTextColor(const Color &color);
    Color getTextColor() const { return textColor_; }

    void setTextFormat(const std::string &format);
    const std::string &getTextFormat() const { return textFormat_; }

    // ------------------------------------------------------------------------
    // 回调设置
    // ------------------------------------------------------------------------
    void setOnValueChange(Function<void(float)> callback);
    void setOnDragStart(Function<void()> callback);
    void setOnDragEnd(Function<void()> callback);

    Rect getBoundingBox() const override;

protected:
    void onDrawWidget(RenderBackend &renderer) override;
    bool onMousePress(const MouseEvent &event) override;
    bool onMouseRelease(const MouseEvent &event) override;
    bool onMouseMove(const MouseEvent &event) override;
    void onMouseEnter() override;
    void onMouseLeave() override;

private:
    float min_ = 0.0f;
    float max_ = 100.0f;
    float value_ = 50.0f;
    float step_ = 0.0f;
    
    bool vertical_ = false;
    float trackSize_ = 6.0f;
    float thumbSize_ = 16.0f;
    
    Color trackColor_ = Color(0.3f, 0.3f, 0.3f, 1.0f);
    Color fillColor_ = Color(0.2f, 0.6f, 1.0f, 1.0f);
    Color thumbColor_ = Color(0.8f, 0.8f, 0.8f, 1.0f);
    Color thumbHoverColor_ = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color thumbPressedColor_ = Color(0.6f, 0.6f, 0.6f, 1.0f);
    
    bool showThumb_ = true;
    bool showFill_ = true;
    
    bool textEnabled_ = false;
    Ptr<FontAtlas> font_;
    Color textColor_ = Colors::White;
    std::string textFormat_ = "{value:.0f}";
    
    bool dragging_ = false;
    bool hovered_ = false;
    
    Function<void(float)> onValueChange_;
    Function<void()> onDragStart_;
    Function<void()> onDragEnd_;

    float valueToPosition(float value) const;
    float positionToValue(float pos) const;
    Rect getThumbRect() const;
    Rect getTrackRect() const;
    std::string formatText() const;
    float snapToStep(float value) const;
};

} // namespace extra2d
