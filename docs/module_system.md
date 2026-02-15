# Extra2D 模块系统

## 概述

Extra2D 采用模块化架构设计，所有核心功能都通过模块系统管理。模块系统提供：

- **统一的生命周期管理**：初始化、关闭、依赖处理
- **优先级排序**：确保模块按正确顺序初始化
- **配置驱动**：每个模块可独立配置
- **解耦设计**：Application 只协调模块，不直接管理任何依赖

## 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                      Application                             │
│  (只负责协调模块，不直接管理任何依赖)                          │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    ModuleRegistry                            │
│  (模块注册表，管理所有模块的生命周期)                          │
└─────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│ Logger Module │   │ Config Module │   │Platform Module│
│   (日志系统)   │   │  (配置管理)    │   │  (平台初始化)  │
└───────────────┘   └───────────────┘   └───────────────┘
        │                     │                     │
        └─────────────────────┼─────────────────────┘
                              ▼
                    ┌───────────────┐
                    │ Window Module │
                    │  (窗口管理)    │
                    └───────────────┘
                              │
                              ▼
                    ┌───────────────┐
                    │ Render Module │
                    │  (渲染系统)    │
                    └───────────────┘
```

## 模块优先级

模块按优先级从小到大初始化，关闭时逆序执行：

| 优先级值 | 枚举名称 | 用途 | 模块示例 |
|---------|---------|------|---------|
| 0 | `Core` | 核心模块，最先初始化 | Logger, Config, Platform, Window |
| 50 | `Input` | 输入处理 | Input |
| 100 | `Graphics` | 图形渲染 | Render |
| 200 | `Audio` | 音频系统 | Audio |
| 300 | `Physics` | 物理系统 | Physics |
| 400 | `Gameplay` | 游戏逻辑 | Gameplay |
| 500 | `UI` | 用户界面 | UI |

## 核心接口

### IModuleConfig

模块配置接口，定义模块的元数据和配置：

```cpp
class IModuleConfig {
public:
    virtual ~IModuleConfig() = default;
    
    // 模块信息
    virtual ModuleInfo getModuleInfo() const = 0;
    
    // 配置管理
    virtual std::string getConfigSectionName() const = 0;
    virtual bool validate() const = 0;
    virtual void resetToDefaults() = 0;
    virtual bool loadFromJson(const void* jsonData) = 0;
    virtual bool saveToJson(void* jsonData) const = 0;
    
    // 平台约束
    virtual void applyPlatformConstraints(PlatformType platform) {}
};
```

### IModuleInitializer

模块初始化器接口，管理模块的生命周期：

```cpp
class IModuleInitializer {
public:
    virtual ~IModuleInitializer() = default;
    
    // 模块标识
    virtual ModuleId getModuleId() const = 0;
    virtual ModulePriority getPriority() const = 0;
    virtual std::vector<ModuleId> getDependencies() const = 0;
    
    // 生命周期
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
    
    // 注册模块
    ModuleId registerModule(
        UniquePtr<IModuleConfig> config,
        ModuleInitializerFactory factory
    );
    
    // 获取模块
    IModuleConfig* getModuleConfig(ModuleId id);
    IModuleInitializer* getInitializer(ModuleId id);
    
    // 初始化顺序
    std::vector<ModuleId> getInitializationOrder() const;
    
    // 查询
    bool hasModule(const std::string& name) const;
    std::vector<std::string> getModuleNames() const;
};
```

## 创建新模块

### 步骤 1：定义配置类

```cpp
// my_module.h
class MyModuleConfig : public IModuleConfig {
public:
    int someSetting = 42;
    std::string somePath;
    
    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.name = "MyModule";
        info.version = "1.0.0";
        info.priority = ModulePriority::Gameplay;
        info.enabled = true;
        return info;
    }
    
    std::string getConfigSectionName() const override {
        return "my_module";
    }
    
    bool validate() const override {
        return someSetting > 0;
    }
    
    void resetToDefaults() override {
        someSetting = 42;
        somePath.clear();
    }
    
    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};
```

### 步骤 2：定义初始化器

```cpp
// my_module.h
class MyModuleInitializer : public IModuleInitializer {
public:
    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Gameplay; }
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

ModuleId get_my_module_id() {
    return s_myModuleId;
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

// 自动注册
namespace {
    struct MyModuleAutoRegister {
        MyModuleAutoRegister() {
            register_my_module();
        }
    };
    
    static MyModuleAutoRegister s_autoRegister;
}

} // namespace extra2d
```

### 步骤 4：在 Application 中使用

```cpp
// application.cpp
#include <extra2d/my_module.h>

bool Application::init(const AppConfig& config) {
    // 注册模块
    register_my_module();
    
    // 初始化所有模块
    return initModules();
}
```

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

**平台支持**：
- Windows：彩色控制台输出
- Linux/macOS：ANSI 彩色输出
- Switch：libnx console

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

**配置**：
```cpp
PlatformModuleConfig config;
config.targetPlatform = PlatformType::Auto; // 自动检测
```

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
config.backend = "sdl2";
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

## 模块依赖

模块可以声明依赖关系：

```cpp
std::vector<ModuleId> getDependencies() const override {
    return { get_window_module_id(), get_config_module_id() };
}
```

ModuleRegistry 会自动解析依赖并按正确顺序初始化。

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
        "backend": "sdl2",
        "title": "My Application",
        "width": 1280,
        "height": 720,
        "vsync": true
    },
    "render": {
        "backend": "opengl",
        "targetFPS": 60,
        "multisamples": 4
    }
}
```

## 最佳实践

### 1. 单一职责

每个模块只负责一个明确的功能领域：

```
✅ Logger Module → 只管理日志
✅ Window Module → 只管理窗口
❌ Game Module → 管理输入、渲染、音频（职责过多）
```

### 2. 依赖最小化

模块应尽量减少对其他模块的依赖：

```cpp
// 好的做法：通过接口获取信息
std::vector<ModuleId> getDependencies() const override {
    return { get_window_module_id() }; // 只依赖窗口
}

// 不好的做法：依赖过多
std::vector<ModuleId> getDependencies() const override {
    return { 
        get_window_module_id(),
        get_config_module_id(),
        get_logger_module_id(),
        get_render_module_id()
    };
}
```

### 3. 延迟初始化

资源在需要时才初始化，不是在构造函数中：

```cpp
bool initialize(const IModuleConfig* config) override {
    if (initialized_) return true;
    
    // 在这里初始化资源
    resource_ = createResource();
    
    initialized_ = true;
    return true;
}
```

### 4. 安全关闭

关闭时要检查状态，避免重复关闭：

```cpp
void shutdown() override {
    if (!initialized_) return; // 避免重复关闭
    
    // 清理资源
    resource_.reset();
    
    initialized_ = false;
}
```

### 5. 使用日志

在关键操作处添加日志：

```cpp
bool initialize(const IModuleConfig* config) override {
    E2D_LOG_INFO("Initializing MyModule...");
    
    if (!doSomething()) {
        E2D_LOG_ERROR("Failed to do something");
        return false;
    }
    
    E2D_LOG_INFO("MyModule initialized successfully");
    return true;
}
```

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

### 检查模块状态

```cpp
auto* initializer = ModuleRegistry::instance().getInitializer(moduleId);
if (initializer && initializer->isInitialized()) {
    E2D_LOG_INFO("Module is initialized");
}
```

## 示例

完整示例请参考：
- [examples/basic/main.cpp](../../examples/basic/main.cpp) - 基础示例
- [Extra2D/src/platform/window_module.cpp](../../Extra2D/src/platform/window_module.cpp) - Window 模块实现
- [Extra2D/src/graphics/render_module.cpp](../../Extra2D/src/graphics/render_module.cpp) - Render 模块实现
