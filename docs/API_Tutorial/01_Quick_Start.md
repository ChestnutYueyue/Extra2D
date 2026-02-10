# Extra2D API 教程 - 01. 快速开始

## 简介

Extra2D 是一个跨平台的 2D 游戏引擎，支持 Windows (MinGW) 和 Nintendo Switch 平台。

## 最小示例

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

int main(int argc, char **argv) {
    // 1. 初始化日志系统
    Logger::init();
    Logger::setLevel(LogLevel::Debug);

    // 2. 获取应用实例
    auto &app = Application::instance();

    // 3. 配置应用
    AppConfig config;
    config.title = "My Game";
    config.width = 1280;
    config.height = 720;
    config.vsync = true;
    config.fpsLimit = 60;

    // 4. 初始化应用
    if (!app.init(config)) {
        E2D_LOG_ERROR("应用初始化失败！");
        return -1;
    }

    // 5. 进入场景
    app.enterScene(makePtr<MyScene>());

    // 6. 运行应用
    app.run();

    return 0;
}
```

## 核心概念

### 应用生命周期

```
Logger::init() → Application::init() → enterScene() → run() → 退出
```

### 场景生命周期

```
onEnter() → onUpdate(dt) → onRender() → onExit()
```

## 下一步

- [02. 场景系统](02_Scene_System.md)
- [03. 节点系统](03_Node_System.md)
- [04. 资源管理](04_Resource_Management.md)
- [05. 输入处理](05_Input_Handling.md)
- [06. 碰撞检测](06_Collision_Detection.md)
- [07. UI 系统](07_UI_System.md)
- [08. 音频系统](08_Audio_System.md)
