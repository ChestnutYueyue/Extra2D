#include <extra2d/ui/slider.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>
#include <cmath>

namespace extra2d {

/**
 * @brief 默认构造函数
 */
Slider::Slider() {
    setAnchor(0.0f, 0.0f);
    setSize(200.0f, 20.0f);
}

/**
 * @brief 创建滑动条对象
 * @return 滑动条对象指针
 */
Ptr<Slider> Slider::create() {
    return makePtr<Slider>();
}

/**
 * @brief 创建带范围的滑动条对象
 * @param min 最小值
 * @param max 最大值
 * @param value 当前值
 * @return 滑动条对象指针
 */
Ptr<Slider> Slider::create(float min, float max, float value) {
    auto slider = makePtr<Slider>();
    slider->setRange(min, max);
    slider->setValue(value);
    return slider;
}

/**
 * @brief 设置数值范围
 * @param min 最小值
 * @param max 最大值
 */
void Slider::setRange(float min, float max) {
    min_ = min;
    max_ = max;
    setValue(value_);
}

/**
 * @brief 设置当前值
 * @param value 新值
 */
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

/**
 * @brief 设置步进值
 * @param step 步进值
 */
void Slider::setStep(float step) {
    step_ = step;
    if (step_ > 0.0f) {
        setValue(value_);
    }
}

/**
 * @brief 设置是否为垂直方向
 * @param vertical 是否垂直
 */
void Slider::setVertical(bool vertical) {
    vertical_ = vertical;
}

/**
 * @brief 设置轨道尺寸
 * @param size 轨道尺寸
 */
void Slider::setTrackSize(float size) {
    trackSize_ = size;
}

/**
 * @brief 设置滑块尺寸
 * @param size 滑块尺寸
 */
void Slider::setThumbSize(float size) {
    thumbSize_ = size;
}

/**
 * @brief 设置轨道颜色
 * @param color 轨道颜色
 */
void Slider::setTrackColor(const Color &color) {
    trackColor_ = color;
}

/**
 * @brief 设置填充颜色
 * @param color 填充颜色
 */
void Slider::setFillColor(const Color &color) {
    fillColor_ = color;
}

/**
 * @brief 设置滑块颜色
 * @param color 滑块颜色
 */
void Slider::setThumbColor(const Color &color) {
    thumbColor_ = color;
}

/**
 * @brief 设置滑块悬停颜色
 * @param color 悬停颜色
 */
void Slider::setThumbHoverColor(const Color &color) {
    thumbHoverColor_ = color;
}

/**
 * @brief 设置滑块按下颜色
 * @param color 按下颜色
 */
void Slider::setThumbPressedColor(const Color &color) {
    thumbPressedColor_ = color;
}

/**
 * @brief 设置是否显示滑块
 * @param show 是否显示
 */
void Slider::setShowThumb(bool show) {
    showThumb_ = show;
}

/**
 * @brief 设置是否显示填充
 * @param show 是否显示
 */
void Slider::setShowFill(bool show) {
    showFill_ = show;
}

/**
 * @brief 设置是否启用文本显示
 * @param enabled 是否启用
 */
void Slider::setTextEnabled(bool enabled) {
    textEnabled_ = enabled;
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void Slider::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void Slider::setTextColor(const Color &color) {
    textColor_ = color;
}

/**
 * @brief 设置文本格式
 * @param format 格式字符串
 */
void Slider::setTextFormat(const std::string &format) {
    textFormat_ = format;
}

/**
 * @brief 设置值改变回调
 * @param callback 回调函数
 */
void Slider::setOnValueChange(Function<void(float)> callback) {
    onValueChange_ = callback;
}

/**
 * @brief 设置拖拽开始回调
 * @param callback 回调函数
 */
void Slider::setOnDragStart(Function<void()> callback) {
    onDragStart_ = callback;
}

/**
 * @brief 设置拖拽结束回调
 * @param callback 回调函数
 */
void Slider::setOnDragEnd(Function<void()> callback) {
    onDragEnd_ = callback;
}

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
Rect Slider::getBoundingBox() const {
    return Rect(getPosition().x, getPosition().y, getSize().width, getSize().height);
}

/**
 * @brief 将值转换为位置
 * @param value 数值
 * @return 位置坐标
 */
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

/**
 * @brief 将位置转换为值
 * @param pos 位置坐标
 * @return 数值
 */
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

/**
 * @brief 获取滑块矩形
 * @return 滑块矩形
 */
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

/**
 * @brief 获取轨道矩形
 * @return 轨道矩形
 */
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

/**
 * @brief 格式化文本
 * @return 格式化后的文本
 */
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

/**
 * @brief 将值对齐到步进
 * @param value 原始值
 * @return 对齐后的值
 */
float Slider::snapToStep(float value) const {
    float steps = std::round((value - min_) / step_);
    return min_ + steps * step_;
}

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
void Slider::onDrawWidget(RenderBackend &renderer) {
    Rect trackRect = getTrackRect();
    
    renderer.fillRect(trackRect, trackColor_);
    
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

/**
 * @brief 鼠标按下事件处理
 * @param event 鼠标事件
 * @return 是否处理了事件
 */
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

/**
 * @brief 鼠标释放事件处理
 * @param event 鼠标事件
 * @return 是否处理了事件
 */
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

/**
 * @brief 鼠标移动事件处理
 * @param event 鼠标事件
 * @return 是否处理了事件
 */
bool Slider::onMouseMove(const MouseEvent &event) {
    if (dragging_) {
        float newValue = positionToValue(vertical_ ? event.y : event.x);
        setValue(newValue);
        return true;
    }
    
    Rect thumbRect = getThumbRect();
    bool wasHovered = hovered_;
    hovered_ = thumbRect.containsPoint(Point(event.x, event.y));
    
    return hovered_ != wasHovered;
}

/**
 * @brief 鼠标进入事件处理
 */
void Slider::onMouseEnter() {
    hovered_ = true;
}

/**
 * @brief 鼠标离开事件处理
 */
void Slider::onMouseLeave() {
    hovered_ = false;
}

} // namespace extra2d
