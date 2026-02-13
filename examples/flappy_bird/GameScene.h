// ============================================================================
// GameScene.h - 游戏主场景
// 描述: 游戏的核心场景，包含小鸟、水管、地面和得分系统
// ============================================================================

#pragma once

#include "BaseScene.h"
#include "Bird.h"
#include "Pipes.h"
#include "Ground.h"
#include "Number.h"

namespace flappybird {

/**
 * @brief 游戏主场景类
 * 游戏的核心场景，处理游戏逻辑、碰撞检测和得分
 */
class GameScene : public BaseScene {
public:
    /**
     * @brief 构造函数
     */
    GameScene();

    /**
     * @brief 场景进入时调用
     */
    void onEnter() override;

    /**
     * @brief 每帧更新时调用
     * @param dt 时间间隔（秒）
     */
    void onUpdate(float dt) override;

private:
    /**
     * @brief 开始游戏
     */
    void startGame();

    /**
     * @brief 处理碰撞事件
     */
    void onHit();

    /**
     * @brief 游戏结束
     */
    void gameOver();

    /**
     * @brief 检查小鸟与水管的碰撞
     * @return 是否发生碰撞
     */
    bool checkCollision();

    Bird* bird_ = nullptr;              // 小鸟
    Pipes* pipes_ = nullptr;            // 水管管理器
    Ground* ground_ = nullptr;          // 地面
    Number* scoreNumber_ = nullptr;     // 得分显示
    extra2d::Ptr<extra2d::Sprite> readySprite_;     // "Get Ready" 提示
    extra2d::Ptr<extra2d::Sprite> tutorialSprite_;  // 操作教程提示

    bool started_ = false;              // 游戏是否已开始
    bool gameOver_ = false;             // 游戏是否已结束
    int score_ = 0;                     // 当前得分
};

} // namespace flappybird
