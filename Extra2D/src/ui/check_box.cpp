#include <extra2d/ui/check_box.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/string.h>

namespace extra2d {

CheckBox::CheckBox() {
    setAnchor(0.0f, 0.0f);
    setSize(boxSize_, boxSize_);
}

Ptr<CheckBox> CheckBox::create() {
    return makePtr<CheckBox>();
}

Ptr<CheckBox> CheckBox::create(const std::string &label) {
    auto cb = makePtr<CheckBox>();
    cb->setLabel(label);
    return cb;
}

// ------------------------------------------------------------------------
// 链式调用构建器方法
// ------------------------------------------------------------------------
CheckBox *CheckBox::withPosition(float x, float y) {
    setPosition(x, y);
    return this;
}

CheckBox *CheckBox::withPosition(const Vec2 &pos) {
    setPosition(pos);
    return this;
}

CheckBox *CheckBox::withAnchor(float x, float y) {
    setAnchor(x, y);
    return this;
}

CheckBox *CheckBox::withAnchor(const Vec2 &anchor) {
    setAnchor(anchor);
    return this;
}

CheckBox *CheckBox::withText(const std::string &text) {
    setLabel(text);
    return this;
}

CheckBox *CheckBox::withFont(Ptr<FontAtlas> font) {
    setFont(font);
    return this;
}

CheckBox *CheckBox::withTextColor(const Color &color) {
    setTextColor(color);
    return this;
}

CheckBox *CheckBox::withSize(float width, float height) {
    setSize(width, height);
    return this;
}

// ------------------------------------------------------------------------
// 链式调用 - 坐标空间设置
// ------------------------------------------------------------------------
CheckBox *CheckBox::withCoordinateSpace(CoordinateSpace space) {
    setCoordinateSpace(space);
    return this;
}

CheckBox *CheckBox::withScreenPosition(float x, float y) {
    setScreenPosition(x, y);
    return this;
}

CheckBox *CheckBox::withScreenPosition(const Vec2 &pos) {
    setScreenPosition(pos);
    return this;
}

CheckBox *CheckBox::withCameraOffset(float x, float y) {
    setCameraOffset(x, y);
    return this;
}

CheckBox *CheckBox::withCameraOffset(const Vec2 &offset) {
    setCameraOffset(offset);
    return this;
}

void CheckBox::setChecked(bool checked) {
    if (checked_ != checked) {
        checked_ = checked;
        if (onStateChange_) {
            onStateChange_(checked_);
        }
    }
}

void CheckBox::toggle() {
    setChecked(!checked_);
}

void CheckBox::setLabel(const std::string &label) {
    label_ = label;
}

void CheckBox::setFont(Ptr<FontAtlas> font) {
    font_ = font;
}

void CheckBox::setTextColor(const Color &color) {
    textColor_ = color;
}

void CheckBox::setBoxSize(float size) {
    boxSize_ = size;
}

void CheckBox::setSpacing(float spacing) {
    spacing_ = spacing;
}

void CheckBox::setCheckedColor(const Color &color) {
    checkedColor_ = color;
}

void CheckBox::setUncheckedColor(const Color &color) {
    uncheckedColor_ = color;
}

void CheckBox::setCheckMarkColor(const Color &color) {
    checkMarkColor_ = color;
}

void CheckBox::setOnStateChange(Function<void(bool)> callback) {
    onStateChange_ = callback;
}

Rect CheckBox::getBoundingBox() const {
    Vec2 pos = getPosition();
    float width = boxSize_;
    
    if (!label_.empty() && font_) {
        Vec2 textSize = font_->measureText(label_);
        width += spacing_ + textSize.x;
    }
    
    return Rect(pos.x, pos.y, width, boxSize_);
}

void CheckBox::onDrawWidget(RenderBackend &renderer) {
    Vec2 pos = getPosition();
    
    // 绘制复选框
    Rect boxRect(pos.x, pos.y + (getSize().height - boxSize_) * 0.5f, boxSize_, boxSize_);
    Color boxColor = checked_ ? checkedColor_ : uncheckedColor_;
    renderer.fillRect(boxRect, boxColor);
    renderer.drawRect(boxRect, Colors::White, 1.0f);
    
    // 绘制勾选标记
    if (checked_) {
        float padding = boxSize_ * 0.2f;
        float x1 = boxRect.origin.x + padding;
        float y1 = boxRect.origin.y + boxSize_ * 0.5f;
        float x2 = boxRect.origin.x + boxSize_ * 0.4f;
        float y2 = boxRect.origin.y + boxSize_ - padding;
        float x3 = boxRect.origin.x + boxSize_ - padding;
        float y3 = boxRect.origin.y + padding;
        
        // 简化的勾选标记绘制
        renderer.drawLine(Vec2(x1, y1), Vec2(x2, y2), checkMarkColor_, 2.0f);
        renderer.drawLine(Vec2(x2, y2), Vec2(x3, y3), checkMarkColor_, 2.0f);
    }
    
    // 绘制标签
    if (!label_.empty() && font_) {
        Vec2 textPos(pos.x + boxSize_ + spacing_, pos.y);
        renderer.drawText(*font_, label_, textPos, textColor_);
    }
}

bool CheckBox::onMousePress(const MouseEvent &event) {
    if (event.button == MouseButton::Left) {
        pressed_ = true;
        return true;
    }
    return false;
}

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
