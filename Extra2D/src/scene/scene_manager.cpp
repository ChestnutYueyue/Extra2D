#include <algorithm>
#include <extra2d/app/application.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/platform/iinput.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

namespace {

/**
 * @brief 命中测试 - 从节点树中找到最上层的可交互节点
 * @param node 要测试的节点
 * @param worldPos 世界坐标位置
 * @return 命中的节点指针，未命中返回nullptr
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

  Rect bounds = node->getBounds();
  if (!bounds.empty() && bounds.containsPoint(worldPos)) {
    return node.get();
  }

  return nullptr;
}

/**
 * @brief 向节点分发事件
 * @param node 目标节点
 * @param event 要分发的事件
 */
void dispatchToNode(Node *node, Event &event) {
  if (!node) {
    return;
  }
  node->getEventDispatcher().dispatch(event);
}

} // namespace

/**
 * @brief 获取场景管理器单例
 * @return 场景管理器的全局唯一实例引用
 */
SceneManager &SceneManager::get() {
  static SceneManager instance;
  return instance;
}

/**
 * @brief 运行指定场景
 * @param scene 要运行的场景智能指针
 *
 * 此方法应在应用启动时调用一次，设置初始场景
 */
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

/**
 * @brief 替换当前场景
 * @param scene 新场景智能指针
 *
 * 移除当前场景并替换为新场景，场景栈大小保持不变
 */
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

/**
 * @brief 进入场景
 * @param scene 要进入的场景智能指针
 *
 * 如果场景栈为空则运行场景，否则替换当前场景
 */
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

/**
 * @brief 压入场景到栈顶
 * @param scene 要压入的场景智能指针
 *
 * 将新场景压入栈顶，暂停当前场景
 */
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

/**
 * @brief 弹出当前场景
 *
 * 移除栈顶场景并恢复上一个场景
 */
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

/**
 * @brief 弹出到根场景
 *
 * 移除所有场景直到只剩根场景
 */
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

/**
 * @brief 弹出到指定名称的场景
 * @param name 目标场景的名称
 *
 * 移除栈顶场景直到找到指定名称的场景
 */
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

/**
 * @brief 获取当前场景
 * @return 当前栈顶场景的智能指针，栈为空时返回nullptr
 */
Ptr<Scene> SceneManager::getCurrentScene() const {
  if (sceneStack_.empty()) {
    return nullptr;
  }
  return sceneStack_.top();
}

/**
 * @brief 获取前一个场景
 * @return 栈顶下一个场景的智能指针，不存在时返回nullptr
 */
Ptr<Scene> SceneManager::getPreviousScene() const {
  if (sceneStack_.size() < 2) {
    return nullptr;
  }

  auto tempStack = sceneStack_;
  tempStack.pop();
  return tempStack.top();
}

/**
 * @brief 获取根场景
 * @return 栈底场景的智能指针，栈为空时返回nullptr
 */
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

/**
 * @brief 通过名称获取场景
 * @param name 场景名称
 * @return 找到的场景智能指针，未找到返回nullptr
 */
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

/**
 * @brief 检查是否存在指定名称的场景
 * @param name 场景名称
 * @return 存在返回true，否则返回false
 */
bool SceneManager::hasScene(const std::string &name) const {
  return getSceneByName(name) != nullptr;
}

/**
 * @brief 更新场景管理器
 * @param dt 帧间隔时间（秒）
 *
 * 更新当前场景并分发指针事件
 */
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

/**
 * @brief 渲染当前场景
 * @param renderer 渲染后端引用
 *
 * 使用当前场景的背景色清除帧缓冲并渲染场景内容
 */
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

/**
 * @brief 收集渲染命令
 * @param commands 渲染命令输出向量
 *
 * 从当前场景收集所有渲染命令
 */
void SceneManager::collectRenderCommands(std::vector<RenderCommand> &commands) {
  if (!sceneStack_.empty()) {
    sceneStack_.top()->collectRenderCommands(commands, 0);
  }
}

/**
 * @brief 结束场景管理器
 *
 * 清空场景栈并触发所有场景的退出回调
 */
void SceneManager::end() {
  while (!sceneStack_.empty()) {
    auto scene = sceneStack_.top();
    scene->onExit();
    scene->onDetachFromScene();
    sceneStack_.pop();
  }
  namedScenes_.clear();
}

/**
 * @brief 清除缓存的场景
 *
 * 清除命名场景缓存
 */
void SceneManager::purgeCachedScenes() { namedScenes_.clear(); }

/**
 * @brief 分发指针事件
 * @param scene 目标场景
 *
 * 处理鼠标悬停、移动、点击和滚轮事件
 */
void SceneManager::dispatchPointerEvents(Scene &scene) {
  auto &input = Application::get().input();
  Vec2 screenPos = input.mouse();

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

  float scrollDelta = input.scrollDelta();
  if (hoverTarget_ && scrollDelta != 0.0f) {
    Event evt = Event::createMouseScroll(Vec2(0.0f, scrollDelta), worldPos);
    dispatchToNode(hoverTarget_, evt);
  }

  if (input.pressed(Mouse::Left)) {
    captureTarget_ = hoverTarget_;
    if (captureTarget_) {
      Event evt = Event::createMouseButtonPress(static_cast<int>(Mouse::Left),
                                                0, worldPos);
      dispatchToNode(captureTarget_, evt);

      Event pressed;
      pressed.type = EventType::UIPressed;
      pressed.data = CustomEvent{0, captureTarget_};
      dispatchToNode(captureTarget_, pressed);
    }
  }

  if (input.released(Mouse::Left)) {
    Node *target = captureTarget_ ? captureTarget_ : hoverTarget_;
    if (target) {
      Event evt = Event::createMouseButtonRelease(static_cast<int>(Mouse::Left),
                                                  0, worldPos);
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
