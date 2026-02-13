#include <extra2d/ui/progress_bar.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>
#include <cmath>

namespace extra2d {

/**
 * @brief 默认构造函数
 */
ProgressBar::ProgressBar() {
    setAnchor(0.0f, 0.0f);
    setSize(200.0f, 20.0f);
}

/**
 * @brief 创建进度条对象
 * @return 进度条对象指针
 */
Ptr<ProgressBar> ProgressBar::create() {
    return makePtr<ProgressBar>();
}

/**
 * @brief 创建带范围的进度条对象
 * @param min 最小值
 * @param max 最大值
 * @param value 当前值
 * @return 进度条对象指针
 */
Ptr<ProgressBar> ProgressBar::create(float min, float max, float value) {
    auto bar = makePtr<ProgressBar>();
    bar->setRange(min, max);
    bar->setValue(value);
    return bar;
}

/**
 * @brief 设置数值范围
 * @param min 最小值
 * @param max 最大值
 */
void ProgressBar::setRange(float min, float max) {
    min_ = min;
    max_ = max;
    setValue(value_);
}

/**
 * @brief 设置当前值
 * @param value 新值
 */
void ProgressBar::setValue(float value) {
    value_ = std::clamp(value, min_, max_);
    
    if (!animatedChangeEnabled_) {
        displayValue_ = value_;
    }
    
    if (delayedDisplayEnabled_) {
        delayTimer_ = delayTime_;
    }
}

/**
 * @brief 获取百分比
 * @return 百分比值（0.0-1.0）
 */
float ProgressBar::getPercent() const {
    if (max_ <= min_) return 0.0f;
    return (displayValue_ - min_) / (max_ - min_);
}

/**
 * @brief 设置方向
 * @param dir 方向枚举
 */
void ProgressBar::setDirection(Direction dir) {
    direction_ = dir;
}

/**
 * @brief 设置背景颜色
 * @param color 背景颜色
 */
void ProgressBar::setBackgroundColor(const Color &color) {
    bgColor_ = color;
}

/**
 * @brief 设置填充颜色
 * @param color 填充颜色
 */
void ProgressBar::setFillColor(const Color &color) {
    fillColor_ = color;
}

/**
 * @brief 设置是否启用渐变填充
 * @param enabled 是否启用
 */
void ProgressBar::setGradientFillEnabled(bool enabled) {
    gradientEnabled_ = enabled;
}

/**
 * @brief 设置渐变结束颜色
 * @param color 结束颜色
 */
void ProgressBar::setFillColorEnd(const Color &color) {
    fillColorEnd_ = color;
}

/**
 * @brief 设置是否启用分段颜色
 * @param enabled 是否启用
 */
void ProgressBar::setSegmentedColorsEnabled(bool enabled) {
    segmentedColorsEnabled_ = enabled;
}

/**
 * @brief 添加颜色分段
 * @param percentThreshold 百分比阈值
 * @param color 颜色
 */
void ProgressBar::addColorSegment(float percentThreshold, const Color &color) {
    colorSegments_.push_back({percentThreshold, color});
    std::sort(colorSegments_.begin(), colorSegments_.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });
}

/**
 * @brief 清除所有颜色分段
 */
void ProgressBar::clearColorSegments() {
    colorSegments_.clear();
}

/**
 * @brief 设置圆角半径
 * @param radius 圆角半径
 */
void ProgressBar::setCornerRadius(float radius) {
    cornerRadius_ = radius;
}

/**
 * @brief 设置是否启用圆角
 * @param enabled 是否启用
 */
void ProgressBar::setRoundedCornersEnabled(bool enabled) {
    roundedCornersEnabled_ = enabled;
}

/**
 * @brief 设置是否启用边框
 * @param enabled 是否启用
 */
void ProgressBar::setBorderEnabled(bool enabled) {
    borderEnabled_ = enabled;
}

/**
 * @brief 设置边框颜色
 * @param color 边框颜色
 */
void ProgressBar::setBorderColor(const Color &color) {
    borderColor_ = color;
}

/**
 * @brief 设置边框宽度
 * @param width 边框宽度
 */
void ProgressBar::setBorderWidth(float width) {
    borderWidth_ = width;
}

/**
 * @brief 设置内边距
 * @param padding 内边距值
 */
void ProgressBar::setPadding(float padding) {
    padding_ = padding;
}

/**
 * @brief 设置是否启用文本显示
 * @param enabled 是否启用
 */
void ProgressBar::setTextEnabled(bool enabled) {
    textEnabled_ = enabled;
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void ProgressBar::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void ProgressBar::setTextColor(const Color &color) {
    textColor_ = color;
}

/**
 * @brief 设置文本格式
 * @param format 格式字符串
 */
void ProgressBar::setTextFormat(const std::string &format) {
    textFormat_ = format;
}

/**
 * @brief 设置是否启用动画变化
 * @param enabled 是否启用
 */
void ProgressBar::setAnimatedChangeEnabled(bool enabled) {
    animatedChangeEnabled_ = enabled;
    if (!enabled) {
        displayValue_ = value_;
    }
}

/**
 * @brief 设置动画速度
 * @param speed 每秒变化量
 */
void ProgressBar::setAnimationSpeed(float speed) {
    animationSpeed_ = speed;
}

/**
 * @brief 设置是否启用延迟显示
 * @param enabled 是否启用
 */
void ProgressBar::setDelayedDisplayEnabled(bool enabled) {
    delayedDisplayEnabled_ = enabled;
}

/**
 * @brief 设置延迟时间
 * @param seconds 延迟秒数
 */
void ProgressBar::setDelayTime(float seconds) {
    delayTime_ = seconds;
}

/**
 * @brief 设置延迟显示填充颜色
 * @param color 填充颜色
 */
void ProgressBar::setDelayedFillColor(const Color &color) {
    delayedFillColor_ = color;
}

/**
 * @brief 设置是否启用条纹效果
 * @param enabled 是否启用
 */
void ProgressBar::setStripedEnabled(bool enabled) {
    stripedEnabled_ = enabled;
}

/**
 * @brief 设置条纹颜色
 * @param color 条纹颜色
 */
void ProgressBar::setStripeColor(const Color &color) {
    stripeColor_ = color;
}

/**
 * @brief 设置条纹移动速度
 * @param speed 移动速度
 */
void ProgressBar::setStripeSpeed(float speed) {
    stripeSpeed_ = speed;
}

/**
 * @brief 获取当前填充颜色
 * @return 当前填充颜色
 */
Color ProgressBar::getCurrentFillColor() const {
    if (segmentedColorsEnabled_ && !colorSegments_.empty()) {
        float percent = getPercent();
        for (const auto &segment : colorSegments_) {
            if (percent >= segment.first) {
                return segment.second;
            }
        }
    }
    return fillColor_;
}

/**
 * @brief 格式化文本
 * @return 格式化后的文本
 */
std::string ProgressBar::formatText() const {
    std::string result = textFormat_;
    
    size_t pos = result.find("{value}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, std::to_string(static_cast<int>(displayValue_)));
    }
    
    pos = result.find("{max}");
    if (pos != std::string::npos) {
        result.replace(pos, 5, std::to_string(static_cast<int>(max_)));
    }
    
    pos = result.find("{percent}");
    if (pos != std::string::npos) {
        int percent = static_cast<int>(getPercent() * 100);
        result.replace(pos, 9, std::to_string(percent));
    }
    
    return result;
}

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
Rect ProgressBar::getBoundingBox() const {
    return Rect(getPosition().x, getPosition().y, getSize().width, getSize().height);
}

/**
 * @brief 更新函数
 * @param deltaTime 帧间隔时间
 */
void ProgressBar::onUpdate(float deltaTime) {
    if (animatedChangeEnabled_ && displayValue_ != value_) {
        float diff = value_ - displayValue_;
        float change = animationSpeed_ * deltaTime;
        
        if (std::abs(diff) <= change) {
            displayValue_ = value_;
        } else {
            displayValue_ += (diff > 0 ? change : -change);
        }
    }
    
    if (delayedDisplayEnabled_) {
        if (delayTimer_ > 0.0f) {
            delayTimer_ -= deltaTime;
        } else {
            float diff = displayValue_ - delayedValue_;
            float change = animationSpeed_ * deltaTime;
            
            if (std::abs(diff) <= change) {
                delayedValue_ = displayValue_;
            } else {
                delayedValue_ += (diff > 0 ? change : -change);
            }
        }
    }
    
    if (stripedEnabled_) {
        stripeOffset_ += stripeSpeed_ * deltaTime;
        if (stripeOffset_ > 20.0f) {
            stripeOffset_ -= 20.0f;
        }
    }
}

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
void ProgressBar::onDrawWidget(RenderBackend &renderer) {
    Vec2 pos = getPosition();
    Size size = getSize();
    
    float bgX = pos.x + padding_;
    float bgY = pos.y + padding_;
    float bgW = size.width - padding_ * 2;
    float bgH = size.height - padding_ * 2;
    Rect bgRect(bgX, bgY, bgW, bgH);
    
    if (roundedCornersEnabled_) {
        fillRoundedRect(renderer, bgRect, bgColor_, cornerRadius_);
    } else {
        renderer.fillRect(bgRect, bgColor_);
    }
    
    float percent = getPercent();
    float fillX = bgX, fillY = bgY, fillW = bgW, fillH = bgH;
    
    switch (direction_) {
        case Direction::LeftToRight:
            fillW = bgW * percent;
            break;
        case Direction::RightToLeft:
            fillW = bgW * percent;
            fillX = bgX + bgW - fillW;
            break;
        case Direction::BottomToTop:
            fillH = bgH * percent;
            fillY = bgY + bgH - fillH;
            break;
        case Direction::TopToBottom:
            fillH = bgH * percent;
            break;
    }
    Rect fillRect(fillX, fillY, fillW, fillH);
    
    if (delayedDisplayEnabled_ && delayedValue_ > displayValue_) {
        float delayedPercent = (delayedValue_ - min_) / (max_ - min_);
        float delayedX = bgX, delayedY = bgY, delayedW = bgW, delayedH = bgH;
        
        switch (direction_) {
            case Direction::LeftToRight:
                delayedW = bgW * delayedPercent;
                break;
            case Direction::RightToLeft:
                delayedW = bgW * delayedPercent;
                delayedX = bgX + bgW - delayedW;
                break;
            case Direction::BottomToTop:
                delayedH = bgH * delayedPercent;
                delayedY = bgY + bgH - delayedH;
                break;
            case Direction::TopToBottom:
                delayedH = bgH * delayedPercent;
                break;
        }
        Rect delayedRect(delayedX, delayedY, delayedW, delayedH);
        
        if (roundedCornersEnabled_) {
            fillRoundedRect(renderer, delayedRect, delayedFillColor_, cornerRadius_);
        } else {
            renderer.fillRect(delayedRect, delayedFillColor_);
        }
    }
    
    if (fillW > 0 && fillH > 0) {
        Color fillColor = getCurrentFillColor();
        
        if (roundedCornersEnabled_) {
            fillRoundedRect(renderer, fillRect, fillColor, cornerRadius_);
        } else {
            renderer.fillRect(fillRect, fillColor);
        }
        
        if (stripedEnabled_) {
            drawStripes(renderer, fillRect);
        }
    }
    
    if (borderEnabled_) {
        if (roundedCornersEnabled_) {
            drawRoundedRect(renderer, bgRect, borderColor_, cornerRadius_);
        } else {
            renderer.drawRect(bgRect, borderColor_, borderWidth_);
        }
    }
    
    if (textEnabled_ && font_) {
        std::string text = formatText();
        Vec2 textSize = font_->measureText(text);
        
        Vec2 textPos(
            pos.x + (size.width - textSize.x) * 0.5f,
            pos.y + (size.height - textSize.y) * 0.5f
        );
        
        renderer.drawText(*font_, text, textPos, textColor_);
    }
}

/**
 * @brief 绘制圆角矩形边框
 * @param renderer 渲染后端
 * @param rect 矩形区域
 * @param color 颜色
 * @param radius 圆角半径
 */
void ProgressBar::drawRoundedRect(RenderBackend &renderer, const Rect &rect, const Color &color, float radius) {
    renderer.drawRect(rect, color, borderWidth_);
}

/**
 * @brief 填充圆角矩形
 * @param renderer 渲染后端
 * @param rect 矩形区域
 * @param color 颜色
 * @param radius 圆角半径
 */
void ProgressBar::fillRoundedRect(RenderBackend &renderer, const Rect &rect, const Color &color, float radius) {
    renderer.fillRect(rect, color);
}

/**
 * @brief 绘制条纹效果
 * @param renderer 渲染后端
 * @param rect 矩形区域
 */
void ProgressBar::drawStripes(RenderBackend &renderer, const Rect &rect) {
    const float stripeWidth = 10.0f;
    const float spacing = 20.0f;
    float rectRight = rect.origin.x + rect.size.width;
    float rectBottom = rect.origin.y + rect.size.height;
    
    for (float x = rect.origin.x - spacing + stripeOffset_; x < rectRight; x += spacing) {
        float x1 = x;
        float y1 = rect.origin.y;
        float x2 = x + stripeWidth;
        float y2 = rectBottom;
        
        if (x1 < rect.origin.x) x1 = rect.origin.x;
        if (x2 > rectRight) x2 = rectRight;
        
        if (x2 > x1) {
            for (int i = 0; i < static_cast<int>(rect.size.height); i += 4) {
                float sy = rect.origin.y + i;
                float sx = x1 + i * 0.5f;
                if (sx < x2) {
                    Rect stripeRect(sx, sy, std::min(2.0f, x2 - sx), 2.0f);
                    renderer.fillRect(stripeRect, stripeColor_);
                }
            }
        }
    }
}

} // namespace extra2d
