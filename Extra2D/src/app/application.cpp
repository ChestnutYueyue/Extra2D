#include <extra2d/app/application.h>
#include <extra2d/audio/audio_engine.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/input.h>
#include <extra2d/platform/window.h>
#include <extra2d/resource/resource_manager.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/utils/logger.h>
#include <extra2d/utils/object_pool.h>
#include <extra2d/utils/timer.h>


#include <chrono>
#include <thread>

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace extra2d {

// 获取当前时间（秒）
static double getTimeSeconds() {
#ifdef __SWITCH__
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<double>(ts.tv_sec) +
         static_cast<double>(ts.tv_nsec) / 1000000000.0;
#else
  // PC 平台使用 chrono
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

  // 确定平台类型
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
    // ========================================
    // 1. 初始化 RomFS 文件系统（Switch 平台）
    // ========================================
    Result rc;
    rc = romfsInit();
    if (R_SUCCEEDED(rc)) {
      E2D_LOG_INFO("RomFS initialized successfully");
    } else {
      E2D_LOG_WARN("romfsInit failed: {:#08X}, will use regular filesystem",
                   rc);
    }

    // ========================================
    // 2. 初始化 nxlink 调试输出（Switch 平台）
    // ========================================
    rc = socketInitializeDefault();
    if (R_FAILED(rc)) {
      E2D_LOG_WARN(
          "socketInitializeDefault failed, nxlink will not be available");
    }
#endif
  }

  // ========================================
  // 3. 创建窗口（包含 SDL_Init + GLES 3.2 上下文创建）
  // ========================================
  window_ = makeUnique<Window>();
  WindowConfig winConfig;
  winConfig.title = config.title;
  winConfig.width = config.width;
  winConfig.height = config.height;
  if (platform == PlatformType::Switch) {
    winConfig.fullscreen = true;
    winConfig.fullscreenDesktop = false; // Switch 使用固定分辨率全屏
    winConfig.resizable = false;
    winConfig.enableCursors = false;
    winConfig.enableDpiScale = false;
  } else {
    // PC 平台默认窗口模式
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

  // ========================================
  // 4. 初始化渲染器
  // ========================================
  renderer_ = RenderBackend::create(config.renderBackend);
  if (!renderer_ || !renderer_->init(window_.get())) {
    E2D_LOG_ERROR("Failed to initialize renderer");
    window_->destroy();
    return false;
  }

  // ========================================
  // 5. 初始化其他子系统
  // ========================================
  sceneManager_ = makeUnique<SceneManager>();
  resourceManager_ = makeUnique<ResourceManager>();
  timerManager_ = makeUnique<TimerManager>();
  eventQueue_ = makeUnique<EventQueue>();
  eventDispatcher_ = makeUnique<EventDispatcher>();
  camera_ = makeUnique<Camera>(0, static_cast<float>(window_->getWidth()),
                               static_cast<float>(window_->getHeight()), 0);

  // 创建视口适配器
  viewportAdapter_ = makeUnique<ViewportAdapter>();
  ViewportConfig vpConfig;
  vpConfig.logicWidth = static_cast<float>(config.width);
  vpConfig.logicHeight = static_cast<float>(config.height);
  vpConfig.mode = ViewportMode::AspectRatio;
  viewportAdapter_->setConfig(vpConfig);

  // 关联到各子系统
  camera_->setViewportAdapter(viewportAdapter_.get());
  input().setViewportAdapter(viewportAdapter_.get());

  // 初始更新
  viewportAdapter_->update(window_->getWidth(), window_->getHeight());

  // 窗口大小回调
  window_->setResizeCallback([this](int width, int height) {
    // 更新视口适配器
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

  // 初始化音频引擎
  AudioEngine::getInstance().initialize();

  // ========================================
  // 6. 预热对象池（自动管理）
  // ========================================
  prewarmObjectPools();

  initialized_ = true;
  running_ = true;

  E2D_LOG_INFO("Application initialized successfully");
  return true;
}

void Application::prewarmObjectPools() {
  E2D_LOG_INFO("Prewarming object pools...");

  auto &poolManager = ObjectPoolManager::getInstance();

  // 预热常用类型的对象池
  // 这些池会在首次使用时自动预热，但提前预热可以避免运行时延迟

  E2D_LOG_INFO("Object pools prewarmed successfully");
}

void Application::shutdown() {
  if (!initialized_)
    return;

  E2D_LOG_INFO("Shutting down application...");

  // 打印 VRAM 统计
  VRAMManager::getInstance().printStats();

  // 打印对象池内存统计
  E2D_LOG_INFO("Object pool memory usage: {} bytes (auto-managed)",
               ObjectPoolManager::getInstance().getPool<Node>()->memoryUsage());

  // 先结束所有场景，确保 onExit() 被正确调用
  if (sceneManager_) {
    sceneManager_->end();
  }

  // ========================================
  // 1. 先清理所有持有 GPU 资源的子系统
  // 必须在渲染器关闭前释放纹理等资源
  // ========================================
  sceneManager_.reset();    // 场景持有纹理引用
  resourceManager_.reset(); // 纹理缓存持有 GPU 纹理
  viewportAdapter_.reset(); // 视口适配器
  camera_.reset();          // 相机可能持有渲染目标

  // ========================================
  // 2. 关闭音频（不依赖 GPU）
  // ========================================
  AudioEngine::getInstance().shutdown();

  // ========================================
  // 3. 清理其他子系统
  // ========================================
  timerManager_.reset();
  eventQueue_.reset();
  eventDispatcher_.reset();

  // ========================================
  // 4. 最后关闭渲染器和窗口
  // 必须在所有 GPU 资源释放后才能关闭 OpenGL 上下文
  // ========================================
  if (renderer_) {
    renderer_->shutdown();
    renderer_.reset();
  }

  // 销毁窗口（包含 SDL_Quit，会销毁 OpenGL 上下文）
  if (window_) {
    window_->destroy();
    window_.reset();
  }

  // Switch 平台清理
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
  // SDL2 on Switch 内部已处理 appletMainLoop
  while (running_ && !window_->shouldClose()) {
    mainLoop();
  }
#else
  // PC 平台主循环
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
  // 计算 delta time
  double currentTime = getTimeSeconds();
  deltaTime_ = static_cast<float>(currentTime - lastFrameTime_);
  lastFrameTime_ = currentTime;

  totalTime_ += deltaTime_;

  // 计算 FPS
  frameCount_++;
  fpsTimer_ += deltaTime_;
  if (fpsTimer_ >= 1.0f) {
    currentFps_ = frameCount_;
    frameCount_ = 0;
    fpsTimer_ -= 1.0f;
  }

  // 处理窗口事件（SDL_PollEvent + 输入更新）
  window_->pollEvents();

  // 处理事件队列
  if (eventDispatcher_ && eventQueue_) {
    eventDispatcher_->processQueue(*eventQueue_);
  }

  // 更新
  if (!paused_) {
    update();
  }

  // 渲染
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

  // 应用视口适配器
  if (viewportAdapter_) {
    const auto &vp = viewportAdapter_->getViewport();
    renderer_->setViewport(static_cast<int>(vp.origin.x),
                           static_cast<int>(vp.origin.y),
                           static_cast<int>(vp.size.width),
                           static_cast<int>(vp.size.height));
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

AudioEngine &Application::audio() { return AudioEngine::getInstance(); }

SceneManager &Application::scenes() { return *sceneManager_; }

ResourceManager &Application::resources() { return *resourceManager_; }

TimerManager &Application::timers() { return *timerManager_; }

EventQueue &Application::eventQueue() { return *eventQueue_; }

EventDispatcher &Application::eventDispatcher() { return *eventDispatcher_; }

Camera &Application::camera() { return *camera_; }

ViewportAdapter &Application::viewportAdapter() { return *viewportAdapter_; }

void Application::enterScene(Ptr<Scene> scene) { enterScene(scene, nullptr); }

void Application::enterScene(Ptr<Scene> scene,
                             Ptr<class TransitionScene> transitionScene) {
  if (sceneManager_ && scene) {
    scene->setViewportSize(static_cast<float>(window_->getWidth()),
                           static_cast<float>(window_->getHeight()));
    sceneManager_->enterScene(scene, transitionScene);
  }
}

} // namespace extra2d
