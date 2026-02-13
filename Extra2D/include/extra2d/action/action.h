#pragma once

#include <extra2d/core/types.h>
#include <functional>

namespace extra2d {

class Node;

/**
 * @brief 动作状态枚举
 */
enum class ActionState {
    Idle,
    Running,
    Paused,
    Completed
};

/**
 * @brief 动作基类
 * 
 * 所有动作的基类，定义了动作的核心接口。
 * 动作用于修改 Node 的属性，实现动画效果。
 */
class Action {
public:
    using CompletionCallback = std::function<void()>;

    Action();
    virtual ~Action() = default;

    Action(const Action&) = delete;
    Action& operator=(const Action&) = delete;
    Action(Action&&) = default;
    Action& operator=(Action&&) = default;

    /**
     * @brief 检查动作是否完成
     * @return true 如果动作已完成
     */
    virtual bool isDone() const;

    /**
     * @brief 使用目标节点启动动作
     * @param target 目标节点
     */
    virtual void startWithTarget(Node* target);

    /**
     * @brief 停止动作
     */
    virtual void stop();

    /**
     * @brief 每帧调用的步进函数
     * @param dt 帧时间间隔
     */
    virtual void step(float dt);

    /**
     * @brief 更新动作状态
     * @param time 归一化时间 [0, 1]
     */
    virtual void update(float time);

    /**
     * @brief 克隆动作
     * @return 动作的深拷贝
     */
    virtual Action* clone() const = 0;

    /**
     * @brief 创建反向动作
     * @return 反向动作
     */
    virtual Action* reverse() const = 0;

    /**
     * @brief 暂停动作
     */
    void pause();

    /**
     * @brief 恢复动作
     */
    void resume();

    /**
     * @brief 重启动作
     */
    void restart();

    /**
     * @brief 设置完成回调
     * @param callback 回调函数
     */
    void setCompletionCallback(const CompletionCallback& callback) {
        completionCallback_ = callback;
    }

    /**
     * @brief 获取目标节点
     * @return 目标节点指针
     */
    Node* getTarget() const { return target_; }

    /**
     * @brief 获取原始目标节点
     * @return 原始目标节点指针
     */
    Node* getOriginalTarget() const { return originalTarget_; }

    /**
     * @brief 获取动作状态
     * @return 当前状态
     */
    ActionState getState() const { return state_; }

    /**
     * @brief 获取标签
     * @return 标签值
     */
    int getTag() const { return tag_; }

    /**
     * @brief 设置标签
     * @param tag 标签值
     */
    void setTag(int tag) { tag_ = tag; }

    /**
     * @brief 获取标志位
     * @return 标志位
     */
    unsigned int getFlags() const { return flags_; }

    /**
     * @brief 设置标志位
     * @param flags 标志位
     */
    void setFlags(unsigned int flags) { flags_ = flags; }

protected:
    /**
     * @brief 动作开始时调用
     */
    virtual void onStart() {}

    /**
     * @brief 动作完成时调用
     */
    virtual void onComplete() {
        if (completionCallback_) {
            completionCallback_();
        }
    }

    /**
     * @brief 设置动作完成状态
     */
    void setDone() { state_ = ActionState::Completed; }

    Node* target_ = nullptr;
    Node* originalTarget_ = nullptr;
    ActionState state_ = ActionState::Idle;
    int tag_ = -1;
    unsigned int flags_ = 0;
    CompletionCallback completionCallback_;
};

} // namespace extra2d
