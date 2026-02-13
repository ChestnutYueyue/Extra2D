#include "extra2d/action/action_instant_actions.h"
#include "extra2d/scene/node.h"

namespace extra2d {

// ============================================================================
// 回调动作
// ============================================================================

CallFunc* CallFunc::create(const Callback& callback) {
    auto* action = new CallFunc();
    action->callback_ = callback;
    return action;
}

void CallFunc::execute() {
    if (callback_) {
        callback_();
    }
}

ActionInstant* CallFunc::clone() const {
    return CallFunc::create(callback_);
}

ActionInstant* CallFunc::reverse() const {
    return CallFunc::create(callback_);
}

// CallFuncN
CallFuncN* CallFuncN::create(const Callback& callback) {
    auto* action = new CallFuncN();
    action->callback_ = callback;
    return action;
}

void CallFuncN::execute() {
    if (callback_ && target_) {
        callback_(target_);
    }
}

ActionInstant* CallFuncN::clone() const {
    return CallFuncN::create(callback_);
}

ActionInstant* CallFuncN::reverse() const {
    return CallFuncN::create(callback_);
}

// ============================================================================
// 位置动作
// ============================================================================

Place* Place::create(const Vec2& position) {
    auto* action = new Place();
    action->position_ = position;
    return action;
}

void Place::execute() {
    if (target_) {
        target_->setPosition(position_);
    }
}

ActionInstant* Place::clone() const {
    return Place::create(position_);
}

ActionInstant* Place::reverse() const {
    return Place::create(position_);
}

// ============================================================================
// 翻转动作
// ============================================================================

FlipX* FlipX::create(bool flipX) {
    auto* action = new FlipX();
    action->flipX_ = flipX;
    return action;
}

void FlipX::execute() {
    if (target_) {
        target_->setFlipX(flipX_);
    }
}

ActionInstant* FlipX::clone() const {
    return FlipX::create(flipX_);
}

ActionInstant* FlipX::reverse() const {
    return FlipX::create(!flipX_);
}

// FlipY
FlipY* FlipY::create(bool flipY) {
    auto* action = new FlipY();
    action->flipY_ = flipY;
    return action;
}

void FlipY::execute() {
    if (target_) {
        target_->setFlipY(flipY_);
    }
}

ActionInstant* FlipY::clone() const {
    return FlipY::create(flipY_);
}

ActionInstant* FlipY::reverse() const {
    return FlipY::create(!flipY_);
}

// ============================================================================
// 可见性动作
// ============================================================================

Show* Show::create() {
    return new Show();
}

void Show::execute() {
    if (target_) {
        target_->setVisible(true);
    }
}

ActionInstant* Show::clone() const {
    return Show::create();
}

ActionInstant* Show::reverse() const {
    return Hide::create();
}

// Hide
Hide* Hide::create() {
    return new Hide();
}

void Hide::execute() {
    if (target_) {
        target_->setVisible(false);
    }
}

ActionInstant* Hide::clone() const {
    return Hide::create();
}

ActionInstant* Hide::reverse() const {
    return Show::create();
}

// ToggleVisibility
ToggleVisibility* ToggleVisibility::create() {
    return new ToggleVisibility();
}

void ToggleVisibility::execute() {
    if (target_) {
        target_->setVisible(!target_->isVisible());
    }
}

ActionInstant* ToggleVisibility::clone() const {
    return ToggleVisibility::create();
}

ActionInstant* ToggleVisibility::reverse() const {
    return ToggleVisibility::create();
}

// ============================================================================
// 节点管理动作
// ============================================================================

RemoveSelf* RemoveSelf::create() {
    return new RemoveSelf();
}

void RemoveSelf::execute() {
    if (target_) {
        target_->removeFromParent();
    }
}

ActionInstant* RemoveSelf::clone() const {
    return RemoveSelf::create();
}

ActionInstant* RemoveSelf::reverse() const {
    return RemoveSelf::create();
}

} // namespace extra2d
