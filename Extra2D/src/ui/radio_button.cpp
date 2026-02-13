#include <extra2d/ui/radio_button.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 */
RadioButton::RadioButton() {
    setAnchor(0.0f, 0.0f);
    setSize(circleSize_, circleSize_);
}

/**
 * @brief 创建单选按钮对象
 * @return 单选按钮对象指针
 */
Ptr<RadioButton> RadioButton::create() {
    return makePtr<RadioButton>();
}

/**
 * @brief 创建带标签的单选按钮对象
 * @param label 标签文本
 * @return 单选按钮对象指针
 */
Ptr<RadioButton> RadioButton::create(const std::string &label) {
    auto rb = makePtr<RadioButton>();
    rb->setLabel(label);
    return rb;
}

/**
 * @brief 设置选中状态
 * @param selected 是否选中
 */
void RadioButton::setSelected(bool selected) {
    if (selected_ != selected) {
        selected_ = selected;
        if (onStateChange_) {
            onStateChange_(selected_);
        }
    }
}

/**
 * @brief 设置标签文本
 * @param label 标签文本
 */
void RadioButton::setLabel(const std::string &label) {
    label_ = label;
}

/**
 * @brief 设置字体
 * @param font 字体图集指针
 */
void RadioButton::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

/**
 * @brief 设置文本颜色
 * @param color 文本颜色
 */
void RadioButton::setTextColor(const Color &color) {
    textColor_ = color;
}

/**
 * @brief 设置圆形尺寸
 * @param size 圆形尺寸
 */
void RadioButton::setCircleSize(float size) {
    circleSize_ = size;
}

/**
 * @brief 设置间距
 * @param spacing 间距值
 */
void RadioButton::setSpacing(float spacing) {
    spacing_ = spacing;
}

/**
 * @brief 设置选中颜色
 * @param color 选中颜色
 */
void RadioButton::setSelectedColor(const Color &color) {
    selectedColor_ = color;
}

/**
 * @brief 设置未选中颜色
 * @param color 未选中颜色
 */
void RadioButton::setUnselectedColor(const Color &color) {
    unselectedColor_ = color;
}

/**
 * @brief 设置圆点颜色
 * @param color 圆点颜色
 */
void RadioButton::setDotColor(const Color &color) {
    dotColor_ = color;
}

/**
 * @brief 设置分组ID
 * @param groupId 分组ID
 */
void RadioButton::setGroupId(int groupId) {
    groupId_ = groupId;
}

/**
 * @brief 设置状态改变回调
 * @param callback 回调函数
 */
void RadioButton::setOnStateChange(Function<void(bool)> callback) {
    onStateChange_ = callback;
}

/**
 * @brief 获取边界框
 * @return 边界矩形
 */
Rect RadioButton::getBoundingBox() const {
    Vec2 pos = getPosition();
    float width = circleSize_;
    
    if (!label_.empty() && font_) {
        Vec2 textSize = font_->measureText(label_);
        width += spacing_ + textSize.x;
    }
    
    return Rect(pos.x, pos.y, width, circleSize_);
}

/**
 * @brief 绘制组件
 * @param renderer 渲染后端
 */
void RadioButton::onDrawWidget(RenderBackend &renderer) {
    Vec2 pos = getPosition();
    float centerX = pos.x + circleSize_ * 0.5f;
    float centerY = pos.y + getSize().height * 0.5f;
    float radius = circleSize_ * 0.5f;
    
    Color circleColor = selected_ ? selectedColor_ : unselectedColor_;
    renderer.drawCircle(Vec2(centerX, centerY), radius, circleColor, true);
    renderer.drawCircle(Vec2(centerX, centerY), radius, Colors::White, false, 1.0f);
    
    if (selected_) {
        float dotRadius = radius * 0.4f;
        renderer.drawCircle(Vec2(centerX, centerY), dotRadius, dotColor_, true);
    }
    
    if (!label_.empty() && font_) {
        Vec2 textPos(pos.x + circleSize_ + spacing_, pos.y);
        renderer.drawText(*font_, label_, textPos, textColor_);
    }
}

/**
 * @brief 鼠标按下事件处理
 * @param event 鼠标事件
 * @return 是否处理了事件
 */
bool RadioButton::onMousePress(const MouseEvent &event) {
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
bool RadioButton::onMouseRelease(const MouseEvent &event) {
    if (event.button == MouseButton::Left && pressed_) {
        pressed_ = false;
        Vec2 pos = getPosition();
        float centerX = pos.x + circleSize_ * 0.5f;
        float centerY = pos.y + getSize().height * 0.5f;
        float radius = circleSize_ * 0.5f;
        
        float dx = event.x - centerX;
        float dy = event.y - centerY;
        if (dx * dx + dy * dy <= radius * radius) {
            setSelected(true);
        }
        return true;
    }
    return false;
}

// ============================================================================
// RadioButtonGroup 实现
// ============================================================================

/**
 * @brief 添加单选按钮到组
 * @param button 单选按钮指针
 */
void RadioButtonGroup::addButton(RadioButton *button) {
    if (button && std::find(buttons_.begin(), buttons_.end(), button) == buttons_.end()) {
        buttons_.push_back(button);
        button->setOnStateChange([this, button](bool selected) {
            if (selected) {
                selectButton(button);
            }
        });
        
        if (button->isSelected() && !selectedButton_) {
            selectedButton_ = button;
        }
    }
}

/**
 * @brief 从组中移除单选按钮
 * @param button 单选按钮指针
 */
void RadioButtonGroup::removeButton(RadioButton *button) {
    auto it = std::find(buttons_.begin(), buttons_.end(), button);
    if (it != buttons_.end()) {
        buttons_.erase(it);
        if (selectedButton_ == button) {
            selectedButton_ = nullptr;
        }
    }
}

/**
 * @brief 选择指定的单选按钮
 * @param button 单选按钮指针
 */
void RadioButtonGroup::selectButton(RadioButton *button) {
    if (selectedButton_ == button) return;
    
    if (selectedButton_) {
        selectedButton_->setSelected(false);
    }
    
    selectedButton_ = button;
    if (button) {
        button->setSelected(true);
    }
    
    if (onSelectionChange_) {
        onSelectionChange_(button);
    }
}

/**
 * @brief 设置选择改变回调
 * @param callback 回调函数
 */
void RadioButtonGroup::setOnSelectionChange(Function<void(RadioButton*)> callback) {
    onSelectionChange_ = callback;
}

} // namespace extra2d
