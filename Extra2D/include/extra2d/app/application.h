#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/platform/window.h>

namespace extra2d {

class Input;
class SceneManager;
class TimerManager;
class EventQueue;
class EventDispatcher;
class Camera;
class ViewportAdapter;

enum class PlatformType { Auto = 0, PC, Switch };

struct AppConfig {
  std::string title = "Easy2D Application";
  int width = 800;
  int height = 600;
  bool fullscreen = false;
  bool resizable = true;
  bool vsync = true;
  int fpsLimit = 0;
  BackendType renderBackend = BackendType::OpenGL;
  int msaaSamples = 0;
  PlatformType platform = PlatformType::Auto;
  bool enableCursors = true;
  bool enableDpiScale = false;
};

/**
 * @brief Application 单例 - 应用主控
 */
class Application {
public:
  static Application &get();

  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  bool init(const AppConfig &config);
  void shutdown();
  void run();
  void quit();

  void pause();
  void resume();
  bool isPaused() const { return paused_; }
  bool isRunning() const { return running_; }

  Window &window() { return *window_; }
  RenderBackend &renderer() { return *renderer_; }
  Input &input();
  SceneManager &scenes();
  TimerManager &timers();
  EventQueue &eventQueue();
  EventDispatcher &eventDispatcher();
  Camera &camera();
  ViewportAdapter &viewportAdapter();

  void enterScene(Ptr<class Scene> scene);

  float deltaTime() const { return deltaTime_; }
  float totalTime() const { return totalTime_; }
  int fps() const { return currentFps_; }

  const AppConfig &getConfig() const { return config_; }

private:
  Application() = default;
  ~Application();

  void mainLoop();
  void update();
  void render();

  AppConfig config_;

  UniquePtr<Window> window_;
  UniquePtr<RenderBackend> renderer_;
  UniquePtr<SceneManager> sceneManager_;
  UniquePtr<TimerManager> timerManager_;
  UniquePtr<EventQueue> eventQueue_;
  UniquePtr<EventDispatcher> eventDispatcher_;
  UniquePtr<Camera> camera_;
  UniquePtr<ViewportAdapter> viewportAdapter_;

  bool initialized_ = false;
  bool running_ = false;
  bool paused_ = false;
  bool shouldQuit_ = false;

  float deltaTime_ = 0.0f;
  float totalTime_ = 0.0f;
  double lastFrameTime_ = 0.0;
  int frameCount_ = 0;
  float fpsTimer_ = 0.0f;
  int currentFps_ = 0;
};

} // namespace extra2d
