// ============================================================================
// Bird.h - 小鸟类
// 描述: 玩家控制的小鸟角色，包含飞行动画和物理效果
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 小鸟类
 * 游戏主角，包含飞行动画、重力模拟和状态管理
 */
class Bird : public extra2d::Node {
public:
    /**
     * @brief 小鸟状态枚举
     */
    enum class Status {
        Still,      // 静止不动
        Idle,       // 上下浮动（菜单展示）
        StartToFly, // 开始飞行
        Fly         // 飞行中
    };

    /**
     * @brief 构造函数
     */
    Bird();

    /**
     * @brief 析构函数
     */
    ~Bird();

    /**
     * @brief 每帧更新
     * @param dt 时间间隔（秒）
     */
    void onUpdate(float dt) override;

    /**
     * @brief 渲染小鸟
     * @param renderer 渲染后端
     */
    void onRender(extra2d::RenderBackend& renderer) override;

    /**
     * @brief 进入场景时调用
     */
    void onEnter() override;

    /**
     * @brief 模拟下落
     * @param dt 时间间隔（秒）
     */
    void fall(float dt);

    /**
     * @brief 跳跃
     */
    void jump();

    /**
     * @brief 死亡
     */
    void die();

    /**
     * @brief 设置小鸟状态
     * @param status 新状态
     */
    void setStatus(Status status);

    /**
     * @brief 获取当前状态
     * @return 当前状态
     */
    Status getStatus() const { return status_; }

    /**
     * @brief 检查是否存活
     * @return 是否存活
     */
    bool isLiving() const { return living_; }

    /**
     * @brief 获取边界框（用于碰撞检测）
     * @return 边界框
     */
    extra2d::Rect getBoundingBox() const override;

private:
    /**
     * @brief 初始化动画
     */
    void initAnimations();

    bool living_ = true;                    // 是否存活
    float speed_ = 0.0f;                    // 垂直速度
    float rotation_ = 0.0f;                 // 旋转角度
    Status status_ = Status::Idle;          // 当前状态

    // 动画相关
    extra2d::Ptr<extra2d::AnimatedSprite> animSprite_;  // 动画精灵
    float idleTimer_ = 0.0f;                            // 闲置动画计时器
    float idleOffset_ = 0.0f;                           // 闲置偏移量

    // 物理常量
    static constexpr float gravity = 1440.0f;   // 重力加速度
    static constexpr float jumpSpeed = 432.0f;  // 跳跃初速度
};

} // namespace flappybird
