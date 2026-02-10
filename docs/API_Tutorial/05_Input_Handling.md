# Extra2D API 教程 - 05. 输入处理

## 输入系统

Extra2D 提供统一的输入处理接口，支持键盘和游戏手柄。

### 获取输入管理器

```cpp
auto &input = Application::instance().input();
```

## 游戏手柄输入

Extra2D 提供了 `GamepadButton` 和 `GamepadAxis` 命名空间来映射 SDL 按键。

### 检测按键按下

```cpp
void onUpdate(float dt) override {
    auto &input = Application::instance().input();
    
    // 检测按键按下（每帧只触发一次）
    if (input.isButtonPressed(GamepadButton::A)) {
        // A 键被按下
        jump();
    }
    
    if (input.isButtonPressed(GamepadButton::B)) {
        // B 键被按下
        attack();
    }
}
```

### 检测按键按住

```cpp
void onUpdate(float dt) override {
    auto &input = Application::instance().input();
    
    // 检测按键按住（每帧都触发）
    if (input.isButtonDown(GamepadButton::DPadLeft)) {
        // 左方向键按住
        moveLeft();
    }
    
    if (input.isButtonDown(GamepadButton::DPadRight)) {
        // 右方向键按住
        moveRight();
    }
}
```

### 按键映射表

| Extra2D 枚举 | 对应按键 |
|-------------|----------|
| `GamepadButton::A` | A 键 (Xbox) / × 键 (PlayStation) |
| `GamepadButton::B` | B 键 (Xbox) / ○ 键 (PlayStation) |
| `GamepadButton::X` | X 键 (Xbox) / □ 键 (PlayStation) |
| `GamepadButton::Y` | Y 键 (Xbox) / △ 键 (PlayStation) |
| `GamepadButton::LeftBumper` | 左肩键 (LB/L1) |
| `GamepadButton::RightBumper` | 右肩键 (RB/R1) |
| `GamepadButton::Back` | 返回键 (View/Share) |
| `GamepadButton::Start` | 开始键 (Menu/Options) |
| `GamepadButton::Guide` | 主页键 (Xbox/PS) |
| `GamepadButton::LeftThumb` | 左摇杆按下 (L3) |
| `GamepadButton::RightThumb` | 右摇杆按下 (R3) |
| `GamepadButton::DPadUp` | 方向键上 |
| `GamepadButton::DPadDown` | 方向键下 |
| `GamepadButton::DPadLeft` | 方向键左 |
| `GamepadButton::DPadRight` | 方向键右 |

### PlayStation 风格别名

| Extra2D 枚举 | 对应按键 |
|-------------|----------|
| `GamepadButton::Cross` | A |
| `GamepadButton::Circle` | B |
| `GamepadButton::Square` | X |
| `GamepadButton::Triangle` | Y |

## 摇杆输入

### 获取摇杆值

```cpp
void onUpdate(float dt) override {
    auto &input = Application::instance().input();
    
    // 左摇杆（范围 -1.0 到 1.0）
    float leftX = input.getAxis(GamepadAxis::LeftX);
    float leftY = input.getAxis(GamepadAxis::LeftY);
    
    // 右摇杆
    float rightX = input.getAxis(GamepadAxis::RightX);
    float rightY = input.getAxis(GamepadAxis::RightY);
    
    // 使用摇杆值移动
    if (std::abs(leftX) > 0.1f || std::abs(leftY) > 0.1f) {
        Vec2 velocity(leftX * speed, leftY * speed);
        player->setPosition(player->getPosition() + velocity * dt);
    }
}
```

### 摇杆轴映射表

| Extra2D 枚举 | 说明 |
|-------------|------|
| `GamepadAxis::LeftX` | 左摇杆 X 轴 |
| `GamepadAxis::LeftY` | 左摇杆 Y 轴 |
| `GamepadAxis::RightX` | 右摇杆 X 轴 |
| `GamepadAxis::RightY` | 右摇杆 Y 轴 |
| `GamepadAxis::LeftTrigger` | 左扳机 (LT/L2) |
| `GamepadAxis::RightTrigger` | 右扳机 (RT/R2) |

## 键盘输入

### 检测键盘按键

```cpp
void onUpdate(float dt) override {
    auto &input = Application::instance().input();
    
    // 检测按键按下
    if (input.isKeyPressed(SDLK_SPACE)) {
        jump();
    }
    
    // 检测按键按住
    if (input.isKeyDown(SDLK_LEFT)) {
        moveLeft();
    }
    
    if (input.isKeyDown(SDLK_RIGHT)) {
        moveRight();
    }
}
```

## 完整示例

```cpp
class Player : public Node {
public:
    void onUpdate(float dt) override {
        Node::onUpdate(dt);
        
        auto &input = Application::instance().input();
        Vec2 velocity(0.0f, 0.0f);
        
        // 方向键移动
        if (input.isButtonDown(GamepadButton::DPadLeft)) {
            velocity.x = -speed_;
        } else if (input.isButtonDown(GamepadButton::DPadRight)) {
            velocity.x = speed_;
        }
        
        if (input.isButtonDown(GamepadButton::DPadUp)) {
            velocity.y = -speed_;
        } else if (input.isButtonDown(GamepadButton::DPadDown)) {
            velocity.y = speed_;
        }
        
        // 摇杆移动（如果方向键没有按下）
        if (velocity.x == 0.0f && velocity.y == 0.0f) {
            float axisX = input.getAxis(GamepadAxis::LeftX);
            float axisY = input.getAxis(GamepadAxis::LeftY);
            
            if (std::abs(axisX) > 0.1f) {
                velocity.x = axisX * speed_;
            }
            if (std::abs(axisY) > 0.1f) {
                velocity.y = axisY * speed_;
            }
        }
        
        // 应用移动
        Vec2 pos = getPosition();
        pos = pos + velocity * dt;
        setPosition(pos);
        
        // 动作键
        if (input.isButtonPressed(GamepadButton::A)) {
            jump();
        }
        
        if (input.isButtonPressed(GamepadButton::B)) {
            attack();
        }
        
        // 退出游戏
        if (input.isButtonPressed(GamepadButton::Start)) {
            Application::instance().quit();
        }
    }
    
private:
    float speed_ = 200.0f;
    
    void jump() {
        // 跳跃逻辑
    }
    
    void attack() {
        // 攻击逻辑
    }
};
```

## 下一步

- [06. 碰撞检测](06_Collision_Detection.md)
- [07. UI 系统](07_UI_System.md)
