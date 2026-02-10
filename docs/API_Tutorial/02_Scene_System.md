# Extra2D API 教程 - 02. 场景系统

## 场景基础

场景(Scene)是游戏的基本组织单位，负责管理节点和渲染。

### 创建场景

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

class MyScene : public Scene {
public:
    // 场景进入时调用
    void onEnter() override {
        // 必须先调用父类的 onEnter()
        Scene::onEnter();
        
        // 设置背景颜色
        setBackgroundColor(Color(0.1f, 0.1f, 0.3f, 1.0f));
        
        E2D_LOG_INFO("场景已进入");
    }
    
    // 每帧更新时调用
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        // dt 是时间间隔（秒）
    }
    
    // 渲染时调用
    void onRender(RenderBackend &renderer) override {
        Scene::onRender(renderer);
        // 绘制自定义内容
    }
    
    // 场景退出时调用
    void onExit() override {
        // 清理资源
        Scene::onExit();
    }
};
```

### 重要提示

**必须调用 `Scene::onEnter()`**：
```cpp
void onEnter() override {
    Scene::onEnter();  // 必须调用！
    // 你的初始化代码
}
```

如果不调用，会导致：
- `running_` 状态未设置
- 子节点无法正确注册到空间索引
- 碰撞检测失效

## 场景管理

### 进入场景

```cpp
// 进入新场景
app.enterScene(makePtr<MyScene>());

// 替换当前场景（带过渡效果）
app.scenes().replaceScene(
    makePtr<PlayScene>(),
    TransitionType::Fade,  // 淡入淡出
    0.25f                   // 过渡时间（秒）
);
```

### 场景过渡类型

```cpp
enum class TransitionType {
    None,       // 无过渡
    Fade,       // 淡入淡出
    SlideLeft,  // 向左滑动
    SlideRight, // 向右滑动
    SlideUp,    // 向上滑动
    SlideDown   // 向下滑动
};
```

## 场景配置

### 视口设置

```cpp
void onEnter() override {
    Scene::onEnter();
    
    // 设置视口大小（影响坐标系）
    setViewportSize(1280.0f, 720.0f);
    
    // 设置背景颜色
    setBackgroundColor(Colors::Black);
    
    // 启用/禁用空间索引
    setSpatialIndexingEnabled(true);
}
```

### 空间索引

```cpp
// 获取空间管理器
auto &spatialManager = getSpatialManager();

// 切换空间索引策略
spatialManager.setStrategy(SpatialStrategy::QuadTree);   // 四叉树
spatialManager.setStrategy(SpatialStrategy::SpatialHash); // 空间哈希

// 查询所有碰撞
auto collisions = queryCollisions();
```

## 完整示例

```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 设置视口和背景
        setViewportSize(1280.0f, 720.0f);
        setBackgroundColor(Color(0.1f, 0.2f, 0.3f, 1.0f));
        
        // 启用空间索引
        setSpatialIndexingEnabled(true);
        
        E2D_LOG_INFO("游戏场景已加载");
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 检查退出按键
        auto &input = Application::instance().input();
        if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_START)) {
            Application::instance().quit();
        }
    }
    
    void onRender(RenderBackend &renderer) override {
        Scene::onRender(renderer);
        
        // 绘制 FPS
        auto &app = Application::instance();
        std::string fpsText = "FPS: " + std::to_string(app.fps());
        // ...
    }
    
    void onExit() override {
        E2D_LOG_INFO("游戏场景退出");
        Scene::onExit();
    }
};
```

## 下一步

- [03. 节点系统](03_Node_System.md)
- [04. 资源管理](04_Resource_Management.md)
