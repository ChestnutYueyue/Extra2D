#pragma once

#include <extra2d/action/action_interval.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/color.h>
#include <functional>
#include <vector>

namespace extra2d {

// ============================================================================
// 移动动作
// ============================================================================

/**
 * @brief 相对移动动作
 */
class MoveBy : public ActionInterval {
public:
    static MoveBy* create(float duration, const Vec2& delta);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Vec2 delta_;
    Vec2 startPosition_;
};

/**
 * @brief 绝对移动动作
 */
class MoveTo : public ActionInterval {
public:
    static MoveTo* create(float duration, const Vec2& position);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Vec2 endPosition_;
    Vec2 startPosition_;
    Vec2 delta_;
};

// ============================================================================
// 跳跃动作
// ============================================================================

/**
 * @brief 相对跳跃动作
 */
class JumpBy : public ActionInterval {
public:
    static JumpBy* create(float duration, const Vec2& position, float height, int jumps);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Vec2 startPosition_;
    Vec2 delta_;
    float height_ = 0.0f;
    int jumps_ = 1;
};

/**
 * @brief 绝对跳跃动作
 */
class JumpTo : public JumpBy {
public:
    static JumpTo* create(float duration, const Vec2& position, float height, int jumps);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;

    Vec2 endPosition_;
};

// ============================================================================
// 贝塞尔曲线动作
// ============================================================================

/**
 * @brief 贝塞尔曲线配置
 */
struct BezierConfig {
    Vec2 controlPoint1;
    Vec2 controlPoint2;
    Vec2 endPosition;
};

/**
 * @brief 相对贝塞尔曲线动作
 */
class BezierBy : public ActionInterval {
public:
    static BezierBy* create(float duration, const BezierConfig& config);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;
    static float bezierat(float a, float b, float c, float d, float t);

    BezierConfig config_;
    Vec2 startPosition_;
};

/**
 * @brief 绝对贝塞尔曲线动作
 */
class BezierTo : public BezierBy {
public:
    static BezierTo* create(float duration, const BezierConfig& config);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;

    BezierConfig originalConfig_;
};

// ============================================================================
// 缩放动作
// ============================================================================

/**
 * @brief 相对缩放动作
 */
class ScaleBy : public ActionInterval {
public:
    static ScaleBy* create(float duration, float scale);
    static ScaleBy* create(float duration, float scaleX, float scaleY);
    static ScaleBy* create(float duration, const Vec2& scale);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Vec2 deltaScale_;
    Vec2 startScale_;
};

/**
 * @brief 绝对缩放动作
 */
class ScaleTo : public ActionInterval {
public:
    static ScaleTo* create(float duration, float scale);
    static ScaleTo* create(float duration, float scaleX, float scaleY);
    static ScaleTo* create(float duration, const Vec2& scale);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Vec2 endScale_;
    Vec2 startScale_;
    Vec2 delta_;
};

// ============================================================================
// 旋转动作
// ============================================================================

/**
 * @brief 相对旋转动作
 */
class RotateBy : public ActionInterval {
public:
    static RotateBy* create(float duration, float deltaAngle);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    float deltaAngle_ = 0.0f;
    float startAngle_ = 0.0f;
};

/**
 * @brief 绝对旋转动作
 */
class RotateTo : public ActionInterval {
public:
    static RotateTo* create(float duration, float angle);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    float endAngle_ = 0.0f;
    float startAngle_ = 0.0f;
    float deltaAngle_ = 0.0f;
};

// ============================================================================
// 淡入淡出动作
// ============================================================================

/**
 * @brief 淡入动作
 */
class FadeIn : public ActionInterval {
public:
    static FadeIn* create(float duration);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    float startOpacity_ = 0.0f;
};

/**
 * @brief 淡出动作
 */
class FadeOut : public ActionInterval {
public:
    static FadeOut* create(float duration);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    float startOpacity_ = 0.0f;
};

/**
 * @brief 淡入到指定透明度动作
 */
class FadeTo : public ActionInterval {
public:
    static FadeTo* create(float duration, float opacity);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    float endOpacity_ = 0.0f;
    float startOpacity_ = 0.0f;
    float deltaOpacity_ = 0.0f;
};

// ============================================================================
// 闪烁动作
// ============================================================================

/**
 * @brief 闪烁动作
 */
class Blink : public ActionInterval {
public:
    static Blink* create(float duration, int times);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    int times_ = 1;
    int currentTimes_ = 0;
    bool originalVisible_ = true;
};

// ============================================================================
// 色调动作
// ============================================================================

/**
 * @brief 色调变化动作
 */
class TintTo : public ActionInterval {
public:
    static TintTo* create(float duration, uint8_t red, uint8_t green, uint8_t blue);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Color3B startColor_;
    Color3B endColor_;
    Color3B deltaColor_;
};

/**
 * @brief 相对色调变化动作
 */
class TintBy : public ActionInterval {
public:
    static TintBy* create(float duration, int16_t deltaRed, int16_t deltaGreen, int16_t deltaBlue);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    Color3B startColor_;
    int16_t deltaR_ = 0;
    int16_t deltaG_ = 0;
    int16_t deltaB_ = 0;
};

// ============================================================================
// 组合动作
// ============================================================================

/**
 * @brief 序列动作
 */
class Sequence : public ActionInterval {
public:
    static Sequence* create(ActionInterval* action1, ...);
    static Sequence* create(const std::vector<ActionInterval*>& actions);

    ~Sequence();

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    std::vector<ActionInterval*> actions_;
    size_t currentIndex_ = 0;
    float split_ = 0.0f;
    float last_ = -1.0f;
};

/**
 * @brief 并行动作
 */
class Spawn : public ActionInterval {
public:
    static Spawn* create(ActionInterval* action1, ...);
    static Spawn* create(const std::vector<ActionInterval*>& actions);

    ~Spawn();

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    std::vector<ActionInterval*> actions_;
};

/**
 * @brief 重复动作
 */
class Repeat : public ActionInterval {
public:
    static Repeat* create(ActionInterval* action, int times);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

    bool isDone() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    ActionInterval* innerAction_ = nullptr;
    int times_ = 1;
    int currentTimes_ = 0;
};

/**
 * @brief 永久重复动作
 */
class RepeatForever : public ActionInterval {
public:
    static RepeatForever* create(ActionInterval* action);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

    bool isDone() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    ActionInterval* innerAction_ = nullptr;
};

/**
 * @brief 延时动作
 */
class DelayTime : public ActionInterval {
public:
    static DelayTime* create(float duration);

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onUpdate(float progress) override {}
};

/**
 * @brief 反向时间动作
 */
class ReverseTime : public ActionInterval {
public:
    static ReverseTime* create(ActionInterval* action);

    ~ReverseTime();

    ActionInterval* clone() const override;
    ActionInterval* reverse() const override;

protected:
    void onStart() override;
    void onUpdate(float progress) override;

    ActionInterval* innerAction_ = nullptr;
};

} // namespace extra2d
