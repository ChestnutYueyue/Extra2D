#pragma once

#include <extra2d/scene/transition_scene.h>

namespace extra2d {

// ============================================================================
// 翻页过渡场景
// 实现原理：
// 1. 前半段：旧场景翻转消失
// 2. 后半段：新场景翻转出现
// ============================================================================
class TransitionFlipScene : public TransitionScene {
public:
  enum class Axis { Horizontal, Vertical };

  /**
   * @brief 创建翻页过渡场景
   * @param duration 过渡持续时间（秒）
   * @param inScene 要进入的目标场景
   * @param axis 翻转轴（水平或垂直）
   */
  TransitionFlipScene(float duration, Ptr<Scene> inScene,
                      Axis axis = Axis::Horizontal);

  static Ptr<TransitionFlipScene> create(float duration, Ptr<Scene> inScene,
                                         Axis axis = Axis::Horizontal);

protected:
  void onTransitionStart() override;
  void renderContent(RenderBackend &renderer) override;

private:
  Axis axis_;
};

} // namespace extra2d
