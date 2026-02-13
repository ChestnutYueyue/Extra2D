#include <extra2d/scene/transition_fade_scene.h>
#include <extra2d/app/application.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_target.h>
#include <extra2d/utils/logger.h>
#include <glm/gtc/matrix_transform.hpp>

namespace extra2d {

TransitionFadeScene::TransitionFadeScene(float duration, Ptr<Scene> inScene,
                                         const Color &color)
    : TransitionScene(duration, inScene), maskColor_(color) {}

Ptr<TransitionFadeScene> TransitionFadeScene::create(float duration,
                                                     Ptr<Scene> inScene,
                                                     const Color &color) {
  return makePtr<TransitionFadeScene>(duration, inScene, color);
}

void TransitionFadeScene::onTransitionStart() {
  E2D_LOG_DEBUG("TransitionFadeScene::onTransitionStart - 启动淡入淡出过渡");

  // 使用一个定时器来更新进度
  // 由于我们没有直接的动作系统集成到 Scene，使用简单的 update 逻辑
  // 实际进度更新由 SceneManager 的 update 驱动
}

void TransitionFadeScene::renderContent(RenderBackend &renderer) {
  // 获取窗口大小
  auto &app = Application::instance();
  float windowWidth = static_cast<float>(app.window().getWidth());
  float windowHeight = static_cast<float>(app.window().getHeight());

  // 计算当前进度
  elapsed_ += 1.0f / 60.0f;  // 假设 60fps，实际应该由 update 传递 dt
  progress_ = duration_ > 0.0f ? std::min(1.0f, elapsed_ / duration_) : 1.0f;

  // 检查是否需要切换场景（进度过半时）
  if (!hasSwitched_ && progress_ >= 0.5f) {
    hideOutShowIn();
  }

  // 根据进度渲染
  if (progress_ < 0.5f) {
    // 第一阶段：显示旧场景
    drawOutScene(renderer);
  } else {
    // 第二阶段：显示新场景
    drawInScene(renderer);
  }

  // 绘制遮罩层
  // 计算遮罩透明度
  float maskAlpha;
  if (progress_ < 0.5f) {
    // 前半段：从透明到不透明
    maskAlpha = progress_ * 2.0f;  // 0 -> 1
  } else {
    // 后半段：从不透明到透明
    maskAlpha = (1.0f - progress_) * 2.0f;  // 1 -> 0
  }

  // 设置视口为整个窗口
  renderer.setViewport(0, 0, static_cast<int>(windowWidth),
                       static_cast<int>(windowHeight));

  // 设置正交投影
  glm::mat4 overlayVP =
      glm::ortho(0.0f, windowWidth, windowHeight, 0.0f, -1.0f, 1.0f);
  renderer.setViewProjection(overlayVP);

  // 绘制遮罩
  Color maskColor = maskColor_;
  maskColor.a = maskAlpha;
  renderer.fillRect(Rect(0.0f, 0.0f, windowWidth, windowHeight), maskColor);

  // 检查是否完成
  if (progress_ >= 1.0f && !isFinished_) {
    finish();
  }
}

void TransitionFadeScene::hideOutShowIn() {
  hasSwitched_ = true;
  E2D_LOG_DEBUG("TransitionFadeScene::hideOutShowIn - 切换场景显示");
}

} // namespace extra2d
