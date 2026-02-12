// ============================================================================
// Ground.h - 地面类
// 描述: 游戏底部不断向左滚动的地面
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 地面类
 * 游戏底部的滚动地面，由两块地面拼接而成
 */
class Ground : public extra2d::Node {
public:
    /**
     * @brief 构造函数
     */
    Ground();

    /**
     * @brief 每帧更新
     * @param dt 时间间隔（秒）
     */
    void onUpdate(float dt) override;

    /**
     * @brief 停止地面滚动
     */
    void stop();

    /**
     * @brief 获取地面高度
     * @return 地面高度
     */
    float getHeight() const;

private:
    extra2d::Ptr<extra2d::Sprite> ground1_;     // 第一块地面
    extra2d::Ptr<extra2d::Sprite> ground2_;     // 第二块地面
    
    static constexpr float speed = 120.0f;      // 滚动速度（像素/秒）
    bool moving_ = true;                        // 是否正在滚动
};

} // namespace flappybird
