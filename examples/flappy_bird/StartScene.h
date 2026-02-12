// ============================================================================
// StartScene.h - 开始菜单场景
// 描述: 显示游戏标题、开始按钮和版权信息
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 开始场景类
 * 游戏主菜单，包含开始游戏按钮和版权信息
 */
class StartScene : public extra2d::Scene {
public:
    /**
     * @brief 构造函数
     */
    StartScene();

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
     * @brief 创建菜单按钮
     */
    void createMenuButtons();

    /**
     * @brief 开始游戏
     */
    void startGame();

    extra2d::Ptr<extra2d::Button> playBtn_;    // 开始按钮
    extra2d::Ptr<extra2d::Button> shareBtn_;   // 分享按钮
    extra2d::Ptr<extra2d::Sprite> title_;      // 标题精灵
    float titleFinalY_ = 0.0f;                 // 标题最终Y位置
    float titleAnimTime_ = 0.0f;               // 标题动画时间
    static constexpr float TITLE_ANIM_DURATION = 0.5f; // 标题动画持续时间
};

} // namespace flappybird
