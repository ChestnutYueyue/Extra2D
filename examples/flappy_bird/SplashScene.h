// ============================================================================
// SplashScene.h - 启动场景
// 描述: 显示游戏 Logo，2秒后自动跳转到开始场景
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 启动场景类
 * 显示游戏 Logo，短暂延迟后进入主菜单
 */
class SplashScene : public extra2d::Scene {
public:
    /**
     * @brief 构造函数
     */
    SplashScene();

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
     * @brief 跳转到开始场景
     */
    void gotoStartScene();

    float timer_ = 0.0f;        // 计时器
    const float delay_ = 2.0f;  // 延迟时间（秒）
};

} // namespace flappybird
