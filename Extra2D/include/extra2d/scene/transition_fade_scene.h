#pragma once

#include <extra2d/scene/transition_scene.h>
#include <extra2d/scene/sprite.h>
#include <extra2d/core/color.h>

namespace extra2d {

// ============================================================================
// 淡入淡出过渡场景
// 实现原理：
// 1. 创建一个纯色精灵作为遮罩层
// 2. 第一阶段：遮罩从透明淡入到不透明（黑屏），同时显示旧场景
// 3. 切换显示新场景
// 4. 第二阶段：遮罩从不透明淡出到透明，显示新场景
// ============================================================================
class TransitionFadeScene : public TransitionScene {
public:
  /**
   * @brief 创建淡入淡出过渡场景
   * @param duration 过渡持续时间（秒）
   * @param inScene 要进入的目标场景
   * @param color 遮罩颜色（默认为黑色）
   */
  TransitionFadeScene(float duration, Ptr<Scene> inScene,
                      const Color &color = Colors::Black);

  static Ptr<TransitionFadeScene> create(float duration, Ptr<Scene> inScene,
                                         const Color &color = Colors::Black);

protected:
  /**
   * @brief 启动过渡动画
   * 创建遮罩层并运行动作序列
   */
  void onTransitionStart() override;

  /**
   * @brief 渲染内容
   * 根据进度控制新旧场景的显示
   */
  void renderContent(RenderBackend &renderer) override;

private:
  /**
   * @brief 隐藏退出场景，显示进入场景
   */
  void hideOutShowIn();

  Color maskColor_;           // 遮罩颜色
  bool hasSwitched_ = false;  // 是否已经切换场景
};

} // namespace extra2d
