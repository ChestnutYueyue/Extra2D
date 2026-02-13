#pragma once

#include <extra2d/scene/transition_scene.h>

namespace extra2d {

// ============================================================================
// 方块/马赛克过渡场景
// 实现原理：
// 1. 将屏幕分成多个方块
// 2. 方块逐个消失，显示新场景
// ============================================================================
class TransitionBoxScene : public TransitionScene {
public:
  /**
   * @brief 创建方块过渡场景
   * @param duration 过渡持续时间（秒）
   * @param inScene 要进入的目标场景
   * @param divisions 方块分割数（默认为 8，表示 8x8 网格）
   */
  TransitionBoxScene(float duration, Ptr<Scene> inScene, int divisions = 8);

  static Ptr<TransitionBoxScene> create(float duration, Ptr<Scene> inScene,
                                        int divisions = 8);

protected:
  void onTransitionStart() override;
  void renderContent(RenderBackend &renderer) override;

private:
  int divisions_;
};

} // namespace extra2d
