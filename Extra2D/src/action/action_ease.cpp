#include "extra2d/action/action_ease.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace extra2d {

// ============================================================================
// ActionEase 基类
// ============================================================================

ActionEase::~ActionEase() {
    delete innerAction_;
}

bool ActionEase::initWithAction(ActionInterval* action) {
    if (!action) {
        return false;
    }
    innerAction_ = action;
    duration_ = action->getDuration();
    return true;
}

void ActionEase::startWithTarget(Node* target) {
    ActionInterval::startWithTarget(target);
    innerAction_->startWithTarget(target);
}

void ActionEase::stop() {
    innerAction_->stop();
    ActionInterval::stop();
}

void ActionEase::update(float time) {
    innerAction_->update(time);
}

// ============================================================================
// 指数缓动
// ============================================================================

EaseExponentialIn* EaseExponentialIn::create(ActionInterval* action) {
    auto* ease = new EaseExponentialIn();
    ease->initWithAction(action);
    return ease;
}

void EaseExponentialIn::update(float time) {
    innerAction_->update(easeInExpo(time));
}

ActionInterval* EaseExponentialIn::clone() const {
    return EaseExponentialIn::create(innerAction_->clone());
}

ActionInterval* EaseExponentialIn::reverse() const {
    return EaseExponentialOut::create(innerAction_->reverse());
}

EaseExponentialOut* EaseExponentialOut::create(ActionInterval* action) {
    auto* ease = new EaseExponentialOut();
    ease->initWithAction(action);
    return ease;
}

void EaseExponentialOut::update(float time) {
    innerAction_->update(easeOutExpo(time));
}

ActionInterval* EaseExponentialOut::clone() const {
    return EaseExponentialOut::create(innerAction_->clone());
}

ActionInterval* EaseExponentialOut::reverse() const {
    return EaseExponentialIn::create(innerAction_->reverse());
}

EaseExponentialInOut* EaseExponentialInOut::create(ActionInterval* action) {
    auto* ease = new EaseExponentialInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseExponentialInOut::update(float time) {
    innerAction_->update(easeInOutExpo(time));
}

ActionInterval* EaseExponentialInOut::clone() const {
    return EaseExponentialInOut::create(innerAction_->clone());
}

ActionInterval* EaseExponentialInOut::reverse() const {
    return EaseExponentialInOut::create(innerAction_->reverse());
}

// ============================================================================
// 正弦缓动
// ============================================================================

EaseSineIn* EaseSineIn::create(ActionInterval* action) {
    auto* ease = new EaseSineIn();
    ease->initWithAction(action);
    return ease;
}

void EaseSineIn::update(float time) {
    innerAction_->update(easeInSine(time));
}

ActionInterval* EaseSineIn::clone() const {
    return EaseSineIn::create(innerAction_->clone());
}

ActionInterval* EaseSineIn::reverse() const {
    return EaseSineOut::create(innerAction_->reverse());
}

EaseSineOut* EaseSineOut::create(ActionInterval* action) {
    auto* ease = new EaseSineOut();
    ease->initWithAction(action);
    return ease;
}

void EaseSineOut::update(float time) {
    innerAction_->update(easeOutSine(time));
}

ActionInterval* EaseSineOut::clone() const {
    return EaseSineOut::create(innerAction_->clone());
}

ActionInterval* EaseSineOut::reverse() const {
    return EaseSineIn::create(innerAction_->reverse());
}

EaseSineInOut* EaseSineInOut::create(ActionInterval* action) {
    auto* ease = new EaseSineInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseSineInOut::update(float time) {
    innerAction_->update(easeInOutSine(time));
}

ActionInterval* EaseSineInOut::clone() const {
    return EaseSineInOut::create(innerAction_->clone());
}

ActionInterval* EaseSineInOut::reverse() const {
    return EaseSineInOut::create(innerAction_->reverse());
}

// ============================================================================
// 弹性缓动
// ============================================================================

EaseElasticIn* EaseElasticIn::create(ActionInterval* action, float period) {
    auto* ease = new EaseElasticIn();
    ease->initWithAction(action);
    ease->period_ = period;
    return ease;
}

void EaseElasticIn::update(float time) {
    float newT = 0.0f;
    if (time == 0.0f || time == 1.0f) {
        newT = time;
    } else {
        float s = period_ / 4.0f;
        time = time - 1.0f;
        newT = -std::pow(2.0f, 10.0f * time) * std::sin((time - s) * M_PI * 2.0f / period_);
    }
    innerAction_->update(newT);
}

ActionInterval* EaseElasticIn::clone() const {
    return EaseElasticIn::create(innerAction_->clone(), period_);
}

ActionInterval* EaseElasticIn::reverse() const {
    return EaseElasticOut::create(innerAction_->reverse(), period_);
}

EaseElasticOut* EaseElasticOut::create(ActionInterval* action, float period) {
    auto* ease = new EaseElasticOut();
    ease->initWithAction(action);
    ease->period_ = period;
    return ease;
}

void EaseElasticOut::update(float time) {
    float newT = 0.0f;
    if (time == 0.0f || time == 1.0f) {
        newT = time;
    } else {
        float s = period_ / 4.0f;
        newT = std::pow(2.0f, -10.0f * time) * std::sin((time - s) * M_PI * 2.0f / period_) + 1.0f;
    }
    innerAction_->update(newT);
}

ActionInterval* EaseElasticOut::clone() const {
    return EaseElasticOut::create(innerAction_->clone(), period_);
}

ActionInterval* EaseElasticOut::reverse() const {
    return EaseElasticIn::create(innerAction_->reverse(), period_);
}

EaseElasticInOut* EaseElasticInOut::create(ActionInterval* action, float period) {
    auto* ease = new EaseElasticInOut();
    ease->initWithAction(action);
    ease->period_ = period;
    return ease;
}

void EaseElasticInOut::update(float time) {
    float newT = 0.0f;
    if (time == 0.0f || time == 1.0f) {
        newT = time;
    } else {
        time = time * 2.0f;
        if (period_ == 0.0f) {
            period_ = 0.3f * 1.5f;
        }
        float s = period_ / 4.0f;
        if (time < 1.0f) {
            time -= 1.0f;
            newT = -0.5f * std::pow(2.0f, 10.0f * time) * std::sin((time - s) * M_PI * 2.0f / period_);
        } else {
            time -= 1.0f;
            newT = std::pow(2.0f, -10.0f * time) * std::sin((time - s) * M_PI * 2.0f / period_) * 0.5f + 1.0f;
        }
    }
    innerAction_->update(newT);
}

ActionInterval* EaseElasticInOut::clone() const {
    return EaseElasticInOut::create(innerAction_->clone(), period_);
}

ActionInterval* EaseElasticInOut::reverse() const {
    return EaseElasticInOut::create(innerAction_->reverse(), period_);
}

// ============================================================================
// 弹跳缓动
// ============================================================================

EaseBounceIn* EaseBounceIn::create(ActionInterval* action) {
    auto* ease = new EaseBounceIn();
    ease->initWithAction(action);
    return ease;
}

void EaseBounceIn::update(float time) {
    innerAction_->update(easeInBounce(time));
}

ActionInterval* EaseBounceIn::clone() const {
    return EaseBounceIn::create(innerAction_->clone());
}

ActionInterval* EaseBounceIn::reverse() const {
    return EaseBounceOut::create(innerAction_->reverse());
}

EaseBounceOut* EaseBounceOut::create(ActionInterval* action) {
    auto* ease = new EaseBounceOut();
    ease->initWithAction(action);
    return ease;
}

void EaseBounceOut::update(float time) {
    innerAction_->update(easeOutBounce(time));
}

ActionInterval* EaseBounceOut::clone() const {
    return EaseBounceOut::create(innerAction_->clone());
}

ActionInterval* EaseBounceOut::reverse() const {
    return EaseBounceIn::create(innerAction_->reverse());
}

EaseBounceInOut* EaseBounceInOut::create(ActionInterval* action) {
    auto* ease = new EaseBounceInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseBounceInOut::update(float time) {
    innerAction_->update(easeInOutBounce(time));
}

ActionInterval* EaseBounceInOut::clone() const {
    return EaseBounceInOut::create(innerAction_->clone());
}

ActionInterval* EaseBounceInOut::reverse() const {
    return EaseBounceInOut::create(innerAction_->reverse());
}

// ============================================================================
// 回震缓动
// ============================================================================

EaseBackIn* EaseBackIn::create(ActionInterval* action) {
    auto* ease = new EaseBackIn();
    ease->initWithAction(action);
    return ease;
}

void EaseBackIn::update(float time) {
    innerAction_->update(easeInBack(time));
}

ActionInterval* EaseBackIn::clone() const {
    return EaseBackIn::create(innerAction_->clone());
}

ActionInterval* EaseBackIn::reverse() const {
    return EaseBackOut::create(innerAction_->reverse());
}

EaseBackOut* EaseBackOut::create(ActionInterval* action) {
    auto* ease = new EaseBackOut();
    ease->initWithAction(action);
    return ease;
}

void EaseBackOut::update(float time) {
    innerAction_->update(easeOutBack(time));
}

ActionInterval* EaseBackOut::clone() const {
    return EaseBackOut::create(innerAction_->clone());
}

ActionInterval* EaseBackOut::reverse() const {
    return EaseBackIn::create(innerAction_->reverse());
}

EaseBackInOut* EaseBackInOut::create(ActionInterval* action) {
    auto* ease = new EaseBackInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseBackInOut::update(float time) {
    innerAction_->update(easeInOutBack(time));
}

ActionInterval* EaseBackInOut::clone() const {
    return EaseBackInOut::create(innerAction_->clone());
}

ActionInterval* EaseBackInOut::reverse() const {
    return EaseBackInOut::create(innerAction_->reverse());
}

// ============================================================================
// 二次缓动
// ============================================================================

EaseQuadIn* EaseQuadIn::create(ActionInterval* action) {
    auto* ease = new EaseQuadIn();
    ease->initWithAction(action);
    return ease;
}

void EaseQuadIn::update(float time) {
    innerAction_->update(easeInQuad(time));
}

ActionInterval* EaseQuadIn::clone() const {
    return EaseQuadIn::create(innerAction_->clone());
}

ActionInterval* EaseQuadIn::reverse() const {
    return EaseQuadOut::create(innerAction_->reverse());
}

EaseQuadOut* EaseQuadOut::create(ActionInterval* action) {
    auto* ease = new EaseQuadOut();
    ease->initWithAction(action);
    return ease;
}

void EaseQuadOut::update(float time) {
    innerAction_->update(easeOutQuad(time));
}

ActionInterval* EaseQuadOut::clone() const {
    return EaseQuadOut::create(innerAction_->clone());
}

ActionInterval* EaseQuadOut::reverse() const {
    return EaseQuadIn::create(innerAction_->reverse());
}

EaseQuadInOut* EaseQuadInOut::create(ActionInterval* action) {
    auto* ease = new EaseQuadInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseQuadInOut::update(float time) {
    innerAction_->update(easeInOutQuad(time));
}

ActionInterval* EaseQuadInOut::clone() const {
    return EaseQuadInOut::create(innerAction_->clone());
}

ActionInterval* EaseQuadInOut::reverse() const {
    return EaseQuadInOut::create(innerAction_->reverse());
}

// ============================================================================
// 三次缓动
// ============================================================================

EaseCubicIn* EaseCubicIn::create(ActionInterval* action) {
    auto* ease = new EaseCubicIn();
    ease->initWithAction(action);
    return ease;
}

void EaseCubicIn::update(float time) {
    innerAction_->update(easeInCubic(time));
}

ActionInterval* EaseCubicIn::clone() const {
    return EaseCubicIn::create(innerAction_->clone());
}

ActionInterval* EaseCubicIn::reverse() const {
    return EaseCubicOut::create(innerAction_->reverse());
}

EaseCubicOut* EaseCubicOut::create(ActionInterval* action) {
    auto* ease = new EaseCubicOut();
    ease->initWithAction(action);
    return ease;
}

void EaseCubicOut::update(float time) {
    innerAction_->update(easeOutCubic(time));
}

ActionInterval* EaseCubicOut::clone() const {
    return EaseCubicOut::create(innerAction_->clone());
}

ActionInterval* EaseCubicOut::reverse() const {
    return EaseCubicIn::create(innerAction_->reverse());
}

EaseCubicInOut* EaseCubicInOut::create(ActionInterval* action) {
    auto* ease = new EaseCubicInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseCubicInOut::update(float time) {
    innerAction_->update(easeInOutCubic(time));
}

ActionInterval* EaseCubicInOut::clone() const {
    return EaseCubicInOut::create(innerAction_->clone());
}

ActionInterval* EaseCubicInOut::reverse() const {
    return EaseCubicInOut::create(innerAction_->reverse());
}

// ============================================================================
// 四次缓动
// ============================================================================

EaseQuartIn* EaseQuartIn::create(ActionInterval* action) {
    auto* ease = new EaseQuartIn();
    ease->initWithAction(action);
    return ease;
}

void EaseQuartIn::update(float time) {
    innerAction_->update(easeInQuart(time));
}

ActionInterval* EaseQuartIn::clone() const {
    return EaseQuartIn::create(innerAction_->clone());
}

ActionInterval* EaseQuartIn::reverse() const {
    return EaseQuartOut::create(innerAction_->reverse());
}

EaseQuartOut* EaseQuartOut::create(ActionInterval* action) {
    auto* ease = new EaseQuartOut();
    ease->initWithAction(action);
    return ease;
}

void EaseQuartOut::update(float time) {
    innerAction_->update(easeOutQuart(time));
}

ActionInterval* EaseQuartOut::clone() const {
    return EaseQuartOut::create(innerAction_->clone());
}

ActionInterval* EaseQuartOut::reverse() const {
    return EaseQuartIn::create(innerAction_->reverse());
}

EaseQuartInOut* EaseQuartInOut::create(ActionInterval* action) {
    auto* ease = new EaseQuartInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseQuartInOut::update(float time) {
    innerAction_->update(easeInOutQuart(time));
}

ActionInterval* EaseQuartInOut::clone() const {
    return EaseQuartInOut::create(innerAction_->clone());
}

ActionInterval* EaseQuartInOut::reverse() const {
    return EaseQuartInOut::create(innerAction_->reverse());
}

// ============================================================================
// 五次缓动
// ============================================================================

EaseQuintIn* EaseQuintIn::create(ActionInterval* action) {
    auto* ease = new EaseQuintIn();
    ease->initWithAction(action);
    return ease;
}

void EaseQuintIn::update(float time) {
    innerAction_->update(easeInQuint(time));
}

ActionInterval* EaseQuintIn::clone() const {
    return EaseQuintIn::create(innerAction_->clone());
}

ActionInterval* EaseQuintIn::reverse() const {
    return EaseQuintOut::create(innerAction_->reverse());
}

EaseQuintOut* EaseQuintOut::create(ActionInterval* action) {
    auto* ease = new EaseQuintOut();
    ease->initWithAction(action);
    return ease;
}

void EaseQuintOut::update(float time) {
    innerAction_->update(easeOutQuint(time));
}

ActionInterval* EaseQuintOut::clone() const {
    return EaseQuintOut::create(innerAction_->clone());
}

ActionInterval* EaseQuintOut::reverse() const {
    return EaseQuintIn::create(innerAction_->reverse());
}

EaseQuintInOut* EaseQuintInOut::create(ActionInterval* action) {
    auto* ease = new EaseQuintInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseQuintInOut::update(float time) {
    innerAction_->update(easeInOutQuint(time));
}

ActionInterval* EaseQuintInOut::clone() const {
    return EaseQuintInOut::create(innerAction_->clone());
}

ActionInterval* EaseQuintInOut::reverse() const {
    return EaseQuintInOut::create(innerAction_->reverse());
}

// ============================================================================
// 圆形缓动
// ============================================================================

EaseCircleIn* EaseCircleIn::create(ActionInterval* action) {
    auto* ease = new EaseCircleIn();
    ease->initWithAction(action);
    return ease;
}

void EaseCircleIn::update(float time) {
    innerAction_->update(easeInCirc(time));
}

ActionInterval* EaseCircleIn::clone() const {
    return EaseCircleIn::create(innerAction_->clone());
}

ActionInterval* EaseCircleIn::reverse() const {
    return EaseCircleOut::create(innerAction_->reverse());
}

EaseCircleOut* EaseCircleOut::create(ActionInterval* action) {
    auto* ease = new EaseCircleOut();
    ease->initWithAction(action);
    return ease;
}

void EaseCircleOut::update(float time) {
    innerAction_->update(easeOutCirc(time));
}

ActionInterval* EaseCircleOut::clone() const {
    return EaseCircleOut::create(innerAction_->clone());
}

ActionInterval* EaseCircleOut::reverse() const {
    return EaseCircleIn::create(innerAction_->reverse());
}

EaseCircleInOut* EaseCircleInOut::create(ActionInterval* action) {
    auto* ease = new EaseCircleInOut();
    ease->initWithAction(action);
    return ease;
}

void EaseCircleInOut::update(float time) {
    innerAction_->update(easeInOutCirc(time));
}

ActionInterval* EaseCircleInOut::clone() const {
    return EaseCircleInOut::create(innerAction_->clone());
}

ActionInterval* EaseCircleInOut::reverse() const {
    return EaseCircleInOut::create(innerAction_->reverse());
}

// ============================================================================
// 自定义缓动
// ============================================================================

EaseCustom* EaseCustom::create(ActionInterval* action, EaseFunction easeFunc) {
    auto* ease = new EaseCustom();
    ease->initWithAction(action);
    ease->easeFunc_ = easeFunc;
    return ease;
}

void EaseCustom::update(float time) {
    if (easeFunc_) {
        innerAction_->update(easeFunc_(time));
    } else {
        innerAction_->update(time);
    }
}

ActionInterval* EaseCustom::clone() const {
    return EaseCustom::create(innerAction_->clone(), easeFunc_);
}

ActionInterval* EaseCustom::reverse() const {
    return EaseCustom::create(innerAction_->reverse(), easeFunc_);
}

} // namespace extra2d
