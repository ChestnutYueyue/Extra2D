#pragma once

#include <extra2d/action/action.h>

namespace extra2d {

/**
 * @brief 有限时间动作基类
 * 
 * 所有具有持续时间的动作的基类。
 * 继承自 Action，添加了持续时间属性。
 */
class FiniteTimeAction : public Action {
public:
    FiniteTimeAction() = default;
    explicit FiniteTimeAction(float duration);
    virtual ~FiniteTimeAction() = default;

    /**
     * @brief 获取动作持续时间
     * @return 持续时间（秒）
     */
    float getDuration() const { return duration_; }

    /**
     * @brief 设置动作持续时间
     * @param duration 持续时间（秒）
     */
    void setDuration(float duration) { duration_ = duration; }

    /**
     * @brief 克隆动作
     * @return 动作的深拷贝
     */
    FiniteTimeAction* clone() const override = 0;

    /**
     * @brief 创建反向动作
     * @return 反向动作
     */
    FiniteTimeAction* reverse() const override = 0;

protected:
    float duration_ = 0.0f;
};

} // namespace extra2d
