#include <extra2d/ui/progress_bar.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>
#include <cmath>

namespace extra2d {

ProgressBar::ProgressBar() {
    setAnchor(0.0f, 0.0f);
    setSize(200.0f, 20.0f);
}

Ptr<ProgressBar> ProgressBar::create() {
    return makePtr<ProgressBar>();
}

Ptr<ProgressBar> ProgressBar::create(float min, float max, float value) {
    auto bar = makePtr<ProgressBar>();
    bar->setRange(min, max);
    bar->setValue(value);
    return bar;
}

// ============================================================================
// 链式调用构建器方法实现
// ============================================================================

ProgressBar *ProgressBar::withPosition(float x, float y) {
    setPosition(x, y);
    return this;
}

ProgressBar *ProgressBar::withPosition(const Vec2 &pos) {
    setPosition(pos);
    return this;
}

ProgressBar *ProgressBar::withAnchor(float x, float y) {
    setAnchor(x, y);
    return this;
}

ProgressBar *ProgressBar::withAnchor(const Vec2 &anchor) {
    setAnchor(anchor);
    return this;
}

ProgressBar *ProgressBar::withSize(float width, float height) {
    setSize(width, height);
    return this;
}

ProgressBar *ProgressBar::withProgress(float progress) {
    setValue(min_ + progress * (max_ - min_));
    return this;
}

ProgressBar *ProgressBar::withCoordinateSpace(CoordinateSpace space) {
    setCoordinateSpace(space);
    return this;
}

ProgressBar *ProgressBar::withScreenPosition(float x, float y) {
    setScreenPosition(x, y);
    return this;
}

ProgressBar *ProgressBar::withScreenPosition(const Vec2 &pos) {
    setScreenPosition(pos);
    return this;
}

ProgressBar *ProgressBar::withCameraOffset(float x, float y) {
    setCameraOffset(x, y);
    return this;
}

ProgressBar *ProgressBar::withCameraOffset(const Vec2 &offset) {
    setCameraOffset(offset);
    return this;
}

void ProgressBar::setRange(float min, float max) {
    min_ = min;
    max_ = max;
    setValue(value_);
}

void ProgressBar::setValue(float value) {
    value_ = std::clamp(value, min_, max_);
    
    if (!animatedChangeEnabled_) {
        displayValue_ = value_;
    }
    
    if (delayedDisplayEnabled_) {
        delayTimer_ = delayTime_;
    }
}

float ProgressBar::getPercent() const {
    if (max_ <= min_) return 0.0f;
    return (displayValue_ - min_) / (max_ - min_);
}

void ProgressBar::setDirection(Direction dir) {
    direction_ = dir;
}

void ProgressBar::setBackgroundColor(const Color &color) {
    bgColor_ = color;
}

void ProgressBar::setFillColor(const Color &color) {
    fillColor_ = color;
}

void ProgressBar::setGradientFillEnabled(bool enabled) {
    gradientEnabled_ = enabled;
}

void ProgressBar::setFillColorEnd(const Color &color) {
    fillColorEnd_ = color;
}

void ProgressBar::setSegmentedColorsEnabled(bool enabled) {
    segmentedColorsEnabled_ = enabled;
}

void ProgressBar::addColorSegment(float percentThreshold, const Color &color) {
    colorSegments_.push_back({percentThreshold, color});
    std::sort(colorSegments_.begin(), colorSegments_.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });
}

void ProgressBar::clearColorSegments() {
    colorSegments_.clear();
}

void ProgressBar::setCornerRadius(float radius) {
    cornerRadius_ = radius;
}

void ProgressBar::setRoundedCornersEnabled(bool enabled) {
    roundedCornersEnabled_ = enabled;
}

void ProgressBar::setBorderEnabled(bool enabled) {
    borderEnabled_ = enabled;
}

void ProgressBar::setBorderColor(const Color &color) {
    borderColor_ = color;
}

void ProgressBar::setBorderWidth(float width) {
    borderWidth_ = width;
}

void ProgressBar::setPadding(float padding) {
    padding_ = padding;
}

void ProgressBar::setTextEnabled(bool enabled) {
    textEnabled_ = enabled;
}

void ProgressBar::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

void ProgressBar::setTextColor(const Color &color) {
    textColor_ = color;
}

void ProgressBar::setTextFormat(const std::string &format) {
    textFormat_ = format;
}

void ProgressBar::setAnimatedChangeEnabled(bool enabled) {
    animatedChangeEnabled_ = enabled;
    if (!enabled) {
        displayValue_ = value_;
    }
}

void ProgressBar::setAnimationSpeed(float speed) {
    animationSpeed_ = speed;
}

void ProgressBar::setDelayedDisplayEnabled(bool enabled) {
    delayedDisplayEnabled_ = enabled;
}

void ProgressBar::setDelayTime(float seconds) {
    delayTime_ = seconds;
}

void ProgressBar::setDelayedFillColor(const Color &color) {
    delayedFillColor_ = color;
}

void ProgressBar::setStripedEnabled(bool enabled) {
    stripedEnabled_ = enabled;
}

void ProgressBar::setStripeColor(const Color &color) {
    stripeColor_ = color;
}

void ProgressBar::setStripeSpeed(float speed) {
    stripeSpeed_ = speed;
}

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

Rect ProgressBar::getBoundingBox() const {
    return Rect(getPosition().x, getPosition().y, getSize().width, getSize().height);
}

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

void ProgressBar::onDrawWidget(RenderBackend &renderer) {
    Vec2 pos = getPosition();
    Size size = getSize();
    
    // 计算实际绘制区域
    float bgX = pos.x + padding_;
    float bgY = pos.y + padding_;
    float bgW = size.width - padding_ * 2;
    float bgH = size.height - padding_ * 2;
    Rect bgRect(bgX, bgY, bgW, bgH);
    
    // 绘制背景
    if (roundedCornersEnabled_) {
        fillRoundedRect(renderer, bgRect, bgColor_, cornerRadius_);
    } else {
        renderer.fillRect(bgRect, bgColor_);
    }
    
    // 计算填充区域
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
    
    // 绘制延迟显示效果
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
    
    // 绘制填充
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
    
    // 绘制边框
    if (borderEnabled_) {
        if (roundedCornersEnabled_) {
            drawRoundedRect(renderer, bgRect, borderColor_, cornerRadius_);
        } else {
            renderer.drawRect(bgRect, borderColor_, borderWidth_);
        }
    }
    
    // 绘制文本
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

void ProgressBar::drawRoundedRect(RenderBackend &renderer, const Rect &rect, const Color &color, float radius) {
    renderer.drawRect(rect, color, borderWidth_);
}

void ProgressBar::fillRoundedRect(RenderBackend &renderer, const Rect &rect, const Color &color, float radius) {
    renderer.fillRect(rect, color);
}

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
