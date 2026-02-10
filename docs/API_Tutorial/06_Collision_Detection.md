# Extra2D API 教程 - 06. 碰撞检测

## 空间索引系统

Extra2D 内置了空间索引系统，用于高效地进行碰撞检测。

### 启用空间索引

```cpp
void onEnter() override {
    Scene::onEnter();
    
    // 启用空间索引
    setSpatialIndexingEnabled(true);
}
```

## 碰撞节点

### 创建可碰撞节点

```cpp
class PhysicsNode : public Node {
public:
    PhysicsNode(float size, const Color &color) 
        : size_(size), color_(color), isColliding_(false) {
        // 启用空间索引（关键！）
        setSpatialIndexed(true);
    }
    
    // 必须实现 getBoundingBox()
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - size_ / 2, pos.y - size_ / 2, size_, size_);
    }
    
    void setColliding(bool colliding) { isColliding_ = colliding; }
    
    void onRender(RenderBackend &renderer) override {
        Vec2 pos = getPosition();
        
        // 碰撞时变红色
        Color fillColor = isColliding_ ? Color(1.0f, 0.2f, 0.2f, 0.9f) : color_;
        renderer.fillRect(Rect(pos.x - size_ / 2, pos.y - size_ / 2, size_, size_), 
                         fillColor);
    }
    
private:
    float size_;
    Color color_;
    bool isColliding_;
};
```

## 碰撞检测

### 查询所有碰撞

```cpp
void performCollisionDetection() {
    // 清除之前的碰撞状态
    for (auto &node : nodes_) {
        node->setColliding(false);
    }
    
    // 查询所有碰撞（使用空间索引）
    auto collisions = queryCollisions();
    
    // 标记碰撞的节点
    for (const auto &[nodeA, nodeB] : collisions) {
        if (auto boxA = dynamic_cast<PhysicsNode *>(nodeA)) {
            boxA->setColliding(true);
        }
        if (auto boxB = dynamic_cast<PhysicsNode *>(nodeB)) {
            boxB->setColliding(true);
        }
    }
}
```

## 空间索引策略

### 切换策略

```cpp
void onEnter() override {
    Scene::onEnter();
    
    // 启用空间索引
    setSpatialIndexingEnabled(true);
    
    // 设置空间索引策略
    auto &spatialManager = getSpatialManager();
    spatialManager.setStrategy(SpatialStrategy::QuadTree);   // 四叉树
    // 或
    spatialManager.setStrategy(SpatialStrategy::SpatialHash); // 空间哈希
}

// 切换策略
void toggleStrategy() {
    auto &spatialManager = getSpatialManager();
    SpatialStrategy current = spatialManager.getCurrentStrategy();
    
    if (current == SpatialStrategy::QuadTree) {
        spatialManager.setStrategy(SpatialStrategy::SpatialHash);
    } else {
        spatialManager.setStrategy(SpatialStrategy::QuadTree);
    }
}
```

### 策略对比

| 策略 | 适用场景 | 特点 |
|------|----------|------|
| QuadTree | 节点分布不均匀 | 适合稀疏场景 |
| SpatialHash | 节点分布均匀 | 适合密集场景 |

## 完整示例

```cpp
class CollisionScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 启用空间索引
        setSpatialIndexingEnabled(true);
        
        // 创建碰撞节点
        createNodes(100);
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 更新节点位置
        for (auto &node : nodes_) {
            node->update(dt);
        }
        
        // 执行碰撞检测
        performCollisionDetection();
    }
    
    void onRender(RenderBackend &renderer) override {
        Scene::onRender(renderer);
        
        // 绘制碰撞统计
        std::string text = "Collisions: " + std::to_string(collisionCount_);
        // ...
    }
    
private:
    std::vector<Ptr<PhysicsNode>> nodes_;
    size_t collisionCount_ = 0;
    
    void createNodes(size_t count) {
        for (size_t i = 0; i < count; ++i) {
            auto node = makePtr<PhysicsNode>(20.0f, Color(0.5f, 0.5f, 0.9f, 0.7f));
            node->setPosition(randomPosition());
            addChild(node);
            nodes_.push_back(node);
        }
    }
    
    void performCollisionDetection() {
        // 清除碰撞状态
        for (auto &node : nodes_) {
            node->setColliding(false);
        }
        
        // 查询碰撞
        auto collisions = queryCollisions();
        collisionCount_ = collisions.size();
        
        // 标记碰撞节点
        for (const auto &[nodeA, nodeB] : collisions) {
            if (auto boxA = dynamic_cast<PhysicsNode *>(nodeA)) {
                boxA->setColliding(true);
            }
            if (auto boxB = dynamic_cast<PhysicsNode *>(nodeB)) {
                boxB->setColliding(true);
            }
        }
    }
};
```

## 注意事项

### 必须调用 Scene::onEnter()

```cpp
void onEnter() override {
    Scene::onEnter();  // 必须调用！
    
    // 否则子节点无法注册到空间索引
    setSpatialIndexingEnabled(true);
}
```

### 必须实现 getBoundingBox()

```cpp
class MyNode : public Node {
public:
    MyNode() {
        setSpatialIndexed(true);
    }
    
    // 必须实现！
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - width_/2, pos.y - height_/2, width_, height_);
    }
};
```

## 下一步

- [07. UI 系统](07_UI_System.md)
- [08. 音频系统](08_Audio_System.md)
