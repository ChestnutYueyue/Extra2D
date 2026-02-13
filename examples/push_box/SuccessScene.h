// ============================================================================
// SuccessScene.h - Push Box 通关场景
// ============================================================================

#pragma once

#include "BaseScene.h"
#include <extra2d/extra2d.h>

namespace pushbox {

/**
 * @brief Push Box 通关场景
 */
class SuccessScene : public BaseScene {
public:
    /**
     * @brief 构造函数
     */
    SuccessScene();

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
    extra2d::Ptr<extra2d::Text> selectorText_;
};

} // namespace pushbox
