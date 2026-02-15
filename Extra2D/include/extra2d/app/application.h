#pragma once

#include <extra2d/core/types.h>
#include <extra2d/config/app_config.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/config/module_config.h>
#include <extra2d/config/platform_config.h>
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
 * @brief Application 单例 - 应用主控
 * 
 * 负责管理应用程序的整个生命周期，包括初始化、主循环、渲染和关闭。
 * 集成了 ConfigManager 和 ModuleRegistry 系统进行配置和模块管理。
 */
class Application {
public:
    /**
     * @brief 获取单例实例
     * @return Application 实例引用
     */
    static Application& get();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * @brief 使用默认配置初始化
     * @return 初始化成功返回 true
     */
    bool init();

    /**
     * @brief 使用配置结构初始化
     * @param config 应用配置
     * @return 初始化成功返回 true
     */
    bool init(const AppConfig& config);

    /**
     * @brief 从配置文件初始化
     * @param configPath 配置文件路径（支持 .json 和 .ini）
     * @return 初始化成功返回 true
     */
    bool init(const std::string& configPath);

    /**
     * @brief 关闭应用程序
     */
    void shutdown();

    /**
     * @brief 运行主循环
     */
    void run();

    /**
     * @brief 退出应用程序
     */
    void quit();

    /**
     * @brief 暂停应用程序
     */
    void pause();

    /**
     * @brief 恢复应用程序
     */
    void resume();

    /**
     * @brief 检查是否暂停
     * @return 暂停状态返回 true
     */
    bool isPaused() const { return paused_; }

    /**
     * @brief 检查是否运行中
     * @return 运行中返回 true
     */
    bool isRunning() const { return running_; }

    /**
     * @brief 获取窗口接口
     * @return 窗口接口引用
     */
    IWindow& window() { return *window_; }

    /**
     * @brief 获取渲染后端
     * @return 渲染后端引用
     */
    RenderBackend& renderer() { return *renderer_; }

    /**
     * @brief 获取输入接口
     * @return 输入接口引用
     */
    IInput& input();

    /**
     * @brief 获取场景管理器
     * @return 场景管理器引用
     */
    SceneManager& scenes();

    /**
     * @brief 获取定时器管理器
     * @return 定时器管理器引用
     */
    TimerManager& timers();

    /**
     * @brief 获取事件队列
     * @return 事件队列引用
     */
    EventQueue& eventQueue();

    /**
     * @brief 获取事件分发器
     * @return 事件分发器引用
     */
    EventDispatcher& eventDispatcher();

    /**
     * @brief 获取相机
     * @return 相机引用
     */
    Camera& camera();

    /**
     * @brief 获取视口适配器
     * @return 视口适配器引用
     */
    ViewportAdapter& viewportAdapter();

    /**
     * @brief 进入场景
     * @param scene 场景指针
     */
    void enterScene(Ptr<class Scene> scene);

    /**
     * @brief 获取帧时间
     * @return 帧时间（秒）
     */
    float deltaTime() const { return deltaTime_; }

    /**
     * @brief 获取总运行时间
     * @return 总运行时间（秒）
     */
    float totalTime() const { return totalTime_; }

    /**
     * @brief 获取当前帧率
     * @return 帧率（FPS）
     */
    int fps() const { return currentFps_; }

    /**
     * @brief 获取配置管理器
     * @return 配置管理器引用
     */
    ConfigManager& config();

    /**
     * @brief 获取应用配置
     * @return 应用配置常量引用
     */
    const AppConfig& getConfig() const;

private:
    Application() = default;
    ~Application();

    /**
     * @brief 内部初始化实现
     * @return 初始化成功返回 true
     */
    bool initImpl();

    /**
     * @brief 主循环
     */
    void mainLoop();

    /**
     * @brief 更新逻辑
     */
    void update();

    /**
     * @brief 渲染
     */
    void render();

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

    ModuleId windowModuleId_ = INVALID_MODULE_ID;
    ModuleId inputModuleId_ = INVALID_MODULE_ID;
    ModuleId renderModuleId_ = INVALID_MODULE_ID;
};

} // namespace extra2d
