#pragma once

#include <extra2d/scene/transition_scene.h>

namespace extra2d {

// ============================================================================
// 缩放过渡场景
// 实现原理：
// 1. 旧场景缩小消失
// 2. 新场景放大出现
// ============================================================================
class TransitionScaleScene : public TransitionScene {
public:
  /**
   * @brief 创建缩放过渡场景
   * @param duration 过渡持续时间（秒）
   * @param inScene 要进入的目标场景
   */
  TransitionScaleScene(float duration, Ptr<Scene> inScene);

  static Ptr<TransitionScaleScene> create(float duration, Ptr<Scene> inScene);

protected:
  void onTransitionStart() override;
  void renderContent(RenderBackend &renderer) override;
};

} // namespace extra2d
