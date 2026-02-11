#include <extra2d/ui/radio_button.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>

namespace extra2d {

RadioButton::RadioButton() {
    setAnchor(0.0f, 0.0f);
    setSize(circleSize_, circleSize_);
}

Ptr<RadioButton> RadioButton::create() {
    return makePtr<RadioButton>();
}

Ptr<RadioButton> RadioButton::create(const std::string &label) {
    auto rb = makePtr<RadioButton>();
    rb->setLabel(label);
    return rb;
}

// ------------------------------------------------------------------------
// 链式调用构建器方法
// ------------------------------------------------------------------------
RadioButton *RadioButton::withPosition(float x, float y) {
    setPosition(x, y);
    return this;
}

RadioButton *RadioButton::withPosition(const Vec2 &pos) {
    setPosition(pos);
    return this;
}

RadioButton *RadioButton::withAnchor(float x, float y) {
    setAnchor(x, y);
    return this;
}

RadioButton *RadioButton::withAnchor(const Vec2 &anchor) {
    setAnchor(anchor);
    return this;
}

RadioButton *RadioButton::withText(const std::string &text) {
    setLabel(text);
    return this;
}

RadioButton *RadioButton::withFont(Ptr<FontAtlas> font) {
    setFont(font);
    return this;
}

RadioButton *RadioButton::withTextColor(const Color &color) {
    setTextColor(color);
    return this;
}

RadioButton *RadioButton::withSize(float width, float height) {
    setSize(width, height);
    return this;
}

// ------------------------------------------------------------------------
// 链式调用 - 坐标空间设置
// ------------------------------------------------------------------------
RadioButton *RadioButton::withCoordinateSpace(CoordinateSpace space) {
    setCoordinateSpace(space);
    return this;
}

RadioButton *RadioButton::withScreenPosition(float x, float y) {
    setScreenPosition(x, y);
    return this;
}

RadioButton *RadioButton::withScreenPosition(const Vec2 &pos) {
    setScreenPosition(pos);
    return this;
}

RadioButton *RadioButton::withCameraOffset(float x, float y) {
    setCameraOffset(x, y);
    return this;
}

RadioButton *RadioButton::withCameraOffset(const Vec2 &offset) {
    setCameraOffset(offset);
    return this;
}

// ------------------------------------------------------------------------
// 普通设置方法
// ------------------------------------------------------------------------
void RadioButton::setSelected(bool selected) {
    if (selected_ != selected) {
        selected_ = selected;
        if (onStateChange_) {
            onStateChange_(selected_);
        }
    }
}

void RadioButton::setLabel(const std::string &label) {
    label_ = label;
}

void RadioButton::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

void RadioButton::setTextColor(const Color &color) {
    textColor_ = color;
}

void RadioButton::setCircleSize(float size) {
    circleSize_ = size;
}

void RadioButton::setSpacing(float spacing) {
    spacing_ = spacing;
}

void RadioButton::setSelectedColor(const Color &color) {
    selectedColor_ = color;
}

void RadioButton::setUnselectedColor(const Color &color) {
    unselectedColor_ = color;
}

void RadioButton::setDotColor(const Color &color) {
    dotColor_ = color;
}

void RadioButton::setGroupId(int groupId) {
    groupId_ = groupId;
}

void RadioButton::setOnStateChange(Function<void(bool)> callback) {
    onStateChange_ = callback;
}

Rect RadioButton::getBoundingBox() const {
    Vec2 pos = getPosition();
    float width = circleSize_;
    
    if (!label_.empty() && font_) {
        Vec2 textSize = font_->measureText(label_);
        width += spacing_ + textSize.x;
    }
    
    return Rect(pos.x, pos.y, width, circleSize_);
}

void RadioButton::onDrawWidget(RenderBackend &renderer) {
    Vec2 pos = getPosition();
    float centerX = pos.x + circleSize_ * 0.5f;
    float centerY = pos.y + getSize().height * 0.5f;
    float radius = circleSize_ * 0.5f;
    
    // 绘制外圆
    Color circleColor = selected_ ? selectedColor_ : unselectedColor_;
    renderer.drawCircle(Vec2(centerX, centerY), radius, circleColor, true);
    renderer.drawCircle(Vec2(centerX, centerY), radius, Colors::White, false, 1.0f);
    
    // 绘制内圆点
    if (selected_) {
        float dotRadius = radius * 0.4f;
        renderer.drawCircle(Vec2(centerX, centerY), dotRadius, dotColor_, true);
    }
    
    // 绘制标签
    if (!label_.empty() && font_) {
        Vec2 textPos(pos.x + circleSize_ + spacing_, pos.y);
        renderer.drawText(*font_, label_, textPos, textColor_);
    }
}

bool RadioButton::onMousePress(const MouseEvent &event) {
    if (event.button == MouseButton::Left) {
        pressed_ = true;
        return true;
    }
    return false;
}

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

void RadioButtonGroup::removeButton(RadioButton *button) {
    auto it = std::find(buttons_.begin(), buttons_.end(), button);
    if (it != buttons_.end()) {
        buttons_.erase(it);
        if (selectedButton_ == button) {
            selectedButton_ = nullptr;
        }
    }
}

void RadioButtonGroup::selectButton(RadioButton *button) {
    if (selectedButton_ == button) return;
    
    // 取消之前的选择
    if (selectedButton_) {
        selectedButton_->setSelected(false);
    }
    
    // 选择新的按钮
    selectedButton_ = button;
    if (button) {
        button->setSelected(true);
    }
    
    if (onSelectionChange_) {
        onSelectionChange_(button);
    }
}

void RadioButtonGroup::setOnSelectionChange(Function<void(RadioButton*)> callback) {
    onSelectionChange_ = callback;
}

} // namespace extra2d
