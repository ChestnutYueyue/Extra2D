#pragma once

#include <extra2d/core/types.h>
#include <extra2d/config/app_config.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/config/module_config.h>
#include <extra2d/platform/iwindow.h>
#include <string>

namespace extra2d {

class IInput;
class SceneManager;
class TimerManager;
class EventQueue;
class EventDispatcher;
class Camera;
class ViewportAdapter;
class RenderBackend;

class Application {
public:
    static Application& get();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool init();
    bool init(const AppConfig& config);
    bool init(const std::string& configPath);

    void shutdown();
    void run();
    void quit();
    void pause();
    void resume();

    bool isPaused() const { return paused_; }
    bool isRunning() const { return running_; }

    IWindow& window() { return *window_; }
    RenderBackend& renderer();
    IInput& input();

    SceneManager& scenes();
    TimerManager& timers();
    EventQueue& eventQueue();
    EventDispatcher& eventDispatcher();
    Camera& camera();
    ViewportAdapter& viewportAdapter();

    void enterScene(Ptr<class Scene> scene);

    float deltaTime() const { return deltaTime_; }
    float totalTime() const { return totalTime_; }
    int fps() const { return currentFps_; }

    ConfigManager& config();
    const AppConfig& getConfig() const;

private:
    Application() = default;
    ~Application();

    bool initModules();
    void mainLoop();
    void update();
    void render();

    IWindow* window_ = nullptr;
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
