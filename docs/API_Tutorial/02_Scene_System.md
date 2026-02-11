# 02. 场景系统

Extra2D 的场景系统提供了游戏内容的分层管理和切换功能。本教程将详细介绍场景的生命周期、切换和过渡效果。

## 完整示例

参考 `examples/push_box/` 中的实现：

- `StartScene.h/cpp` - 开始菜单场景
- `PlayScene.h/cpp` - 游戏主场景
- `SuccessScene.h/cpp` - 通关场景

## 场景基础

### 创建场景

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

class GameScene : public Scene {
public:
    void onEnter() override {
        // 必须先调用父类方法
        Scene::onEnter();
        
        // 设置背景色
        setBackgroundColor(Color(0.1f, 0.1f, 0.3f, 1.0f));
        
        // 设置视口大小（用于UI布局）
        setViewportSize(1280.0f, 720.0f);
    }
    
    void onExit() override {
        // 清理资源
        removeAllChildren();
        
        Scene::onExit();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 游戏逻辑更新
    }
};
```

### 场景切换

```cpp
// 进入场景（无过渡）
app.enterScene(makePtr<GameScene>());

// 进入场景（有过渡效果）
app.enterScene(makePtr<GameScene>(), TransitionType::Fade, 0.5f);

// 替换当前场景
app.scenes().replaceScene(makePtr<NewScene>());

// 推入场景（保留当前场景）
app.scenes().pushScene(makePtr<NewScene>());

// 弹出场景（返回上一个场景）
app.scenes().popScene();
```

### 过渡效果类型

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

## 场景管理器

通过 `app.scenes()` 访问场景管理器：

```cpp
auto& scenes = app.scenes();

// 获取当前场景
auto current = scenes.currentScene();

// 获取场景栈深度
size_t depth = scenes.stackDepth();

// 清空场景栈
scenes.clearStack();
```

## 场景生命周期

```
创建场景 (makePtr<Scene>)
    ↓
进入场景 (enterScene)
    ↓
onEnter() - 初始化资源
    ↓
主循环
  ├── onUpdate(dt) - 每帧更新
  └── onRender(renderer) - 每帧渲染
    ↓
退出场景
    ↓
onExit() - 清理资源
    ↓
场景销毁
```

## 推箱子示例场景结构

```
┌─────────────────────────────────────┐
│           StartScene                │
│  ┌─────────────────────────────┐    │
│  │      开始菜单界面            │    │
│  │  - 新游戏                   │    │
│  │  - 继续游戏                 │    │
│  │  - 退出                     │    │
│  └─────────────────────────────┘    │
└──────────────┬──────────────────────┘
               │ 选择"新游戏"
               ↓
┌─────────────────────────────────────┐
│           PlayScene                 │
│  ┌─────────────────────────────┐    │
│  │       游戏主界面             │    │
│  │  - 地图渲染                 │    │
│  │  - 玩家控制                 │    │
│  │  - 关卡信息                 │    │
│  └─────────────────────────────┘    │
└──────────────┬──────────────────────┘
               │ 通关
               ↓
┌─────────────────────────────────────┐
│          SuccessScene               │
│  ┌─────────────────────────────┐    │
│  │       通关界面               │    │
│  │  - 显示成绩                 │    │
│  │  - 下一关/返回菜单          │    │
│  └─────────────────────────────┘    │
└─────────────────────────────────────┘
```

## 代码示例：菜单场景

```cpp
class MenuScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        auto& app = Application::instance();
        auto& resources = app.resources();
        
        // 加载背景
        auto bgTex = resources.loadTexture("assets/bg.jpg");
        if (bgTex) {
            auto bg = Sprite::create(bgTex);
            bg->setAnchor(0.0f, 0.0f);
            addChild(bg);
        }
        
        // 加载字体
        font_ = resources.loadFont("assets/font.ttf", 28, true);
        
        // 创建菜单按钮
        createMenuButtons();
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        auto& input = Application::instance().input();
        
        // 方向键导航
        if (input.isButtonPressed(GamepadButton::DPadUp)) {
            selectedIndex_ = (selectedIndex_ - 1 + menuCount_) % menuCount_;
            updateMenuColors();
        }
        else if (input.isButtonPressed(GamepadButton::DPadDown)) {
            selectedIndex_ = (selectedIndex_ + 1) % menuCount_;
            updateMenuColors();
        }
        
        // A键确认
        if (input.isButtonPressed(GamepadButton::A)) {
            executeMenuItem();
        }
    }
    
private:
    void createMenuButtons() {
        float centerX = 640.0f;
        float startY = 300.0f;
        float spacing = 50.0f;
        
        for (int i = 0; i < menuCount_; ++i) {
            auto btn = Button::create();
            btn->setFont(font_);
            btn->setAnchor(0.5f, 0.5f);
            btn->setPosition(centerX, startY + i * spacing);
            addChild(btn);
            buttons_.push_back(btn);
        }
        
        buttons_[0]->setText("开始游戏");
        buttons_[1]->setText("设置");
        buttons_[2]->setText("退出");
        
        updateMenuColors();
    }
    
    void updateMenuColors() {
        for (int i = 0; i < buttons_.size(); ++i) {
            auto color = (i == selectedIndex_) ? Colors::Red : Colors::White;
            buttons_[i]->setTextColor(color);
        }
    }
    
    void executeMenuItem() {
        switch (selectedIndex_) {
            case 0:
                Application::instance().scenes().replaceScene(
                    makePtr<GameScene>(), TransitionType::Fade, 0.25f);
                break;
            case 1:
                // 打开设置
                break;
            case 2:
                Application::instance().quit();
                break;
        }
    }
    
    Ptr<FontAtlas> font_;
    std::vector<Ptr<Button>> buttons_;
    int selectedIndex_ = 0;
    int menuCount_ = 3;
};
```

## 最佳实践

1. **始终在 onEnter 中调用 Scene::onEnter()** - 确保场景正确初始化
2. **在 onExit 中清理资源** - 避免内存泄漏
3. **使用过渡效果** - 提升用户体验
4. **分离场景逻辑** - 每个场景负责自己的功能

## 下一步

- [03. 节点系统](./03_Node_System.md) - 学习节点和精灵的使用
- [04. 资源管理](./04_Resource_Management.md) - 深入了解资源加载
