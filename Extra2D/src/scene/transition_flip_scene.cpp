#include <extra2d/scene/transition_flip_scene.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/core/math_types.h>

namespace extra2d {

TransitionFlipScene::TransitionFlipScene(float duration, Ptr<Scene> inScene,
                                         Axis axis)
    : TransitionScene(duration, inScene), axis_(axis) {}

Ptr<TransitionFlipScene> TransitionFlipScene::create(float duration,
                                                     Ptr<Scene> inScene,
                                                     Axis axis) {
  return makePtr<TransitionFlipScene>(duration, inScene, axis);
}

void TransitionFlipScene::onTransitionStart() {
  // 翻页过渡不需要特殊的初始化
}

void TransitionFlipScene::renderContent(RenderBackend &renderer) {
  // 更新进度
  elapsed_ += 1.0f / 60.0f;
  progress_ = duration_ > 0.0f ? std::min(1.0f, elapsed_ / duration_) : 1.0f;

  // 缓动函数
  float easeProgress = progress_ < 0.5f ? 2.0f * progress_ * progress_
                                        : -1.0f + (4.0f - 2.0f * progress_) * progress_;

  float angle = easeProgress * PI_F;  // 180度翻转

  if (progress_ < 0.5f) {
    // 前半段：翻转源场景
    if (outScene_) {
      float currentAngle = angle;

      Camera *camera = outScene_->getActiveCamera();
      float originalRotation = camera ? camera->getRotation() : 0.0f;

      if (axis_ == Axis::Horizontal) {
        // 水平轴翻转 - 模拟绕X轴旋转
        if (camera) {
          camera->setRotation(originalRotation + currentAngle * RAD_TO_DEG);
        }
      } else {
        // 垂直轴翻转 - 模拟绕Y轴旋转
        if (camera) {
          camera->setRotation(originalRotation - currentAngle * RAD_TO_DEG);
        }
      }

      outScene_->renderContent(renderer);

      if (camera) {
        camera->setRotation(originalRotation);
      }
    }
  } else {
    // 后半段：翻转目标场景
    if (inScene_) {
      float currentAngle = angle - PI_F;

      Camera *camera = inScene_->getActiveCamera();
      float originalRotation = camera ? camera->getRotation() : 0.0f;

      if (axis_ == Axis::Horizontal) {
        if (camera) {
          camera->setRotation(originalRotation + currentAngle * RAD_TO_DEG);
        }
      } else {
        if (camera) {
          camera->setRotation(originalRotation - currentAngle * RAD_TO_DEG);
        }
      }

      inScene_->renderContent(renderer);

      if (camera) {
        camera->setRotation(originalRotation);
      }
    }
  }

  // 检查是否完成
  if (progress_ >= 1.0f && !isFinished_) {
    finish();
  }
}

} // namespace extra2d
