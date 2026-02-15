# Extra2D 模块系统

## 概述

Extra2D 采用模块化架构设计，所有核心功能通过模块系统和服务系统管理。系统提供：

- **统一的生命周期管理**：初始化、关闭、依赖处理
- **优先级排序**：确保模块/服务按正确顺序初始化
- **配置驱动**：每个模块可独立配置
- **依赖注入**：通过服务定位器解耦模块间依赖
- **可测试性**：支持 Mock 服务进行单元测试

## 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                      Application                             │
│  (协调模块和服务，通过服务定位器获取依赖)                       │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌─────────────────────────────┐   ┌─────────────────────────────┐
│      ModuleRegistry         │   │      ServiceLocator         │
│  (模块注册表，管理平台级模块) │   │  (服务定位器，管理运行时服务) │
└─────────────────────────────┘   └─────────────────────────────┘
              │                               │
        ┌─────┴─────┐                 ┌───────┴───────┐
        ▼           ▼                 ▼               ▼
┌───────────┐ ┌───────────┐   ┌───────────┐   ┌───────────┐
│  Config   │ │  Window   │   │  Scene    │   │  Timer    │
│  Module   │ │  Module   │   │  Service  │   │  Service  │
└───────────┘ └───────────┘   └───────────┘   └───────────┘
                                    │               │
                              ┌─────┴─────┐   ┌─────┴─────┐
                              ▼           ▼   ▼           ▼
                        ┌──────────┐ ┌──────────┐ ┌──────────┐
                        │  Event   │ │  Camera  │ │   ...    │
                        │  Service │ │  Service │ │          │
                        └──────────┘ └──────────┘ └──────────┘
```

## 模块 vs 服务

| 特性 | 模块 (Module) | 服务 (Service) |
|-----|--------------|---------------|
| 用途 | 平台级初始化 | 运行时功能 |
| 生命周期 | Application 管理 | ServiceLocator 管理 |
| 依赖方式 | 通过 ModuleRegistry | 通过 ServiceLocator |
| 可替换性 | 编译时确定 | 运行时可替换 |
| 测试支持 | 需要重构 | 原生支持 Mock |
| 示例 | Window, Render, Config | Scene, Timer, Event, Camera |

## 模块优先级

模块按优先级从小到大初始化，关闭时逆序执行：

| 优先级值 | 枚举名称 | 用途 | 模块示例 |
|---------|---------|------|---------|
| 0 | `Core` | 核心模块，最先初始化 | Logger, Config, Platform, Window |
| 100 | `Graphics` | 图形渲染 | Render |
| 200 | `Audio` | 音频系统 | Audio |
| 500 | `Resource` | 资源管理 | Resource |

## 服务优先级

服务按优先级从小到大初始化：

| 优先级值 | 枚举名称 | 用途 | 服务示例 |
|---------|---------|------|---------|
| 0 | `Core` | 核心服务 | - |
| 100 | `Event` | 事件系统 | EventService |
| 200 | `Timer` | 计时器 | TimerService |
| 300 | `Scene` | 场景管理 | SceneService |
| 400 | `Camera` | 相机系统 | CameraService |
| 500 | `Resource` | 资源管理 | - |
| 600 | `Audio` | 音频系统 | - |

---

## 模块系统

### IModuleConfig

模块配置接口，定义模块的元数据和配置：

```cpp
class IModuleConfig {
public:
    virtual ~IModuleConfig() = default;
    
    virtual ModuleInfo getModuleInfo() const = 0;
    virtual std::string getConfigSectionName() const = 0;
    virtual bool validate() const = 0;
    virtual void resetToDefaults() = 0;
    virtual bool loadFromJson(const void* jsonData) = 0;
    virtual bool saveToJson(void* jsonData) const = 0;
    virtual void applyPlatformConstraints(PlatformType platform) {}
};
```

### IModuleInitializer

模块初始化器接口，管理模块的生命周期：

```cpp
class IModuleInitializer {
public:
    virtual ~IModuleInitializer() = default;
    
    virtual ModuleId getModuleId() const = 0;
    virtual ModulePriority getPriority() const = 0;
    virtual std::vector<ModuleId> getDependencies() const = 0;
    
    virtual bool initialize(const IModuleConfig* config) = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
};
```

### ModuleRegistry

模块注册表，管理所有模块：

```cpp
class ModuleRegistry {
public:
    static ModuleRegistry& instance();
    
    ModuleId registerModule(
        UniquePtr<IModuleConfig> config,
        ModuleInitializerFactory factory
    );
    
    IModuleConfig* getModuleConfig(ModuleId id);
    IModuleInitializer* getInitializer(ModuleId id);
    std::vector<ModuleId> getInitializationOrder() const;
};
```

---

## 服务系统

### IService

服务接口基类，所有服务必须实现：

```cpp
class IService {
public:
    virtual ~IService() = default;
    
    virtual ServiceInfo getServiceInfo() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void pause();
    virtual void resume();
    virtual void update(float deltaTime);
    virtual bool isInitialized() const;
    
    ServiceState getState() const;
    const std::string& getName() const;
};
```

### ServiceLocator

服务定位器，实现依赖注入和服务发现：

```cpp
class ServiceLocator {
public:
    static ServiceLocator& instance();
    
    // 注册服务实例
    template<typename T>
    void registerService(SharedPtr<T> service);
    
    // 注册服务工厂（延迟创建）
    template<typename T>
    void registerFactory(ServiceFactory<T> factory);
    
    // 获取服务
    template<typename T>
    SharedPtr<T> getService() const;
    
    // 检查服务是否存在
    template<typename T>
    bool hasService() const;
    
    // 批量操作
    bool initializeAll();
    void shutdownAll();
    void updateAll(float deltaTime);
    void pauseAll();
    void resumeAll();
    void clear();
};
```

### 内置服务

#### SceneService

场景管理服务，包装 SceneManager：

```cpp
class ISceneService : public IService {
public:
    virtual void runWithScene(Ptr<Scene> scene) = 0;
    virtual void replaceScene(Ptr<Scene> scene) = 0;
    virtual void pushScene(Ptr<Scene> scene) = 0;
    virtual void popScene() = 0;
    
    virtual Ptr<Scene> getCurrentScene() const = 0;
    virtual size_t getSceneCount() const = 0;
    
    virtual void render(RenderBackend& renderer) = 0;
};
```

#### TimerService

计时器服务，包装 TimerManager：

```cpp
class ITimerService : public IService {
public:
    virtual uint32 addTimer(float delay, Timer::Callback callback) = 0;
    virtual uint32 addRepeatingTimer(float interval, Timer::Callback callback) = 0;
    virtual void cancelTimer(uint32 timerId) = 0;
    virtual void pauseTimer(uint32 timerId) = 0;
    virtual void resumeTimer(uint32 timerId) = 0;
};
```

#### EventService

事件服务，整合 EventQueue 和 EventDispatcher：

```cpp
class IEventService : public IService {
public:
    virtual void pushEvent(const Event& event) = 0;
    virtual bool pollEvent(Event& event) = 0;
    
    virtual ListenerId addListener(EventType type, EventCallback callback) = 0;
    virtual void removeListener(ListenerId id) = 0;
    
    virtual void dispatch(Event& event) = 0;
    virtual void processQueue() = 0;
};
```

#### CameraService

相机服务，整合 Camera 和 ViewportAdapter：

```cpp
class ICameraService : public IService {
public:
    virtual void setPosition(const Vec2& position) = 0;
    virtual void setZoom(float zoom) = 0;
    virtual void setRotation(float degrees) = 0;
    
    virtual glm::mat4 getViewProjectionMatrix() const = 0;
    virtual Vec2 screenToWorld(const Vec2& screenPos) const = 0;
    
    virtual void setViewportConfig(const ViewportConfig& config) = 0;
    virtual void updateViewport(int width, int height) = 0;
};
```

---

## 使用示例

### 在 Application 中使用服务

```cpp
// 获取服务
auto sceneService = Application::get().scenes();
auto timerService = Application::get().timers();
auto eventService = Application::get().events();
auto cameraService = Application::get().camera();

// 使用场景服务
sceneService->pushScene(myScene);

// 使用计时器服务
timerService->addTimer(1.0f, []() {
    E2D_LOG_INFO("Timer fired!");
});

// 使用事件服务
eventService->addListener(EventType::KeyDown, [](Event& e) {
    E2D_LOG_INFO("Key pressed: {}", e.key.keycode);
});

// 使用相机服务
cameraService->setPosition(Vec2(100.0f, 200.0f));
cameraService->setZoom(2.0f);
```

### 注册自定义服务

```cpp
// 定义服务接口
class IAudioService : public IService {
public:
    virtual void playSound(const std::string& path) = 0;
    virtual void stopAll() = 0;
};

// 实现服务
class AudioService : public IAudioService {
public:
    ServiceInfo getServiceInfo() const override {
        ServiceInfo info;
        info.name = "AudioService";
        info.priority = ServicePriority::Audio;
        return info;
    }
    
    bool initialize() override {
        // 初始化音频系统...
        setState(ServiceState::Running);
        return true;
    }
    
    void shutdown() override {
        // 清理音频系统...
        setState(ServiceState::Stopped);
    }
    
    void playSound(const std::string& path) override {
        // 播放音效...
    }
    
    void stopAll() override {
        // 停止所有音效...
    }
};

// 注册服务
Application::get().registerService<IAudioService>(makeShared<AudioService>());

// 使用服务
auto audio = Application::get().getService<IAudioService>();
audio->playSound("explosion.wav");
```

### 测试时注入 Mock 服务

```cpp
// 创建 Mock 服务
class MockSceneService : public ISceneService {
public:
    std::vector<std::string> sceneHistory;
    
    void pushScene(Ptr<Scene> scene) override {
        sceneHistory.push_back("push:" + scene->getName());
    }
    
    void popScene() override {
        sceneHistory.push_back("pop");
    }
    
    // 实现其他必要方法...
};

// 测试代码
void testSceneNavigation() {
    // 注入 Mock 服务
    auto mockService = makeShared<MockSceneService>();
    ServiceLocator::instance().registerService<ISceneService>(mockService);
    
    // 执行测试
    auto sceneService = ServiceLocator::instance().getService<ISceneService>();
    sceneService->pushScene(createTestScene("level1"));
    sceneService->pushScene(createTestScene("level2"));
    sceneService->popScene();
    
    // 验证结果
    assert(mockService->sceneHistory.size() == 3);
    assert(mockService->sceneHistory[0] == "push:level1");
    assert(mockService->sceneHistory[1] == "push:level2");
    assert(mockService->sceneHistory[2] == "pop");
}
```

---

## 创建新模块

### 步骤 1：定义配置类

```cpp
class MyModuleConfig : public IModuleConfig {
public:
    int someSetting = 42;
    
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.name = "MyModule";
        info.version = "1.0.0";
        info.priority = ModulePriority::Graphics;
        info.enabled = true;
        return info;
    }
    
    std::string getConfigSectionName() const override { return "my_module"; }
    bool validate() const override { return someSetting > 0; }
    void resetToDefaults() override { someSetting = 42; }
    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};
```

### 步骤 2：定义初始化器

```cpp
class MyModuleInitializer : public IModuleInitializer {
public:
    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Graphics; }
    std::vector<ModuleId> getDependencies() const override { return {}; }
    
    bool initialize(const IModuleConfig* config) override {
        if (initialized_) return true;
        
        const MyModuleConfig* cfg = dynamic_cast<const MyModuleConfig*>(config);
        if (!cfg) return false;
        
        // 执行初始化逻辑...
        
        initialized_ = true;
        E2D_LOG_INFO("MyModule initialized");
        return true;
    }
    
    void shutdown() override {
        if (!initialized_) return;
        // 执行清理逻辑...
        initialized_ = false;
        E2D_LOG_INFO("MyModule shutdown");
    }
    
    bool isInitialized() const override { return initialized_; }
    void setModuleId(ModuleId id) { moduleId_ = id; }

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
};
```

### 步骤 3：注册模块

```cpp
// my_module.cpp
#include <extra2d/config/module_registry.h>

namespace extra2d {

static ModuleId s_myModuleId = INVALID_MODULE_ID;

ModuleId get_my_module_id() { return s_myModuleId; }

void register_my_module() {
    if (s_myModuleId != INVALID_MODULE_ID) return;
    
    s_myModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<MyModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<MyModuleInitializer>();
            initializer->setModuleId(s_myModuleId);
            return initializer;
        }
    );
}

} // namespace extra2d
```

---

## 内置模块

### Logger 模块

**职责**：管理日志系统初始化和关闭

**配置**：
```cpp
LoggerModuleConfig config;
config.logLevel = LogLevel::Info;
config.consoleOutput = true;
config.fileOutput = true;
config.logFilePath = "app.log";
```

---

### Config 模块

**职责**：管理 ConfigManager 和应用配置

**配置**：
```cpp
ConfigModuleConfig config;
config.configPath = "config.json";
config.appConfig = AppConfig::createDefault();
```

---

### Platform 模块

**职责**：平台检测和平台特定初始化

**平台特定操作**：
- Switch：初始化 romfs 和 socket

---

### Window 模块

**职责**：窗口创建和后端管理

**后端支持**：
- SDL2：跨平台
- GLFW：跨平台
- Switch：原生

**配置**：
```cpp
WindowModuleConfig config;
config.windowConfig.title = "My App";
config.windowConfig.width = 1280;
config.windowConfig.height = 720;
config.windowConfig.vsync = true;
```

---

### Render 模块

**职责**：渲染器初始化和管理

**配置**：
```cpp
RenderModuleConfig config;
config.backend = BackendType::OpenGL;
config.vsync = true;
config.targetFPS = 60;
config.multisamples = 4;
```

---

## 配置文件格式

模块配置使用 JSON 格式：

```json
{
    "logger": {
        "logLevel": 2,
        "consoleOutput": true,
        "fileOutput": false
    },
    "window": {
        "title": "My Application",
        "width": 1280,
        "height": 720,
        "vsync": true
    },
    "render": {
        "targetFPS": 60,
        "multisamples": 4
    }
}
```

---

## 最佳实践

### 1. 模块用于平台初始化，服务用于运行时功能

```cpp
// 模块：平台级初始化
class WindowModule : public IModuleInitializer {
    bool initialize(const IModuleConfig* config) override {
        // 创建窗口、初始化 OpenGL 上下文
    }
};

// 服务：运行时功能
class SceneService : public ISceneService {
    void update(float deltaTime) override {
        // 每帧更新场景
    }
};
```

### 2. 通过服务定位器解耦依赖

```cpp
// 好的做法：通过服务定位器获取依赖
void MyNode::onUpdate(float dt) {
    auto camera = ServiceLocator::instance().getService<ICameraService>();
    auto pos = camera->screenToWorld(mousePos);
}

// 不好的做法：直接依赖 Application
void MyNode::onUpdate(float dt) {
    auto& camera = Application::get().camera();  // 耦合度高
}
```

### 3. 使用接口便于测试

```cpp
// 定义接口
class IAudioService : public IService { ... };

// 生产环境使用真实实现
auto audio = makeShared<AudioService>();

// 测试环境使用 Mock
auto audio = makeShared<MockAudioService>();
```

### 4. 安全关闭

```cpp
void shutdown() override {
    if (!initialized_) return; // 避免重复关闭
    
    // 清理资源
    resource_.reset();
    
    setState(ServiceState::Stopped);
    initialized_ = false;
}
```

---

## 调试

### 查看模块初始化顺序

```cpp
auto order = ModuleRegistry::instance().getInitializationOrder();
for (ModuleId id : order) {
    auto* config = ModuleRegistry::instance().getModuleConfig(id);
    if (config) {
        auto info = config->getModuleInfo();
        E2D_LOG_INFO("Module: {} (priority: {})", 
                     info.name, static_cast<int>(info.priority));
    }
}
```

### 查看服务状态

```cpp
auto services = ServiceLocator::instance().getAllServices();
for (const auto& service : services) {
    auto info = service->getServiceInfo();
    E2D_LOG_INFO("Service: {} (state: {})", 
                 info.name, static_cast<int>(info.state));
}
```

---

## 示例

完整示例请参考：
- [examples/basic/main.cpp](../../examples/basic/main.cpp) - 基础示例
- [Extra2D/src/services/scene_service.cpp](../../Extra2D/src/services/scene_service.cpp) - Scene 服务实现
- [Extra2D/src/services/event_service.cpp](../../Extra2D/src/services/event_service.cpp) - Event 服务实现
- [Extra2D/src/platform/window_module.cpp](../../Extra2D/src/platform/window_module.cpp) - Window 模块实现
- [Extra2D/src/graphics/render_module.cpp](../../Extra2D/src/graphics/render_module.cpp) - Render 模块实现
