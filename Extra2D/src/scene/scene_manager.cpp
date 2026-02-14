#include <algorithm>
#include <extra2d/app/application.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/platform/input.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

namespace {

/**
 * @brief 命中测试 - 从节点树中找到最上层的可交互节点
 */
Node *hitTestTopmost(const Ptr<Node> &node, const Vec2 &worldPos) {
  if (!node || !node->isVisible()) {
    return nullptr;
  }

  std::vector<Ptr<Node>> children = node->getChildren();
  std::stable_sort(children.begin(), children.end(),
                   [](const Ptr<Node> &a, const Ptr<Node> &b) {
                     return a->getZOrder() < b->getZOrder();
                   });

  for (auto it = children.rbegin(); it != children.rend(); ++it) {
    if (Node *hit = hitTestTopmost(*it, worldPos)) {
      return hit;
    }
  }

  if (node->getEventDispatcher().getTotalListenerCount() == 0) {
    return nullptr;
  }

  Rect bounds = node->getBoundingBox();
  if (!bounds.empty() && bounds.containsPoint(worldPos)) {
    return node.get();
  }

  return nullptr;
}

/**
 * @brief 向节点分发事件
 */
void dispatchToNode(Node *node, Event &event) {
  if (!node) {
    return;
  }
  node->getEventDispatcher().dispatch(event);
}

} // namespace

SceneManager &SceneManager::getInstance() {
  static SceneManager instance;
  return instance;
}

void SceneManager::runWithScene(Ptr<Scene> scene) {
  if (!scene) {
    return;
  }

  if (!sceneStack_.empty()) {
    E2D_LOG_WARN("SceneManager: runWithScene should only be called once");
    return;
  }

  scene->onEnter();
  scene->onAttachToScene(scene.get());
  sceneStack_.push(scene);
}

void SceneManager::replaceScene(Ptr<Scene> scene) {
  if (!scene || isTransitioning_) {
    return;
  }

  if (sceneStack_.empty()) {
    runWithScene(scene);
    return;
  }

  auto oldScene = sceneStack_.top();
  oldScene->onExit();
  oldScene->onDetachFromScene();
  sceneStack_.pop();

  scene->onEnter();
  scene->onAttachToScene(scene.get());
  sceneStack_.push(scene);
}

void SceneManager::enterScene(Ptr<Scene> scene) {
  if (!scene || isTransitioning_) {
    return;
  }

  if (sceneStack_.empty()) {
    runWithScene(scene);
  } else {
    replaceScene(scene);
  }
}

void SceneManager::pushScene(Ptr<Scene> scene) {
  if (!scene || isTransitioning_) {
    return;
  }

  if (!sceneStack_.empty()) {
    sceneStack_.top()->pause();
  }

  scene->onEnter();
  scene->onAttachToScene(scene.get());
  sceneStack_.push(scene);
}

void SceneManager::popScene() {
  if (sceneStack_.size() <= 1 || isTransitioning_) {
    return;
  }

  auto current = sceneStack_.top();
  current->onExit();
  current->onDetachFromScene();
  sceneStack_.pop();

  if (!sceneStack_.empty()) {
    sceneStack_.top()->resume();
  }
}

void SceneManager::popToRootScene() {
  if (sceneStack_.size() <= 1 || isTransitioning_) {
    return;
  }

  while (sceneStack_.size() > 1) {
    auto scene = sceneStack_.top();
    scene->onExit();
    scene->onDetachFromScene();
    sceneStack_.pop();
  }

  sceneStack_.top()->resume();
}

void SceneManager::popToScene(const std::string &name) {
  if (isTransitioning_) {
    return;
  }

  std::stack<Ptr<Scene>> tempStack;
  Ptr<Scene> target = nullptr;

  while (!sceneStack_.empty()) {
    auto scene = sceneStack_.top();
    if (scene->getName() == name) {
      target = scene;
      break;
    }
    scene->onExit();
    scene->onDetachFromScene();
    sceneStack_.pop();
  }

  if (target) {
    target->resume();
  }
}

Ptr<Scene> SceneManager::getCurrentScene() const {
  if (sceneStack_.empty()) {
    return nullptr;
  }
  return sceneStack_.top();
}

Ptr<Scene> SceneManager::getPreviousScene() const {
  if (sceneStack_.size() < 2) {
    return nullptr;
  }

  auto tempStack = sceneStack_;
  tempStack.pop();
  return tempStack.top();
}

Ptr<Scene> SceneManager::getRootScene() const {
  if (sceneStack_.empty()) {
    return nullptr;
  }

  auto tempStack = sceneStack_;
  Ptr<Scene> root;
  while (!tempStack.empty()) {
    root = tempStack.top();
    tempStack.pop();
  }
  return root;
}

Ptr<Scene> SceneManager::getSceneByName(const std::string &name) const {
  auto it = namedScenes_.find(name);
  if (it != namedScenes_.end()) {
    return it->second;
  }

  auto tempStack = sceneStack_;
  while (!tempStack.empty()) {
    auto scene = tempStack.top();
    if (scene->getName() == name) {
      return scene;
    }
    tempStack.pop();
  }

  return nullptr;
}

bool SceneManager::hasScene(const std::string &name) const {
  return getSceneByName(name) != nullptr;
}

void SceneManager::update(float dt) {
  if (isTransitioning_) {
    hoverTarget_ = nullptr;
    captureTarget_ = nullptr;
    hasLastPointerWorld_ = false;
  }

  if (!sceneStack_.empty()) {
    auto &scene = *sceneStack_.top();
    scene.updateScene(dt);
    if (!isTransitioning_) {
      dispatchPointerEvents(scene);
    }
  }
}

void SceneManager::render(RenderBackend &renderer) {
  Color clearColor = Colors::Black;
  if (!sceneStack_.empty()) {
    clearColor = sceneStack_.top()->getBackgroundColor();
  }

  E2D_LOG_TRACE("SceneManager::render - beginFrame with color({}, {}, {})",
                clearColor.r, clearColor.g, clearColor.b);
  renderer.beginFrame(clearColor);

  if (!sceneStack_.empty()) {
    E2D_LOG_TRACE("SceneManager::render - rendering scene content");
    sceneStack_.top()->renderContent(renderer);
  } else {
    E2D_LOG_WARN("SceneManager::render - no scene to render");
  }

  renderer.endFrame();
  E2D_LOG_TRACE("SceneManager::render - endFrame");
}

void SceneManager::collectRenderCommands(std::vector<RenderCommand> &commands) {
  if (!sceneStack_.empty()) {
    sceneStack_.top()->collectRenderCommands(commands, 0);
  }
}

void SceneManager::end() {
  while (!sceneStack_.empty()) {
    auto scene = sceneStack_.top();
    scene->onExit();
    scene->onDetachFromScene();
    sceneStack_.pop();
  }
  namedScenes_.clear();
}

void SceneManager::purgeCachedScenes() { namedScenes_.clear(); }

void SceneManager::dispatchPointerEvents(Scene &scene) {
  auto &input = Application::instance().input();
  Vec2 screenPos = input.getMousePosition();

  Vec2 worldPos = screenPos;
  if (auto *camera = scene.getActiveCamera()) {
    worldPos = camera->screenToWorld(screenPos);
  }

  Ptr<Node> root = scene.shared_from_this();
  Node *newHover = hitTestTopmost(root, worldPos);

  if (newHover != hoverTarget_) {
    if (hoverTarget_) {
      Event evt;
      evt.type = EventType::UIHoverExit;
      evt.data = CustomEvent{0, hoverTarget_};
      dispatchToNode(hoverTarget_, evt);
    }
    hoverTarget_ = newHover;
    if (hoverTarget_) {
      Event evt;
      evt.type = EventType::UIHoverEnter;
      evt.data = CustomEvent{0, hoverTarget_};
      dispatchToNode(hoverTarget_, evt);
    }
  }

  if (!hasLastPointerWorld_) {
    lastPointerWorld_ = worldPos;
    hasLastPointerWorld_ = true;
  }

  Vec2 delta = worldPos - lastPointerWorld_;
  if (hoverTarget_ && (delta.x != 0.0f || delta.y != 0.0f)) {
    Event evt = Event::createMouseMove(worldPos, delta);
    dispatchToNode(hoverTarget_, evt);
  }

  float scrollDelta = input.getMouseScrollDelta();
  if (hoverTarget_ && scrollDelta != 0.0f) {
    Event evt = Event::createMouseScroll(Vec2(0.0f, scrollDelta), worldPos);
    dispatchToNode(hoverTarget_, evt);
  }

  if (input.isMousePressed(MouseButton::Left)) {
    captureTarget_ = hoverTarget_;
    if (captureTarget_) {
      Event evt = Event::createMouseButtonPress(
          static_cast<int>(MouseButton::Left), 0, worldPos);
      dispatchToNode(captureTarget_, evt);

      Event pressed;
      pressed.type = EventType::UIPressed;
      pressed.data = CustomEvent{0, captureTarget_};
      dispatchToNode(captureTarget_, pressed);
    }
  }

  if (input.isMouseReleased(MouseButton::Left)) {
    Node *target = captureTarget_ ? captureTarget_ : hoverTarget_;
    if (target) {
      Event evt = Event::createMouseButtonRelease(
          static_cast<int>(MouseButton::Left), 0, worldPos);
      dispatchToNode(target, evt);

      Event released;
      released.type = EventType::UIReleased;
      released.data = CustomEvent{0, target};
      dispatchToNode(target, released);
    }

    if (captureTarget_ && captureTarget_ == hoverTarget_) {
      Event clicked;
      clicked.type = EventType::UIClicked;
      clicked.data = CustomEvent{0, captureTarget_};
      dispatchToNode(captureTarget_, clicked);
    }

    captureTarget_ = nullptr;
  }

  lastPointerWorld_ = worldPos;
}

} // namespace extra2d
