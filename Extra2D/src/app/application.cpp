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
 * @return 从某个固定时间点开始的秒数
 *
 * 使用高精度时钟获取当前时间，在Switch平台使用clock_gettime，
 * 其他平台使用std::chrono::steady_clock
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

/**
 * @brief 获取Application单例实例
 * @return Application单例的引用
 */
Application &Application::get() {
  static Application instance;
  return instance;
}

/**
 * @brief 析构函数，自动关闭应用程序
 */
Application::~Application() { shutdown(); }

/**
 * @brief 初始化应用程序
 * @param config 应用程序配置
 * @return 初始化成功返回true，失败返回false
 *
 * 初始化窗口、渲染器、场景管理器、定时器管理器、事件系统、相机和视口适配器等核心组件
 */
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

/**
 * @brief 关闭应用程序
 *
 * 释放所有资源，包括场景管理器、渲染器、窗口等，并关闭平台相关服务
 */
void Application::shutdown() {
  if (!initialized_)
    return;

  E2D_LOG_INFO("Shutting down application...");

  VRAMMgr::get().printStats();

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

/**
 * @brief 运行应用程序主循环
 *
 * 进入应用程序主循环，持续处理事件、更新和渲染直到应用程序退出
 */
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

/**
 * @brief 请求退出应用程序
 *
 * 设置退出标志，主循环将在下一次迭代时退出
 */
void Application::quit() {
  shouldQuit_ = true;
  running_ = false;
}

/**
 * @brief 暂停应用程序
 *
 * 暂停应用程序更新，渲染继续进行
 */
void Application::pause() {
  if (!paused_) {
    paused_ = true;
    E2D_LOG_INFO("Application paused");
  }
}

/**
 * @brief 恢复应用程序
 *
 * 恢复应用程序更新，重置帧时间以避免大的deltaTime跳跃
 */
void Application::resume() {
  if (paused_) {
    paused_ = false;
    lastFrameTime_ = getTimeSeconds();
    E2D_LOG_INFO("Application resumed");
  }
}

/**
 * @brief 主循环迭代
 *
 * 执行一次主循环迭代，包括计算帧时间、处理事件、更新和渲染
 */
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

/**
 * @brief 更新应用程序状态
 *
 * 更新定时器管理器和场景管理器
 */
void Application::update() {
  if (timerManager_) {
    timerManager_->update(deltaTime_);
  }

  if (sceneManager_) {
    sceneManager_->update(deltaTime_);
  }
}

/**
 * @brief 渲染应用程序画面
 *
 * 设置视口并渲染当前场景
 */
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

/**
 * @brief 获取输入管理器
 * @return 输入管理器的引用
 */
Input &Application::input() { return *window_->getInput(); }

/**
 * @brief 获取场景管理器
 * @return 场景管理器的引用
 */
SceneManager &Application::scenes() { return *sceneManager_; }

/**
 * @brief 获取定时器管理器
 * @return 定时器管理器的引用
 */
TimerManager &Application::timers() { return *timerManager_; }

/**
 * @brief 获取事件队列
 * @return 事件队列的引用
 */
EventQueue &Application::eventQueue() { return *eventQueue_; }

/**
 * @brief 获取事件分发器
 * @return 事件分发器的引用
 */
EventDispatcher &Application::eventDispatcher() { return *eventDispatcher_; }

/**
 * @brief 获取相机
 * @return 相机的引用
 */
Camera &Application::camera() { return *camera_; }

/**
 * @brief 获取视口适配器
 * @return 视口适配器的引用
 */
ViewportAdapter &Application::viewportAdapter() { return *viewportAdapter_; }

/**
 * @brief 进入指定场景
 * @param scene 要进入的场景
 *
 * 设置场景的视口大小并将其设置为当前场景
 */
void Application::enterScene(Ptr<Scene> scene) {
  if (sceneManager_ && scene) {
    scene->setViewportSize(static_cast<float>(window_->getWidth()),
                           static_cast<float>(window_->getHeight()));
    sceneManager_->enterScene(scene);
  }
}

} // namespace extra2d
