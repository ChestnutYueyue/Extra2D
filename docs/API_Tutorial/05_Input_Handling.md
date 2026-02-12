# 05. 输入处理

Extra2D 提供了统一的输入处理系统，支持手柄、键盘等多种输入设备。

## 输入管理器

通过 `Application::instance().input()` 访问输入管理器：

```cpp
auto& input = Application::instance().input();
```

## 按键检测

### 检测方法

```cpp
// 按键是否按下（持续触发）
if (input.isButtonDown(GamepadButton::A)) {
    // 每帧都会触发，只要按键保持按下
}

// 按键是否刚按下（单次触发）
if (input.isButtonPressed(GamepadButton::A)) {
    // 只在按下瞬间触发一次
}

// 按键是否刚释放
if (input.isButtonReleased(GamepadButton::A)) {
    // 只在释放瞬间触发一次
}
```

### 常用按键

| 按键 | 说明 | Xbox 对应 | Switch 对应 |
|------|------|-----------|-------------|
| `GamepadButton::A` | A 键 | A 键 | A 键 |
| `GamepadButton::B` | B 键 | B 键 | B 键 |
| `GamepadButton::X` | X 键 | X 键 | X 键 |
| `GamepadButton::Y` | Y 键 | Y 键 | Y 键 |
| `GamepadButton::Start` | 开始键 | Menu 键 | + 键 |
| `GamepadButton::Back` | 返回键 | View 键 | - 键 |
| `GamepadButton::Guide` | 导航键 | Xbox 键 | Home 键 |
| `GamepadButton::DPadUp` | 方向上 | 方向键上 | 方向键上 |
| `GamepadButton::DPadDown` | 方向下 | 方向键下 | 方向键下 |
| `GamepadButton::DPadLeft` | 方向左 | 方向键左 | 方向键左 |
| `GamepadButton::DPadRight` | 方向右 | 方向键右 | 方向键右 |
| `GamepadButton::LeftStick` | 左摇杆按下 | L3 | L3 |
| `GamepadButton::RightStick` | 右摇杆按下 | R3 | R3 |
| `GamepadButton::LeftShoulder` | 左肩键 | LB | L |
| `GamepadButton::RightShoulder` | 右肩键 | RB | R |
| `GamepadButton::LeftTrigger` | 左扳机键 | LT | ZL |
| `GamepadButton::RightTrigger` | 右扳机键 | RT | ZR |

## 摇杆输入

### 获取摇杆值

```cpp
// 获取左摇杆位置（范围 -1.0 到 1.0）
Vec2 leftStick = input.getLeftStick();

// 获取右摇杆位置
Vec2 rightStick = input.getRightStick();

// 应用摇杆输入
float speed = 200.0f;
player->setPosition(player->getPosition() + leftStick * speed * dt);
```

### 摇杆死区

```cpp
// 设置摇杆死区（默认 0.15）
input.setStickDeadZone(0.2f);
```

## 完整示例

### 菜单导航

参考 `examples/flappy_bird/StartScene.cpp`：

```cpp
void StartScene::onUpdate(float dt) {
    Scene::onUpdate(dt);

    auto& input = Application::instance().input();

    // A 键开始游戏
    if (input.isButtonPressed(GamepadButton::A)) {
        ResLoader::playMusic(MusicType::Click);
        startGame();
    }

    // BACK 键退出游戏
    if (input.isButtonPressed(GamepadButton::Back)) {
        ResLoader::playMusic(MusicType::Click);
        auto &app = Application::instance();
        app.quit();
    }
}
```

### Game Over 界面手柄控制

参考 `examples/flappy_bird/GameOverLayer.cpp`：

```cpp
void GameOverLayer::onUpdate(float dt) {
    Node::onUpdate(dt);

    // 检测手柄按键
    auto &input = extra2d::Application::instance().input();
    
    // A 键重新开始游戏
    if (input.isButtonPressed(extra2d::GamepadButton::A)) {
        ResLoader::playMusic(MusicType::Click);
        auto &app = extra2d::Application::instance();
        app.scenes().replaceScene(extra2d::makePtr<GameScene>(),
                                  extra2d::TransitionType::Fade, 0.5f);
    }
    
    // B 键返回主菜单
    if (input.isButtonPressed(extra2d::GamepadButton::B)) {
        ResLoader::playMusic(MusicType::Click);
        auto &app = extra2d::Application::instance();
        app.scenes().replaceScene(extra2d::makePtr<StartScene>(),
                                  extra2d::TransitionType::Fade, 0.5f);
    }
}
```

### 玩家移动

```cpp
void Player::update(float dt) {
    auto& input = Application::instance().input();
    
    Vec2 moveDir;
    
    // 方向键移动
    if (input.isButtonDown(GamepadButton::DPadLeft)) {
        moveDir.x -= 1;
    }
    if (input.isButtonDown(GamepadButton::DPadRight)) {
        moveDir.x += 1;
    }
    if (input.isButtonDown(GamepadButton::DPadUp)) {
        moveDir.y -= 1;
    }
    if (input.isButtonDown(GamepadButton::DPadDown)) {
        moveDir.y += 1;
    }
    
    // 摇杆移动
    Vec2 stick = input.getLeftStick();
    if (stick.length() > 0.1f) {
        moveDir = stick;
    }
    
    // 应用移动
    if (moveDir.length() > 0) {
        moveDir.normalize();
        setPosition(getPosition() + moveDir * speed_ * dt);
    }
    
    // 跳跃
    if (input.isButtonPressed(GamepadButton::A)) {
        jump();
    }
}
```

## 输入映射

### 自定义按键映射

```cpp
// 定义动作
enum class Action {
    Jump,
    Attack,
    Pause
};

// 映射按键到动作
std::unordered_map<Action, GamepadButton> actionMap = {
    {Action::Jump, GamepadButton::A},
    {Action::Attack, GamepadButton::B},
    {Action::Pause, GamepadButton::Start}
};

// 检查动作
bool isActionPressed(Action action) {
    auto& input = Application::instance().input();
    return input.isButtonPressed(actionMap[action]);
}
```

## 最佳实践

1. **使用 isButtonPressed 进行菜单操作** - 避免持续触发
2. **使用 isButtonDown 进行移动控制** - 实现流畅移动
3. **支持多种输入方式** - 同时支持方向键和摇杆
4. **添加输入缓冲** - 提升操作手感
5. **为常用操作分配标准按键**：
   - **A 键**：确认、跳跃、主要动作
   - **B 键**：取消、返回、次要动作
   - **Start 键**：暂停菜单
   - **Back 键**：返回上一级/退出

## 下一步

- [06. 碰撞检测](./06_Collision_Detection.md) - 学习碰撞检测系统
- [07. UI 系统](./07_UI_System.md) - 学习 UI 控件使用
