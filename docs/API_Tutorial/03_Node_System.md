# 03. 节点系统

Extra2D 的节点系统是构建游戏对象的基础。所有可见的游戏元素都是节点的子类。

## 核心节点类型

```
Node (基类)
├── Sprite (精灵)
├── Text (文本)
├── Button (按钮)
├── Widget (UI控件基类)
│   ├── Label (标签)
│   ├── CheckBox (复选框)
│   ├── RadioButton (单选按钮)
│   ├── Slider (滑块)
│   └── ProgressBar (进度条)
└── 自定义节点...
```

## 基础节点操作

### 创建和添加节点

```cpp
// 创建精灵
auto sprite = Sprite::create(texture);
sprite->setPosition(Vec2(100, 200));
sprite->setAnchor(Vec2(0.5f, 0.5f));  // 中心锚点
addChild(sprite);

// 创建文本
auto text = Text::create("Hello World", font);
text->setPosition(Vec2(400, 300));
text->setTextColor(Color(1, 1, 1, 1));
addChild(text);
```

### 节点属性

```cpp
// 位置（相对于父节点的本地坐标）
node->setPosition(Vec2(x, y));
Vec2 pos = node->getPosition();

// 旋转（角度，单位：度）
node->setRotation(45.0f);
float angle = node->getRotation();

// 缩放
node->setScale(Vec2(2.0f, 2.0f));
Vec2 scale = node->getScale();

// 锚点（0,0 左上角，0.5,0.5 中心，1,1 右下角）
node->setAnchor(Vec2(0.5f, 0.5f));

// 可见性
node->setVisible(true);
bool visible = node->isVisible();

// Z轴顺序
node->setZOrder(10);
```

## ⚠️ 重要：坐标系与变换系统

### 坐标系说明

Extra2D 使用以下坐标系：

1. **本地坐标系（Local Space）**：相对于父节点的坐标
   - 原点 `(0, 0)` 是父节点的锚点位置
   - `setPosition(x, y)` 设置的是本地坐标

2. **世界坐标系（World Space）**：相对于场景根节点的绝对坐标
   - 通过 `convertToWorldSpace()` 转换
   - 通过 `getWorldTransform()` 获取变换矩阵

3. **屏幕坐标系（Screen Space）**：相对于屏幕左上角的坐标
   - 原点 `(0, 0)` 在屏幕左上角
   - Y轴向下为正方向

### 锚点（Anchor）机制

**锚点定义了节点的原点位置**：

```cpp
// 锚点 (0, 0) - 左上角为原点
sprite->setAnchor(Vec2(0.0f, 0.0f));
sprite->setPosition(Vec2(100, 100));  // 左上角在 (100, 100)

// 锚点 (0.5, 0.5) - 中心为原点（推荐）
sprite->setAnchor(Vec2(0.5f, 0.5f));
sprite->setPosition(Vec2(100, 100));  // 中心在 (100, 100)

// 锚点 (1, 1) - 右下角为原点
sprite->setAnchor(Vec2(1.0f, 1.0f));
sprite->setPosition(Vec2(100, 100));  // 右下角在 (100, 100)
```

**⚠️ 重要：锚点偏移在渲染时处理，不在本地变换中**

这意味着：
- `getLocalTransform()` 返回的矩阵**不包含**锚点偏移
- 锚点偏移在 `GLSpriteBatch::addVertices()` 中应用
- 这样可以避免锚点偏移被父节点的缩放影响

### 父子变换传递

**变换层级**：
```
Scene (世界坐标系)
└── GameOverLayer (本地坐标 + 父变换 = 世界坐标)
    └── Panel (本地坐标 + 父变换 = 世界坐标)
        └── ScoreNumber (本地坐标 + 父变换 = 世界坐标)
```

**关键方法**：

```cpp
// 获取本地变换矩阵（不包含锚点偏移）
glm::mat4 localTransform = node->getLocalTransform();

// 获取世界变换矩阵（包含所有父节点的变换）
glm::mat4 worldTransform = node->getWorldTransform();

// 本地坐标转世界坐标
Vec2 worldPos = node->convertToWorldSpace(Vec2::Zero());

// 世界坐标转本地坐标
Vec2 localPos = node->convertToNodeSpace(worldPos);
```

**变换更新流程**：

1. **标记脏状态**：当位置、旋转、缩放改变时
   ```cpp
   void Node::setPosition(const Vec2& pos) {
       position_ = pos;
       markTransformDirty();  // 标记变换需要更新
   }
   ```

2. **递归标记**：`markTransformDirty()` 会递归标记所有子节点
   ```cpp
   void Node::markTransformDirty() {
       transformDirty_ = true;
       worldTransformDirty_ = true;
       for (auto& child : children_) {
           child->markTransformDirty();  // 递归标记子节点
       }
   }
   ```

3. **批量更新**：在渲染前，`Scene::renderContent()` 调用 `batchUpdateTransforms()`
   ```cpp
   void Scene::renderContent(RenderBackend& renderer) {
       batchUpdateTransforms();  // 更新所有节点的世界变换
       // ... 渲染
   }
   ```

### 正确使用变换的示例

```cpp
// 创建层级结构
auto panel = Sprite::create(panelTexture);
panel->setAnchor(Vec2(0.5f, 0.5f));  // 中心锚点
panel->setPosition(Vec2(0, 256));     // 相对于父节点的中心
addChild(panel);

// 添加子节点到 panel
auto scoreNumber = makePtr<Number>();
scoreNumber->setPosition(Vec2(95.0f, 10.0f));  // 相对于 panel 中心
panel->addChild(scoreNumber);

// 当 panel 移动时，scoreNumber 会自动跟随
// 因为 scoreNumber 的世界变换包含了 panel 的世界变换
```

### 常见错误

**错误1：混淆本地坐标和世界坐标**
```cpp
// ❌ 错误：在世界坐标系中设置位置，但节点是子节点
child->setPosition(worldPos);  // 这会被解释为本地坐标

// ✅ 正确：使用 convertToNodeSpace 转换
child->setPosition(parent->convertToNodeSpace(worldPos));
```

**错误2：锚点设置不当**
```cpp
// ❌ 错误：锚点 (0,0) 但期望中心对齐
sprite->setAnchor(Vec2(0.0f, 0.0f));
sprite->setPosition(Vec2(100, 100));  // 左上角在 (100, 100)

// ✅ 正确：使用中心锚点
sprite->setAnchor(Vec2(0.5f, 0.5f));
sprite->setPosition(Vec2(100, 100));  // 中心在 (100, 100)
```

**错误3：忽略父节点变换**
```cpp
// ❌ 错误：直接设置世界坐标，忽略父节点
child->setPosition(Vec2(100, 100));  // 这是本地坐标！

// ✅ 正确：考虑父节点的世界变换
// 如果父节点在 (50, 50)，子节点相对于父节点应该是 (50, 50)
child->setPosition(Vec2(50, 50));
```

## 精灵（Sprite）

### 创建精灵

```cpp
auto& resources = Application::instance().resources();

// 从纹理创建
auto texture = resources.loadTexture("assets/player.png");
auto sprite = Sprite::create(texture);

// 设置精灵属性
sprite->setPosition(Vec2(400, 300));
sprite->setAnchor(Vec2(0.5f, 0.5f));

// 切换纹理
auto newTexture = resources.loadTexture("assets/player2.png");
sprite->setTexture(newTexture);
```

### 从图集创建精灵（Texture Atlas）

```cpp
// 加载图集纹理
auto atlasTexture = resources.loadTexture("assets/atlas.png");

// 创建精灵，指定图集中的矩形区域
Rect spriteRect(100, 100, 32, 32);  // x, y, width, height
auto sprite = Sprite::create(atlasTexture, spriteRect);
```

### 精灵渲染流程

```cpp
void Sprite::onDraw(RenderBackend& renderer) {
    // 1. 获取世界变换矩阵
    auto worldTransform = getWorldTransform();
    
    // 2. 从世界变换中提取位置和缩放
    float worldX = worldTransform[3][0];
    float worldY = worldTransform[3][1];
    float worldScaleX = glm::length(glm::vec2(worldTransform[0][0], worldTransform[0][1]));
    float worldScaleY = glm::length(glm::vec2(worldTransform[1][0], worldTransform[1][1]));
    
    // 3. 计算目标矩形（不包含锚点偏移）
    Rect destRect(worldX, worldY, width * worldScaleX, height * worldScaleY);
    
    // 4. 提取旋转角度
    float worldRotation = std::atan2(worldTransform[0][1], worldTransform[0][0]);
    
    // 5. 绘制精灵（锚点偏移在 GLSpriteBatch 中处理）
    renderer.drawSprite(*texture_, destRect, srcRect, color_, worldRotation, anchor);
}
```

## 按钮（Button）

### 创建按钮

```cpp
auto button = Button::create();
button->setFont(font);
button->setText("点击我");
button->setPosition(Vec2(400, 300));
button->setCustomSize(200.0f, 60.0f);

// 设置颜色
button->setTextColor(Colors::White);
button->setBackgroundColor(
    Colors::Blue,    // 正常状态
    Colors::Green,   // 悬停状态
    Colors::Red      // 按下状态
);

// 设置点击回调
button->setOnClick([]() {
    E2D_LOG_INFO("按钮被点击！");
});

addChild(button);
```

### 使用图片按钮

```cpp
// 从图集创建按钮
auto buttonFrame = ResLoader::getKeyFrame("button_play");
if (buttonFrame) {
    auto button = Button::create();
    button->setBackgroundImage(
        buttonFrame->getTexture(),
        buttonFrame->getRect()  // 使用图集中的矩形区域
    );
    button->setPosition(Vec2(screenWidth / 2, 300));
    button->setOnClick([]() {
        // 处理点击
    });
    addChild(button);
}
```

### 按钮坐标空间

**按钮使用世界坐标空间（World Space）**：

```cpp
// Button::getBoundingBox() 返回世界坐标系的包围盒
Rect Button::getBoundingBox() const {
    auto pos = getRenderPosition();  // 获取世界坐标位置
    // ... 计算包围盒
    return Rect(x0, y0, w, h);  // 世界坐标
}
```

**这意味着**：
- 按钮的位置是相对于父节点的本地坐标
- 但按钮的点击检测使用世界坐标
- 父节点的变换会自动应用到按钮

### 按钮事件处理

```cpp
// 在 onUpdate 中检测手柄按键
void GameOverLayer::onUpdate(float dt) {
    Node::onUpdate(dt);
    
    auto& input = Application::instance().input();
    
    // A 键触发重新开始按钮
    if (input.isButtonPressed(GamepadButton::A)) {
        restartBtn_->getEventDispatcher().dispatch(EventType::UIClicked);
    }
}
```

## 节点层级管理

### 父子关系

```cpp
// 添加子节点
parent->addChild(child);

// 移除子节点
parent->removeChild(child);

// 移除所有子节点
parent->removeAllChildren();

// 获取父节点
Node* parent = child->getParent();

// 获取子节点列表
const auto& children = parent->getChildren();
```

### Z轴顺序

```cpp
// 设置Z轴顺序（值越大越在上层）
node->setZOrder(10);

// 重新排序子节点
parent->reorderChild(child, newZOrder);
```

## 自定义节点

### 继承 Node 创建自定义节点

```cpp
class Player : public Node {
public:
    static Ptr<Player> create(Ptr<Texture> texture) {
        auto player = makePtr<Player>();
        if (player->init(texture)) {
            return player;
        }
        return nullptr;
    }
    
    bool init(Ptr<Texture> texture) {
        sprite_ = Sprite::create(texture);
        addChild(sprite_);
        return true;
    }
    
    void onUpdate(float dt) override {
        // 更新逻辑
        velocity_.y += gravity_ * dt;
        setPosition(getPosition() + velocity_ * dt);
        
        // 重要：调用父类的 onUpdate，确保子节点也被更新
        Node::onUpdate(dt);
    }
    
    void jump() {
        velocity_.y = jumpForce_;
    }
    
private:
    Ptr<Sprite> sprite_;
    Vec2 velocity_;
    float gravity_ = -980.0f;
    float jumpForce_ = 500.0f;
};

// 使用
auto player = Player::create(texture);
scene->addChild(player);
```

## 完整示例：GameOverLayer

```cpp
void GameOverLayer::onEnter() {
    Node::onEnter();
    
    auto& app = extra2d::Application::instance();
    float screenWidth = static_cast<float>(app.getConfig().width);
    float screenHeight = static_cast<float>(app.getConfig().height);
    
    // 整体居中（x 坐标相对于屏幕中心）
    setPosition(extra2d::Vec2(screenWidth / 2.0f, screenHeight));
    
    // 显示 "Game Over" 文字
    auto gameOverFrame = ResLoader::getKeyFrame("text_game_over");
    if (gameOverFrame) {
        auto gameOver = extra2d::Sprite::create(
            gameOverFrame->getTexture(),
            gameOverFrame->getRect()
        );
        gameOver->setAnchor(extra2d::Vec2(0.5f, 0.0f));
        gameOver->setPosition(extra2d::Vec2(0.0f, 120.0f));
        addChild(gameOver);
    }
    
    // 初始化得分面板
    initPanel(score_, screenHeight);
    
    // 初始化按钮
    initButtons();
    
    // 创建向上移动的动画
    auto moveAction = extra2d::makePtr<extra2d::MoveBy>(
        1.0f, extra2d::Vec2(0.0f, -screenHeight)
    );
    runAction(moveAction);
}

void GameOverLayer::initPanel(int score, float screenHeight) {
    // 显示得分板
    auto panelFrame = ResLoader::getKeyFrame("score_panel");
    if (!panelFrame) return;
    
    auto panel = extra2d::Sprite::create(
        panelFrame->getTexture(),
        panelFrame->getRect()
    );
    panel->setAnchor(extra2d::Vec2(0.5f, 0.5f));
    panel->setPosition(extra2d::Vec2(0.0f, screenHeight / 2.0f));
    addChild(panel);
    
    // 显示本局得分（相对于 panel 的本地坐标）
    auto scoreNumber = extra2d::makePtr<Number>();
    scoreNumber->setLittleNumber(score);
    scoreNumber->setPosition(extra2d::Vec2(95.0f, 10.0f));
    panel->addChild(scoreNumber);
    
    // 显示最高分
    static int bestScore = 0;
    if (score > bestScore) bestScore = score;
    
    auto bestNumber = extra2d::makePtr<Number>();
    bestNumber->setLittleNumber(bestScore);
    bestNumber->setPosition(extra2d::Vec2(95.0f, 50.0f));
    panel->addChild(bestNumber);
    
    // 显示 "New" 标记（如果破了记录）
    if (score >= bestScore && score > 0) {
        auto newFrame = ResLoader::getKeyFrame("new");
        if (newFrame) {
            auto newSprite = extra2d::Sprite::create(
                newFrame->getTexture(),
                newFrame->getRect()
            );
            newSprite->setAnchor(extra2d::Vec2(0.5f, 0.5f));
            newSprite->setPosition(extra2d::Vec2(80.0f, 25.0f));
            panel->addChild(newSprite);
        }
    }
}
```

## 最佳实践

1. **始终使用中心锚点（0.5, 0.5）**：除非有特殊需求，否则使用中心锚点可以使定位更直观

2. **理解本地坐标和世界坐标**：
   - `setPosition()` 设置的是本地坐标
   - 使用 `convertToWorldSpace()` 和 `convertToNodeSpace()` 进行转换

3. **利用父子关系**：
   - 将相关的 UI 元素组织为父子关系
   - 父节点移动时，子节点会自动跟随

4. **在 onEnter 中初始化**：
   - 不要在构造函数中创建子节点
   - 在 `onEnter()` 中初始化，此时 `weak_from_this()` 可用

5. **调用父类的虚函数**：
   - 重写 `onUpdate()` 时，记得调用 `Node::onUpdate(dt)`
   - 重写 `onEnter()` 时，记得调用 `Node::onEnter()`

6. **使用 batchUpdateTransforms**：
   - 引擎会自动在渲染前调用
   - 如果需要强制更新变换，可以手动调用

7. **避免双重引用**：
   - 节点通过 `addChild()` 添加到场景后，由场景统一管理
   - **不要**额外存储 `shared_ptr` 到 vector 中，避免双重引用问题
   - 使用 `getChildren()` 访问子节点，配合 `dynamic_cast` 筛选特定类型

   ```cpp
   // ❌ 错误：双重引用
   class BadScene : public Scene {
   private:
       std::vector<Ptr<Sprite>> sprites_;  // 不要这样做！
   public:
       void createSprite() {
           auto sprite = Sprite::create(texture);
           addChild(sprite);
           sprites_.push_back(sprite);  // 双重引用！
       }
   };
   
   // ✅ 正确：通过 getChildren() 访问
   class GoodScene : public Scene {
   public:
       void createSprite() {
           auto sprite = Sprite::create(texture);
           addChild(sprite);  // 场景统一管理
       }
       
       void updateSprites() {
           for (const auto& child : getChildren()) {
               if (auto sprite = dynamic_cast<Sprite*>(child.get())) {
                   // 处理 sprite
               }
           }
       }
   };
   ```

## 动画系统

### 动作类型

Extra2D 提供了丰富的动作类：

| 动作类 | 说明 |
|--------|------|
| `MoveTo` | 移动到指定位置 |
| `MoveBy` | 移动指定偏移量 |
| `ScaleTo` | 缩放到指定比例 |
| `ScaleBy` | 缩放指定比例 |
| `RotateTo` | 旋转到指定角度 |
| `RotateBy` | 旋转指定角度 |
| `FadeIn` | 淡入 |
| `FadeOut` | 淡出 |
| `FadeTo` | 淡化到指定透明度 |
| `Delay` | 延迟 |
| `Sequence` | 顺序执行多个动作 |
| `Spawn` | 同时执行多个动作 |
| `Loop` | 循环执行动作 |
| `CallFunc` | 回调函数 |

### 运行动作

```cpp
// 移动动画
auto moveAction = makePtr<MoveBy>(1.0f, Vec2(0.0f, -100.0f));
node->runAction(moveAction);

// 缩放动画
auto scaleAction = makePtr<ScaleTo>(0.5f, 2.0f);
node->runAction(scaleAction);

// 淡入淡出
auto fadeOut = makePtr<FadeOut>(0.3f);
auto fadeIn = makePtr<FadeIn>(0.3f);
node->runAction(fadeOut);
```

### 动画完成回调

使用 `setCompletionCallback` 在动画完成时执行回调：

```cpp
// 参考 examples/flappy_bird/GameOverLayer.cpp
void GameOverLayer::onEnter() {
    Node::onEnter();

    // 创建向上移动的动画
    auto moveAction = extra2d::makePtr<extra2d::MoveBy>(
        1.0f, extra2d::Vec2(0.0f, -screenHeight));
    
    // 设置动画完成回调
    moveAction->setCompletionCallback([this]() {
        animationDone_ = true;
        
        // 动画完成后启用按钮
        if (restartBtn_)
            restartBtn_->setEnabled(true);
        if (menuBtn_)
            menuBtn_->setEnabled(true);
    });
    
    runAction(moveAction);
}
```

### 顺序和并行动画

```cpp
// 顺序执行：先移动，再缩放，最后淡出
auto sequence = makePtr<Sequence>({
    new MoveTo(1.0f, Vec2(100, 100)),
    new ScaleTo(0.5f, 2.0f),
    new FadeOut(0.3f)
});
node->runAction(sequence);

// 并行执行：同时移动和旋转
auto spawn = makePtr<Spawn>({
    new MoveTo(1.0f, Vec2(100, 100)),
    new RotateBy(1.0f, 360.0f)
});
node->runAction(spawn);

// 循环执行
auto loop = makePtr<Loop(new RotateBy(1.0f, 360.0f), 5);  // 旋转5次
node->runAction(loop);
```

### 动画进度回调

```cpp
auto action = makePtr<MoveTo>(2.0f, Vec2(100, 100));
action->setProgressCallback([](float progress) {
    // progress: 0.0 - 1.0
    E2D_LOG_INFO("动画进度: {}%", progress * 100);
});
node->runAction(action);
```

### 停止动画

```cpp
// 停止所有动画
node->stopAllActions();

// 停止特定动画（需要先设置 tag）
action->setTag(1);
node->runAction(action);
node->stopActionByTag(1);
```

## 下一步

- [04. 资源管理](./04_Resource_Management.md) - 深入了解资源加载
- [05. 输入处理](./05_Input_Handling.md) - 学习输入处理
- [06. 碰撞检测](./06_Collision_Detection.md) - 学习碰撞检测系统
