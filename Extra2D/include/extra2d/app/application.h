#pragma once

#include <extra2d/core/types.h>
#include <extra2d/graphics/render_backend.h>
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

/**
 * @brief 平台类型
 */
enum class PlatformType { Auto = 0, PC, Switch };

/**
 * @brief 应用配置
 */
struct AppConfig {
    std::string title = "Extra2D Application";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    int fpsLimit = 0;
    BackendType renderBackend = BackendType::OpenGL;
    int msaaSamples = 0;
    PlatformType platform = PlatformType::Auto;
    bool enableCursors = true;
    bool enableDpiScale = false;
    
    std::string backend = "sdl2";
};

/**
 * @brief Application 单例 - 应用主控
 */
class Application {
public:
    static Application& get();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * @brief 使用默认配置初始化
     */
    bool init();

    /**
     * @brief 使用配置结构初始化
     */
    bool init(const AppConfig& config);

    /**
     * @brief 从配置文件初始化
     * @param path 配置文件路径（支持 .json 和 .ini）
     */
    bool init(const std::string& path);

    void shutdown();
    void run();
    void quit();

    void pause();
    void resume();
    bool isPaused() const { return paused_; }
    bool isRunning() const { return running_; }

    IWindow& window() { return *window_; }
    RenderBackend& renderer() { return *renderer_; }
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

    const AppConfig& getConfig() const { return config_; }

private:
    Application() = default;
    ~Application();

    bool initImpl();
    void mainLoop();
    void update();
    void render();

    AppConfig config_;

    UniquePtr<IWindow> window_;
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
