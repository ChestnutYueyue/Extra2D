#pragma once

#include <extra2d/scene/scene.h>
#include <functional>

namespace extra2d {

// ============================================================================
// 过渡方向
// ============================================================================
enum class TransitionDirection { Left, Right, Up, Down };

// ============================================================================
// 过渡效果类型
// ============================================================================
enum class TransitionType {
  None,
  Fade,
  SlideLeft,
  SlideRight,
  SlideUp,
  SlideDown,
  Scale,
  Flip,
  Box
};

// ============================================================================
// 过渡场景基类 - 继承自 Scene，作为中介场景管理过渡效果
// 设计参考 Cocos2d-x 的 TransitionScene
// ============================================================================
class TransitionScene : public Scene {
public:
  using FinishCallback = std::function<void()>;

  /**
   * @brief 创建过渡场景
   * @param duration 过渡持续时间（秒）
   * @param inScene 要进入的目标场景
   */
  TransitionScene(float duration, Ptr<Scene> inScene);
  ~TransitionScene() override = default;

  // ------------------------------------------------------------------------
  // 场景管理
  // ------------------------------------------------------------------------

  /**
   * @brief 获取要进入的场景
   */
  Ptr<Scene> getInScene() const { return inScene_; }

  /**
   * @brief 获取要退出的场景
   */
  Ptr<Scene> getOutScene() const { return outScene_; }

  /**
   * @brief 设置要退出的场景（由 SceneManager 调用）
   */
  void setOutScene(Ptr<Scene> outScene) { outScene_ = outScene; }

  /**
   * @brief 设置过渡完成回调
   */
  void setFinishCallback(FinishCallback callback) { finishCallback_ = callback; }

  /**
   * @brief 获取过渡持续时间
   */
  float getDuration() const { return duration_; }

  /**
   * @brief 获取当前进度 [0, 1]
   */
  float getProgress() const { return progress_; }

  /**
   * @brief 是否已完成
   */
  bool isFinished() const { return isFinished_; }

  /**
   * @brief 完成过渡，通知 SceneManager 切换到目标场景
   */
  void finish();

  // ------------------------------------------------------------------------
  // 渲染 - 在 TransitionScene 上渲染新旧两个子场景
  // ------------------------------------------------------------------------
  void renderContent(RenderBackend &renderer) override;

  // ------------------------------------------------------------------------
  // 生命周期
  // ------------------------------------------------------------------------
  void onEnter() override;
  void onExit() override;

protected:
  /**
   * @brief 子类实现具体的过渡逻辑
   * 在 onEnter 中设置动画，动画完成后调用 finish()
   */
  virtual void onTransitionStart() = 0;

  /**
   * @brief 绘制源场景（旧场景）
   */
  virtual void drawOutScene(RenderBackend &renderer);

  /**
   * @brief 绘制目标场景（新场景）
   */
  virtual void drawInScene(RenderBackend &renderer);

  float duration_;
  float elapsed_ = 0.0f;
  float progress_ = 0.0f;
  bool isFinished_ = false;

  Ptr<Scene> inScene_;   // 要进入的场景
  Ptr<Scene> outScene_;  // 要退出的场景

  FinishCallback finishCallback_;
};

} // namespace extra2d
