#include <extra2d/app/application.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/input.h>
#include <extra2d/platform/window.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/utils/logger.h>
#include <extra2d/utils/timer.h>

#include <chrono>
#include <thread>

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace extra2d {

/**
 * @brief 获取当前时间（秒）
 */
static double getTimeSeconds() {
#ifdef __SWITCH__
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<double>(ts.tv_sec) +
         static_cast<double>(ts.tv_nsec) / 1000000000.0;
#else
  using namespace std::chrono;
  auto now = steady_clock::now();
  auto duration = now.time_since_epoch();
  return duration_cast<std::chrono::duration<double>>(duration).count();
#endif
}

Application &Application::instance() {
  static Application instance;
  return instance;
}

Application::~Application() { shutdown(); }

bool Application::init(const AppConfig &config) {
  if (initialized_) {
    E2D_LOG_WARN("Application already initialized");
    return true;
  }

  config_ = config;

  PlatformType platform = config_.platform;
  if (platform == PlatformType::Auto) {
#ifdef __SWITCH__
    platform = PlatformType::Switch;
#else
    platform = PlatformType::PC;
#endif
  }

  if (platform == PlatformType::Switch) {
#ifdef __SWITCH__
    Result rc;
    rc = romfsInit();
    if (R_SUCCEEDED(rc)) {
      E2D_LOG_INFO("RomFS initialized successfully");
    } else {
      E2D_LOG_WARN("romfsInit failed: {:#08X}, will use regular filesystem",
                   rc);
    }

    rc = socketInitializeDefault();
    if (R_FAILED(rc)) {
      E2D_LOG_WARN(
          "socketInitializeDefault failed, nxlink will not be available");
    }
#endif
  }

  window_ = makeUnique<Window>();
  WindowConfig winConfig;
  winConfig.title = config.title;
  winConfig.width = config.width;
  winConfig.height = config.height;
  if (platform == PlatformType::Switch) {
    winConfig.fullscreen = true;
    winConfig.fullscreenDesktop = false;
    winConfig.resizable = false;
    winConfig.enableCursors = false;
    winConfig.enableDpiScale = false;
  } else {
    winConfig.fullscreen = config.fullscreen;
    winConfig.resizable = config.resizable;
    winConfig.enableCursors = config.enableCursors;
    winConfig.enableDpiScale = config.enableDpiScale;
  }
  winConfig.vsync = config.vsync;
  winConfig.msaaSamples = config.msaaSamples;

  if (!window_->create(winConfig)) {
    E2D_LOG_ERROR("Failed to create window");
    return false;
  }

  renderer_ = RenderBackend::create(config.renderBackend);
  if (!renderer_ || !renderer_->init(window_.get())) {
    E2D_LOG_ERROR("Failed to initialize renderer");
    window_->destroy();
    return false;
  }

  sceneManager_ = makeUnique<SceneManager>();
  timerManager_ = makeUnique<TimerManager>();
  eventQueue_ = makeUnique<EventQueue>();
  eventDispatcher_ = makeUnique<EventDispatcher>();
  camera_ = makeUnique<Camera>(0, static_cast<float>(window_->getWidth()),
                               static_cast<float>(window_->getHeight()), 0);

  viewportAdapter_ = makeUnique<ViewportAdapter>();
  ViewportConfig vpConfig;
  vpConfig.logicWidth = static_cast<float>(config.width);
  vpConfig.logicHeight = static_cast<float>(config.height);
  vpConfig.mode = ViewportMode::AspectRatio;
  viewportAdapter_->setConfig(vpConfig);

  camera_->setViewportAdapter(viewportAdapter_.get());
  input().setViewportAdapter(viewportAdapter_.get());

  viewportAdapter_->update(window_->getWidth(), window_->getHeight());

  window_->setResizeCallback([this](int width, int height) {
    if (viewportAdapter_) {
      viewportAdapter_->update(width, height);
    }

    if (camera_) {
      camera_->applyViewportAdapter();
    }

    if (sceneManager_) {
      auto currentScene = sceneManager_->getCurrentScene();
      if (currentScene) {
        currentScene->setViewportSize(static_cast<float>(width),
                                      static_cast<float>(height));
      }
    }
  });

  initialized_ = true;
  running_ = true;

  E2D_LOG_INFO("Application initialized successfully");
  return true;
}

void Application::shutdown() {
  if (!initialized_)
    return;

  E2D_LOG_INFO("Shutting down application...");

  VRAMManager::getInstance().printStats();

  if (sceneManager_) {
    sceneManager_->end();
  }

  sceneManager_.reset();
  viewportAdapter_.reset();
  camera_.reset();

  timerManager_.reset();
  eventQueue_.reset();
  eventDispatcher_.reset();

  if (renderer_) {
    renderer_->shutdown();
    renderer_.reset();
  }

  if (window_) {
    window_->destroy();
    window_.reset();
  }

  PlatformType platform = config_.platform;
  if (platform == PlatformType::Auto) {
#ifdef __SWITCH__
    platform = PlatformType::Switch;
#else
    platform = PlatformType::PC;
#endif
  }
  if (platform == PlatformType::Switch) {
#ifdef __SWITCH__
    romfsExit();
    socketExit();
#endif
  }

  initialized_ = false;
  running_ = false;

  E2D_LOG_INFO("Application shutdown complete");
}

void Application::run() {
  if (!initialized_) {
    E2D_LOG_ERROR("Application not initialized");
    return;
  }

  lastFrameTime_ = getTimeSeconds();

#ifdef __SWITCH__
  while (running_ && !window_->shouldClose()) {
    mainLoop();
  }
#else
  while (running_ && !window_->shouldClose()) {
    mainLoop();
  }
#endif
}

void Application::quit() {
  shouldQuit_ = true;
  running_ = false;
}

void Application::pause() {
  if (!paused_) {
    paused_ = true;
    E2D_LOG_INFO("Application paused");
  }
}

void Application::resume() {
  if (paused_) {
    paused_ = false;
    lastFrameTime_ = getTimeSeconds();
    E2D_LOG_INFO("Application resumed");
  }
}

void Application::mainLoop() {
  double currentTime = getTimeSeconds();
  deltaTime_ = static_cast<float>(currentTime - lastFrameTime_);
  lastFrameTime_ = currentTime;

  totalTime_ += deltaTime_;

  frameCount_++;
  fpsTimer_ += deltaTime_;
  if (fpsTimer_ >= 1.0f) {
    currentFps_ = frameCount_;
    frameCount_ = 0;
    fpsTimer_ -= 1.0f;
  }

  window_->pollEvents();

  if (eventDispatcher_ && eventQueue_) {
    eventDispatcher_->processQueue(*eventQueue_);
  }

  if (!paused_) {
    update();
  }

  render();

  if (!config_.vsync && config_.fpsLimit > 0) {
    double frameEndTime = getTimeSeconds();
    double frameTime = frameEndTime - currentTime;
    double target = 1.0 / static_cast<double>(config_.fpsLimit);
    if (frameTime < target) {
      auto sleepSeconds = target - frameTime;
      std::this_thread::sleep_for(std::chrono::duration<double>(sleepSeconds));
    }
  }
}

void Application::update() {
  if (timerManager_) {
    timerManager_->update(deltaTime_);
  }

  if (sceneManager_) {
    sceneManager_->update(deltaTime_);
  }
}

void Application::render() {
  if (!renderer_) {
    E2D_LOG_ERROR("Render failed: renderer is null");
    return;
  }

  if (viewportAdapter_) {
    const auto &vp = viewportAdapter_->getViewport();
    renderer_->setViewport(
        static_cast<int>(vp.origin.x), static_cast<int>(vp.origin.y),
        static_cast<int>(vp.size.width), static_cast<int>(vp.size.height));
  } else {
    renderer_->setViewport(0, 0, window_->getWidth(), window_->getHeight());
  }

  if (sceneManager_) {
    sceneManager_->render(*renderer_);
  } else {
    E2D_LOG_WARN("Render: sceneManager is null");
  }

  window_->swapBuffers();
}

Input &Application::input() { return *window_->getInput(); }

SceneManager &Application::scenes() { return *sceneManager_; }

TimerManager &Application::timers() { return *timerManager_; }

EventQueue &Application::eventQueue() { return *eventQueue_; }

EventDispatcher &Application::eventDispatcher() { return *eventDispatcher_; }

Camera &Application::camera() { return *camera_; }

ViewportAdapter &Application::viewportAdapter() { return *viewportAdapter_; }

void Application::enterScene(Ptr<Scene> scene) {
  if (sceneManager_ && scene) {
    scene->setViewportSize(static_cast<float>(window_->getWidth()),
                           static_cast<float>(window_->getHeight()));
    sceneManager_->enterScene(scene);
  }
}

} // namespace extra2d
