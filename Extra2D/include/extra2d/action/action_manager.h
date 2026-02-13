#pragma once

#include <extra2d/action/action.h>
#include <unordered_map>
#include <vector>
#include <functional>

namespace extra2d {

/**
 * @brief 动作管理器
 * 
 * 单例模式，集中管理所有动作的调度和生命周期。
 * 负责动作的添加、移除、暂停、恢复和更新。
 */
class ActionManager {
public:
    /**
     * @brief 获取单例实例
     * @return ActionManager 实例指针
     */
    static ActionManager* getInstance();

    /**
     * @brief 销毁单例实例
     */
    static void destroyInstance();

    /**
     * @brief 添加动作到管理器
     * @param action 动作指针
     * @param target 目标节点
     * @param paused 是否暂停
     */
    void addAction(Action* action, Node* target, bool paused = false);

    /**
     * @brief 移除指定动作
     * @param action 动作指针
     */
    void removeAction(Action* action);

    /**
     * @brief 根据标签移除动作
     * @param tag 标签值
     * @param target 目标节点
     */
    void removeActionByTag(int tag, Node* target);

    /**
     * @brief 根据标志位移除动作
     * @param flags 标志位
     * @param target 目标节点
     */
    void removeActionsByFlags(unsigned int flags, Node* target);

    /**
     * @brief 移除目标节点的所有动作
     * @param target 目标节点
     */
    void removeAllActionsFromTarget(Node* target);

    /**
     * @brief 移除所有动作
     */
    void removeAllActions();

    /**
     * @brief 根据标签获取动作
     * @param tag 标签值
     * @param target 目标节点
     * @return 动作指针，未找到返回 nullptr
     */
    Action* getActionByTag(int tag, Node* target);

    /**
     * @brief 获取目标节点的动作数量
     * @param target 目标节点
     * @return 动作数量
     */
    size_t getActionCount(Node* target) const;

    /**
     * @brief 暂停目标节点的所有动作
     * @param target 目标节点
     */
    void pauseTarget(Node* target);

    /**
     * @brief 恢复目标节点的所有动作
     * @param target 目标节点
     */
    void resumeTarget(Node* target);

    /**
     * @brief 检查目标节点是否暂停
     * @param target 目标节点
     * @return true 如果暂停
     */
    bool isPaused(Node* target) const;

    /**
     * @brief 更新所有动作（每帧调用）
     * @param dt 帧时间间隔
     */
    void update(float dt);

private:
    ActionManager();
    ~ActionManager();

    struct ActionElement {
        std::vector<Action*> actions;
        Node* target = nullptr;
        bool paused = false;
        int actionIndex = 0;
        Action* currentAction = nullptr;
        bool currentActionSalvaged = false;
    };

    using ActionMap = std::unordered_map<Node*, ActionElement>;

    void removeActionAt(size_t index, ActionElement& element);
    void deleteAction(Action* action);

    ActionMap targets_;
    static ActionManager* instance_;
};

} // namespace extra2d
