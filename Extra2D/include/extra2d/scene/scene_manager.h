#pragma once

#include <extra2d/core/types.h>
#include <extra2d/scene/scene.h>

#include <functional>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace extra2d {

struct RenderCommand;

/**
 * @brief 场景管理器 - 管理场景的生命周期和切换
 */
class SceneManager {
public:
  using TransitionCallback = std::function<void()>;

  static SceneManager &getInstance();

  void runWithScene(Ptr<Scene> scene);
  void replaceScene(Ptr<Scene> scene);
  void pushScene(Ptr<Scene> scene);
  void popScene();
  void popToRootScene();
  void popToScene(const std::string &name);

  Ptr<Scene> getCurrentScene() const;
  Ptr<Scene> getPreviousScene() const;
  Ptr<Scene> getRootScene() const;
  Ptr<Scene> getSceneByName(const std::string &name) const;

  size_t getSceneCount() const { return sceneStack_.size(); }
  bool isEmpty() const { return sceneStack_.empty(); }
  bool hasScene(const std::string &name) const;

  void update(float dt);
  void render(RenderBackend &renderer);
  void collectRenderCommands(std::vector<RenderCommand> &commands);

  bool isTransitioning() const { return isTransitioning_; }
  void setTransitionCallback(TransitionCallback callback) {
    transitionCallback_ = callback;
  }

  void end();
  void purgeCachedScenes();

public:
  SceneManager() = default;
  ~SceneManager() = default;
  SceneManager(const SceneManager &) = delete;
  SceneManager &operator=(const SceneManager &) = delete;

  void enterScene(Ptr<Scene> scene);

private:
  void doSceneSwitch();
  void dispatchPointerEvents(Scene &scene);

  std::stack<Ptr<Scene>> sceneStack_;
  std::unordered_map<std::string, Ptr<Scene>> namedScenes_;

  bool isTransitioning_ = false;
  TransitionCallback transitionCallback_;

  Ptr<Scene> nextScene_;
  bool sendCleanupToScene_ = false;

  Node *hoverTarget_ = nullptr;
  Node *captureTarget_ = nullptr;
  Vec2 lastPointerWorld_ = Vec2::Zero();
  bool hasLastPointerWorld_ = false;
};

} // namespace extra2d
