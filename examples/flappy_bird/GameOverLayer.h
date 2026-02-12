// ============================================================================
// GameOverLayer.h - 游戏结束层
// 描述: 显示游戏结束界面、得分和奖牌
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 游戏结束层类
 * 显示游戏结束后的得分面板和按钮
 */
class GameOverLayer : public extra2d::Node {
public:
    /**
     * @brief 构造函数
     * @param score 本局得分
     */
    GameOverLayer(int score);

    /**
     * @brief 进入场景时调用
     */
    void onEnter() override;

    /**
     * @brief 每帧更新时调用
     * @param dt 时间间隔
     */
    void onUpdate(float dt) override;

private:
    /**
     * @brief 初始化得分面板
     * @param score 本局得分
     * @param screenHeight 屏幕高度
     */
    void initPanel(int score, float screenHeight);

    /**
     * @brief 初始化按钮
     */
    void initButtons();

    /**
     * @brief 根据得分获取奖牌
     * @param score 得分
     * @return 奖牌精灵帧
     */
    extra2d::Ptr<extra2d::SpriteFrame> getMedal(int score);

    int score_ = 0;  // 本局得分
};

} // namespace flappybird
