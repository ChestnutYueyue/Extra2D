#include <extra2d/scene/transition_scene.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

TransitionScene::TransitionScene(float duration, Ptr<Scene> inScene)
    : duration_(duration), inScene_(inScene) {}

void TransitionScene::onEnter() {
  // 调用基类的 onEnter
  Scene::onEnter();

  // 调用退出场景的 onExitTransitionDidStart
  if (outScene_) {
    outScene_->onExitTransitionDidStart();
  }

  // 调用进入场景的 onEnter
  if (inScene_) {
    inScene_->onEnter();
    inScene_->onAttachToScene(inScene_.get());
  }

  // 启动过渡
  onTransitionStart();
}

void TransitionScene::onExit() {
  // 调用退出场景的 onExit
  if (outScene_) {
    outScene_->onExit();
    outScene_->onDetachFromScene();
  }

  // 调用进入场景的 onEnterTransitionDidFinish
  if (inScene_) {
    inScene_->onEnterTransitionDidFinish();
  }

  // 调用基类的 onExit
  Scene::onExit();
}

void TransitionScene::finish() {
  if (isFinished_) {
    return;
  }

  isFinished_ = true;

  E2D_LOG_DEBUG("TransitionScene::finish - 过渡完成，切换到目标场景");

  // 调用完成回调，通知 SceneManager 进行场景切换
  if (finishCallback_) {
    finishCallback_();
  }
}

void TransitionScene::renderContent(RenderBackend &renderer) {
  // 在 TransitionScene 上渲染新旧两个子场景
  // 子类可以重写此方法来控制渲染顺序和效果

  // 默认先渲染退出场景，再渲染进入场景（在上方）
  drawOutScene(renderer);
  drawInScene(renderer);
}

void TransitionScene::drawOutScene(RenderBackend &renderer) {
  if (outScene_) {
    outScene_->renderContent(renderer);
  }
}

void TransitionScene::drawInScene(RenderBackend &renderer) {
  if (inScene_) {
    inScene_->renderContent(renderer);
  }
}

} // namespace extra2d
