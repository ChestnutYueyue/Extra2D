#include <extra2d/app/application.h>
#include <extra2d/config/config_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/event/event_queue.h>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/render_module.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/iinput.h>
#include <extra2d/platform/platform_init_module.h>
#include <extra2d/platform/window_module.h>
#include <extra2d/scene/scene_manager.h>
#include <extra2d/utils/timer.h>

#include <chrono>
#include <thread>

namespace extra2d {

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
        return true;
    }

    register_config_module();
    register_platform_module();
    register_window_module();
    register_render_module();

    auto* configInit = ModuleRegistry::instance().getInitializer(get_config_module_id());
    if (configInit) {
        auto* cfgInit = dynamic_cast<ConfigModuleInitializer*>(configInit);
        if (cfgInit) {
            cfgInit->setAppConfig(config);
        }
    }

    return initModules();
}

bool Application::init(const std::string& configPath) {
    if (initialized_) {
        return true;
    }

    register_config_module();
    register_platform_module();
    register_window_module();
    register_render_module();

    auto* configInit = ModuleRegistry::instance().getInitializer(get_config_module_id());
    if (configInit) {
        auto* cfgInit = dynamic_cast<ConfigModuleInitializer*>(configInit);
        if (cfgInit) {
            cfgInit->setConfigPath(configPath);
        }
    }

    return initModules();
}

bool Application::initModules() {
    auto initOrder = ModuleRegistry::instance().getInitializationOrder();
    
    for (ModuleId moduleId : initOrder) {
        auto* initializer = ModuleRegistry::instance().getInitializer(moduleId);
        if (!initializer) {
            continue;
        }

        auto* moduleConfig = ModuleRegistry::instance().getModuleConfig(moduleId);
        if (!moduleConfig) {
            continue;
        }

        auto info = moduleConfig->getModuleInfo();
        if (!info.enabled) {
            continue;
        }

        if (info.name == "Render") {
            continue;
        }

        if (!initializer->initialize(moduleConfig)) {
            return false;
        }
    }

    auto* windowInit = ModuleRegistry::instance().getInitializer(get_window_module_id());
    if (!windowInit || !windowInit->isInitialized()) {
        return false;
    }

    auto* windowModule = dynamic_cast<WindowModuleInitializer*>(windowInit);
    if (!windowModule) {
        return false;
    }

    window_ = windowModule->getWindow();
    if (!window_) {
        return false;
    }

    auto* renderInit = ModuleRegistry::instance().getInitializer(get_render_module_id());
    if (renderInit) {
        auto* renderModule = dynamic_cast<RenderModuleInitializer*>(renderInit);
        if (renderModule) {
            renderModule->setWindow(window_);
            
            auto* renderConfig = ModuleRegistry::instance().getModuleConfig(get_render_module_id());
            if (renderConfig && !renderInit->initialize(renderConfig)) {
                return false;
            }
        }
    }

    sceneManager_ = makeUnique<SceneManager>();
    timerManager_ = makeUnique<TimerManager>();
    eventQueue_ = makeUnique<EventQueue>();
    eventDispatcher_ = makeUnique<EventDispatcher>();
    camera_ = makeUnique<Camera>(0, static_cast<float>(window_->width()),
                                  static_cast<float>(window_->height()), 0);

    viewportAdapter_ = makeUnique<ViewportAdapter>();
    ViewportConfig vpConfig;
    vpConfig.logicWidth = static_cast<float>(window_->width());
    vpConfig.logicHeight = static_cast<float>(window_->height());
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

    return true;
}

void Application::shutdown() {
    if (!initialized_)
        return;

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

    window_ = nullptr;

    auto initOrder = ModuleRegistry::instance().getInitializationOrder();
    
    for (auto it = initOrder.rbegin(); it != initOrder.rend(); ++it) {
        ModuleId moduleId = *it;
        auto* initializer = ModuleRegistry::instance().getInitializer(moduleId);
        if (initializer && initializer->isInitialized()) {
            initializer->shutdown();
        }
    }

    initialized_ = false;
    running_ = false;
}

void Application::run() {
    if (!initialized_) {
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
    }
}

void Application::resume() {
    if (paused_) {
        paused_ = false;
        lastFrameTime_ = getTimeSeconds();
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
    auto* renderInit = ModuleRegistry::instance().getInitializer(get_render_module_id());
    RenderBackend* renderer = nullptr;
    if (renderInit) {
        auto* renderModule = dynamic_cast<RenderModuleInitializer*>(renderInit);
        if (renderModule) {
            renderer = renderModule->getRenderer();
        }
    }

    if (!renderer) {
        return;
    }

    if (viewportAdapter_) {
        const auto& vp = viewportAdapter_->getViewport();
        renderer->setViewport(
            static_cast<int>(vp.origin.x), static_cast<int>(vp.origin.y),
            static_cast<int>(vp.size.width), static_cast<int>(vp.size.height));
    } else {
        renderer->setViewport(0, 0, window_->width(), window_->height());
    }

    if (sceneManager_) {
        sceneManager_->render(*renderer);
    }

    window_->swap();
}

IInput& Application::input() { 
    return *window_->input(); 
}

RenderBackend& Application::renderer() {
    auto* renderInit = ModuleRegistry::instance().getInitializer(get_render_module_id());
    if (renderInit) {
        auto* renderModule = dynamic_cast<RenderModuleInitializer*>(renderInit);
        if (renderModule && renderModule->getRenderer()) {
            return *renderModule->getRenderer();
        }
    }
    static RenderBackend* dummy = nullptr;
    if (!dummy) {
        dummy = RenderBackend::create(BackendType::OpenGL).release();
    }
    return *dummy;
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
