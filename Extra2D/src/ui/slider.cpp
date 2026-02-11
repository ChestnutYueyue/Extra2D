#include <extra2d/ui/slider.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>
#include <cmath>

namespace extra2d {

Slider::Slider() {
    setAnchor(0.0f, 0.0f);
    setSize(200.0f, 20.0f);
}

Ptr<Slider> Slider::create() {
    return makePtr<Slider>();
}

Ptr<Slider> Slider::create(float min, float max, float value) {
    auto slider = makePtr<Slider>();
    slider->setRange(min, max);
    slider->setValue(value);
    return slider;
}

// ============================================================================
// 链式调用构建器方法实现
// ============================================================================

/**
 * @brief 设置位置（浮点坐标）
 * @param x X坐标
 * @param y Y坐标
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withPosition(float x, float y) {
    setPosition(x, y);
    return this;
}

/**
 * @brief 设置位置（Vec2坐标）
 * @param pos 位置向量
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withPosition(const Vec2 &pos) {
    setPosition(pos);
    return this;
}

/**
 * @brief 设置锚点（浮点坐标）
 * @param x X锚点（0-1）
 * @param y Y锚点（0-1）
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withAnchor(float x, float y) {
    setAnchor(x, y);
    return this;
}

/**
 * @brief 设置锚点（Vec2坐标）
 * @param anchor 锚点向量
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withAnchor(const Vec2 &anchor) {
    setAnchor(anchor);
    return this;
}

/**
 * @brief 设置尺寸
 * @param width 宽度
 * @param height 高度
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withSize(float width, float height) {
    setSize(width, height);
    return this;
}

/**
 * @brief 设置最小值
 * @param min 最小值
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withMinValue(float min) {
    min_ = min;
    setValue(value_);
    return this;
}

/**
 * @brief 设置最大值
 * @param max 最大值
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withMaxValue(float max) {
    max_ = max;
    setValue(value_);
    return this;
}

/**
 * @brief 设置当前值
 * @param value 当前值
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withValue(float value) {
    setValue(value);
    return this;
}

/**
 * @brief 设置坐标空间
 * @param space 坐标空间类型
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withCoordinateSpace(CoordinateSpace space) {
    setCoordinateSpace(space);
    return this;
}

/**
 * @brief 设置屏幕位置（浮点坐标）
 * @param x X屏幕坐标
 * @param y Y屏幕坐标
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withScreenPosition(float x, float y) {
    setScreenPosition(x, y);
    return this;
}

/**
 * @brief 设置屏幕位置（Vec2坐标）
 * @param pos 屏幕位置向量
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withScreenPosition(const Vec2 &pos) {
    setScreenPosition(pos);
    return this;
}

/**
 * @brief 设置相机偏移（浮点坐标）
 * @param x X偏移量
 * @param y Y偏移量
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withCameraOffset(float x, float y) {
    setCameraOffset(x, y);
    return this;
}

/**
 * @brief 设置相机偏移（Vec2坐标）
 * @param offset 偏移向量
 * @return 返回this指针，支持链式调用
 */
Slider *Slider::withCameraOffset(const Vec2 &offset) {
    setCameraOffset(offset);
    return this;
}

void Slider::setRange(float min, float max) {
    min_ = min;
    max_ = max;
    setValue(value_);
}

void Slider::setValue(float value) {
    float newValue = std::clamp(value, min_, max_);
    if (step_ > 0.0f) {
        newValue = snapToStep(newValue);
    }
    
    if (value_ != newValue) {
        value_ = newValue;
        if (onValueChange_) {
            onValueChange_(value_);
        }
    }
}

void Slider::setStep(float step) {
    step_ = step;
    if (step_ > 0.0f) {
        setValue(value_);
    }
}

void Slider::setVertical(bool vertical) {
    vertical_ = vertical;
}

void Slider::setTrackSize(float size) {
    trackSize_ = size;
}

void Slider::setThumbSize(float size) {
    thumbSize_ = size;
}

void Slider::setTrackColor(const Color &color) {
    trackColor_ = color;
}

void Slider::setFillColor(const Color &color) {
    fillColor_ = color;
}

void Slider::setThumbColor(const Color &color) {
    thumbColor_ = color;
}

void Slider::setThumbHoverColor(const Color &color) {
    thumbHoverColor_ = color;
}

void Slider::setThumbPressedColor(const Color &color) {
    thumbPressedColor_ = color;
}

void Slider::setShowThumb(bool show) {
    showThumb_ = show;
}

void Slider::setShowFill(bool show) {
    showFill_ = show;
}

void Slider::setTextEnabled(bool enabled) {
    textEnabled_ = enabled;
}

void Slider::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

void Slider::setTextColor(const Color &color) {
    textColor_ = color;
}

void Slider::setTextFormat(const std::string &format) {
    textFormat_ = format;
}

void Slider::setOnValueChange(Function<void(float)> callback) {
    onValueChange_ = callback;
}

void Slider::setOnDragStart(Function<void()> callback) {
    onDragStart_ = callback;
}

void Slider::setOnDragEnd(Function<void()> callback) {
    onDragEnd_ = callback;
}

Rect Slider::getBoundingBox() const {
    return Rect(getPosition().x, getPosition().y, getSize().width, getSize().height);
}

float Slider::valueToPosition(float value) const {
    Vec2 pos = getPosition();
    Size size = getSize();
    
    float percent = (value - min_) / (max_ - min_);
    
    if (vertical_) {
        return pos.y + size.height - percent * size.height;
    } else {
        return pos.x + percent * size.width;
    }
}

float Slider::positionToValue(float pos) const {
    Vec2 widgetPos = getPosition();
    Size size = getSize();
    
    float percent;
    if (vertical_) {
        percent = (widgetPos.y + size.height - pos) / size.height;
    } else {
        percent = (pos - widgetPos.x) / size.width;
    }
    
    percent = std::clamp(percent, 0.0f, 1.0f);
    return min_ + percent * (max_ - min_);
}

Rect Slider::getThumbRect() const {
    Vec2 pos = getPosition();
    Size size = getSize();
    
    float thumbPos = valueToPosition(value_);
    
    if (vertical_) {
        return Rect(
            pos.x + (size.width - thumbSize_) * 0.5f,
            thumbPos - thumbSize_ * 0.5f,
            thumbSize_,
            thumbSize_
        );
    } else {
        return Rect(
            thumbPos - thumbSize_ * 0.5f,
            pos.y + (size.height - thumbSize_) * 0.5f,
            thumbSize_,
            thumbSize_
        );
    }
}

Rect Slider::getTrackRect() const {
    Vec2 pos = getPosition();
    Size size = getSize();
    
    if (vertical_) {
        return Rect(
            pos.x + (size.width - trackSize_) * 0.5f,
            pos.y,
            trackSize_,
            size.height
        );
    } else {
        return Rect(
            pos.x,
            pos.y + (size.height - trackSize_) * 0.5f,
            size.width,
            trackSize_
        );
    }
}

std::string Slider::formatText() const {
    std::string result = textFormat_;
    
    size_t pos = result.find("{value}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, std::to_string(static_cast<int>(value_)));
    }
    
    pos = result.find("{value:");
    if (pos != std::string::npos) {
        size_t endPos = result.find("}", pos);
        if (endPos != std::string::npos) {
            std::string format = result.substr(pos + 7, endPos - pos - 8);
            result.replace(pos, endPos - pos + 1, std::to_string(static_cast<int>(value_)));
        }
    }
    
    return result;
}

float Slider::snapToStep(float value) const {
    float steps = std::round((value - min_) / step_);
    return min_ + steps * step_;
}

void Slider::onDrawWidget(RenderBackend &renderer) {
    Rect trackRect = getTrackRect();
    
    // 绘制轨道背景
    renderer.fillRect(trackRect, trackColor_);
    
    // 绘制填充部分
    if (showFill_) {
        float percent = (value_ - min_) / (max_ - min_);
        float fillX = trackRect.origin.x;
        float fillY = trackRect.origin.y;
        float fillW = trackRect.size.width;
        float fillH = trackRect.size.height;
        
        if (vertical_) {
            fillH = trackRect.size.height * percent;
            fillY = trackRect.origin.y + trackRect.size.height - fillH;
        } else {
            fillW = trackRect.size.width * percent;
        }
        
        Rect fillRect(fillX, fillY, fillW, fillH);
        renderer.fillRect(fillRect, fillColor_);
    }
    
    // 绘制滑块
    if (showThumb_) {
        Rect thumbRect = getThumbRect();
        Color thumbColor = thumbColor_;
        
        if (dragging_) {
            thumbColor = thumbPressedColor_;
        } else if (hovered_) {
            thumbColor = thumbHoverColor_;
        }
        
        renderer.fillRect(thumbRect, thumbColor);
        renderer.drawRect(thumbRect, Colors::White, 1.0f);
    }
    
    // 绘制文本
    if (textEnabled_ && font_) {
        std::string text = formatText();
        Vec2 textSize = font_->measureText(text);
        Vec2 pos = getPosition();
        Size size = getSize();
        
        Vec2 textPos(
            pos.x + size.width + 10.0f,
            pos.y + (size.height - textSize.y) * 0.5f
        );
        
        renderer.drawText(*font_, text, textPos, textColor_);
    }
}

bool Slider::onMousePress(const MouseEvent &event) {
    if (event.button == MouseButton::Left) {
        Rect thumbRect = getThumbRect();
        
        if (thumbRect.containsPoint(Point(event.x, event.y))) {
            dragging_ = true;
            if (onDragStart_) {
                onDragStart_();
            }
            return true;
        }
        
        // 点击轨道直接跳转
        Rect trackRect = getTrackRect();
        if (trackRect.containsPoint(Point(event.x, event.y))) {
            float newValue = positionToValue(vertical_ ? event.y : event.x);
            setValue(newValue);
            dragging_ = true;
            if (onDragStart_) {
                onDragStart_();
            }
            return true;
        }
    }
    return false;
}

bool Slider::onMouseRelease(const MouseEvent &event) {
    if (event.button == MouseButton::Left && dragging_) {
        dragging_ = false;
        if (onDragEnd_) {
            onDragEnd_();
        }
        return true;
    }
    return false;
}

bool Slider::onMouseMove(const MouseEvent &event) {
    if (dragging_) {
        float newValue = positionToValue(vertical_ ? event.y : event.x);
        setValue(newValue);
        return true;
    }
    
    // 检查悬停
    Rect thumbRect = getThumbRect();
    bool wasHovered = hovered_;
    hovered_ = thumbRect.containsPoint(Point(event.x, event.y));
    
    return hovered_ != wasHovered;
}

void Slider::onMouseEnter() {
    hovered_ = true;
}

void Slider::onMouseLeave() {
    hovered_ = false;
}

} // namespace extra2d
