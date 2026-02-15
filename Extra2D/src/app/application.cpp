#include <extra2d/app/application.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/iinput.h>
#include <extra2d/platform/platform_module.h>
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
 * @return 当前时间戳（秒）
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

Application& Application::get() {
    static Application instance;
    return instance;
}

Application::~Application() { 
    shutdown(); 
}

bool Application::init() {
    AppConfig cfg;
    return init(cfg);
}

bool Application::init(const AppConfig& config) {
    if (initialized_) {
        E2D_LOG_WARN("Application already initialized");
        return true;
    }

    E2D_LOG_INFO("Initializing application with config...");

    if (!ConfigManager::instance().initialize()) {
        E2D_LOG_ERROR("Failed to initialize ConfigManager");
        return false;
    }

    ConfigManager::instance().setAppConfig(config);

    return initImpl();
}

bool Application::init(const std::string& configPath) {
    if (initialized_) {
        E2D_LOG_WARN("Application already initialized");
        return true;
    }

    E2D_LOG_INFO("Initializing application from config file: {}", configPath);

    if (!ConfigManager::instance().initialize(configPath)) {
        E2D_LOG_WARN("Failed to load config from file, using defaults");
        if (!ConfigManager::instance().initialize()) {
            E2D_LOG_ERROR("Failed to initialize ConfigManager");
            return false;
        }
    }

    return initImpl();
}

bool Application::initImpl() {
    auto& configMgr = ConfigManager::instance();
    AppConfig& appConfig = configMgr.appConfig();

    PlatformType platform = appConfig.targetPlatform;
    if (platform == PlatformType::Auto) {
#ifdef __SWITCH__
        platform = PlatformType::Switch;
#else
#ifdef _WIN32
        platform = PlatformType::Windows;
#elif defined(__linux__)
        platform = PlatformType::Linux;
#elif defined(__APPLE__)
        platform = PlatformType::macOS;
#else
        platform = PlatformType::Windows;
#endif
#endif
    }

    E2D_LOG_INFO("Target platform: {} ({})", getPlatformTypeName(platform), 
                 static_cast<int>(platform));

    UniquePtr<PlatformConfig> platformConfig = createPlatformConfig(platform);
    if (!platformConfig) {
        E2D_LOG_ERROR("Failed to create platform config");
        return false;
    }

    appConfig.applyPlatformConstraints(*platformConfig);

    const auto& capabilities = platformConfig->capabilities();
    E2D_LOG_INFO("Platform capabilities: windowed={}, fullscreen={}, cursor={}, DPI={}",
                 capabilities.supportsWindowed, capabilities.supportsFullscreen,
                 capabilities.supportsCursor, capabilities.supportsDPIAwareness);

    if (platform == PlatformType::Switch) {
#ifdef __SWITCH__
        Result rc;
        rc = romfsInit();
        if (R_SUCCEEDED(rc)) {
            E2D_LOG_INFO("RomFS initialized successfully");
        } else {
            E2D_LOG_WARN("romfsInit failed: {:#08X}, will use regular filesystem", rc);
        }

        rc = socketInitializeDefault();
        if (R_FAILED(rc)) {
            E2D_LOG_WARN("socketInitializeDefault failed, nxlink will not be available");
        }
#endif
    }

    auto initOrder = ModuleRegistry::instance().getInitializationOrder();
    E2D_LOG_INFO("Initializing {} registered modules...", initOrder.size());

    for (ModuleId moduleId : initOrder) {
        auto initializer = ModuleRegistry::instance().createInitializer(moduleId);
        if (!initializer) {
            continue;
        }

        auto* moduleConfig = ModuleRegistry::instance().getModuleConfig(moduleId);
        if (!moduleConfig) {
            E2D_LOG_WARN("Module {} has no config, skipping", moduleId);
            continue;
        }

        auto info = moduleConfig->getModuleInfo();
        if (!info.enabled) {
            E2D_LOG_INFO("Module '{}' is disabled, skipping", info.name);
            continue;
        }

        E2D_LOG_INFO("Initializing module '{}' (priority: {})", 
                     info.name, static_cast<int>(info.priority));

        if (!initializer->initialize(moduleConfig)) {
            E2D_LOG_ERROR("Failed to initialize module '{}'", info.name);
        }
    }

    std::string backend = "sdl2";
#ifdef __SWITCH__
    backend = "switch";
#endif

    if (!BackendFactory::has(backend)) {
        E2D_LOG_ERROR("Backend '{}' not available", backend);
        auto backends = BackendFactory::backends();
        if (backends.empty()) {
            E2D_LOG_ERROR("No backends registered!");
            return false;
        }
        backend = backends[0];
        E2D_LOG_WARN("Using fallback backend: {}", backend);
    }

    window_ = BackendFactory::createWindow(backend);
    if (!window_) {
        E2D_LOG_ERROR("Failed to create window for backend: {}", backend);
        return false;
    }

    WindowConfigData winConfig = appConfig.window;
    
    if (platform == PlatformType::Switch) {
        winConfig.mode = WindowMode::Fullscreen;
        winConfig.resizable = false;
    }

    if (!window_->create(winConfig)) {
        E2D_LOG_ERROR("Failed to create window");
        return false;
    }

    renderer_ = RenderBackend::create(appConfig.render.backend);
    if (!renderer_ || !renderer_->init(window_.get())) {
        E2D_LOG_ERROR("Failed to initialize renderer");
        window_->destroy();
        return false;
    }

    sceneManager_ = makeUnique<SceneManager>();
    timerManager_ = makeUnique<TimerManager>();
    eventQueue_ = makeUnique<EventQueue>();
    eventDispatcher_ = makeUnique<EventDispatcher>();
    camera_ = makeUnique<Camera>(0, static_cast<float>(window_->width()),
                                  static_cast<float>(window_->height()), 0);

    viewportAdapter_ = makeUnique<ViewportAdapter>();
    ViewportConfig vpConfig;
    vpConfig.logicWidth = static_cast<float>(appConfig.window.width);
    vpConfig.logicHeight = static_cast<float>(appConfig.window.height);
    vpConfig.mode = ViewportMode::AspectRatio;
    viewportAdapter_->setConfig(vpConfig);

    camera_->setViewportAdapter(viewportAdapter_.get());

    viewportAdapter_->update(window_->width(), window_->height());

    window_->onResize([this](int width, int height) {
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
    E2D_LOG_INFO("  Window: {}x{}", window_->width(), window_->height());
    E2D_LOG_INFO("  Backend: {}", backend);
    E2D_LOG_INFO("  VSync: {}", appConfig.render.vsync);
    E2D_LOG_INFO("  Target FPS: {}", appConfig.render.targetFPS);

    return true;
}

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

    auto modules = ModuleRegistry::instance().getAllModules();
    auto initOrder = ModuleRegistry::instance().getInitializationOrder();
    
    for (auto it = initOrder.rbegin(); it != initOrder.rend(); ++it) {
        ModuleId moduleId = *it;
        auto initializer = ModuleRegistry::instance().createInitializer(moduleId);
        if (initializer && initializer->isInitialized()) {
            auto* moduleConfig = ModuleRegistry::instance().getModuleConfig(moduleId);
            if (moduleConfig) {
                auto info = moduleConfig->getModuleInfo();
                E2D_LOG_INFO("Shutting down module '{}'", info.name);
            }
            initializer->shutdown();
        }
    }

    PlatformType platform = ConfigManager::instance().appConfig().targetPlatform;
    if (platform == PlatformType::Auto) {
#ifdef __SWITCH__
        platform = PlatformType::Switch;
#else
        platform = PlatformType::Windows;
#endif
    }
    
    if (platform == PlatformType::Switch) {
#ifdef __SWITCH__
        romfsExit();
        socketExit();
#endif
    }

    ConfigManager::instance().shutdown();

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

    while (running_ && !window_->shouldClose()) {
        mainLoop();
    }
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

    window_->poll();

    if (eventDispatcher_ && eventQueue_) {
        eventDispatcher_->processQueue(*eventQueue_);
    }

    if (!paused_) {
        update();
    }

    render();

    const auto& appConfig = ConfigManager::instance().appConfig();
    if (!appConfig.render.vsync && appConfig.render.isFPSCapped()) {
        double frameEndTime = getTimeSeconds();
        double frameTime = frameEndTime - currentTime;
        double target = 1.0 / static_cast<double>(appConfig.render.targetFPS);
        if (frameTime < target) {
            auto sleepSeconds = target - frameTime;
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepSeconds));
        }
    }

    ConfigManager::instance().update(deltaTime_);
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
        const auto& vp = viewportAdapter_->getViewport();
        renderer_->setViewport(
            static_cast<int>(vp.origin.x), static_cast<int>(vp.origin.y),
            static_cast<int>(vp.size.width), static_cast<int>(vp.size.height));
    } else {
        renderer_->setViewport(0, 0, window_->width(), window_->height());
    }

    if (sceneManager_) {
        sceneManager_->render(*renderer_);
    } else {
        E2D_LOG_WARN("Render: sceneManager is null");
    }

    window_->swap();
}

IInput& Application::input() { 
    return *window_->input(); 
}

SceneManager& Application::scenes() { 
    return *sceneManager_; 
}

TimerManager& Application::timers() { 
    return *timerManager_; 
}

EventQueue& Application::eventQueue() { 
    return *eventQueue_; 
}

EventDispatcher& Application::eventDispatcher() { 
    return *eventDispatcher_; 
}

Camera& Application::camera() { 
    return *camera_; 
}

ViewportAdapter& Application::viewportAdapter() { 
    return *viewportAdapter_; 
}

void Application::enterScene(Ptr<Scene> scene) {
    if (sceneManager_ && scene) {
        scene->setViewportSize(static_cast<float>(window_->width()),
                               static_cast<float>(window_->height()));
        sceneManager_->enterScene(scene);
    }
}

ConfigManager& Application::config() {
    return ConfigManager::instance();
}

const AppConfig& Application::getConfig() const {
    return ConfigManager::instance().appConfig();
}

} // namespace extra2d
