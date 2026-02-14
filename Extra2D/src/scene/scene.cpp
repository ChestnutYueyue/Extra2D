#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/scene/scene.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

Scene::Scene() { defaultCamera_ = makePtr<Camera>(); }

void Scene::setCamera(Ptr<Camera> camera) { camera_ = camera; }

void Scene::setViewportSize(float width, float height) {
  viewportSize_ = Size(width, height);
  if (defaultCamera_) {
    defaultCamera_->setViewport(0, width, height, 0);
  } else if (camera_) {
    camera_->setViewport(0, width, height, 0);
  }
}

void Scene::setViewportSize(const Size &size) {
  setViewportSize(size.width, size.height);
}

void Scene::renderScene(RenderBackend &renderer) {
  if (!isVisible())
    return;

  // Begin frame with background color
  renderer.beginFrame(backgroundColor_);
  renderContent(renderer);
  renderer.endFrame();
}

void Scene::renderContent(RenderBackend &renderer) {
  if (!isVisible())
    return;

  // 在渲染前批量更新所有节点的世界变换
  batchUpdateTransforms();

  Camera *activeCam = getActiveCamera();
  if (activeCam) {
    renderer.setViewProjection(activeCam->getViewProjectionMatrix());
  }

  renderer.beginSpriteBatch();
  render(renderer);
  renderer.endSpriteBatch();
}

void Scene::updateScene(float dt) {
  if (!paused_) {
    update(dt);
  }
}

void Scene::onEnter() {
  Node::onEnter();
}

void Scene::onExit() {
  Node::onExit();
}

void Scene::collectRenderCommands(std::vector<RenderCommand> &commands,
                                  int parentZOrder) {
  if (!isVisible())
    return;

  // 从场景的子节点开始收集渲染命令
  Node::collectRenderCommands(commands, parentZOrder);
}

Ptr<Scene> Scene::create() { return makePtr<Scene>(); }

} // namespace extra2d
