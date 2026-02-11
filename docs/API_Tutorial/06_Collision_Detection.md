# 06. 碰撞检测

Extra2D 提供了基于空间索引的高效碰撞检测系统，支持四叉树和空间哈希两种策略。

## 完整示例

参考示例代码：
- `examples/collision_demo/main.cpp` - 基础碰撞检测演示
- `examples/spatial_index_demo/main.cpp` - 空间索引性能演示

## 启用碰撞检测

### 1. 创建可碰撞节点

```cpp
class CollidableBox : public Node {
public:
    CollidableBox(float width, float height, const Color& color)
        : width_(width), height_(height), color_(color), isColliding_(false) {
        // 启用空间索引 - 这是关键！
        setSpatialIndexed(true);
    }
    
    // 必须实现 getBoundingBox 方法
    Rect getBoundingBox() const override {
        Vec2 pos = getPosition();
        return Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_);
    }
    
    void setColliding(bool colliding) { isColliding_ = colliding; }
    
    void onRender(RenderBackend& renderer) override {
        Vec2 pos = getPosition();
        
        // 碰撞时变红色
        Color fillColor = isColliding_ ? Color(1.0f, 0.2f, 0.2f, 0.8f) : color_;
        renderer.fillRect(
            Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
            fillColor
        );
        
        // 绘制边框
        Color borderColor = isColliding_ ? Color(1.0f, 0.0f, 0.0f, 1.0f)
                                         : Color(1.0f, 1.0f, 1.0f, 0.5f);
        renderer.drawRect(
            Rect(pos.x - width_ / 2, pos.y - height_ / 2, width_, height_),
            borderColor, 2.0f
        );
    }
    
private:
    float width_, height_;
    Color color_;
    bool isColliding_;
};
```

### 2. 执行碰撞检测

```cpp
class GameScene : public Scene {
public:
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 清除之前的碰撞状态
        for (auto& box : boxes_) {
            box->setColliding(false);
        }
        
        // 使用场景的空间索引查询所有碰撞
        auto collisions = queryCollisions();
        
        // 处理碰撞
        for (const auto& [nodeA, nodeB] : collisions) {
            if (auto boxA = dynamic_cast<CollidableBox*>(nodeA)) {
                boxA->setColliding(true);
            }
            if (auto boxB = dynamic_cast<CollidableBox*>(nodeB)) {
                boxB->setColliding(true);
            }
        }
    }
    
private:
    std::vector<Ptr<CollidableBox>> boxes_;
};
```

## 空间索引策略

### 切换策略

```cpp
// 获取空间管理器
auto& spatialManager = getSpatialManager();

// 切换到四叉树
spatialManager.setStrategy(SpatialStrategy::QuadTree);

// 切换到空间哈希
spatialManager.setStrategy(SpatialStrategy::SpatialHash);

// 获取当前策略名称
const char* name = spatialManager.getStrategyName();
```

### 策略对比

| 策略 | 适用场景 | 特点 |
|------|---------|------|
| QuadTree | 节点分布不均匀 | 分层划分，适合稀疏分布 |
| SpatialHash | 节点分布均匀 | 均匀网格，适合密集分布 |

## 性能演示

`examples/spatial_index_demo/main.cpp` 展示了空间索引的性能优势：

```cpp
class SpatialIndexDemoScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        // 创建100个碰撞节点
        createNodes(100);
        
        E2D_LOG_INFO("创建了 {} 个碰撞节点", nodes_.size());
        E2D_LOG_INFO("空间索引已启用: {}", isSpatialIndexingEnabled());
    }
    
    void onUpdate(float dt) override {
        Scene::onUpdate(dt);
        
        // 更新所有节点位置
        for (auto& node : nodes_) {
            node->update(dt, screenWidth_, screenHeight_);
        }
        
        // 使用空间索引进行碰撞检测
        performCollisionDetection();
        
        // 按 X 键切换索引策略
        auto& input = Application::instance().input();
        if (input.isButtonPressed(GamepadButton::X)) {
            toggleSpatialStrategy();
        }
    }
    
private:
    void performCollisionDetection() {
        // 清除之前的碰撞状态
        for (auto& node : nodes_) {
            node->setColliding(false);
        }
        
        // 使用引擎自带的空间索引进行碰撞检测
        auto collisions = queryCollisions();
        
        // 标记碰撞的节点
        for (const auto& [nodeA, nodeB] : collisions) {
            if (auto boxA = dynamic_cast<PhysicsNode*>(nodeA)) {
                boxA->setColliding(true);
            }
            if (auto boxB = dynamic_cast<PhysicsNode*>(nodeB)) {
                boxB->setColliding(true);
            }
        }
    }
    
    void toggleSpatialStrategy() {
        auto& spatialManager = getSpatialManager();
        SpatialStrategy currentStrategy = spatialManager.getCurrentStrategy();
        
        if (currentStrategy == SpatialStrategy::QuadTree) {
            spatialManager.setStrategy(SpatialStrategy::SpatialHash);
            E2D_LOG_INFO("切换到空间哈希策略");
        } else {
            spatialManager.setStrategy(SpatialStrategy::QuadTree);
            E2D_LOG_INFO("切换到四叉树策略");
        }
    }
};
```

## 关键要点

1. **必须调用 `setSpatialIndexed(true)`** - 启用节点的空间索引
2. **必须实现 `getBoundingBox()`** - 返回准确的边界框
3. **在 `onEnter()` 中调用 `Scene::onEnter()`** - 确保节点正确注册到空间索引
4. **使用 `queryCollisions()`** - 自动利用空间索引优化检测

## 下一步

- [07. UI 系统](./07_UI_System.md) - 学习 UI 控件使用
- [08. 音频系统](./08_Audio_System.md) - 学习音频播放
