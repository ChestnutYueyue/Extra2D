# Extra2D API 教程 - 04. 资源管理

## 资源管理器

Extra2D 使用资源管理器来统一加载和管理资源。

### 获取资源管理器

```cpp
auto &resources = Application::instance().resources();
```

## 字体资源

### 加载字体

```cpp
// 加载字体（路径，大小，使用后备字体）
auto font = resources.loadFont("assets/font.ttf", 48, true);

if (!font) {
    E2D_LOG_ERROR("字体加载失败！");
}
```

### 使用字体

```cpp
void onRender(RenderBackend &renderer) override {
    if (font) {
        renderer.drawText(*font, "Hello World", Vec2(100.0f, 100.0f), 
                         Colors::White);
    }
}
```

## 纹理资源

### 加载纹理

```cpp
// 加载纹理
auto texture = resources.loadTexture("assets/player.png");

if (!texture) {
    E2D_LOG_ERROR("纹理加载失败！");
}
```

### 创建精灵

```cpp
auto sprite = Sprite::create(texture);
sprite->setPosition(Vec2(640.0f, 360.0f));
addChild(sprite);
```

## 音效资源

### 加载音效

```cpp
// 加载音效
auto sound = resources.loadSound("assets/jump.wav");

// 播放音效
sound->play();

// 循环播放
sound->play(true);

// 停止播放
sound->stop();
```

## 资源路径解析

Extra2D 的资源管理器支持多平台路径解析：

### 路径优先级

1. **原始路径**: `assets/font.ttf`
2. **romfs 路径**: `romfs:/assets/font.ttf` (Switch)
3. **sdmc 路径**: `sdmc:/assets/font.ttf` (Switch SD卡)
4. **可执行文件相对路径** (Windows)

### 使用示例

```cpp
// 所有平台使用相同的路径
auto font = resources.loadFont("assets/font.ttf", 48, true);
auto texture = resources.loadTexture("assets/images/player.png");
auto sound = resources.loadSound("assets/audio/jump.wav");
```

## 完整示例

```cpp
class GameScene : public Scene {
public:
    void onEnter() override {
        Scene::onEnter();
        
        auto &resources = Application::instance().resources();
        
        // 加载字体
        titleFont_ = resources.loadFont("assets/font.ttf", 60, true);
        infoFont_ = resources.loadFont("assets/font.ttf", 24, true);
        
        // 加载纹理
        playerTexture_ = resources.loadTexture("assets/player.png");
        enemyTexture_ = resources.loadTexture("assets/enemy.png");
        
        // 创建精灵
        player_ = Sprite::create(playerTexture_);
        player_->setPosition(Vec2(640.0f, 360.0f));
        addChild(player_);
    }
    
    void onRender(RenderBackend &renderer) override {
        Scene::onRender(renderer);
        
        // 绘制文字
        if (titleFont_) {
            renderer.drawText(*titleFont_, "Game Title", 
                            Vec2(50.0f, 50.0f), Colors::White);
        }
        
        if (infoFont_) {
            std::string fps = "FPS: " + std::to_string(Application::instance().fps());
            renderer.drawText(*infoFont_, fps, 
                            Vec2(50.0f, 100.0f), Colors::Yellow);
        }
    }
    
private:
    Ptr<FontAtlas> titleFont_;
    Ptr<FontAtlas> infoFont_;
    Ptr<Texture> playerTexture_;
    Ptr<Texture> enemyTexture_;
    Ptr<Sprite> player_;
};
```

## 下一步

- [05. 输入处理](05_Input_Handling.md)
- [06. 碰撞检测](06_Collision_Detection.md)
