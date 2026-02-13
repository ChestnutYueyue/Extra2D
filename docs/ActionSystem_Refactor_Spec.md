# Extra2D 动作系统重构规格文档

## 一、概述

本文档描述了 Extra2D 动作系统的重构方案，参考 Cocos2d-x 的动作系统设计模式，实现一个更加完善、易用、功能丰富的动作系统。

## 二、现有系统分析

### 2.1 当前类层次结构

```
Action (基类)
├── IntervalAction (有限时间动作基类)
│   ├── MoveBy/MoveTo
│   ├── ScaleBy/ScaleTo
│   ├── RotateBy/RotateTo
│   ├── FadeIn/FadeOut/FadeTo
│   ├── Sequence
│   ├── Spawn
│   ├── Delay
│   └── Loop
└── InstantAction (瞬时动作基类)
    └── CallFunc
```

### 2.2 与 Cocos2d-x 的差异

| 特性 | Extra2D 当前 | Cocos2d-x | 差异说明 |
|------|-------------|-----------|---------|
| 类层次 | Action -> IntervalAction/InstantAction | Action -> FiniteTimeAction -> ActionInterval/ActionInstant | 缺少 FiniteTimeAction 中间层 |
| 动作管理 | Node 内部管理 | ActionManager 单例集中管理 | 缺少集中式管理 |
| 缓动动作 | EaseAction 静态方法 | ActionEase 类系（装饰器模式） | 不够灵活，无法组合 |
| 速度控制 | Action::speed_ 成员 | Speed 动作包装器 | 缺少动态速度控制 |
| 跟随动作 | 无 | Follow 类 | 缺失 |
| 跳跃动作 | 无 | JumpBy/JumpTo | 缺失 |
| 贝塞尔动作 | 无 | BezierBy/BezierTo | 缺失 |
| 闪烁动作 | 无 | Blink | 缺失 |
| 色调动作 | 无 | TintBy/TintTo | 缺失 |
| 重复动作 | Loop | Repeat/RepeatForever | 命名和功能差异 |
| 反向动作 | reverse() 方法 | reverse() 方法 | 相同 |
| 克隆动作 | clone() 方法 | clone() 方法 | 相同 |

## 三、重构目标

### 3.1 核心目标

1. **完善类层次结构** - 添加 FiniteTimeAction 中间层
2. **集中式动作管理** - 实现 ActionManager 单例
3. **装饰器模式缓动** - 实现 ActionEase 类系
4. **丰富动作类型** - 添加跳跃、贝塞尔、闪烁、色调等动作
5. **统一API风格** - 与 Cocos2d-x API 保持一致

### 3.2 设计原则

1. **组合优于继承** - 使用装饰器模式实现缓动和速度控制
2. **单一职责** - ActionManager 只负责动作调度
3. **开放封闭** - 易于扩展新动作类型
4. **接口隔离** - 不同类型动作提供不同接口

## 四、新类层次结构

```
Action (基类)
├── FiniteTimeAction (有限时间动作基类)
│   ├── ActionInterval (时间间隔动作)
│   │   ├── MoveBy/MoveTo
│   │   ├── MoveBy3D/MoveTo3D (新增)
│   │   ├── JumpBy/JumpTo (新增)
│   │   ├── BezierBy/BezierTo (新增)
│   │   ├── ScaleBy/ScaleTo
│   │   ├── RotateBy/RotateTo
│   │   ├── FadeIn/FadeOut/FadeTo
│   │   ├── Blink (新增)
│   │   ├── TintBy/TintTo (新增)
│   │   ├── Sequence
│   │   ├── Spawn
│   │   ├── DelayTime
│   │   ├── Repeat (重命名自 Loop)
│   │   ├── RepeatForever (新增)
│   │   ├── ReverseTime (新增)
│   │   └── ActionEase (缓动动作基类，新增)
│   │       ├── EaseIn/EaseOut/EaseInOut
│   │       ├── EaseSineIn/EaseSineOut/EaseSineInOut
│   │       ├── EaseExponentialIn/EaseExponentialOut/EaseExponentialInOut
│   │       ├── EaseBounceIn/EaseBounceOut/EaseBounceInOut
│   │       ├── EaseElasticIn/EaseElasticOut/EaseElasticInOut
│   │       ├── EaseBackIn/EaseBackOut/EaseBackInOut
│   │       └── EaseBezier (新增)
│   └── ActionInstant (瞬时动作)
│       ├── CallFunc
│       ├── CallFuncN (新增)
│       ├── Place (新增)
│       ├── FlipX/FlipY (新增)
│       ├── Show/Hide (新增)
│       ├── ToggleVisibility (新增)
│       └── RemoveSelf (新增)
├── Speed (速度控制动作，新增)
├── Follow (跟随动作，新增)
└── TargetedAction (目标动作，新增)
```

## 五、核心类设计

### 5.1 Action 基类

```cpp
class Action {
public:
    virtual ~Action() = default;
    
    // 核心接口
    virtual bool isDone() const;
    virtual void startWithTarget(Node* target);
    virtual void stop();
    virtual void step(float dt);
    virtual void update(float time);
    
    // 克隆和反向
    virtual Action* clone() const = 0;
    virtual Action* reverse() const = 0;
    
    // 属性访问
    Node* getTarget() const;
    Node* getOriginalTarget() const;
    int getTag() const;
    void setTag(int tag);
    unsigned int getFlags() const;
    void setFlags(unsigned int flags);
    
protected:
    Node* target_ = nullptr;
    Node* originalTarget_ = nullptr;
    int tag_ = -1;
    unsigned int flags_ = 0;
};
```

### 5.2 FiniteTimeAction 中间层

```cpp
class FiniteTimeAction : public Action {
public:
    float getDuration() const { return duration_; }
    void setDuration(float duration) { duration_ = duration; }
    
    virtual FiniteTimeAction* clone() const = 0;
    virtual FiniteTimeAction* reverse() const = 0;
    
protected:
    float duration_ = 0.0f;
};
```

### 5.3 ActionInterval 时间间隔动作

```cpp
class ActionInterval : public FiniteTimeAction {
public:
    float getElapsed() const { return elapsed_; }
    bool isDone() const override;
    
    void startWithTarget(Node* target) override;
    void step(float dt) override;
    
    void setAmplitudeRate(float amp) { amplitudeRate_ = amp; }
    float getAmplitudeRate() const { return amplitudeRate_; }
    
protected:
    float elapsed_ = 0.0f;
    bool firstTick_ = true;
    float amplitudeRate_ = 1.0f;
    EaseFunction easeFunc_ = nullptr;  // 内置缓动函数
};
```

### 5.4 ActionInstant 瞬时动作

```cpp
class ActionInstant : public FiniteTimeAction {
public:
    bool isDone() const override { return true; }
    void step(float dt) override;
    
protected:
    bool done_ = false;
};
```

### 5.5 ActionManager 动作管理器

```cpp
class ActionManager {
public:
    static ActionManager* getInstance();
    
    // 动作管理
    void addAction(Action* action, Node* target, bool paused = false);
    void removeAction(Action* action);
    void removeActionByTag(int tag, Node* target);
    void removeAllActionsFromTarget(Node* target);
    void removeAllActions();
    
    // 查询
    Action* getActionByTag(int tag, Node* target);
    size_t getActionCount(Node* target) const;
    
    // 暂停/恢复
    void pauseTarget(Node* target);
    void resumeTarget(Node* target);
    bool isPaused(Node* target) const;
    
    // 更新（每帧调用）
    void update(float dt);
    
private:
    struct ActionElement {
        std::vector<Action*> actions;
        Node* target;
        bool paused;
        int actionIndex;
        Action* currentAction;
        bool currentActionSalvaged;
    };
    
    std::unordered_map<Node*, ActionElement> targets_;
    static ActionManager* instance_;
};
```

### 5.6 ActionEase 缓动动作基类

```cpp
class ActionEase : public ActionInterval {
public:
    static ActionEase* create(ActionInterval* action);
    
    ActionInterval* getInnerAction() const { return innerAction_; }
    
    void startWithTarget(Node* target) override;
    void stop() override;
    void update(float time) override;
    Action* reverse() const override;
    Action* clone() const override;
    
protected:
    ActionInterval* innerAction_ = nullptr;
};
```

### 5.7 Speed 速度控制动作

```cpp
class Speed : public Action {
public:
    static Speed* create(ActionInterval* action, float speed);
    
    float getSpeed() const { return speed_; }
    void setSpeed(float speed) { speed_ = speed; }
    
    void startWithTarget(Node* target) override;
    void stop() override;
    void step(float dt) override;
    bool isDone() const override;
    Action* reverse() const override;
    Action* clone() const override;
    
protected:
    ActionInterval* innerAction_ = nullptr;
    float speed_ = 1.0f;
};
```

## 六、新增动作类型

### 6.1 JumpBy/JumpTo 跳跃动作

```cpp
class JumpBy : public ActionInterval {
public:
    static JumpBy* create(float duration, const Vec2& position, 
                          float height, int jumps);
    
protected:
    Vec2 startPosition_;
    Vec2 delta_;
    float height_;
    int jumps_;
};

class JumpTo : public JumpBy {
    // 继承实现，目标位置版本
};
```

### 6.2 BezierBy/BezierTo 贝塞尔动作

```cpp
struct BezierConfig {
    Vec2 controlPoint1;
    Vec2 controlPoint2;
    Vec2 endPosition;
};

class BezierBy : public ActionInterval {
public:
    static BezierBy* create(float duration, const BezierConfig& config);
    
protected:
    BezierConfig config_;
    Vec2 startPosition_;
};
```

### 6.3 Blink 闪烁动作

```cpp
class Blink : public ActionInterval {
public:
    static Blink* create(float duration, int times);
    
protected:
    int times_;
    int currentTimes_;
    bool originalVisible_;
};
```

### 6.4 TintBy/TintTo 色调动作

```cpp
class TintTo : public ActionInterval {
public:
    static TintTo* create(float duration, 
                          uint8_t red, uint8_t green, uint8_t blue);
    
protected:
    Color3B startColor_;
    Color3B endColor_;
    Color3B deltaColor_;
};
```

### 6.5 Follow 跟随动作

```cpp
class Follow : public Action {
public:
    static Follow* create(Node* followedNode, 
                          const Rect& boundary = Rect::ZERO);
    
    bool isDone() const override;
    void step(float dt) override;
    
protected:
    Node* followedNode_ = nullptr;
    Rect boundary_;
    bool boundarySet_ = false;
};
```

### 6.6 瞬时动作扩展

```cpp
// 放置到指定位置
class Place : public ActionInstant {
public:
    static Place* create(const Vec2& position);
};

// X/Y轴翻转
class FlipX : public ActionInstant {
public:
    static FlipX* create(bool flipX);
};

class FlipY : public ActionInstant {
public:
    static FlipY* create(bool flipY);
};

// 显示/隐藏
class Show : public ActionInstant {
public:
    static Show* create();
};

class Hide : public ActionInstant {
public:
    static Hide* create();
};

// 切换可见性
class ToggleVisibility : public ActionInstant {
public:
    static ToggleVisibility* create();
};

// 移除自身
class RemoveSelf : public ActionInstant {
public:
    static RemoveSelf* create();
};

// 带Node参数的回调
class CallFuncN : public ActionInstant {
public:
    static CallFuncN* create(const std::function<void(Node*)>& func);
};
```

## 七、缓动动作完整实现

### 7.1 通用缓动包装器

```cpp
// 通用缓动包装器
class EaseCustom : public ActionEase {
public:
    static EaseCustom* create(ActionInterval* action, 
                               EaseFunction easeFunc);
};

// 指数缓动
class EaseExponentialIn : public ActionEase { /* ... */ };
class EaseExponentialOut : public ActionEase { /* ... */ };
class EaseExponentialInOut : public ActionEase { /* ... */ };

// 正弦缓动
class EaseSineIn : public ActionEase { /* ... */ };
class EaseSineOut : public ActionEase { /* ... */ };
class EaseSineInOut : public ActionEase { /* ... */ };

// 弹性缓动
class EaseElasticIn : public ActionEase { 
public:
    static EaseElasticIn* create(ActionInterval* action, float period = 0.3f);
};
class EaseElasticOut : public ActionEase { /* ... */ };
class EaseElasticInOut : public ActionEase { /* ... */ };

// 弹跳缓动
class EaseBounceIn : public ActionEase { /* ... */ };
class EaseBounceOut : public ActionEase { /* ... */ };
class EaseBounceInOut : public ActionEase { /* ... */ };

// 回震缓动
class EaseBackIn : public ActionEase { /* ... */ };
class EaseBackOut : public ActionEase { /* ... */ };
class EaseBackInOut : public ActionEase { /* ... */ };

// 贝塞尔缓动（自定义曲线）
class EaseBezier : public ActionEase {
public:
    static EaseBezier* create(ActionInterval* action);
    void setBezierParamer(float p0, float p1, float p2, float p3);
};
```

## 八、Node 类接口更新

### 8.1 动作相关接口

```cpp
class Node {
public:
    // 运行动作
    Action* runAction(Action* action);
    
    // 停止动作
    void stopAllActions();
    void stopAction(Action* action);
    void stopActionByTag(int tag);
    void stopActionsByFlags(unsigned int flags);
    
    // 获取动作
    Action* getActionByTag(int tag);
    size_t getNumberOfRunningActions() const;
    
    // 暂停/恢复所有动作
    void pauseAllActions();
    void resumeAllActions();
    
    // 检查是否有动作在运行
    bool isRunningActions() const;
};
```

## 九、使用示例

### 9.1 基本动作

```cpp
// 移动
auto moveTo = MoveTo::create(2.0f, Vec2(100, 100));
sprite->runAction(moveTo);

// 跳跃
auto jump = JumpBy::create(2.0f, Vec2(200, 0), 100, 3);
sprite->runAction(jump);

// 贝塞尔曲线
BezierConfig bezier;
bezier.controlPoint1 = Vec2(100, 200);
bezier.controlPoint2 = Vec2(200, 100);
bezier.endPosition = Vec2(300, 150);
auto bezierAction = BezierTo::create(3.0f, bezier);
sprite->runAction(bezierAction);
```

### 9.2 组合动作

```cpp
// 序列动作
auto seq = Sequence::create(
    MoveTo::create(1.0f, Vec2(100, 100)),
    DelayTime::create(0.5f),
    FadeOut::create(1.0f),
    CallFunc::create([](){ log("Done!"); }),
    nullptr
);
sprite->runAction(seq);

// 并行动作
auto spawn = Spawn::create(
    MoveTo::create(2.0f, Vec2(200, 200)),
    RotateBy::create(2.0f, 360),
    FadeIn::create(2.0f),
    nullptr
);
sprite->runAction(spawn);

// 重复动作
auto repeat = Repeat::create(MoveBy::create(1.0f, Vec2(50, 0)), 5);
sprite->runAction(repeat);

// 永久重复
auto repeatForever = RepeatForever::create(
    Sequence::create(
        MoveBy::create(1.0f, Vec2(100, 0)),
        MoveBy::create(1.0f, Vec2(-100, 0)),
        nullptr
    )
);
sprite->runAction(repeatForever);
```

### 9.3 缓动动作

```cpp
// 使用缓动包装器
auto move = MoveTo::create(3.0f, Vec2(500, 300));
auto easeMove = EaseElasticOut::create(move, 0.5f);
sprite->runAction(easeMove);

// 指数缓动
auto jump = JumpBy::create(2.0f, Vec2(200, 0), 100, 1);
auto easeJump = EaseExponentialOut::create(jump);
sprite->runAction(easeJump);

// 弹跳缓动
auto scale = ScaleTo::create(1.0f, 2.0f);
auto bounceScale = EaseBounceOut::create(scale);
sprite->runAction(bounceScale);
```

### 9.4 速度控制

```cpp
// 慢动作回放
auto action = MoveTo::create(5.0f, Vec2(500, 300));
auto speed = Speed::create(action, 0.5f);  // 半速
sprite->runAction(speed);

// 动态调整速度
speed->setSpeed(2.0f);  // 2倍速
```

### 9.5 跟随动作

```cpp
// 相机跟随玩家
auto follow = Follow::create(player, Rect(0, 0, 2000, 2000));
camera->runAction(follow);
```

## 十、文件结构

```
Extra2D/include/extra2d/action/
├── action.h              # Action 基类
├── finite_time_action.h  # FiniteTimeAction 中间层
├── action_interval.h     # ActionInterval 及其子类
├── action_instant.h      # ActionInstant 及其子类
├── action_ease.h         # 缓动动作类系
├── action_manager.h      # ActionManager
├── ease_functions.h      # 缓动函数
└── action_factory.h      # 动作工厂（可选）

Extra2D/src/action/
├── action.cpp
├── finite_time_action.cpp
├── action_interval.cpp
├── action_instant.cpp
├── action_ease.cpp
├── action_manager.cpp
└── ease_functions.cpp
```

## 十一、实现优先级

### 第一阶段（核心重构）
1. Action 基类重构
2. FiniteTimeAction 中间层
3. ActionInterval 重构
4. ActionInstant 重构
5. ActionManager 实现

### 第二阶段（新增动作）
1. JumpBy/JumpTo
2. BezierBy/BezierTo
3. Blink
4. TintBy/TintTo
5. Follow
6. Speed

### 第三阶段（缓动系统）
1. ActionEase 基类
2. 各类缓动包装器
3. EaseCustom 自定义缓动

### 第四阶段（瞬时动作扩展）
1. Place
2. FlipX/FlipY
3. Show/Hide
4. ToggleVisibility
5. RemoveSelf
6. CallFuncN

## 十二、兼容性考虑

### 12.1 API 变更

- 动作创建方式改为 `create()` 静态工厂方法
- `Loop` 重命名为 `Repeat`
- `Delay` 重命名为 `DelayTime`
- 动作管理由 `ActionManager` 集中处理

### 12.2 迁移指南

```cpp
// 旧写法
auto action = std::make_shared<MoveTo>(2.0f, Vec2(100, 100));
sprite->runAction(action);

// 新写法（推荐）
auto action = MoveTo::create(2.0f, Vec2(100, 100));
sprite->runAction(action);

// 旧写法（缓动）
auto action = EaseAction::create(MoveTo::create(2.0f, Vec2(100, 100)), easeInQuad);

// 新写法（缓动）
auto action = EaseQuadIn::create(MoveTo::create(2.0f, Vec2(100, 100)));
```

## 十三、性能优化

1. **对象池集成** - 动作对象使用对象池管理
2. **批量更新** - ActionManager 统一调度减少调用开销
3. **延迟删除** - 动作完成时标记删除，统一清理
4. **缓存友好** - 连续存储同一类型动作

## 十四、测试计划

1. 单元测试：每个动作类型的独立测试
2. 集成测试：组合动作测试
3. 性能测试：大量动作并发测试
4. 内存测试：动作对象生命周期测试
