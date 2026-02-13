#pragma once

#include <extra2d/action/action_interval.h>
#include <extra2d/action/ease.h>

namespace extra2d {

/**
 * @brief 缓动动作基类
 * 
 * 使用装饰器模式包装其他动作，实现缓动效果。
 */
class ActionEase : public ActionInterval {
public:
    virtual ~ActionEase();

    /**
     * @brief 获取内部动作
     * @return 内部动作指针
     */
    ActionInterval* getInnerAction() const { return innerAction_; }

    void startWithTarget(Node* target) override;
    void stop() override;
    void update(float time) override;
    ActionInterval* clone() const override = 0;
    ActionInterval* reverse() const override = 0;

protected:
    ActionEase() = default;
    bool initWithAction(ActionInterval* action);
    void onUpdate(float progress) override {}

    ActionInterval* innerAction_ = nullptr;
};

// ============================================================================
// 指数缓动
// ============================================================================

class EaseExponentialIn : public ActionEase {
public:
    static EaseExponentialIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseExponentialOut : public ActionEase {
public:
    static EaseExponentialOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseExponentialInOut : public ActionEase {
public:
    static EaseExponentialInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 正弦缓动
// ============================================================================

class EaseSineIn : public ActionEase {
public:
    static EaseSineIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseSineOut : public ActionEase {
public:
    static EaseSineOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseSineInOut : public ActionEase {
public:
    static EaseSineInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 弹性缓动
// ============================================================================

class EaseElasticIn : public ActionEase {
public:
    static EaseElasticIn* create(ActionInterval* action, float period = 0.3f);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;

protected:
    float period_ = 0.3f;
};

class EaseElasticOut : public ActionEase {
public:
    static EaseElasticOut* create(ActionInterval* action, float period = 0.3f);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;

protected:
    float period_ = 0.3f;
};

class EaseElasticInOut : public ActionEase {
public:
    static EaseElasticInOut* create(ActionInterval* action, float period = 0.3f);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;

protected:
    float period_ = 0.3f;
};

// ============================================================================
// 弹跳缓动
// ============================================================================

class EaseBounceIn : public ActionEase {
public:
    static EaseBounceIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseBounceOut : public ActionEase {
public:
    static EaseBounceOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseBounceInOut : public ActionEase {
public:
    static EaseBounceInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 回震缓动
// ============================================================================

class EaseBackIn : public ActionEase {
public:
    static EaseBackIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseBackOut : public ActionEase {
public:
    static EaseBackOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseBackInOut : public ActionEase {
public:
    static EaseBackInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 二次缓动
// ============================================================================

class EaseQuadIn : public ActionEase {
public:
    static EaseQuadIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseQuadOut : public ActionEase {
public:
    static EaseQuadOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseQuadInOut : public ActionEase {
public:
    static EaseQuadInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 三次缓动
// ============================================================================

class EaseCubicIn : public ActionEase {
public:
    static EaseCubicIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseCubicOut : public ActionEase {
public:
    static EaseCubicOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseCubicInOut : public ActionEase {
public:
    static EaseCubicInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 四次缓动
// ============================================================================

class EaseQuartIn : public ActionEase {
public:
    static EaseQuartIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseQuartOut : public ActionEase {
public:
    static EaseQuartOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseQuartInOut : public ActionEase {
public:
    static EaseQuartInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 五次缓动
// ============================================================================

class EaseQuintIn : public ActionEase {
public:
    static EaseQuintIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseQuintOut : public ActionEase {
public:
    static EaseQuintOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseQuintInOut : public ActionEase {
public:
    static EaseQuintInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 圆形缓动
// ============================================================================

class EaseCircleIn : public ActionEase {
public:
    static EaseCircleIn* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseCircleOut : public ActionEase {
public:
    static EaseCircleOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

class EaseCircleInOut : public ActionEase {
public:
    static EaseCircleInOut* create(ActionInterval* action);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;
};

// ============================================================================
// 自定义缓动
// ============================================================================

/**
 * @brief 自定义缓动动作
 */
class EaseCustom : public ActionEase {
public:
    static EaseCustom* create(ActionInterval* action, EaseFunction easeFunc);
    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;
    void update(float time) override;

protected:
    EaseFunction easeFunc_ = nullptr;
};

} // namespace extra2d
