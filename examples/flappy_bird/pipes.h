// ============================================================================
// Pipes.h - 水管管理器
// 描述: 管理多个水管的生成、移动和回收
// ============================================================================

#pragma once

#include <extra2d/extra2d.h>
#include "Pipe.h"

namespace flappybird {

/**
 * @brief 水管管理器类
 * 管理游戏中的所有水管，负责生成、移动和回收
 */
class Pipes : public extra2d::Node {
public:
    /**
     * @brief 构造函数
     */
    Pipes();

    /**
     * @brief 析构函数
     */
    ~Pipes();

    /**
     * @brief 每帧更新
     * @param dt 时间间隔（秒）
     */
    void onUpdate(float dt) override;

    /**
     * @brief 进入场景时调用
     */
    void onEnter() override;

    /**
     * @brief 开始移动水管
     */
    void start();

    /**
     * @brief 停止移动水管
     */
    void stop();

    /**
     * @brief 获取当前水管数组
     * @return 水管指针数组
     */
    Pipe* getPipe(int index) { return (index >= 0 && index < 3) ? pipes_[index] : nullptr; }

private:
    /**
     * @brief 添加一根新水管
     */
    void addPipe();

    static constexpr int maxPipes = 3;      // 最大水管数量
    static constexpr float pipeSpeed = 120.0f;  // 水管移动速度（像素/秒）
    static constexpr float pipeSpacing = 200.0f; // 水管间距

    Pipe* pipes_[maxPipes];                 // 水管数组
    int pipeCount_ = 0;                     // 当前水管数量
    bool moving_ = false;                   // 是否正在移动
};

} // namespace flappybird
