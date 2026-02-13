#include <extra2d/ui/check_box.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 */
CheckBox::CheckBox() {
    setAnchor(0.0f, 0.0f);
    setSize(boxSize_, boxSize_);
}

/**
 * @brief 创建复选框对象
 * @return 复选框对象指针
 */
Ptr<CheckBox> CheckBox::create() {
    return makePtr<CheckBox>();
}

/**
 * @brief 创建带标签的复选框对象
 * @param label 标签文本
 * @return 复选框对象指针
 */
Ptr<CheckBox> CheckBox::create(const std::string &label) {
    auto cb = makePtr<CheckBox>();
    cb->setLabel(label);
    return cb;
}

/**
 * @brief 设置选中状态
 * @param checked 是否选中
 */
void CheckBox::setChecked(bool checked) {
    if (checked_ != checked) {
        checked_ = checked;
        if (onStateChange_) {
            onStateChange_(checked_);
        }
    }
}

/**
 * @brief 切换选中状态
 */
void CheckBox::toggle() {
    setChecked(!checked_);
}

/**
 * @brief 设置标签文本
 * @param label 标签文本
 */
void CheckBox::setLabel(const std::string &label) {
    label_ = label;
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void CheckBox::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void CheckBox::setTextColor(const Color &color) {
    textColor_ = color;
}

/**
 * @brief 设置复选框尺寸
 * @param size 复选框尺寸
 */
void CheckBox::setBoxSize(float size) {
    boxSize_ = size;
}

/**
 * @brief 设置间距
 * @param spacing 间距值
 */
void CheckBox::setSpacing(float spacing) {
    spacing_ = spacing;
}

/**
 * @brief 设置选中颜色
 * @param color 选中颜色
 */
void CheckBox::setCheckedColor(const Color &color) {
    checkedColor_ = color;
}

/**
 * @brief 设置未选中颜色
 * @param color 未选中颜色
 */
void CheckBox::setUncheckedColor(const Color &color) {
    uncheckedColor_ = color;
}

/**
 * @brief 设置勾选标记颜色
 * @param color 勾选标记颜色
 */
void CheckBox::setCheckMarkColor(const Color &color) {
    checkMarkColor_ = color;
}

/**
 * @brief 设置状态改变回调
 * @param callback 回调函数
 */
void CheckBox::setOnStateChange(Function<void(bool)> callback) {
    onStateChange_ = callback;
}

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
Rect CheckBox::getBoundingBox() const {
    Vec2 pos = getPosition();
    float width = boxSize_;
    
    if (!label_.empty() && font_) {
        Vec2 textSize = font_->measureText(label_);
        width += spacing_ + textSize.x;
    }
    
    return Rect(pos.x, pos.y, width, boxSize_);
}

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
void CheckBox::onDrawWidget(RenderBackend &renderer) {
    Vec2 pos = getPosition();
    
    Rect boxRect(pos.x, pos.y + (getSize().height - boxSize_) * 0.5f, boxSize_, boxSize_);
    Color boxColor = checked_ ? checkedColor_ : uncheckedColor_;
    renderer.fillRect(boxRect, boxColor);
    renderer.drawRect(boxRect, Colors::White, 1.0f);
    
    if (checked_) {
        float padding = boxSize_ * 0.2f;
        float x1 = boxRect.origin.x + padding;
        float y1 = boxRect.origin.y + boxSize_ * 0.5f;
        float x2 = boxRect.origin.x + boxSize_ * 0.4f;
        float y2 = boxRect.origin.y + boxSize_ - padding;
        float x3 = boxRect.origin.x + boxSize_ - padding;
        float y3 = boxRect.origin.y + padding;
        
        renderer.drawLine(Vec2(x1, y1), Vec2(x2, y2), checkMarkColor_, 2.0f);
        renderer.drawLine(Vec2(x2, y2), Vec2(x3, y3), checkMarkColor_, 2.0f);
    }
    
    if (!label_.empty() && font_) {
        Vec2 textPos(pos.x + boxSize_ + spacing_, pos.y);
        renderer.drawText(*font_, label_, textPos, textColor_);
    }
}

/**
 * @brief 鼠标按下事件处理
 * @param event 鼠标事件
 * @return 是否处理了事件
 */
bool CheckBox::onMousePress(const MouseEvent &event) {
    if (event.button == MouseButton::Left) {
        pressed_ = true;
        return true;
    }
    return false;
}

/**
 * @brief 鼠标释放事件处理
 * @param event 鼠标事件
 * @return 是否处理了事件
 */
bool CheckBox::onMouseRelease(const MouseEvent &event) {
    if (event.button == MouseButton::Left && pressed_) {
        pressed_ = false;
        Vec2 pos = getPosition();
        Rect boxRect(pos.x, pos.y, boxSize_, boxSize_);
        if (boxRect.containsPoint(Point(event.x, event.y))) {
            toggle();
        }
        return true;
    }
    return false;
}

} // namespace extra2d
