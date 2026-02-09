#include <easy2d/app/application.h>
#include <easy2d/platform/window.h>
#include <easy2d/platform/input.h>
#include <easy2d/audio/audio_engine.h>
#include <easy2d/scene/scene_manager.h>
#include <easy2d/resource/resource_manager.h>
#include <easy2d/utils/timer.h>
#include <easy2d/event/event_queue.h>
#include <easy2d/event/event_dispatcher.h>
#include <easy2d/graphics/camera.h>
#include <easy2d/graphics/render_backend.h>
#include <easy2d/graphics/vram_manager.h>
#include <easy2d/utils/logger.h>

#include <switch.h>
#include <chrono>
#include <thread>
#include <time.h>

namespace easy2d {

// 获取当前时间（秒）
static double getTimeSeconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / 1000000000.0;
}

Application& Application::instance() {
    static Application instance;
    return instance;
}

Application::~Application() {
    shutdown();
}

bool Application::init(const AppConfig& config) {
    if (initialized_) {
        E2D_LOG_WARN("Application already initialized");
        return true;
    }

    config_ = config;

    // ========================================
    // 1. 初始化 RomFS 文件系统（必须在 SDL_Init 之前）
    // ========================================
    Result rc;
    rc = romfsInit();
    if (R_SUCCEEDED(rc)) {
        E2D_LOG_INFO("RomFS initialized successfully");
    } else {
        E2D_LOG_WARN("romfsInit failed: {:#08X}, will use regular filesystem", rc);
    }

    // ========================================
    // 2. 初始化 nxlink 调试输出（可选）
    // ========================================
    rc = socketInitializeDefault();
    if (R_FAILED(rc)) {
        E2D_LOG_WARN("socketInitializeDefault failed, nxlink will not be available");
    }

    // ========================================
    // 3. 创建窗口（包含 SDL_Init + GLES 3.2 上下文创建）
    // ========================================
    window_ = makeUnique<Window>();
    WindowConfig winConfig;
    winConfig.title = config.title;
    winConfig.width = 1280;
    winConfig.height = 720;
    winConfig.fullscreen = true;
    winConfig.resizable = false;
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

    // 窗口大小回调（Switch 上不会触发，但保留接口）
    window_->setResizeCallback([this](int width, int height) {
        if (camera_) {
            camera_->setViewport(0, static_cast<float>(width),
                                 static_cast<float>(height), 0);
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

    // 添加 romfs:/ 到资源搜索路径
    resourceManager_->addSearchPath("romfs:/");

    initialized_ = true;
    running_ = true;

    E2D_LOG_INFO("Application initialized successfully");
    return true;
}

void Application::shutdown() {
    if (!initialized_) return;

    E2D_LOG_INFO("Shutting down application...");

    // 打印 VRAM 统计
    VRAMManager::getInstance().printStats();

    // 清理子系统
    sceneManager_.reset();
    resourceManager_.reset();
    timerManager_.reset();
    eventQueue_.reset();
    eventDispatcher_.reset();
    camera_.reset();

    // 关闭音频
    AudioEngine::getInstance().shutdown();

    // 关闭渲染器
    if (renderer_) {
        renderer_->shutdown();
        renderer_.reset();
    }

    // 销毁窗口（包含 SDL_Quit）
    if (window_) {
        window_->destroy();
        window_.reset();
    }

    // Switch 清理
    romfsExit();
    socketExit();

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

    // SDL2 on Switch 内部已处理 appletMainLoop
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

    renderer_->setViewport(0, 0, window_->getWidth(), window_->getHeight());

    if (sceneManager_) {
        sceneManager_->render(*renderer_);
    } else {
        E2D_LOG_WARN("Render: sceneManager is null");
    }

    window_->swapBuffers();
}

Input& Application::input() {
    return *window_->getInput();
}

AudioEngine& Application::audio() {
    return AudioEngine::getInstance();
}

SceneManager& Application::scenes() {
    return *sceneManager_;
}

ResourceManager& Application::resources() {
    return *resourceManager_;
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

void Application::enterScene(Ptr<Scene> scene) {
    enterScene(scene, nullptr);
}

void Application::enterScene(Ptr<Scene> scene, Ptr<class Transition> transition) {
    if (sceneManager_ && scene) {
        scene->setViewportSize(static_cast<float>(window_->getWidth()),
                               static_cast<float>(window_->getHeight()));
        sceneManager_->enterScene(scene, transition);
    }
}

} // namespace easy2d
