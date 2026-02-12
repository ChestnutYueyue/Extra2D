// ============================================================================
// Pipe.h - 水管类
// 描述: 游戏中的障碍物，由上下两根水管组成
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>

namespace flappybird {

/**
 * @brief 水管类
 * 由上下两根水管组成的障碍物
 */
class Pipe : public extra2d::Node {
public:
    /**
     * @brief 构造函数
     */
    Pipe();

    /**
     * @brief 析构函数
     */
    ~Pipe();

    /**
     * @brief 进入场景时调用
     */
    void onEnter() override;

    /**
     * @brief 获取边界框（用于碰撞检测）
     * @return 边界框
     */
    extra2d::Rect getBoundingBox() const override;

    /**
     * @brief 获取上水管边界框
     * @return 边界框
     */
    extra2d::Rect getTopPipeBox() const;

    /**
     * @brief 获取下水管边界框
     * @return 边界框
     */
    extra2d::Rect getBottomPipeBox() const;

    bool scored = false;  // 是否已计分

private:
    extra2d::Ptr<extra2d::Sprite> topPipe_;     // 上水管
    extra2d::Ptr<extra2d::Sprite> bottomPipe_;  // 下水管
    float gapHeight_ = 120.0f;                  // 间隙高度
};

} // namespace flappybird
