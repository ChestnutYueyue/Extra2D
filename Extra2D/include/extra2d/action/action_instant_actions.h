#pragma once

#include <extra2d/action/action_instant.h>
#include <extra2d/core/math_types.h>
#include <functional>

namespace extra2d {

// ============================================================================
// 回调动作
// ============================================================================

/**
 * @brief 无参数回调动作
 */
class CallFunc : public ActionInstant {
public:
    using Callback = std::function<void()>;

    static CallFunc* create(const Callback& callback);

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;

    Callback callback_;
};

/**
 * @brief 带节点参数的回调动作
 */
class CallFuncN : public ActionInstant {
public:
    using Callback = std::function<void(Node*)>;

    static CallFuncN* create(const Callback& callback);

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;

    Callback callback_;
};

// ============================================================================
// 位置动作
// ============================================================================

/**
 * @brief 瞬间放置到指定位置
 */
class Place : public ActionInstant {
public:
    static Place* create(const Vec2& position);

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;

    Vec2 position_;
};

// ============================================================================
// 翻转动作
// ============================================================================

/**
 * @brief X轴翻转动作
 */
class FlipX : public ActionInstant {
public:
    static FlipX* create(bool flipX);

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;

    bool flipX_ = false;
};

/**
 * @brief Y轴翻转动作
 */
class FlipY : public ActionInstant {
public:
    static FlipY* create(bool flipY);

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;

    bool flipY_ = false;
};

// ============================================================================
// 可见性动作
// ============================================================================

/**
 * @brief 显示动作
 */
class Show : public ActionInstant {
public:
    static Show* create();

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;
};

/**
 * @brief 隐藏动作
 */
class Hide : public ActionInstant {
public:
    static Hide* create();

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;
};

/**
 * @brief 切换可见性动作
 */
class ToggleVisibility : public ActionInstant {
public:
    static ToggleVisibility* create();

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;
};

// ============================================================================
// 节点管理动作
// ============================================================================

/**
 * @brief 移除自身动作
 */
class RemoveSelf : public ActionInstant {
public:
    static RemoveSelf* create();

    ActionInstant* clone() const override;
    ActionInstant* reverse() const override;

protected:
    void execute() override;
};

} // namespace extra2d
