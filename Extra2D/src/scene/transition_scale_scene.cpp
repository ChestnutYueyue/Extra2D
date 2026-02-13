#include <extra2d/scene/transition_scale_scene.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_backend.h>
#include <algorithm>

namespace extra2d {

TransitionScaleScene::TransitionScaleScene(float duration, Ptr<Scene> inScene)
    : TransitionScene(duration, inScene) {}

Ptr<TransitionScaleScene> TransitionScaleScene::create(float duration,
                                                       Ptr<Scene> inScene) {
  return makePtr<TransitionScaleScene>(duration, inScene);
}

void TransitionScaleScene::onTransitionStart() {
  // 缩放过渡不需要特殊的初始化
}

void TransitionScaleScene::renderContent(RenderBackend &renderer) {
  // 更新进度
  elapsed_ += 1.0f / 60.0f;
  progress_ = duration_ > 0.0f ? std::min(1.0f, elapsed_ / duration_) : 1.0f;

  // 缓动函数
  float easeProgress = progress_ < 0.5f ? 2.0f * progress_ * progress_
                                        : -1.0f + (4.0f - 2.0f * progress_) * progress_;

  // 源场景：缩小消失
  if (outScene_) {
    float scale = std::max(0.01f, 1.0f - easeProgress);

    Camera *camera = outScene_->getActiveCamera();
    float originalZoom = camera ? camera->getZoom() : 1.0f;

    if (camera) {
      camera->setZoom(originalZoom * scale);
    }

    outScene_->renderContent(renderer);

    if (camera) {
      camera->setZoom(originalZoom);
    }
  }

  // 目标场景：放大出现
  if (inScene_) {
    float scale = std::max(0.01f, easeProgress);

    Camera *camera = inScene_->getActiveCamera();
    float originalZoom = camera ? camera->getZoom() : 1.0f;

    if (camera) {
      camera->setZoom(originalZoom * scale);
    }

    inScene_->renderContent(renderer);

    if (camera) {
      camera->setZoom(originalZoom);
    }
  }

  // 检查是否完成
  if (progress_ >= 1.0f && !isFinished_) {
    finish();
  }
}

} // namespace extra2d
