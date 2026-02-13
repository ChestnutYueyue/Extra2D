#pragma once

#include <extra2d/scene/transition_scene.h>

namespace extra2d {

// ============================================================================
// 滑动过渡场景
// 实现原理：
// 1. 旧场景向指定方向滑出
// 2. 新场景从相反方向滑入
// ============================================================================
class TransitionSlideScene : public TransitionScene {
public:
  /**
   * @brief 创建滑动过渡场景
   * @param duration 过渡持续时间（秒）
   * @param inScene 要进入的目标场景
   * @param direction 滑动方向
   */
  TransitionSlideScene(float duration, Ptr<Scene> inScene,
                       TransitionDirection direction);

  static Ptr<TransitionSlideScene> create(float duration, Ptr<Scene> inScene,
                                          TransitionDirection direction);

protected:
  void onTransitionStart() override;
  void renderContent(RenderBackend &renderer) override;

private:
  TransitionDirection direction_;
};

} // namespace extra2d
