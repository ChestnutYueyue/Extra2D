#include <extra2d/scene/transition_slide_scene.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_backend.h>

namespace extra2d {

TransitionSlideScene::TransitionSlideScene(float duration, Ptr<Scene> inScene,
                                           TransitionDirection direction)
    : TransitionScene(duration, inScene), direction_(direction) {}

Ptr<TransitionSlideScene> TransitionSlideScene::create(
    float duration, Ptr<Scene> inScene, TransitionDirection direction) {
  return makePtr<TransitionSlideScene>(duration, inScene, direction);
}

void TransitionSlideScene::onTransitionStart() {
  // 滑动过渡不需要特殊的初始化
}

void TransitionSlideScene::renderContent(RenderBackend &renderer) {
  // 获取视口尺寸
  float screenWidth = 800.0f;
  float screenHeight = 600.0f;

  if (outScene_) {
    Size viewportSize = outScene_->getViewportSize();
    if (viewportSize.width > 0 && viewportSize.height > 0) {
      screenWidth = viewportSize.width;
      screenHeight = viewportSize.height;
    }
  } else if (inScene_) {
    Size viewportSize = inScene_->getViewportSize();
    if (viewportSize.width > 0 && viewportSize.height > 0) {
      screenWidth = viewportSize.width;
      screenHeight = viewportSize.height;
    }
  }

  // 更新进度
  elapsed_ += 1.0f / 60.0f;
  progress_ = duration_ > 0.0f ? std::min(1.0f, elapsed_ / duration_) : 1.0f;

  // 缓动函数
  float easeProgress = progress_ < 0.5f ? 2.0f * progress_ * progress_
                                        : -1.0f + (4.0f - 2.0f * progress_) * progress_;

  // 渲染退出场景（滑出）
  if (outScene_) {
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    switch (direction_) {
    case TransitionDirection::Left:
      offsetX = -screenWidth * easeProgress;
      break;
    case TransitionDirection::Right:
      offsetX = screenWidth * easeProgress;
      break;
    case TransitionDirection::Up:
      offsetY = -screenHeight * easeProgress;
      break;
    case TransitionDirection::Down:
      offsetY = screenHeight * easeProgress;
      break;
    }

    Camera *camera = outScene_->getActiveCamera();
    Vec2 originalPos = camera ? camera->getPosition() : Vec2::Zero();

    if (camera) {
      camera->setPosition(originalPos.x + offsetX, originalPos.y + offsetY);
    }

    outScene_->renderContent(renderer);

    if (camera) {
      camera->setPosition(originalPos);
    }
  }

  // 渲染进入场景（滑入）
  if (inScene_) {
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    switch (direction_) {
    case TransitionDirection::Left:
      offsetX = screenWidth * (1.0f - easeProgress);
      break;
    case TransitionDirection::Right:
      offsetX = -screenWidth * (1.0f - easeProgress);
      break;
    case TransitionDirection::Up:
      offsetY = screenHeight * (1.0f - easeProgress);
      break;
    case TransitionDirection::Down:
      offsetY = -screenHeight * (1.0f - easeProgress);
      break;
    }

    Camera *camera = inScene_->getActiveCamera();
    Vec2 originalPos = camera ? camera->getPosition() : Vec2::Zero();

    if (camera) {
      camera->setPosition(originalPos.x + offsetX, originalPos.y + offsetY);
    }

    inScene_->renderContent(renderer);

    if (camera) {
      camera->setPosition(originalPos);
    }
  }

  // 检查是否完成
  if (progress_ >= 1.0f && !isFinished_) {
    finish();
  }
}

} // namespace extra2d
