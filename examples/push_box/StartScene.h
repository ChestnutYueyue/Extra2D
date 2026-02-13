// ============================================================================
// StartScene.h - Push Box 开始场景
// ============================================================================

#pragma once

#include "BaseScene.h"
#include <extra2d/extra2d.h>

namespace pushbox {

/**
 * @brief Push Box 开始场景（主菜单）
 */
class StartScene : public BaseScene {
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
     * @brief 每帧更新
     * @param dt 帧间隔时间
     */
    void onUpdate(float dt) override;

private:
    /**
     * @brief 更新菜单颜色
     */
    void updateMenuColors();

    /**
     * @brief 执行选中的菜单项
     */
    void executeMenuItem();

    /**
     * @brief 开始新游戏
     */
    void startNewGame();

    /**
     * @brief 继续游戏
     */
    void continueGame();

    /**
     * @brief 退出游戏
     */
    void exitGame();

    extra2d::Ptr<extra2d::FontAtlas> font_;
    extra2d::Ptr<extra2d::Button> startBtn_;
    extra2d::Ptr<extra2d::Button> resumeBtn_;
    extra2d::Ptr<extra2d::Button> exitBtn_;
    extra2d::Ptr<extra2d::Button> soundBtn_;
    int selectedIndex_ = 0;
    int menuCount_ = 3;
};

} // namespace pushbox
