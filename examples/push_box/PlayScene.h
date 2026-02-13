// ============================================================================
// PlayScene.h - Push Box 游戏场景
// ============================================================================

#pragma once

#include "BaseScene.h"
#include "data.h"
#include <extra2d/extra2d.h>

namespace pushbox {

/**
 * @brief Push Box 游戏场景
 */
class PlayScene : public BaseScene {
public:
    /**
     * @brief 构造函数
     * @param level 关卡编号
     */
    explicit PlayScene(int level);

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
     * @brief 刷新地图显示
     */
    void flush();

    /**
     * @brief 设置关卡
     * @param level 关卡编号
     */
    void setLevel(int level);

    /**
     * @brief 设置步数
     * @param step 步数
     */
    void setStep(int step);

    /**
     * @brief 移动玩家
     * @param dx X方向偏移
     * @param dy Y方向偏移
     * @param direct 方向（1=上，2=下，3=左，4=右）
     */
    void move(int dx, int dy, int direct);

    /**
     * @brief 游戏通关
     */
    void gameOver();

    int step_ = 0;
    int menuIndex_ = 0;
    Map map_{};

    extra2d::Ptr<extra2d::FontAtlas> font28_;
    extra2d::Ptr<extra2d::FontAtlas> font20_;

    extra2d::Ptr<extra2d::Text> levelText_;
    extra2d::Ptr<extra2d::Text> stepText_;
    extra2d::Ptr<extra2d::Text> bestText_;
    extra2d::Ptr<extra2d::Text> restartText_;
    extra2d::Ptr<extra2d::Text> soundToggleText_;
    extra2d::Ptr<extra2d::Node> mapLayer_;

    extra2d::Ptr<extra2d::Button> soundBtn_;

    extra2d::Ptr<extra2d::Texture> texWall_;
    extra2d::Ptr<extra2d::Texture> texPoint_;
    extra2d::Ptr<extra2d::Texture> texFloor_;
    extra2d::Ptr<extra2d::Texture> texBox_;
    extra2d::Ptr<extra2d::Texture> texBoxInPoint_;

    extra2d::Ptr<extra2d::Texture> texMan_[5];
    extra2d::Ptr<extra2d::Texture> texManPush_[5];
};

} // namespace pushbox
