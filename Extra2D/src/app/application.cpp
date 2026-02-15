#include <extra2d/app/application.h>
#include <extra2d/config/config_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/graphics/render_module.h>
#include <extra2d/graphics/render_config.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/iinput.h>
#include <extra2d/platform/input_module.h>
#include <extra2d/platform/platform_init_module.h>
#include <extra2d/platform/window_module.h>
#include <extra2d/services/scene_service.h>
#include <extra2d/services/timer_service.h>
#include <extra2d/services/event_service.h>
#include <extra2d/services/camera_service.h>

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
    register_input_module();
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
    register_input_module();
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

void Application::registerCoreServices() {
    auto& locator = ServiceLocator::instance();

    if (!locator.hasService<ISceneService>()) {
        locator.registerService<ISceneService>(makeShared<SceneService>());
    }

    if (!locator.hasService<ITimerService>()) {
        locator.registerService<ITimerService>(makeShared<TimerService>());
    }

    if (!locator.hasService<ICameraService>()) {
        auto cameraService = makeShared<CameraService>();
        if (window_) {
            cameraService->setViewport(0, static_cast<float>(window_->width()),
                                        static_cast<float>(window_->height()), 0);
            ViewportConfig vpConfig;
            vpConfig.logicWidth = static_cast<float>(window_->width());
            vpConfig.logicHeight = static_cast<float>(window_->height());
            vpConfig.mode = ViewportMode::AspectRatio;
            cameraService->setViewportConfig(vpConfig);
            cameraService->updateViewport(window_->width(), window_->height());
        }
        locator.registerService<ICameraService>(cameraService);
    }
}

bool Application::initModules() {
    auto& locator = ServiceLocator::instance();
    
    if (!locator.hasService<IEventService>()) {
        locator.registerService<IEventService>(makeShared<EventService>());
    }
    
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

    registerCoreServices();

    if (!ServiceLocator::instance().initializeAll()) {
        return false;
    }

    auto cameraService = ServiceLocator::instance().getService<ICameraService>();
    if (cameraService && window_) {
        window_->onResize([this, cameraService](int width, int height) {
            cameraService->updateViewport(width, height);
            cameraService->applyViewportAdapter();

            auto sceneService = ServiceLocator::instance().getService<ISceneService>();
            if (sceneService) {
                auto currentScene = sceneService->getCurrentScene();
                if (currentScene) {
                    currentScene->setViewportSize(static_cast<float>(width),
                                                  static_cast<float>(height));
                }
            }
        });
    }

    initialized_ = true;
    running_ = true;

    return true;
}

void Application::shutdown() {
    if (!initialized_)
        return;

    VRAMMgr::get().printStats();

    ServiceLocator::instance().clear();

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
        ServiceLocator::instance().pauseAll();
    }
}

void Application::resume() {
    if (paused_) {
        paused_ = false;
        ServiceLocator::instance().resumeAll();
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

    auto eventService = ServiceLocator::instance().getService<IEventService>();
    if (eventService) {
        eventService->processQueue();
    }

    if (!paused_) {
        update();
    }

    render();

    const auto& appConfig = ConfigManager::instance().appConfig();
    
    auto* renderConfig = ModuleRegistry::instance().getModuleConfig(get_render_module_id());
    auto* renderModuleConfig = dynamic_cast<const RenderModuleConfig*>(renderConfig);
    
    if (renderModuleConfig && !renderModuleConfig->vsync && renderModuleConfig->targetFPS > 0) {
        double frameEndTime = getTimeSeconds();
        double frameTime = frameEndTime - currentTime;
        double target = 1.0 / static_cast<double>(renderModuleConfig->targetFPS);
        if (frameTime < target) {
            auto sleepSeconds = target - frameTime;
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepSeconds));
        }
    }

    ConfigManager::instance().update(deltaTime_);
}

void Application::update() {
    ServiceLocator::instance().updateAll(deltaTime_);
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

    auto cameraService = ServiceLocator::instance().getService<ICameraService>();
    if (cameraService) {
        const auto& vp = cameraService->getViewportResult().viewport;
        renderer->setViewport(
            static_cast<int>(vp.origin.x), static_cast<int>(vp.origin.y),
            static_cast<int>(vp.size.width), static_cast<int>(vp.size.height));
    } else {
        renderer->setViewport(0, 0, window_->width(), window_->height());
    }

    auto sceneService = ServiceLocator::instance().getService<ISceneService>();
    if (sceneService) {
        sceneService->render(*renderer);
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

SharedPtr<ISceneService> Application::scenes() {
    return ServiceLocator::instance().getService<ISceneService>();
}

SharedPtr<ITimerService> Application::timers() {
    return ServiceLocator::instance().getService<ITimerService>();
}

SharedPtr<IEventService> Application::events() {
    return ServiceLocator::instance().getService<IEventService>();
}

SharedPtr<ICameraService> Application::camera() {
    return ServiceLocator::instance().getService<ICameraService>();
}

void Application::enterScene(Ptr<Scene> scene) {
    auto sceneService = ServiceLocator::instance().getService<ISceneService>();
    if (sceneService && scene) {
        scene->setViewportSize(static_cast<float>(window_->width()),
                               static_cast<float>(window_->height()));
        sceneService->enterScene(scene);
    }
}

ConfigManager& Application::config() {
    return ConfigManager::instance();
}

const AppConfig& Application::getConfig() const {
    return ConfigManager::instance().appConfig();
}

} 
