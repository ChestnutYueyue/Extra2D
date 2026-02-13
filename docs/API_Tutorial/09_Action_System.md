# 动作系统 (Action System)

Extra2D 的动作系统提供了一套完整的动画解决方案，参考 Cocos2d-x 的设计模式，支持丰富的动作类型和缓动效果。

## 目录

1. [概述](#概述)
2. [核心概念](#核心概念)
3. [基本动作](#基本动作)
4. [组合动作](#组合动作)
5. [缓动动作](#缓动动作)
6. [特殊动作](#特殊动作)
7. [瞬时动作](#瞬时动作)
8. [动作管理](#动作管理)
9. [完整示例](#完整示例)

---

## 概述

动作系统用于在节点上创建动画效果，通过修改节点的属性（位置、缩放、旋转、透明度等）实现各种动画。

### 类层次结构

```
Action (基类)
├── FiniteTimeAction (有限时间动作)
│   ├── ActionInterval (时间间隔动作)
│   │   ├── MoveBy/MoveTo
│   │   ├── JumpBy/JumpTo
│   │   ├── BezierBy/BezierTo
│   │   ├── ScaleBy/ScaleTo
│   │   ├── RotateBy/RotateTo
│   │   ├── FadeIn/FadeOut/FadeTo
│   │   ├── Blink
│   │   ├── TintBy/TintTo
│   │   ├── Sequence
│   │   ├── Spawn
│   │   ├── Repeat/RepeatForever
│   │   ├── DelayTime
│   │   ├── ReverseTime
│   │   └── ActionEase (缓动动作基类)
│   └── ActionInstant (瞬时动作)
│       ├── CallFunc/CallFuncN
│       ├── Place
│       ├── FlipX/FlipY
│       ├── Show/Hide
│       ├── ToggleVisibility
│       └── RemoveSelf
├── Speed (速度控制)
├── Follow (跟随动作)
└── TargetedAction (目标动作)
```

---

## 核心概念

### 创建动作

所有动作都使用静态工厂方法 `create()` 创建：

```cpp
// 创建移动动作
auto move = MoveTo::create(2.0f, Vec2(100, 100));

// 创建缩放动作
auto scale = ScaleTo::create(1.0f, 2.0f);

// 创建旋转动作
auto rotate = RotateBy::create(3.0f, 360.0f);
```

### 运行动作

通过节点的 `runAction()` 方法运行动作：

```cpp
sprite->runAction(move);
```

### 动作完成回调

使用 `setCompletionCallback()` 设置动作完成时的回调：

```cpp
auto action = MoveTo::create(2.0f, Vec2(100, 100));
action->setCompletionCallback([]() {
    E2D_LOG_INFO("动作完成！");
});
sprite->runAction(action);
```

---

## 基本动作

### 移动动作

```cpp
// MoveTo - 移动到指定位置
auto moveTo = MoveTo::create(2.0f, Vec2(100, 100));

// MoveBy - 相对移动
auto moveBy = MoveBy::create(2.0f, Vec2(50, 0));  // 向右移动 50 像素
```

### 跳跃动作

```cpp
// JumpTo - 跳跃到指定位置
auto jumpTo = JumpTo::create(2.0f, Vec2(100, 100), 50, 3);  // 跳到 (100,100)，高度 50，跳 3 次

// JumpBy - 相对跳跃
auto jumpBy = JumpBy::create(2.0f, Vec2(100, 0), 50, 3);  // 向右跳跃 100 像素
```

### 贝塞尔曲线动作

```cpp
BezierConfig bezier;
bezier.controlPoint1 = Vec2(100, 200);
bezier.controlPoint2 = Vec2(200, 100);
bezier.endPosition = Vec2(300, 150);

// BezierTo - 贝塞尔曲线移动到指定位置
auto bezierTo = BezierTo::create(3.0f, bezier);

// BezierBy - 相对贝塞尔曲线移动
auto bezierBy = BezierBy::create(3.0f, bezier);
```

### 缩放动作

```cpp
// ScaleTo - 缩放到指定比例
auto scaleTo = ScaleTo::create(1.0f, 2.0f);        // 缩放到 2 倍
auto scaleToXY = ScaleTo::create(1.0f, 2.0f, 1.5f); // X 缩放 2 倍，Y 缩放 1.5 倍

// ScaleBy - 相对缩放
auto scaleBy = ScaleBy::create(1.0f, 0.5f);  // 缩小一半
```

### 旋转动作

```cpp
// RotateTo - 旋转到指定角度
auto rotateTo = RotateTo::create(2.0f, 90.0f);  // 旋转到 90 度

// RotateBy - 相对旋转
auto rotateBy = RotateBy::create(2.0f, 360.0f);  // 旋转 360 度
```

### 淡入淡出动作

```cpp
// FadeIn - 淡入（透明度从 0 到 1）
auto fadeIn = FadeIn::create(1.0f);

// FadeOut - 淡出（透明度从 1 到 0）
auto fadeOut = FadeOut::create(1.0f);

// FadeTo - 淡入到指定透明度
auto fadeTo = FadeTo::create(1.0f, 0.5f);  // 淡入到 50% 透明度
```

### 闪烁动作

```cpp
// Blink - 闪烁
auto blink = Blink::create(2.0f, 5);  // 2 秒内闪烁 5 次
```

### 色调动作

```cpp
// TintTo - 变化到指定颜色
auto tintTo = TintTo::create(1.0f, 255, 0, 0);  // 变为红色

// TintBy - 相对颜色变化
auto tintBy = TintBy::create(1.0f, 50, 0, 0);  // 红色通道增加 50
```

---

## 组合动作

### Sequence - 序列动作

按顺序依次执行多个动作：

```cpp
auto sequence = Sequence::create(
    MoveTo::create(1.0f, Vec2(100, 100)),
    DelayTime::create(0.5f),
    FadeOut::create(1.0f),
    CallFunc::create([]() { E2D_LOG_INFO("完成"); }),
    nullptr  // 必须以 nullptr 结尾
);
sprite->runAction(sequence);
```

### Spawn - 并行动作

同时执行多个动作：

```cpp
auto spawn = Spawn::create(
    MoveTo::create(2.0f, Vec2(200, 200)),
    RotateBy::create(2.0f, 360),
    FadeIn::create(2.0f),
    nullptr
);
sprite->runAction(spawn);
```

### Repeat - 重复动作

```cpp
// Repeat - 重复指定次数
auto repeat = Repeat::create(
    Sequence::create(
        MoveBy::create(0.5f, Vec2(50, 0)),
        MoveBy::create(0.5f, Vec2(-50, 0)),
        nullptr
    ),
    5  // 重复 5 次
);

// RepeatForever - 永久重复
auto repeatForever = RepeatForever::create(
    RotateBy::create(1.0f, 360)
);
sprite->runAction(repeatForever);
```

### DelayTime - 延时动作

```cpp
auto sequence = Sequence::create(
    MoveTo::create(1.0f, Vec2(100, 0)),
    DelayTime::create(1.0f),  // 延时 1 秒
    MoveTo::create(1.0f, Vec2(200, 0)),
    nullptr
);
```

### ReverseTime - 反向动作

```cpp
auto move = MoveBy::create(1.0f, Vec2(100, 0));
auto reverse = ReverseTime::create(move->clone());
// 反向执行，相当于 MoveBy(1.0f, Vec2(-100, 0))
```

---

## 缓动动作

缓动动作使用装饰器模式包装其他动作，实现各种缓动效果。

### 使用缓动

```cpp
// 创建基础动作
auto move = MoveTo::create(3.0f, Vec2(500, 300));

// 用缓动包装
auto easeMove = EaseElasticOut::create(move);
sprite->runAction(easeMove);
```

### 可用缓动类型

| 缓动类 | 效果 |
|--------|------|
| `EaseQuadIn/Out/InOut` | 二次缓动 |
| `EaseCubicIn/Out/InOut` | 三次缓动 |
| `EaseQuartIn/Out/InOut` | 四次缓动 |
| `EaseQuintIn/Out/InOut` | 五次缓动 |
| `EaseSineIn/Out/InOut` | 正弦缓动 |
| `EaseExponentialIn/Out/InOut` | 指数缓动 |
| `EaseCircleIn/Out/InOut` | 圆形缓动 |
| `EaseBackIn/Out/InOut` | 回震缓动 |
| `EaseElasticIn/Out/InOut` | 弹性缓动 |
| `EaseBounceIn/Out/InOut` | 弹跳缓动 |

### 缓动效果说明

- **In**: 开始时慢，逐渐加速
- **Out**: 开始时快，逐渐减速
- **InOut**: 开始和结束时慢，中间快

### 示例

```cpp
// 弹性缓动
auto jump = JumpBy::create(2.0f, Vec2(200, 0), 100, 1);
auto elasticJump = EaseElasticOut::create(jump, 0.5f);
sprite->runAction(elasticJump);

// 弹跳缓动
auto scale = ScaleTo::create(1.0f, 2.0f);
auto bounceScale = EaseBounceOut::create(scale);
sprite->runAction(bounceScale);

// 指数缓动
auto move = MoveTo::create(2.0f, Vec2(300, 200));
auto expoMove = EaseExponentialInOut::create(move);
sprite->runAction(expoMove);
```

### 自定义缓动

```cpp
// 使用自定义缓动函数
auto customEase = EaseCustom::create(move, [](float t) {
    // 自定义缓动函数
    return t * t * (3.0f - 2.0f * t);  // smoothstep
});
```

---

## 特殊动作

### Speed - 速度控制

动态控制动作的播放速度：

```cpp
auto move = MoveTo::create(5.0f, Vec2(500, 300));
auto speed = Speed::create(move, 0.5f);  // 半速播放
sprite->runAction(speed);

// 运行时调整速度
speed->setSpeed(2.0f);  // 2 倍速
```

### Follow - 跟随动作

使节点跟随另一个节点移动（常用于相机）：

```cpp
// 无边界跟随
auto follow = Follow::create(player);

// 带边界跟随
Rect boundary(0, 0, 2000, 2000);  // 世界边界
auto followWithBoundary = Follow::create(player, boundary);

camera->runAction(follow);
```

### TargetedAction - 目标动作

在一个节点上运行动作，但作用于另一个节点：

```cpp
// 在 spriteA 上运行，但影响 spriteB
auto targeted = TargetedAction::create(spriteB, MoveTo::create(2.0f, Vec2(100, 100)));
spriteA->runAction(targeted);
```

---

## 瞬时动作

瞬时动作立即完成，没有动画过程。

### CallFunc - 回调动作

```cpp
// 无参数回调
auto callFunc = CallFunc::create([]() {
    E2D_LOG_INFO("回调执行");
});

// 带节点参数的回调
auto callFuncN = CallFuncN::create([](Node* node) {
    E2D_LOG_INFO("节点: %p", node);
});
```

### Place - 放置动作

```cpp
// 立即放置到指定位置
auto place = Place::create(Vec2(100, 100));
```

### FlipX/FlipY - 翻转动作

```cpp
auto flipX = FlipX::create(true);   // 水平翻转
auto flipY = FlipY::create(true);   // 垂直翻转
```

### Show/Hide - 显示/隐藏动作

```cpp
auto show = Show::create();         // 显示
auto hide = Hide::create();         // 隐藏
auto toggle = ToggleVisibility::create();  // 切换可见性
```

### RemoveSelf - 移除自身

```cpp
// 从父节点移除
auto removeSelf = RemoveSelf::create();
```

---

## 动作管理

### 停止动作

```cpp
// 停止所有动作
sprite->stopAllActions();

// 停止指定动作
sprite->stopAction(action);

// 根据标签停止动作
sprite->stopActionByTag(1);

// 根据标志位停止动作
sprite->stopActionsByFlags(0x01);
```

### 查询动作

```cpp
// 获取动作数量
size_t count = sprite->getActionCount();

// 检查是否有动作在运行
bool running = sprite->isRunningActions();

// 根据标签获取动作
Action* action = sprite->getActionByTag(1);
```

### 动作标签和标志位

```cpp
auto action = MoveTo::create(2.0f, Vec2(100, 100));
action->setTag(1);           // 设置标签
action->setFlags(0x01);      // 设置标志位
sprite->runAction(action);
```

---

## 完整示例

### 示例 1：角色移动动画

```cpp
void Player::moveTo(const Vec2& target) {
    // 停止当前移动
    stopActionByTag(TAG_MOVE);
    
    // 创建移动动作
    auto move = MoveTo::create(1.0f, target);
    move->setTag(TAG_MOVE);
    
    // 添加缓动效果
    auto easeMove = EaseQuadInOut::create(move);
    
    runAction(easeMove);
}
```

### 示例 2：UI 弹出动画

```cpp
void UIPanel::show() {
    // 初始状态
    setScale(0.0f);
    setOpacity(0.0f);
    setVisible(true);
    
    // 并行动画：缩放 + 淡入
    auto spawn = Spawn::create(
        EaseBackOut::create(ScaleTo::create(0.3f, 1.0f)),
        FadeIn::create(0.3f),
        nullptr
    );
    
    runAction(spawn);
}
```

### 示例 3：游戏结束动画

```cpp
void GameOverLayer::playAnimation() {
    // 从屏幕底部滑入
    setPosition(Vec2(screenWidth / 2, screenHeight));
    
    auto moveUp = MoveBy::create(1.0f, Vec2(0, -screenHeight));
    moveUp->setCompletionCallback([this]() {
        // 动画完成后启用按钮
        restartBtn_->setEnabled(true);
    });
    
    // 添加缓动效果
    auto easeMove = EaseQuadOut::create(moveUp);
    runAction(easeMove);
}
```

### 示例 4：复杂序列动画

```cpp
void Enemy::playDeathAnimation() {
    auto sequence = Sequence::create(
        // 闪烁
        Blink::create(0.5f, 3),
        // 放大
        ScaleTo::create(0.2f, 1.5f),
        // 淡出
        FadeOut::create(0.3f),
        // 移除自身
        RemoveSelf::create(),
        nullptr
    );
    
    runAction(sequence);
}
```

### 示例 5：循环动画

```cpp
void Coin::startFloating() {
    // 上下浮动
    auto floatUp = MoveBy::create(0.5f, Vec2(0, 10));
    auto floatDown = floatUp->reverse();
    
    auto floatSequence = Sequence::create(
        EaseSineInOut::create(floatUp),
        EaseSineInOut::create(floatDown),
        nullptr
    );
    
    // 永久循环
    auto floatForever = RepeatForever::create(floatSequence);
    floatForever->setTag(TAG_FLOAT);
    
    runAction(floatForever);
}
```

---

## 性能优化建议

1. **复用动作**：对于频繁使用的动作，可以 `clone()` 复用
2. **合理使用标签**：便于管理和停止特定动作
3. **避免过多并发动作**：大量节点同时运行动作会影响性能
4. **及时停止不需要的动作**：节点销毁前调用 `stopAllActions()`

---

## API 参考

### Action 基类

| 方法 | 说明 |
|------|------|
| `isDone()` | 检查动作是否完成 |
| `startWithTarget(node)` | 启动动作 |
| `stop()` | 停止动作 |
| `pause()` | 暂停动作 |
| `resume()` | 恢复动作 |
| `clone()` | 克隆动作 |
| `reverse()` | 创建反向动作 |
| `getTag()` / `setTag()` | 获取/设置标签 |
| `getFlags()` / `setFlags()` | 获取/设置标志位 |
| `setCompletionCallback()` | 设置完成回调 |

### Node 动作接口

| 方法 | 说明 |
|------|------|
| `runAction(action)` | 运行动作 |
| `stopAllActions()` | 停止所有动作 |
| `stopAction(action)` | 停止指定动作 |
| `stopActionByTag(tag)` | 根据标签停止动作 |
| `stopActionsByFlags(flags)` | 根据标志位停止动作 |
| `getActionByTag(tag)` | 根据标签获取动作 |
| `getActionCount()` | 获取动作数量 |
| `isRunningActions()` | 检查是否有动作运行 |
