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
// 位置
node->setPosition(Vec2(x, y));
Vec2 pos = node->getPosition();

// 旋转（角度）
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

### 链式调用（Builder 模式）

```cpp
// 使用链式调用快速配置节点
auto text = Text::create("标题", font)
    ->withPosition(640.0f, 100.0f)
    ->withAnchor(0.5f, 0.5f)
    ->withTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f))
    ->withCoordinateSpace(CoordinateSpace::Screen);
addChild(text);
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

### 精灵动画

```cpp
// 创建帧动画
auto anim = AnimatedSprite::createFromGrid(
    "player.png",  // 纹理
    32, 32,        // 单帧宽高
    100.0f,        // 帧间隔(ms)
    8              // 总帧数
);

// 播放动画
anim->play();

// 设置帧范围
anim->setFrameRange(0, 3);

// 循环播放
anim->setLoop(true);

// 停止动画
anim->stop();
```

## 文本（Text）

### 创建文本

```cpp
// 加载字体
auto font = resources.loadFont("assets/font.ttf", 24, true);

// 创建文本
auto text = Text::create("Hello World", font);
text->setPosition(Vec2(400, 300));
addChild(text);

// 动态修改文本
text->setText("新的文本内容");

// 使用格式化文本
text->setFormat("得分: %d", score);
```

### 文本样式

```cpp
text->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));  // 颜色
text->setFontSize(48);                              // 字体大小
text->setAnchor(Vec2(0.5f, 0.5f));                  // 锚点

// 坐标空间
text->withCoordinateSpace(CoordinateSpace::Screen)  // 屏幕空间
      ->withScreenPosition(100.0f, 50.0f);
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
button->onClick([]() {
    E2D_LOG_INFO("按钮被点击！");
});

addChild(button);
```

### 透明按钮（用于菜单）

```cpp
// 创建纯文本按钮（透明背景）
auto menuBtn = Button::create();
menuBtn->setFont(font);
menuBtn->setText("菜单项");
menuBtn->setTextColor(Colors::Black);
menuBtn->setBackgroundColor(
    Colors::Transparent,
    Colors::Transparent,
    Colors::Transparent
);
menuBtn->setBorder(Colors::Transparent, 0.0f);
menuBtn->setAnchor(0.5f, 0.5f);
menuBtn->setPosition(centerX, centerY);
addChild(menuBtn);
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
    
    void update(float dt) override {
        // 更新逻辑
        velocity_.y += gravity_ * dt;
        setPosition(getPosition() + velocity_ * dt);
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

## 空间索引（碰撞检测）

### 启用空间索引

```cpp
class CollidableNode : public Node {
public:
    CollidableNode() {
        // 启用空间索引
        setSpatialIndexed(true);
    }
    
    // 必须实现 getBoundingBox
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - 25, pos.y - 25, 50, 50);
    }
};
```

### 查询碰撞

```cpp
// 在场景中查询所有碰撞
auto collisions = scene->queryCollisions();

for (const auto& [nodeA, nodeB] : collisions) {
    // 处理碰撞
    handleCollision(nodeA, nodeB);
}
```

## 完整示例

参考 `examples/push_box/PlayScene.cpp` 中的节点使用：

```cpp
void PlayScene::onEnter() {
    Scene::onEnter();
    
    auto& resources = app.resources();
    
    // 加载纹理资源
    texWall_ = resources.loadTexture("assets/images/wall.gif");
    texBox_ = resources.loadTexture("assets/images/box.gif");
    
    // 创建地图层
    mapLayer_ = makePtr<Node>();
    addChild(mapLayer_);
    
    // 创建地图元素
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            char cell = map_[y][x];
            
            if (cell == '#') {
                auto wall = Sprite::create(texWall_);
                wall->setPosition(x * tileSize, y * tileSize);
                mapLayer_->addChild(wall);
            }
            else if (cell == '$') {
                auto box = Sprite::create(texBox_);
                box->setPosition(x * tileSize, y * tileSize);
                mapLayer_->addChild(box);
            }
        }
    }
    
    // 创建UI文本
    font28_ = resources.loadFont("assets/font.ttf", 28, true);
    levelText_ = Text::create("Level: 1", font28_);
    levelText_->setPosition(50, 30);
    addChild(levelText_);
}
```

## 下一步

- [04. 资源管理](./04_Resource_Management.md) - 深入了解资源加载
- [05. 输入处理](./05_Input_Handling.md) - 学习输入处理
- [06. 碰撞检测](./06_Collision_Detection.md) - 学习碰撞检测系统
