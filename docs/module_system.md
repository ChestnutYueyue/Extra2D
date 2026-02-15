# Extra2D 模块系统

## 概述

Extra2D 采用模块化架构设计，所有核心功能通过模块系统和服务系统管理。系统提供：

- **统一的生命周期管理**：初始化、关闭、依赖处理
- **优先级排序**：确保模块/服务按正确顺序初始化
- **模块化配置**：每个模块独立管理自己的配置
- **依赖注入**：通过服务定位器解耦模块间依赖
- **可扩展性**：新增模块无需修改引擎核心代码

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
```

## 模块化配置系统

### 设计原则

Extra2D 采用**模块化配置系统**，遵循开闭原则：

- **AppConfig** 只包含应用级别配置（appName, appVersion, organization 等）
- **各模块配置** 由模块自己管理，实现 `IModuleConfig` 接口
- **新增模块** 无需修改引擎核心代码

### 配置文件结构

```
Extra2D/include/extra2d/
├── config/
│   ├── app_config.h          # 应用级别配置
│   ├── module_config.h       # 模块配置接口
│   └── config_manager.h      # 配置管理器
├── platform/
│   ├── window_config.h       # 窗口模块配置
│   └── input_config.h        # 输入模块配置
├── graphics/
│   └── render_config.h       # 渲染模块配置
├── audio/
│   └── audio_config.h        # 音频模块配置
├── debug/
│   └── debug_config.h        # 调试模块配置
└── resource/
    └── resource_config.h     # 资源模块配置
```

### AppConfig 结构

```cpp
struct AppConfig {
    std::string appName = "Extra2D App";
    std::string appVersion = "1.0.0";
    std::string organization = "";
    std::string configFile = "config.json";
    PlatformType targetPlatform = PlatformType::Auto;
    
    static AppConfig createDefault();
    bool validate() const;
    void reset();
    void merge(const AppConfig& other);
};
```

### 模块配置示例

```cpp
// window_config.h
struct WindowConfigData {
    std::string title = "Extra2D Application";
    int width = 1280;
    int height = 720;
    WindowMode mode = WindowMode::Windowed;
    bool vsync = true;
    bool resizable = true;
    // ...
};

// window_module.h
class WindowModuleConfig : public IModuleConfig {
public:
    WindowConfigData windowConfig;
    
    ModuleInfo getModuleInfo() const override;
    std::string getConfigSectionName() const override { return "window"; }
    bool validate() const override;
    void applyPlatformConstraints(PlatformType platform) override;
    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};
```

---

## 模块 vs 服务

| 特性 | 模块 (Module) | 服务 (Service) |
|-----|--------------|---------------|
| 用途 | 平台级初始化 | 运行时功能 |
| 生命周期 | Application 管理 | ServiceLocator 管理 |
| 配置管理 | 独立配置文件 | 无配置 |
| 依赖方式 | 通过 ModuleRegistry | 通过 ServiceLocator |
| 可替换性 | 编译时确定 | 运行时可替换 |
| 示例 | Window, Render, Input | Scene, Timer, Event, Camera |

## 模块优先级

模块按优先级从小到大初始化，关闭时逆序执行：

| 优先级值 | 枚举名称 | 用途 | 模块示例 |
|---------|---------|------|---------|
| 0 | `Core` | 核心模块，最先初始化 | Config, Platform, Window |
| 50 | `Input` | 输入系统 | Input |
| 100 | `Graphics` | 图形渲染 | Render |
| 200 | `Audio` | 音频系统 | Audio |
| 500 | `Resource` | 资源管理 | Resource |

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
    std::vector<ModuleId> getAllModules() const;
    std::vector<ModuleId> getInitializationOrder() const;
};
```

---

## 创建新模块

### 步骤 1：定义配置数据结构

```cpp
// my_module_config.h
#pragma once

#include <string>

namespace extra2d {

struct MyModuleConfigData {
    int someSetting = 42;
    bool enabled = true;
    std::string path = "default";
};

} 
```

### 步骤 2：定义配置类

```cpp
// my_module.h
#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include "my_module_config.h"

namespace extra2d {

class MyModuleConfig : public IModuleConfig {
public:
    MyModuleConfigData config;
    
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
        info.name = "MyModule";
        info.version = "1.0.0";
        info.priority = ModulePriority::Graphics;
        info.enabled = true;
        return info;
    }
    
    std::string getConfigSectionName() const override { return "my_module"; }
    bool validate() const override { return config.someSetting > 0; }
    
    void resetToDefaults() override {
        config = MyModuleConfigData{};
    }
    
    void applyPlatformConstraints(PlatformType platform) override {
#ifdef __SWITCH__
        config.someSetting = 30;  // Switch 平台优化
#else
        (void)platform;
#endif
    }
    
    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};

} 
```

### 步骤 3：定义初始化器

```cpp
// my_module.h (续)
class MyModuleInitializer : public IModuleInitializer {
public:
    MyModuleInitializer();
    ~MyModuleInitializer() override;
    
    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Graphics; }
    std::vector<ModuleId> getDependencies() const override;
    
    bool initialize(const IModuleConfig* config) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }
    
    void setModuleId(ModuleId id) { moduleId_ = id; }

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
    MyModuleConfigData config_;
};

ModuleId get_my_module_id();
void register_my_module();
```

### 步骤 4：实现模块

```cpp
// my_module.cpp
#include "my_module.h"
#include <extra2d/config/module_registry.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_myModuleId = INVALID_MODULE_ID;

ModuleId get_my_module_id() { return s_myModuleId; }

bool MyModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    try {
        const json& j = *static_cast<const json*>(jsonData);
        if (j.contains("someSetting")) config.someSetting = j["someSetting"].get<int>();
        if (j.contains("enabled")) config.enabled = j["enabled"].get<bool>();
        if (j.contains("path")) config.path = j["path"].get<std::string>();
        return true;
    } catch (...) {
        return false;
    }
}

bool MyModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    try {
        json& j = *static_cast<json*>(jsonData);
        j["someSetting"] = config.someSetting;
        j["enabled"] = config.enabled;
        j["path"] = config.path;
        return true;
    } catch (...) {
        return false;
    }
}

MyModuleInitializer::MyModuleInitializer() : moduleId_(INVALID_MODULE_ID), initialized_(false) {}

MyModuleInitializer::~MyModuleInitializer() { if (initialized_) shutdown(); }

std::vector<ModuleId> MyModuleInitializer::getDependencies() const {
    return {};  // 无依赖
}

bool MyModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const MyModuleConfig* cfg = dynamic_cast<const MyModuleConfig*>(config);
    if (!cfg) {
        E2D_LOG_ERROR("Invalid MyModule config");
        return false;
    }
    
    config_ = cfg->config;
    
    // 执行初始化逻辑...
    
    initialized_ = true;
    E2D_LOG_INFO("MyModule initialized with setting: {}", config_.someSetting);
    return true;
}

void MyModuleInitializer::shutdown() {
    if (!initialized_) return;
    // 执行清理逻辑...
    initialized_ = false;
    E2D_LOG_INFO("MyModule shutdown");
}

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

namespace {
    struct MyModuleAutoRegister {
        MyModuleAutoRegister() { register_my_module(); }
    };
    static MyModuleAutoRegister s_autoRegister;
}

} 
```

---

## 内置模块

### Config 模块

**职责**：管理 ConfigManager 和应用配置

**配置**：
```cpp
AppConfig config;
config.appName = "My Application";
config.appVersion = "1.0.0";
```

---

### Platform 模块

**职责**：平台检测和平台特定初始化

**支持平台**：
- Windows
- Linux
- macOS
- Nintendo Switch

**平台能力查询**：
```cpp
auto* platformConfig = createPlatformConfig();
const auto& caps = platformConfig->capabilities();
if (caps.supportsGamepad) {
    // 支持手柄
}
```

---

### Window 模块

**职责**：窗口创建和管理

**后端**：统一使用 SDL2，支持所有平台

**配置**：
```cpp
WindowConfigData config;
config.title = "My App";
config.width = 1280;
config.height = 720;
config.mode = WindowMode::Windowed;
config.vsync = true;
```

**平台约束**：
- Switch 平台自动强制全屏模式

---

### Input 模块

**职责**：输入设备管理（键盘、鼠标、手柄）

**配置**：
```cpp
InputConfigData config;
config.deadzone = 0.15f;
config.mouseSensitivity = 1.0f;
config.enableVibration = true;
```

**使用示例**：
```cpp
IInput* input = app.window().input();

// 键盘
if (input->pressed(Key::Space)) {
    // 空格键刚按下
}

// 鼠标
if (input->down(Mouse::Left)) {
    Vec2 pos = input->mouse();
}

// 手柄
if (input->gamepad()) {
    Vec2 stick = input->leftStick();
    if (input->pressed(Gamepad::A)) {
        input->vibrate(0.5f, 0.5f);  // 振动反馈
    }
}
```

---

### Render 模块

**职责**：渲染器初始化和管理

**配置**：
```cpp
RenderConfigData config;
config.backend = BackendType::OpenGL;
config.vsync = true;
config.targetFPS = 60;
config.multisamples = 4;
```

---

## 配置文件格式

配置使用 JSON 格式，每个模块有独立的配置节：

```json
{
    "app": {
        "name": "My Application",
        "version": "1.0.0",
        "organization": "MyCompany"
    },
    "window": {
        "title": "My Application",
        "width": 1280,
        "height": 720,
        "mode": "windowed",
        "vsync": true
    },
    "render": {
        "targetFPS": 60,
        "multisamples": 4
    },
    "input": {
        "deadzone": 0.15,
        "mouseSensitivity": 1.0,
        "enableVibration": true
    }
}
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
    virtual void update(float deltaTime);
    virtual bool isInitialized() const;
    
    ServiceState getState() const;
    const std::string& getName() const;
};
```

### 内置服务

| 服务 | 用途 | 优先级 |
|-----|------|-------|
| SceneService | 场景管理 | 300 |
| TimerService | 计时器 | 200 |
| EventService | 事件分发 | 100 |
| CameraService | 相机系统 | 400 |

### 使用服务

```cpp
// 获取服务
auto sceneService = Application::get().scenes();
auto timerService = Application::get().timers();
auto eventService = Application::get().events();

// 使用场景服务
sceneService->pushScene(myScene);

// 使用计时器服务
timerService->addTimer(1.0f, []() {
    E2D_LOG_INFO("Timer fired!");
});

// 使用事件服务
eventService->addListener(EventType::KeyPressed, [](Event& e) {
    auto& keyEvent = std::get<KeyEvent>(e.data);
    E2D_LOG_INFO("Key pressed: {}", keyEvent.keyCode);
});
```

---

## 输入事件系统

### 事件类型

```cpp
enum class EventType {
    // 键盘
    KeyPressed,
    KeyReleased,
    KeyRepeat,
    
    // 鼠标
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
    
    // 手柄
    GamepadConnected,
    GamepadDisconnected,
    GamepadButtonPressed,
    GamepadButtonReleased,
    
    // 触摸
    TouchBegan,
    TouchMoved,
    TouchEnded,
    
    // 窗口
    WindowResize,
    WindowClose,
    // ...
};
```

### 事件监听

```cpp
auto eventService = Application::get().events();

// 监听键盘事件
eventService->addListener(EventType::KeyPressed, [](Event& e) {
    auto& key = std::get<KeyEvent>(e.data);
    E2D_LOG_INFO("Key: {}, mods: {}", key.keyCode, key.mods);
});

// 监听鼠标事件
eventService->addListener(EventType::MouseButtonPressed, [](Event& e) {
    auto& mouse = std::get<MouseButtonEvent>(e.data);
    E2D_LOG_INFO("Mouse button: {} at ({}, {})", 
                 mouse.button, mouse.position.x, mouse.position.y);
});
```

---

## 平台支持

### 支持的平台

| 平台 | 窗口后端 | 图形 API | 特殊处理 |
|-----|---------|---------|---------|
| Windows | SDL2 | OpenGL ES 3.2 | - |
| Linux | SDL2 | OpenGL ES 3.2 | - |
| macOS | SDL2 | OpenGL ES 3.2 | - |
| Nintendo Switch | SDL2 | OpenGL ES 3.2 | romfs, 强制全屏 |

### 平台检测

```cpp
PlatformType platform = PlatformDetector::detect();
const char* name = getPlatformTypeName(platform);

switch (platform) {
    case PlatformType::Windows: // Windows 处理
    case PlatformType::Switch:  // Switch 处理
    // ...
}
```

### 平台能力

```cpp
auto* config = createPlatformConfig();
const auto& caps = config->capabilities();

if (caps.supportsWindowed) { /* 支持窗口模式 */ }
if (caps.supportsGamepad) { /* 支持手柄 */ }
if (caps.supportsTouch) { /* 支持触摸 */ }
```

---

## 最佳实践

### 1. 模块配置独立化

```cpp
// 好的做法：模块管理自己的配置
class WindowModuleConfig : public IModuleConfig {
    WindowConfigData windowConfig;  // 模块内部配置
};

// 不好的做法：所有配置放在 AppConfig
struct AppConfig {
    WindowConfigData window;  // 耦合度高
    RenderConfigData render;
    // ... 新增模块需要修改 AppConfig
};
```

### 2. 使用平台约束

```cpp
void WindowModuleConfig::applyPlatformConstraints(PlatformType platform) {
#ifdef __SWITCH__
    windowConfig.mode = WindowMode::Fullscreen;
    windowConfig.resizable = false;
#else
    (void)platform;
#endif
}
```

### 3. 模块自动注册

```cpp
namespace {
    struct MyModuleAutoRegister {
        MyModuleAutoRegister() { register_my_module(); }
    };
    static MyModuleAutoRegister s_autoRegister;  // 程序启动时自动注册
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

## 场景图系统

### 概述

Extra2D 使用场景图（Scene Graph）管理游戏对象。场景图是一个树形结构，每个节点可以包含子节点，形成层级关系。

### Node 基类

所有场景对象的基类，提供变换、层级管理和渲染功能：

```cpp
class Node : public std::enable_shared_from_this<Node> {
public:
    // 层级管理
    void addChild(Ptr<Node> child);
    void removeChild(Ptr<Node> child);
    void detach();
    void clearChildren();
    
    Ptr<Node> getParent() const;
    const std::vector<Ptr<Node>>& getChildren() const;
    Ptr<Node> findChild(const std::string& name) const;
    
    // 变换属性
    void setPos(const Vec2& pos);
    void setRotation(float degrees);
    void setScale(const Vec2& scale);
    void setAnchor(const Vec2& anchor);
    void setOpacity(float opacity);
    void setVisible(bool visible);
    void setZOrder(int zOrder);
    
    // 世界变换
    Vec2 toWorld(const Vec2& localPos) const;
    Vec2 toLocal(const Vec2& worldPos) const;
    glm::mat4 getLocalTransform() const;
    glm::mat4 getWorldTransform() const;
    
    // 生命周期回调
    virtual void onEnter();
    virtual void onExit();
    virtual void onUpdate(float dt);
    virtual void onRender(RenderBackend& renderer);
};
```

### Scene 类

场景是场景图的根节点，管理相机和视口：

```cpp
class Scene : public Node {
public:
    // 场景属性
    void setBackgroundColor(const Color& color);
    
    // 摄像机
    void setCamera(Ptr<Camera> camera);
    Camera* getActiveCamera() const;
    
    // 视口
    void setViewportSize(float width, float height);
    
    // 渲染和更新
    void renderScene(RenderBackend& renderer);
    void updateScene(float dt);
    
    // 静态创建
    static Ptr<Scene> create();
};
```

### ShapeNode 形状节点

用于绘制几何形状：

```cpp
// 创建形状节点
auto rect = ShapeNode::createFilledRect(Rect(0, 0, 100, 100), Color(1.0f, 0.4f, 0.4f, 1.0f));
auto circle = ShapeNode::createFilledCircle(Vec2(0, 0), 50, Color(0.4f, 0.4f, 1.0f, 1.0f));
auto triangle = ShapeNode::createFilledTriangle(
    Vec2(0, -40), Vec2(-35, 30), Vec2(35, 30),
    Color(0.4f, 1.0f, 0.4f, 1.0f)
);
auto line = ShapeNode::createLine(Vec2(0, 0), Vec2(100, 100), Color(1.0f, 1.0f, 1.0f, 1.0f), 2.0f);
auto polygon = ShapeNode::createFilledPolygon(
    {Vec2(0, -50), Vec2(50, 0), Vec2(0, 50), Vec2(-50, 0)},
    Color(1.0f, 0.4f, 1.0f, 1.0f)
);
```

### 变换继承

子节点继承父节点的变换：

```cpp
auto parent = makeShared<Node>();
parent->setPos(100, 100);
parent->setRotation(45);  // 旋转 45 度

auto child = makeShared<Node>();
child->setPos(50, 0);     // 相对于父节点的位置
parent->addChild(child);

// child 的世界位置 = parent 的变换 * child 的本地位置
// child 会随 parent 一起旋转
```

### 渲染流程

```
Application::render()
  └── CameraService::getViewProjectionMatrix()  // 设置视图投影矩阵
  └── SceneService::render()
        └── Scene::renderContent()
              └── Node::render() (递归)
                    └── pushTransform(localTransform)  // 压入本地变换
                    └── onDraw()                        // 绘制形状
                    └── children::onRender()            // 递归渲染子节点
                    └── popTransform()                  // 弹出变换
```

---

## 视口适配系统

### 概述

视口适配系统确保游戏内容在不同分辨率和宽高比的屏幕上正确显示。

### ViewportAdapter

视口适配器，计算视口位置和缩放：

```cpp
// 视口适配模式
enum class ViewportMode {
    AspectRatio,  // 保持宽高比，可能有黑边
    Stretch,      // 拉伸填满整个窗口
    Center,       // 居中显示，不缩放
    Custom        // 自定义缩放和偏移
};

// 视口配置
struct ViewportConfig {
    float logicWidth = 1920.0f;
    float logicHeight = 1080.0f;
    ViewportMode mode = ViewportMode::AspectRatio;
    Color letterboxColor = Colors::Black;  // 黑边颜色
    float customScale = 1.0f;
    Vec2 customOffset = Vec2::Zero();
};
```

### 使用 CameraService 配置视口

```cpp
auto cameraService = app.camera();
if (cameraService) {
    ViewportConfig vpConfig;
    vpConfig.logicWidth = 1280.0f;   // 逻辑分辨率宽度
    vpConfig.logicHeight = 720.0f;   // 逻辑分辨率高度
    vpConfig.mode = ViewportMode::AspectRatio;  // 保持宽高比
    vpConfig.letterboxColor = Color(0.0f, 0.0f, 0.0f, 1.0f);  // 黑边颜色
    
    cameraService->setViewportConfig(vpConfig);
    cameraService->updateViewport(windowWidth, windowHeight);
    cameraService->applyViewportAdapter();
}
```

### 窗口大小变化处理

当窗口大小变化时，Application 会自动更新视口：

```cpp
// Application 内部处理
window_->onResize([this, cameraService](int width, int height) {
    cameraService->updateViewport(width, height);
    cameraService->applyViewportAdapter();
    
    auto sceneService = ServiceLocator::instance().getService<ISceneService>();
    if (sceneService) {
        auto currentScene = sceneService->getCurrentScene();
        if (currentScene) {
            currentScene->setViewportSize(width, height);
        }
    }
});
```

### 适配模式对比

| 模式 | 描述 | 适用场景 |
|-----|------|---------|
| `AspectRatio` | 保持宽高比，可能有黑边 | 大多数游戏 |
| `Stretch` | 拉伸填满整个窗口 | 不在乎变形的简单游戏 |
| `Center` | 居中显示，不缩放 | 固定分辨率的像素游戏 |
| `Custom` | 自定义缩放和偏移 | 特殊需求 |

---

## 示例

完整示例请参考：
- [examples/basic/main.cpp](../../examples/basic/main.cpp) - 基础示例（场景图、输入事件、视口适配）
- [Extra2D/src/platform/window_module.cpp](../../Extra2D/src/platform/window_module.cpp) - Window 模块实现
- [Extra2D/src/platform/input_module.cpp](../../Extra2D/src/platform/input_module.cpp) - Input 模块实现
- [Extra2D/src/graphics/render_module.cpp](../../Extra2D/src/graphics/render_module.cpp) - Render 模块实现
- [Extra2D/src/scene/node.cpp](../../Extra2D/src/scene/node.cpp) - Node 实现
- [Extra2D/src/scene/shape_node.cpp](../../Extra2D/src/scene/shape_node.cpp) - ShapeNode 实现
