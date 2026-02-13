#include "extra2d/action/action_interval_actions.h"
#include "extra2d/scene/node.h"
#include <algorithm>
#include <cstdarg>

namespace extra2d {

// ============================================================================
// 移动动作
// ============================================================================

MoveBy* MoveBy::create(float duration, const Vec2& delta) {
    auto* action = new MoveBy();
    action->duration_ = duration;
    action->delta_ = delta;
    return action;
}

void MoveBy::onStart() {
    startPosition_ = target_->getPosition();
}

void MoveBy::onUpdate(float progress) {
    Vec2 newPos = startPosition_ + delta_ * progress;
    target_->setPosition(newPos);
}

ActionInterval* MoveBy::clone() const {
    return MoveBy::create(duration_, delta_);
}

ActionInterval* MoveBy::reverse() const {
    return MoveBy::create(duration_, -delta_);
}

// MoveTo
MoveTo* MoveTo::create(float duration, const Vec2& position) {
    auto* action = new MoveTo();
    action->duration_ = duration;
    action->endPosition_ = position;
    return action;
}

void MoveTo::onStart() {
    startPosition_ = target_->getPosition();
    delta_ = endPosition_ - startPosition_;
}

void MoveTo::onUpdate(float progress) {
    Vec2 newPos = startPosition_ + delta_ * progress;
    target_->setPosition(newPos);
}

ActionInterval* MoveTo::clone() const {
    return MoveTo::create(duration_, endPosition_);
}

ActionInterval* MoveTo::reverse() const {
    return MoveTo::create(duration_, startPosition_);
}

// ============================================================================
// 跳跃动作
// ============================================================================

JumpBy* JumpBy::create(float duration, const Vec2& position, float height, int jumps) {
    auto* action = new JumpBy();
    action->duration_ = duration;
    action->delta_ = position;
    action->height_ = height;
    action->jumps_ = jumps;
    return action;
}

void JumpBy::onStart() {
    startPosition_ = target_->getPosition();
}

void JumpBy::onUpdate(float progress) {
    float frac = (progress * jumps_) - static_cast<int>(progress * jumps_);
    float y = height_ * 4.0f * frac * (1.0f - frac);
    y += delta_.y * progress;
    float x = delta_.x * progress;
    target_->setPosition(startPosition_ + Vec2(x, y));
}

ActionInterval* JumpBy::clone() const {
    return JumpBy::create(duration_, delta_, height_, jumps_);
}

ActionInterval* JumpBy::reverse() const {
    return JumpBy::create(duration_, -delta_, height_, jumps_);
}

// JumpTo
JumpTo* JumpTo::create(float duration, const Vec2& position, float height, int jumps) {
    auto* action = new JumpTo();
    action->duration_ = duration;
    action->endPosition_ = position;
    action->height_ = height;
    action->jumps_ = jumps;
    return action;
}

void JumpTo::onStart() {
    JumpBy::onStart();
    delta_ = endPosition_ - startPosition_;
}

ActionInterval* JumpTo::clone() const {
    return JumpTo::create(duration_, endPosition_, height_, jumps_);
}

ActionInterval* JumpTo::reverse() const {
    return JumpTo::create(duration_, startPosition_, height_, jumps_);
}

// ============================================================================
// 贝塞尔曲线动作
// ============================================================================

BezierBy* BezierBy::create(float duration, const BezierConfig& config) {
    auto* action = new BezierBy();
    action->duration_ = duration;
    action->config_ = config;
    return action;
}

void BezierBy::onStart() {
    startPosition_ = target_->getPosition();
}

void BezierBy::onUpdate(float progress) {
    float xa = startPosition_.x;
    float xb = config_.controlPoint1.x;
    float xc = config_.controlPoint2.x;
    float xd = config_.endPosition.x;

    float ya = startPosition_.y;
    float yb = config_.controlPoint1.y;
    float yc = config_.controlPoint2.y;
    float yd = config_.endPosition.y;

    float x = bezierat(xa, xb, xc, xd, progress);
    float y = bezierat(ya, yb, yc, yd, progress);

    target_->setPosition(Vec2(x, y));
}

ActionInterval* BezierBy::clone() const {
    return BezierBy::create(duration_, config_);
}

ActionInterval* BezierBy::reverse() const {
    BezierConfig rev;
    rev.controlPoint1 = config_.controlPoint2 + config_.endPosition;
    rev.controlPoint2 = config_.controlPoint1 + config_.endPosition;
    rev.endPosition = config_.endPosition;
    return BezierBy::create(duration_, rev);
}

float BezierBy::bezierat(float a, float b, float c, float d, float t) {
    return (powf(1 - t, 3) * a +
            3.0f * t * powf(1 - t, 2) * b +
            3.0f * t * t * (1 - t) * c +
            t * t * t * d);
}

// BezierTo
BezierTo* BezierTo::create(float duration, const BezierConfig& config) {
    auto* action = new BezierTo();
    action->duration_ = duration;
    action->originalConfig_ = config;
    return action;
}

void BezierTo::onStart() {
    BezierBy::onStart();
    config_.controlPoint1 = originalConfig_.controlPoint1 - startPosition_;
    config_.controlPoint2 = originalConfig_.controlPoint2 - startPosition_;
    config_.endPosition = originalConfig_.endPosition - startPosition_;
}

ActionInterval* BezierTo::clone() const {
    return BezierTo::create(duration_, originalConfig_);
}

ActionInterval* BezierTo::reverse() const {
    BezierConfig rev;
    rev.controlPoint1 = originalConfig_.controlPoint2;
    rev.controlPoint2 = originalConfig_.controlPoint1;
    rev.endPosition = startPosition_;
    return BezierTo::create(duration_, rev);
}

// ============================================================================
// 缩放动作
// ============================================================================

ScaleBy* ScaleBy::create(float duration, float scale) {
    return create(duration, scale, scale);
}

ScaleBy* ScaleBy::create(float duration, float scaleX, float scaleY) {
    auto* action = new ScaleBy();
    action->duration_ = duration;
    action->deltaScale_ = Vec2(scaleX - 1.0f, scaleY - 1.0f);
    return action;
}

ScaleBy* ScaleBy::create(float duration, const Vec2& scale) {
    return create(duration, scale.x, scale.y);
}

void ScaleBy::onStart() {
    startScale_ = target_->getScale();
}

void ScaleBy::onUpdate(float progress) {
    Vec2 newScale = startScale_ + deltaScale_ * progress;
    target_->setScale(newScale);
}

ActionInterval* ScaleBy::clone() const {
    return ScaleBy::create(duration_, Vec2(startScale_.x + deltaScale_.x, startScale_.y + deltaScale_.y));
}

ActionInterval* ScaleBy::reverse() const {
    return ScaleBy::create(duration_, Vec2(startScale_.x - deltaScale_.x, startScale_.y - deltaScale_.y));
}

// ScaleTo
ScaleTo* ScaleTo::create(float duration, float scale) {
    return create(duration, scale, scale);
}

ScaleTo* ScaleTo::create(float duration, float scaleX, float scaleY) {
    auto* action = new ScaleTo();
    action->duration_ = duration;
    action->endScale_ = Vec2(scaleX, scaleY);
    return action;
}

ScaleTo* ScaleTo::create(float duration, const Vec2& scale) {
    return create(duration, scale.x, scale.y);
}

void ScaleTo::onStart() {
    startScale_ = target_->getScale();
    delta_ = endScale_ - startScale_;
}

void ScaleTo::onUpdate(float progress) {
    Vec2 newScale = startScale_ + delta_ * progress;
    target_->setScale(newScale);
}

ActionInterval* ScaleTo::clone() const {
    return ScaleTo::create(duration_, endScale_);
}

ActionInterval* ScaleTo::reverse() const {
    return ScaleTo::create(duration_, startScale_);
}

// ============================================================================
// 旋转动作
// ============================================================================

RotateBy* RotateBy::create(float duration, float deltaAngle) {
    auto* action = new RotateBy();
    action->duration_ = duration;
    action->deltaAngle_ = deltaAngle;
    return action;
}

void RotateBy::onStart() {
    startAngle_ = target_->getRotation();
}

void RotateBy::onUpdate(float progress) {
    float newAngle = startAngle_ + deltaAngle_ * progress;
    target_->setRotation(newAngle);
}

ActionInterval* RotateBy::clone() const {
    return RotateBy::create(duration_, deltaAngle_);
}

ActionInterval* RotateBy::reverse() const {
    return RotateBy::create(duration_, -deltaAngle_);
}

// RotateTo
RotateTo* RotateTo::create(float duration, float angle) {
    auto* action = new RotateTo();
    action->duration_ = duration;
    action->endAngle_ = angle;
    return action;
}

void RotateTo::onStart() {
    startAngle_ = target_->getRotation();
    deltaAngle_ = endAngle_ - startAngle_;

    if (deltaAngle_ > 180.0f) deltaAngle_ -= 360.0f;
    if (deltaAngle_ < -180.0f) deltaAngle_ += 360.0f;
}

void RotateTo::onUpdate(float progress) {
    float newAngle = startAngle_ + deltaAngle_ * progress;
    target_->setRotation(newAngle);
}

ActionInterval* RotateTo::clone() const {
    return RotateTo::create(duration_, endAngle_);
}

ActionInterval* RotateTo::reverse() const {
    return RotateTo::create(duration_, startAngle_);
}

// ============================================================================
// 淡入淡出动作
// ============================================================================

FadeIn* FadeIn::create(float duration) {
    auto* action = new FadeIn();
    action->duration_ = duration;
    return action;
}

void FadeIn::onStart() {
    startOpacity_ = target_->getOpacity();
    target_->setOpacity(0.0f);
}

void FadeIn::onUpdate(float progress) {
    target_->setOpacity(progress);
}

ActionInterval* FadeIn::clone() const {
    return FadeIn::create(duration_);
}

ActionInterval* FadeIn::reverse() const {
    return FadeOut::create(duration_);
}

// FadeOut
FadeOut* FadeOut::create(float duration) {
    auto* action = new FadeOut();
    action->duration_ = duration;
    return action;
}

void FadeOut::onStart() {
    startOpacity_ = target_->getOpacity();
    target_->setOpacity(1.0f);
}

void FadeOut::onUpdate(float progress) {
    target_->setOpacity(1.0f - progress);
}

ActionInterval* FadeOut::clone() const {
    return FadeOut::create(duration_);
}

ActionInterval* FadeOut::reverse() const {
    return FadeIn::create(duration_);
}

// FadeTo
FadeTo* FadeTo::create(float duration, float opacity) {
    auto* action = new FadeTo();
    action->duration_ = duration;
    action->endOpacity_ = opacity;
    return action;
}

void FadeTo::onStart() {
    startOpacity_ = target_->getOpacity();
    deltaOpacity_ = endOpacity_ - startOpacity_;
}

void FadeTo::onUpdate(float progress) {
    target_->setOpacity(startOpacity_ + deltaOpacity_ * progress);
}

ActionInterval* FadeTo::clone() const {
    return FadeTo::create(duration_, endOpacity_);
}

ActionInterval* FadeTo::reverse() const {
    return FadeTo::create(duration_, startOpacity_);
}

// ============================================================================
// 闪烁动作
// ============================================================================

Blink* Blink::create(float duration, int times) {
    auto* action = new Blink();
    action->duration_ = duration;
    action->times_ = times;
    return action;
}

void Blink::onStart() {
    originalVisible_ = target_->isVisible();
    currentTimes_ = 0;
}

void Blink::onUpdate(float progress) {
    float slice = 1.0f / times_;
    float m = fmodf(progress, slice);
    target_->setVisible(m > slice / 2.0f);
}

ActionInterval* Blink::clone() const {
    return Blink::create(duration_, times_);
}

ActionInterval* Blink::reverse() const {
    return Blink::create(duration_, times_);
}

// ============================================================================
// 色调动作
// ============================================================================

TintTo* TintTo::create(float duration, uint8_t red, uint8_t green, uint8_t blue) {
    auto* action = new TintTo();
    action->duration_ = duration;
    action->endColor_ = Color3B(red, green, blue);
    return action;
}

void TintTo::onStart() {
    startColor_ = target_->getColor();
    deltaColor_ = Color3B(
        static_cast<int16_t>(endColor_.r) - static_cast<int16_t>(startColor_.r),
        static_cast<int16_t>(endColor_.g) - static_cast<int16_t>(startColor_.g),
        static_cast<int16_t>(endColor_.b) - static_cast<int16_t>(startColor_.b)
    );
}

void TintTo::onUpdate(float progress) {
    Color3B newColor(
        static_cast<uint8_t>(startColor_.r + deltaColor_.r * progress),
        static_cast<uint8_t>(startColor_.g + deltaColor_.g * progress),
        static_cast<uint8_t>(startColor_.b + deltaColor_.b * progress)
    );
    target_->setColor(newColor);
}

ActionInterval* TintTo::clone() const {
    return TintTo::create(duration_, endColor_.r, endColor_.g, endColor_.b);
}

ActionInterval* TintTo::reverse() const {
    return TintTo::create(duration_, startColor_.r, startColor_.g, startColor_.b);
}

// TintBy
TintBy* TintBy::create(float duration, int16_t deltaRed, int16_t deltaGreen, int16_t deltaBlue) {
    auto* action = new TintBy();
    action->duration_ = duration;
    action->deltaR_ = deltaRed;
    action->deltaG_ = deltaGreen;
    action->deltaB_ = deltaBlue;
    return action;
}

void TintBy::onStart() {
    startColor_ = target_->getColor();
}

void TintBy::onUpdate(float progress) {
    Color3B newColor(
        static_cast<uint8_t>(startColor_.r + deltaR_ * progress),
        static_cast<uint8_t>(startColor_.g + deltaG_ * progress),
        static_cast<uint8_t>(startColor_.b + deltaB_ * progress)
    );
    target_->setColor(newColor);
}

ActionInterval* TintBy::clone() const {
    return TintBy::create(duration_, deltaR_, deltaG_, deltaB_);
}

ActionInterval* TintBy::reverse() const {
    return TintBy::create(duration_, -deltaR_, -deltaG_, -deltaB_);
}

// ============================================================================
// 组合动作
// ============================================================================

Sequence* Sequence::create(ActionInterval* action1, ...) {
    std::vector<ActionInterval*> actions;
    actions.push_back(action1);

    va_list args;
    va_start(args, action1);
    ActionInterval* action = nullptr;
    while ((action = va_arg(args, ActionInterval*)) != nullptr) {
        actions.push_back(action);
    }
    va_end(args);

    return create(actions);
}

Sequence* Sequence::create(const std::vector<ActionInterval*>& actions) {
    auto* seq = new Sequence();
    seq->duration_ = 0.0f;

    for (auto* action : actions) {
        if (action) {
            seq->actions_.push_back(action);
            seq->duration_ += action->getDuration();
        }
    }
    return seq;
}

Sequence::~Sequence() {
    for (auto* action : actions_) {
        delete action;
    }
}

void Sequence::onStart() {
    currentIndex_ = 0;
    split_ = 0.0f;
    last_ = -1.0f;

    if (!actions_.empty()) {
        actions_[0]->startWithTarget(target_);
    }
}

void Sequence::onUpdate(float progress) {
    float newTime = progress * duration_;

    if (newTime < last_) {
        for (auto* action : actions_) {
            action->stop();
        }
    }
    last_ = newTime;

    float foundSplit = 0.0f;
    size_t found = 0;

    for (size_t i = 0; i < actions_.size(); ++i) {
        foundSplit += actions_[i]->getDuration();
        if (foundSplit > newTime) {
            found = i;
            break;
        } else if (foundSplit == newTime) {
            found = i + 1;
            break;
        }
    }

    if (found != currentIndex_) {
        if (currentIndex_ < actions_.size()) {
            actions_[currentIndex_]->update(1.0f);
        }
        if (found < actions_.size()) {
            actions_[found]->startWithTarget(target_);
        }
        currentIndex_ = found;
    }

    if (currentIndex_ < actions_.size()) {
        float localTime = newTime - (foundSplit - actions_[currentIndex_]->getDuration());
        float localProgress = actions_[currentIndex_]->getDuration() > 0.0f 
            ? localTime / actions_[currentIndex_]->getDuration() : 1.0f;
        actions_[currentIndex_]->update(localProgress);
    }
}

ActionInterval* Sequence::clone() const {
    std::vector<ActionInterval*> cloned;
    for (auto* action : actions_) {
        cloned.push_back(action->clone());
    }
    return Sequence::create(cloned);
}

ActionInterval* Sequence::reverse() const {
    std::vector<ActionInterval*> rev;
    for (auto it = actions_.rbegin(); it != actions_.rend(); ++it) {
        rev.push_back((*it)->reverse());
    }
    return Sequence::create(rev);
}

// Spawn
Spawn* Spawn::create(ActionInterval* action1, ...) {
    std::vector<ActionInterval*> actions;
    actions.push_back(action1);

    va_list args;
    va_start(args, action1);
    ActionInterval* action = nullptr;
    while ((action = va_arg(args, ActionInterval*)) != nullptr) {
        actions.push_back(action);
    }
    va_end(args);

    return create(actions);
}

Spawn* Spawn::create(const std::vector<ActionInterval*>& actions) {
    auto* spawn = new Spawn();
    spawn->duration_ = 0.0f;

    for (auto* action : actions) {
        if (action) {
            spawn->actions_.push_back(action);
            spawn->duration_ = std::max(spawn->duration_, action->getDuration());
        }
    }
    return spawn;
}

Spawn::~Spawn() {
    for (auto* action : actions_) {
        delete action;
    }
}

void Spawn::onStart() {
    for (auto* action : actions_) {
        action->startWithTarget(target_);
    }
}

void Spawn::onUpdate(float progress) {
    for (auto* action : actions_) {
        float localProgress = action->getDuration() > 0.0f
            ? std::min(1.0f, (progress * duration_) / action->getDuration())
            : 1.0f;
        action->update(localProgress);
    }
}

ActionInterval* Spawn::clone() const {
    std::vector<ActionInterval*> cloned;
    for (auto* action : actions_) {
        cloned.push_back(action->clone());
    }
    return Spawn::create(cloned);
}

ActionInterval* Spawn::reverse() const {
    std::vector<ActionInterval*> rev;
    for (auto* action : actions_) {
        rev.push_back(action->reverse());
    }
    return Spawn::create(rev);
}

// Repeat
Repeat* Repeat::create(ActionInterval* action, int times) {
    auto* repeat = new Repeat();
    repeat->innerAction_ = action;
    repeat->times_ = times;
    repeat->duration_ = action->getDuration() * times;
    return repeat;
}

ActionInterval* Repeat::clone() const {
    return Repeat::create(innerAction_->clone(), times_);
}

ActionInterval* Repeat::reverse() const {
    return Repeat::create(innerAction_->reverse(), times_);
}

bool Repeat::isDone() const {
    return currentTimes_ >= times_;
}

void Repeat::onStart() {
    currentTimes_ = 0;
    innerAction_->startWithTarget(target_);
}

void Repeat::onUpdate(float progress) {
    float t = progress * times_;
    int current = static_cast<int>(t);

    if (current > currentTimes_) {
        innerAction_->update(1.0f);
        currentTimes_++;
        if (currentTimes_ < times_) {
            innerAction_->startWithTarget(target_);
        }
    }

    if (currentTimes_ < times_) {
        innerAction_->update(t - current);
    }
}

// RepeatForever
RepeatForever* RepeatForever::create(ActionInterval* action) {
    auto* repeat = new RepeatForever();
    repeat->innerAction_ = action;
    repeat->duration_ = action->getDuration();
    return repeat;
}

ActionInterval* RepeatForever::clone() const {
    return RepeatForever::create(innerAction_->clone());
}

ActionInterval* RepeatForever::reverse() const {
    return RepeatForever::create(innerAction_->reverse());
}

bool RepeatForever::isDone() const {
    return false;
}

void RepeatForever::onStart() {
    innerAction_->startWithTarget(target_);
}

void RepeatForever::onUpdate(float progress) {
    innerAction_->update(progress);
    if (innerAction_->isDone()) {
        innerAction_->startWithTarget(target_);
        elapsed_ = 0.0f;
    }
}

// DelayTime
DelayTime* DelayTime::create(float duration) {
    auto* delay = new DelayTime();
    delay->duration_ = duration;
    return delay;
}

ActionInterval* DelayTime::clone() const {
    return DelayTime::create(duration_);
}

ActionInterval* DelayTime::reverse() const {
    return DelayTime::create(duration_);
}

// ReverseTime
ReverseTime* ReverseTime::create(ActionInterval* action) {
    auto* rev = new ReverseTime();
    rev->innerAction_ = action;
    rev->duration_ = action->getDuration();
    return rev;
}

ReverseTime::~ReverseTime() {
    delete innerAction_;
}

ActionInterval* ReverseTime::clone() const {
    return ReverseTime::create(innerAction_->clone());
}

ActionInterval* ReverseTime::reverse() const {
    return innerAction_->clone();
}

void ReverseTime::onStart() {
    innerAction_->startWithTarget(target_);
}

void ReverseTime::onUpdate(float progress) {
    innerAction_->update(1.0f - progress);
}

} // namespace extra2d
