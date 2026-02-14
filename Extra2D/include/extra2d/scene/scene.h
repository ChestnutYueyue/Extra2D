#pragma once

#include <extra2d/core/color.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/scene/node.h>
#include <vector>

namespace extra2d {

// 前向声明
struct RenderCommand;

// ============================================================================
// 场景类 - 节点容器，管理整个场景图
// ============================================================================
class Scene : public Node {
public:
  Scene();
  ~Scene() override = default;

  // ------------------------------------------------------------------------
  // 场景属性
  // ------------------------------------------------------------------------
  void setBackgroundColor(const Color &color) { backgroundColor_ = color; }
  Color getBackgroundColor() const { return backgroundColor_; }

  // ------------------------------------------------------------------------
  // 摄像机
  // ------------------------------------------------------------------------
  void setCamera(Ptr<Camera> camera);
  Ptr<Camera> getCamera() const { return camera_; }

  Camera *getActiveCamera() const {
    return camera_ ? camera_.get() : defaultCamera_.get();
  }

  // ------------------------------------------------------------------------
  // 视口和尺寸
  // ------------------------------------------------------------------------
  void setViewportSize(float width, float height);
  void setViewportSize(const Size &size);
  Size getViewportSize() const { return viewportSize_; }

  float getWidth() const { return viewportSize_.width; }
  float getHeight() const { return viewportSize_.height; }

  // ------------------------------------------------------------------------
  // 场景状态
  // ------------------------------------------------------------------------
  bool isPaused() const { return paused_; }
  void pause() { paused_ = true; }
  void resume() { paused_ = false; }

  // ------------------------------------------------------------------------
  // 渲染和更新
  // ------------------------------------------------------------------------
  void renderScene(RenderBackend &renderer);
  virtual void renderContent(RenderBackend &renderer);
  void updateScene(float dt);
  void collectRenderCommands(std::vector<RenderCommand> &commands,
                            int parentZOrder = 0) override;

  // ------------------------------------------------------------------------
  // 静态创建方法
  // ------------------------------------------------------------------------
  static Ptr<Scene> create();

protected:
  void onEnter() override;
  void onExit() override;

  // 过渡场景生命周期回调（供 TransitionScene 使用）
  virtual void onExitTransitionDidStart() {}
  virtual void onEnterTransitionDidFinish() {}

  friend class SceneManager;
  friend class TransitionScene;

private:
  Color backgroundColor_ = Colors::Black;
  Size viewportSize_ = Size::Zero();

  Ptr<Camera> camera_;
  Ptr<Camera> defaultCamera_;

  bool paused_ = false;
};

} // namespace extra2d
