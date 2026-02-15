#pragma once

#include <extra2d/core/types.h>
#include <extra2d/core/service_locator.h>
#include <extra2d/config/app_config.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/config/module_config.h>
#include <extra2d/platform/iwindow.h>
#include <string>

namespace extra2d {

class IInput;
class RenderBackend;

/**
 * @brief 应用程序类
 * 使用服务定位器模式管理模块依赖，支持依赖注入和测试Mock
 */
class Application {
public:
    /**
     * @brief 获取单例实例
     * @return 应用程序实例引用
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
     * @brief 使用指定配置初始化
     * @param config 应用配置
     * @return 初始化成功返回 true
     */
    bool init(const AppConfig& config);

    /**
     * @brief 使用配置文件初始化
     * @param configPath 配置文件路径
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
     * @brief 请求退出
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
     * @brief 获取窗口
     * @return 窗口引用
     */
    IWindow& window() { return *window_; }

    /**
     * @brief 获取渲染器
     * @return 渲染器引用
     */
    RenderBackend& renderer();

    /**
     * @brief 获取输入接口
     * @return 输入接口引用
     */
    IInput& input();

    /**
     * @brief 获取场景服务
     * @return 场景服务共享指针
     */
    SharedPtr<class ISceneService> scenes();

    /**
     * @brief 获取计时器服务
     * @return 计时器服务共享指针
     */
    SharedPtr<class ITimerService> timers();

    /**
     * @brief 获取事件服务
     * @return 事件服务共享指针
     */
    SharedPtr<class IEventService> events();

    /**
     * @brief 获取相机服务
     * @return 相机服务共享指针
     */
    SharedPtr<class ICameraService> camera();

    /**
     * @brief 进入场景
     * @param scene 场景指针
     */
    void enterScene(Ptr<class Scene> scene);

    /**
     * @brief 获取帧间隔时间
     * @return 帧间隔时间（秒）
     */
    float deltaTime() const { return deltaTime_; }

    /**
     * @brief 获取总运行时间
     * @return 总运行时间（秒）
     */
    float totalTime() const { return totalTime_; }

    /**
     * @brief 获取当前帧率
     * @return 帧率
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

    /**
     * @brief 注册自定义服务
     * @tparam T 服务接口类型
     * @param service 服务实例
     */
    template<typename T>
    void registerService(SharedPtr<T> service) {
        ServiceLocator::instance().registerService(service);
    }

    /**
     * @brief 获取服务
     * @tparam T 服务接口类型
     * @return 服务共享指针
     */
    template<typename T>
    SharedPtr<T> getService() {
        return ServiceLocator::instance().getService<T>();
    }

private:
    Application() = default;
    ~Application();

    /**
     * @brief 初始化模块
     * @return 初始化成功返回 true
     */
    bool initModules();

    /**
     * @brief 注册核心服务
     */
    void registerCoreServices();

    /**
     * @brief 主循环
     */
    void mainLoop();

    /**
     * @brief 更新
     */
    void update();

    /**
     * @brief 渲染
     */
    void render();

    IWindow* window_ = nullptr;

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

} 
