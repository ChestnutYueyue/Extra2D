#pragma once

#include <extra2d/action/finite_time_action.h>

namespace extra2d {

/**
 * @brief 瞬时动作基类
 * 
 * 瞬间完成的动作，中间没有任何动画效果。
 * 继承自 FiniteTimeAction，持续时间为 0。
 */
class ActionInstant : public FiniteTimeAction {
public:
    ActionInstant();
    virtual ~ActionInstant() = default;

    /**
     * @brief 检查动作是否完成
     * @return 总是返回 true
     */
    bool isDone() const override;

    /**
     * @brief 使用目标节点启动动作
     * @param target 目标节点
     */
    void startWithTarget(Node* target) override;

    /**
     * @brief 每帧调用的步进函数
     * @param dt 帧时间间隔
     */
    void step(float dt) override;

    /**
     * @brief 克隆动作
     * @return 动作的深拷贝
     */
    ActionInstant* clone() const override = 0;

    /**
     * @brief 创建反向动作
     * @return 反向动作
     */
    ActionInstant* reverse() const override = 0;

protected:
    /**
     * @brief 执行瞬时动作
     */
    virtual void execute() = 0;

    bool done_ = false;
};

} // namespace extra2d
