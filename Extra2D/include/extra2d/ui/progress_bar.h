#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/font.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

// ============================================================================
// 进度条组件 - 用于显示进度/百分比
// 适用于血条、能量条、加载进度、经验条等游戏场景
// ============================================================================
class ProgressBar : public Widget {
public:
    ProgressBar();
    ~ProgressBar() override = default;

    // ------------------------------------------------------------------------
    // 静态创建方法
    // ------------------------------------------------------------------------
    static Ptr<ProgressBar> create();
    static Ptr<ProgressBar> create(float min, float max, float value);

    // ------------------------------------------------------------------------
    // 链式调用构建器方法 - 坐标空间支持
    // ------------------------------------------------------------------------
    ProgressBar *withPosition(float x, float y);
    ProgressBar *withPosition(const Vec2 &pos);
    ProgressBar *withAnchor(float x, float y);
    ProgressBar *withAnchor(const Vec2 &anchor);
    ProgressBar *withSize(float width, float height);
    ProgressBar *withProgress(float progress);
    ProgressBar *withCoordinateSpace(CoordinateSpace space);
    ProgressBar *withScreenPosition(float x, float y);
    ProgressBar *withScreenPosition(const Vec2 &pos);
    ProgressBar *withCameraOffset(float x, float y);
    ProgressBar *withCameraOffset(const Vec2 &offset);

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
    // 获取百分比 (0.0 - 1.0)
    // ------------------------------------------------------------------------
    float getPercent() const;

    // ------------------------------------------------------------------------
    // 进度条方向
    // ------------------------------------------------------------------------
    enum class Direction { LeftToRight, RightToLeft, BottomToTop, TopToBottom };
    
    void setDirection(Direction dir);
    Direction getDirection() const { return direction_; }

    // ------------------------------------------------------------------------
    // 颜色设置
    // ------------------------------------------------------------------------
    void setBackgroundColor(const Color &color);
    Color getBackgroundColor() const { return bgColor_; }
    
    void setFillColor(const Color &color);
    Color getFillColor() const { return fillColor_; }
    
    // 渐变填充色（从 fillColor_ 到 fillColorEnd_）
    void setGradientFillEnabled(bool enabled);
    bool isGradientFillEnabled() const { return gradientEnabled_; }
    
    void setFillColorEnd(const Color &color);
    Color getFillColorEnd() const { return fillColorEnd_; }

    // ------------------------------------------------------------------------
    // 分段颜色（根据百分比自动切换颜色）
    // ------------------------------------------------------------------------
    void setSegmentedColorsEnabled(bool enabled);
    bool isSegmentedColorsEnabled() const { return segmentedColorsEnabled_; }
    
    // 设置分段阈值和颜色，例如：>70%绿色, >30%黄色, 其他红色
    void addColorSegment(float percentThreshold, const Color &color);
    void clearColorSegments();

    // ------------------------------------------------------------------------
    // 圆角设置
    // ------------------------------------------------------------------------
    void setCornerRadius(float radius);
    float getCornerRadius() const { return cornerRadius_; }
    
    void setRoundedCornersEnabled(bool enabled);
    bool isRoundedCornersEnabled() const { return roundedCornersEnabled_; }

    // ------------------------------------------------------------------------
    // 边框设置
    // ------------------------------------------------------------------------
    void setBorderEnabled(bool enabled);
    bool isBorderEnabled() const { return borderEnabled_; }
    
    void setBorderColor(const Color &color);
    Color getBorderColor() const { return borderColor_; }
    
    void setBorderWidth(float width);
    float getBorderWidth() const { return borderWidth_; }

    // ------------------------------------------------------------------------
    // 内边距（填充与边框的距离）
    // ------------------------------------------------------------------------
    void setPadding(float padding);
    float getPadding() const { return padding_; }

    // ------------------------------------------------------------------------
    // 文本显示
    // ------------------------------------------------------------------------
    void setTextEnabled(bool enabled);
    bool isTextEnabled() const { return textEnabled_; }
    
    void setFont(Ptr<FontAtlas> font);
    Ptr<FontAtlas> getFont() const { return font_; }
    
    void setTextColor(const Color &color);
    Color getTextColor() const { return textColor_; }
    
    // 文本格式："{value}/{max}", "{percent}%", "{value:.1f}" 等
    void setTextFormat(const std::string &format);
    const std::string &getTextFormat() const { return textFormat_; }

    // ------------------------------------------------------------------------
    // 动画效果
    // ------------------------------------------------------------------------
    void setAnimatedChangeEnabled(bool enabled);
    bool isAnimatedChangeEnabled() const { return animatedChangeEnabled_; }
    
    void setAnimationSpeed(float speed); // 每秒变化量
    float getAnimationSpeed() const { return animationSpeed_; }
    
    // 延迟显示效果（如LOL血条）
    void setDelayedDisplayEnabled(bool enabled);
    bool isDelayedDisplayEnabled() const { return delayedDisplayEnabled_; }
    
    void setDelayTime(float seconds);
    float getDelayTime() const { return delayTime_; }
    
    void setDelayedFillColor(const Color &color);
    Color getDelayedFillColor() const { return delayedFillColor_; }

    // ------------------------------------------------------------------------
    // 条纹效果
    // ------------------------------------------------------------------------
    void setStripedEnabled(bool enabled);
    bool isStripedEnabled() const { return stripedEnabled_; }
    
    void setStripeColor(const Color &color);
    Color getStripeColor() const { return stripeColor_; }
    
    void setStripeSpeed(float speed); // 条纹移动速度
    float getStripeSpeed() const { return stripeSpeed_; }

    Rect getBoundingBox() const override;

protected:
    void onUpdate(float deltaTime) override;
    void onDrawWidget(RenderBackend &renderer) override;

private:
    // 数值
    float min_ = 0.0f;
    float max_ = 100.0f;
    float value_ = 50.0f;
    
    // 方向
    Direction direction_ = Direction::LeftToRight;
    
    // 颜色
    Color bgColor_ = Color(0.2f, 0.2f, 0.2f, 1.0f);
    Color fillColor_ = Color(0.0f, 0.8f, 0.2f, 1.0f);
    Color fillColorEnd_ = Color(0.0f, 0.6f, 0.1f, 1.0f);
    bool gradientEnabled_ = false;
    
    // 分段颜色
    bool segmentedColorsEnabled_ = false;
    std::vector<std::pair<float, Color>> colorSegments_;
    
    // 圆角
    float cornerRadius_ = 4.0f;
    bool roundedCornersEnabled_ = true;
    
    // 边框
    bool borderEnabled_ = false;
    Color borderColor_ = Colors::White;
    float borderWidth_ = 1.0f;
    
    // 内边距
    float padding_ = 2.0f;
    
    // 文本
    bool textEnabled_ = false;
    Ptr<FontAtlas> font_;
    Color textColor_ = Colors::White;
    std::string textFormat_ = "{percent:.0f}%";
    
    // 动画
    bool animatedChangeEnabled_ = false;
    float animationSpeed_ = 100.0f;
    float displayValue_ = 50.0f; // 用于动画的显示值
    
    // 延迟显示
    bool delayedDisplayEnabled_ = false;
    float delayTime_ = 0.3f;
    float delayTimer_ = 0.0f;
    float delayedValue_ = 50.0f;
    Color delayedFillColor_ = Color(1.0f, 0.0f, 0.0f, 0.5f);
    
    // 条纹
    bool stripedEnabled_ = false;
    Color stripeColor_ = Color(1.0f, 1.0f, 1.0f, 0.2f);
    float stripeSpeed_ = 50.0f;
    float stripeOffset_ = 0.0f;
    
    Color getCurrentFillColor() const;
    std::string formatText() const;
    void drawRoundedRect(RenderBackend &renderer, const Rect &rect, const Color &color, float radius);
    void fillRoundedRect(RenderBackend &renderer, const Rect &rect, const Color &color, float radius);
    void drawStripes(RenderBackend &renderer, const Rect &rect);
};

} // namespace extra2d
