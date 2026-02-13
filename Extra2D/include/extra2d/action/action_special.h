#pragma once

#include <extra2d/action/action.h>
#include <extra2d/action/action_interval.h>
#include <extra2d/core/math_types.h>

namespace extra2d {

/**
 * @brief 速度控制动作
 * 
 * 包装其他动作，实现动态速度控制。
 * 可以在运行时调整动作的播放速度。
 */
class Speed : public Action {
public:
    /**
     * @brief 创建速度控制动作
     * @param action 内部动作
     * @param speed 速度倍率（1.0 为正常速度）
     * @return 动作指针
     */
    static Speed* create(ActionInterval* action, float speed);

    ~Speed();

    /**
     * @brief 获取速度倍率
     * @return 速度倍率
     */
    float getSpeed() const { return speed_; }

    /**
     * @brief 设置速度倍率
     * @param speed 速度倍率
     */
    void setSpeed(float speed) { speed_ = speed; }

    /**
     * @brief 获取内部动作
     * @return 内部动作指针
     */
    ActionInterval* getInnerAction() const { return innerAction_; }

    void startWithTarget(Node* target) override;
    void stop() override;
    void step(float dt) override;
    bool isDone() const override;
    Action* clone() const override;
    Action* reverse() const override;

protected:
    Speed() = default;

    ActionInterval* innerAction_ = nullptr;
    float speed_ = 1.0f;
};

/**
 * @brief 跟随动作
 * 
 * 使节点跟随另一个节点移动。
 * 常用于相机跟随玩家。
 */
class Follow : public Action {
public:
    /**
     * @brief 创建跟随动作
     * @param followedNode 被跟随的节点
     * @return 动作指针
     */
    static Follow* create(Node* followedNode);

    /**
     * @brief 创建带边界的跟随动作
     * @param followedNode 被跟随的节点
     * @param boundary 边界矩形
     * @return 动作指针
     */
    static Follow* create(Node* followedNode, const Rect& boundary);

    ~Follow();

    /**
     * @brief 获取被跟随的节点
     * @return 被跟随的节点指针
     */
    Node* getFollowedNode() const { return followedNode_; }

    /**
     * @brief 检查是否设置了边界
     * @return true 如果设置了边界
     */
    bool isBoundarySet() const { return boundarySet_; }

    void startWithTarget(Node* target) override;
    void stop() override;
    void step(float dt) override;
    bool isDone() const override;
    Action* clone() const override;
    Action* reverse() const override;

protected:
    Follow() = default;

    Node* followedNode_ = nullptr;
    Rect boundary_;
    bool boundarySet_ = false;
    Vec2 halfScreenSize_;
    Vec2 fullScreenSize_;
    Vec2 leftBoundary_;
    Vec2 rightBoundary_;
    Vec2 topBoundary_;
    Vec2 bottomBoundary_;
};

/**
 * @brief 目标动作
 * 
 * 允许在一个节点上运行动作，但目标为另一个节点。
 */
class TargetedAction : public Action {
public:
    /**
     * @brief 创建目标动作
     * @param target 目标节点
     * @param action 要运行的动作
     * @return 动作指针
     */
    static TargetedAction* create(Node* target, FiniteTimeAction* action);

    ~TargetedAction();

    /**
     * @brief 获取目标节点
     * @return 目标节点指针
     */
    Node* getTargetNode() const { return targetNode_; }

    /**
     * @brief 设置目标节点
     * @param target 目标节点
     */
    void setTargetNode(Node* target) { targetNode_ = target; }

    /**
     * @brief 获取内部动作
     * @return 内部动作指针
     */
    FiniteTimeAction* getAction() const { return innerAction_; }

    void startWithTarget(Node* target) override;
    void stop() override;
    void step(float dt) override;
    bool isDone() const override;
    Action* clone() const override;
    Action* reverse() const override;

protected:
    TargetedAction() = default;

    Node* targetNode_ = nullptr;
    FiniteTimeAction* innerAction_ = nullptr;
};

} // namespace extra2d
