#pragma once

#include <extra2d/action/finite_time_action.h>
#include <extra2d/action/ease.h>

namespace extra2d {

/**
 * @brief 时间间隔动作基类
 * 
 * 在指定时间内完成的动作，中间会有动画效果。
 * 继承自 FiniteTimeAction。
 */
class ActionInterval : public FiniteTimeAction {
public:
    ActionInterval() = default;
    explicit ActionInterval(float duration);
    virtual ~ActionInterval() = default;

    /**
     * @brief 获取已流逝时间
     * @return 已流逝时间（秒）
     */
    float getElapsed() const { return elapsed_; }

    /**
     * @brief 检查动作是否完成
     * @return true 如果动作已完成
     */
    bool isDone() const override;

    /**
     * @brief 使用目标节点启动动作
     * @param target 目标节点
     */
    void startWithTarget(Node* target) override;

    /**
     * @brief 停止动作
     */
    void stop() override;

    /**
     * @brief 每帧调用的步进函数
     * @param dt 帧时间间隔
     */
    void step(float dt) override;

    /**
     * @brief 设置振幅比率（用于缓动）
     * @param amp 振幅比率
     */
    void setAmplitudeRate(float amp) { amplitudeRate_ = amp; }

    /**
     * @brief 获取振幅比率
     * @return 振幅比率
     */
    float getAmplitudeRate() const { return amplitudeRate_; }

    /**
     * @brief 设置内置缓动函数
     * @param easeFunc 缓动函数
     */
    void setEaseFunction(EaseFunction easeFunc) { easeFunc_ = easeFunc; }

    /**
     * @brief 获取内置缓动函数
     * @return 缓动函数
     */
    EaseFunction getEaseFunction() const { return easeFunc_; }

    /**
     * @brief 克隆动作
     * @return 动作的深拷贝
     */
    ActionInterval* clone() const override = 0;

    /**
     * @brief 创建反向动作
     * @return 反向动作
     */
    ActionInterval* reverse() const override = 0;

protected:
    /**
     * @brief 动作开始时调用
     */
    virtual void onStart() {}

    /**
     * @brief 动作更新时调用
     * @param progress 归一化进度 [0, 1]
     */
    virtual void onUpdate(float progress) = 0;

    float elapsed_ = 0.0f;
    bool firstTick_ = true;
    float amplitudeRate_ = 1.0f;
    EaseFunction easeFunc_ = nullptr;
};

} // namespace extra2d
