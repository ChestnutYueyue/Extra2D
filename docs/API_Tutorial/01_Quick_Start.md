# 01. 快速开始

本教程将带你快速上手 Extra2D 引擎，通过一个简单的 Hello World 示例了解引擎的基本使用方法。

## 示例代码

完整示例位于 `examples/hello_world/main.cpp`：

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

// ============================================================================
// Hello World 场景
// ============================================================================

/**
 * @brief Hello World 场景类
 * 显示简单的 "Hello World" 文字
 */
class HelloWorldScene : public Scene {
public:
  /**
   * @brief 场景进入时调用
   */
  void onEnter() override {
    E2D_LOG_INFO("HelloWorldScene::onEnter - 进入场景");

    // 设置背景颜色为深蓝色
    setBackgroundColor(Color(0.1f, 0.1f, 0.3f, 1.0f));

    // 加载字体（支持多种字体后备）
    auto &resources = Application::instance().resources();
    font_ = resources.loadFont("assets/font.ttf", 48, true);

    if (!font_) {
      E2D_LOG_ERROR("字体加载失败，文字渲染将不可用！");
      return;
    }

    // 创建 "你好世界" 文本组件 - 使用屏幕空间（固定位置，不随相机移动）
    auto text1 = Text::create("你好世界", font_);
    text1->withCoordinateSpace(CoordinateSpace::Screen)
          ->withScreenPosition(640.0f, 360.0f)  // 屏幕中心
          ->withAnchor(0.5f, 0.5f)  // 中心锚点，让文字中心对准位置
          ->withTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    addChild(text1);

    // 创建提示文本组件 - 使用屏幕空间，固定在屏幕底部
    auto text2 = Text::create("退出按键（START 按钮）", font_);
    text2->withCoordinateSpace(CoordinateSpace::Screen)
          ->withScreenPosition(640.0f, 650.0f)  // 屏幕底部
          ->withAnchor(0.5f, 0.5f)
          ->withTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));
    addChild(text2);
  }

  /**
   * @brief 每帧更新时调用
   * @param dt 时间间隔（秒）
   */
  void onUpdate(float dt) override {
    Scene::onUpdate(dt);

    // 检查退出按键
    auto &input = Application::instance().input();

    // 使用手柄 START 按钮退出 (GamepadButton::Start)
    if (input.isButtonPressed(GamepadButton::Start)) {
      E2D_LOG_INFO("退出应用 (START 按钮)");
      Application::instance().quit();
    }
  }

private:
  Ptr<FontAtlas> font_; // 字体图集
};

// ============================================================================
// 程序入口
// ============================================================================

int main(int argc, char **argv)
{
  // 初始化日志系统
  Logger::init();
  Logger::setLevel(LogLevel::Debug);

  // 获取应用实例
  auto &app = Application::instance();

  // 配置应用
  AppConfig config;
  config.title = "Easy2D - Hello World";
  config.width = 1280;
  config.height = 720;
  config.vsync = true;
  config.fpsLimit = 60;

  // 初始化应用
  if (!app.init(config)) {
    E2D_LOG_ERROR("应用初始化失败！");
    return -1;
  }

  // 进入 Hello World 场景
  app.enterScene(makePtr<HelloWorldScene>());

  // 运行应用
  app.run();

  return 0;
}
```

## 核心概念

### 1. 应用生命周期

Extra2D 应用遵循以下生命周期：

```
初始化 (Application::init)
    ↓
进入场景 (enterScene)
    ↓
主循环 (run) → 更新 (onUpdate) → 渲染 (onRender)
    ↓
退出 (quit)
```

### 2. 场景系统

场景是游戏内容的容器，通过继承 `Scene` 类并重写以下方法：

| 方法 | 说明 |
|------|------|
| `onEnter()` | 场景进入时调用，用于初始化资源 |
| `onExit()` | 场景退出时调用，用于清理资源 |
| `onUpdate(dt)` | 每帧更新时调用，用于处理游戏逻辑 |
| `onRender(renderer)` | 渲染时调用，用于自定义绘制 |

### 3. 坐标空间

Extra2D 支持三种坐标空间：

```cpp
// 屏幕空间 - 固定位置，不随相机移动
text->withCoordinateSpace(CoordinateSpace::Screen)
      ->withScreenPosition(640.0f, 360.0f);

// 相机空间 - 跟随相机但保持相对偏移
text->withCoordinateSpace(CoordinateSpace::Camera)
      ->withCameraOffset(50.0f, 50.0f);

// 世界空间 - 随相机移动（默认行为）
text->withCoordinateSpace(CoordinateSpace::World)
      ->withPosition(100.0f, 100.0f);
```

### 4. 输入处理

支持手柄输入检测：

```cpp
auto &input = Application::instance().input();

// 检测按键按下（持续触发）
if (input.isButtonDown(GamepadButton::A)) { }

// 检测按键按下（单次触发）
if (input.isButtonPressed(GamepadButton::A)) { }

// 检测按键释放
if (input.isButtonReleased(GamepadButton::A)) { }
```

常用按键：
- `GamepadButton::A` - A 键
- `GamepadButton::B` - B 键
- `GamepadButton::X` - X 键
- `GamepadButton::Y` - Y 键
- `GamepadButton::Start` - + 键 (Switch)
- `GamepadButton::DPadUp/Down/Left/Right` - 方向键

### 5. 资源加载

通过资源管理器加载字体、纹理等资源：

```cpp
auto &resources = Application::instance().resources();

// 加载字体
auto font = resources.loadFont("assets/font.ttf", 48, true);

// 加载纹理
auto texture = resources.loadTexture("assets/image.png");
```

### 6. 日志系统

使用宏进行日志输出：

```cpp
E2D_LOG_DEBUG("调试信息");
E2D_LOG_INFO("普通信息");
E2D_LOG_WARN("警告信息");
E2D_LOG_ERROR("错误信息");
```

## 下一步

- [02. 场景系统](./02_Scene_System.md) - 深入了解场景管理
- [03. 节点系统](./03_Node_System.md) - 学习节点和精灵的使用
