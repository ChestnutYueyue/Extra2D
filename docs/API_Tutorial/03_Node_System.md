# Extra2D API 教程 - 03. 节点系统

## 节点基础

节点(Node)是游戏对象的基本单位，可以包含子节点，形成树形结构。

### 创建节点

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

class MyNode : public Node {
public:
    MyNode() {
        // 设置位置
        setPosition(Vec2(100.0f, 200.0f));
        
        // 设置旋转（度）
        setRotation(45.0f);
        
        // 设置缩放
        setScale(Vec2(2.0f, 2.0f));
        
        // 设置锚点（0-1范围，默认0.5是中心）
        setAnchor(0.5f, 0.5f);
        
        // 设置可见性
        setVisible(true);
    }
    
    // 每帧更新
    void onUpdate(float dt) override {
        Node::onUpdate(dt);
        // 自定义更新逻辑
    }
    
    // 渲染
    void onRender(RenderBackend &renderer) override {
        Node::onRender(renderer);
        // 自定义渲染
    }
};
```

## 节点层级

### 添加子节点

```cpp
void onEnter() override {
    Scene::onEnter();
    
    // 创建子节点
    auto child = makePtr<MyNode>();
    
    // 添加到场景
    addChild(child);
    
    // 在指定位置添加
    addChild(child, 0);  // z-order = 0
}
```

### 移除子节点

```cpp
// 移除指定子节点
removeChild(child);

// 移除所有子节点
removeAllChildren();

// 通过名称移除
removeChildByName("myNode");
```

### 获取子节点

```cpp
// 获取子节点数量
size_t count = getChildren().size();

// 通过名称查找
auto node = getChildByName("myNode");

// 遍历子节点
for (auto &child : getChildren()) {
    // 处理子节点
}
```

## 空间索引

### 启用空间索引

```cpp
class PhysicsNode : public Node {
public:
    PhysicsNode() {
        // 启用空间索引（用于碰撞检测）
        setSpatialIndexed(true);
    }
    
    // 必须实现 getBoundingBox()
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - 25.0f, pos.y - 25.0f, 50.0f, 50.0f);
    }
};
```

### 边界框

```cpp
// 获取节点边界框
Rect bounds = node->getBoundingBox();

// 检查点是否在边界框内
if (bounds.contains(Vec2(x, y))) {
    // 点在边界框内
}

// 检查两个边界框是否相交
if (bounds.intersects(otherBounds)) {
    // 边界框相交
}
```

## 精灵节点

### 创建精灵

```cpp
// 加载纹理
auto texture = resources.loadTexture("assets/player.png");

// 创建精灵
auto sprite = Sprite::create(texture);

// 设置位置
sprite->setPosition(Vec2(640.0f, 360.0f));

// 设置锚点（中心）
sprite->setAnchor(0.5f, 0.5f);

// 添加到场景
addChild(sprite);
```

### 精灵动画

```cpp
// 创建动画
auto animation = Animation::create("walk", 0.1f);
animation->addFrame(resources.loadTexture("assets/walk1.png"));
animation->addFrame(resources.loadTexture("assets/walk2.png"));
animation->addFrame(resources.loadTexture("assets/walk3.png"));

// 播放动画
sprite->playAnimation(animation, true);  // true = 循环播放

// 停止动画
sprite->stopAnimation();
```

## 完整示例

```cpp
class Player : public Node {
public:
    Player() {
        setSpatialIndexed(true);
        
        // 加载精灵
        auto &resources = Application::instance().resources();
        auto texture = resources.loadTexture("assets/player.png");
        sprite_ = Sprite::create(texture);
        sprite_->setAnchor(0.5f, 0.5f);
        addChild(sprite_);
    }
    
    void onUpdate(float dt) override {
        Node::onUpdate(dt);
        
        // 移动
        Vec2 pos = getPosition();
        pos = pos + velocity_ * dt;
        setPosition(pos);
        
        // 边界检查
        auto &app = Application::instance();
        float width = static_cast<float>(app.getConfig().width);
        float height = static_cast<float>(app.getConfig().height);
        
        if (pos.x < 0 || pos.x > width) {
            velocity_.x = -velocity_.x;
        }
        if (pos.y < 0 || pos.y > height) {
            velocity_.y = -velocity_.y;
        }
    }
    
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - 25.0f, pos.y - 25.0f, 50.0f, 50.0f);
    }
    
private:
    Ptr<Sprite> sprite_;
    Vec2 velocity_{100.0f, 100.0f};
};
```

## 下一步

- [04. 资源管理](04_Resource_Management.md)
- [05. 输入处理](05_Input_Handling.md)
