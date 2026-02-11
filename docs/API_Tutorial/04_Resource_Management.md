# 04. 资源管理

Extra2D 提供了统一的资源管理系统，用于加载和管理游戏中的各种资源。

## 资源管理器

通过 `Application::instance().resources()` 访问资源管理器：

```cpp
auto& resources = Application::instance().resources();
```

## 支持的资源类型

| 资源类型 | 加载方法 | 说明 |
|---------|---------|------|
| 纹理 | `loadTexture()` | 图片文件 (PNG, JPG, etc.) |
| 字体 | `loadFont()` | TrueType 字体文件 |
| 音频 | `loadSound()` / `loadMusic()` | 音频文件 |

## 纹理加载

### 基本用法

```cpp
// 加载纹理
auto texture = resources.loadTexture("assets/images/player.png");

if (texture) {
    // 创建精灵
    auto sprite = Sprite::create(texture);
    addChild(sprite);
}
```

### 纹理缓存

资源管理器会自动缓存已加载的纹理，多次加载同一文件会返回缓存的实例：

```cpp
// 第一次加载 - 从文件读取
auto tex1 = resources.loadTexture("assets/image.png");

// 第二次加载 - 返回缓存
auto tex2 = resources.loadTexture("assets/image.png");

// tex1 和 tex2 指向同一个纹理对象
```

## 字体加载

### 基本用法

```cpp
// 加载字体（指定字号）
auto font24 = resources.loadFont("assets/font.ttf", 24, true);

// 创建文本
auto text = Text::create("Hello World", font24);
addChild(text);
```

### 字体后备

支持设置后备字体，当主字体缺少某些字符时自动使用后备字体：

```cpp
// 加载主字体和后备字体
auto mainFont = resources.loadFont("assets/main.ttf", 24, true);
auto fallbackFont = resources.loadFont("assets/fallback.ttf", 24, true);

// 设置后备字体
mainFont->setFallback(fallbackFont);
```

## 资源路径

### 路径格式

```cpp
// 相对路径（相对于工作目录）
auto tex = resources.loadTexture("assets/images/player.png");

// Switch 平台使用 romfs
auto tex = resources.loadTexture("romfs:/images/player.png");

// SD 卡路径
auto tex = resources.loadTexture("sdmc:/switch/game/images/player.png");
```

### 路径辅助函数

```cpp
// 获取平台特定的资源路径
std::string path = ResourceManager::getPlatformPath("images/player.png");
// Windows: "assets/images/player.png"
// Switch: "romfs:/images/player.png"
```

## 资源释放

### 自动释放

资源使用智能指针管理，当没有引用时会自动释放：

```cpp
{
    auto tex = resources.loadTexture("assets/temp.png");
    // 使用纹理...
} // 超出作用域，如果没有其他引用，纹理自动释放
```

### 手动清理缓存

```cpp
// 清理未使用的资源
resources.cleanupUnused();

// 清空所有缓存（谨慎使用）
resources.clearCache();
```

## 完整示例

参考 `examples/push_box/StartScene.cpp`：

```cpp
void StartScene::onEnter() {
    Scene::onEnter();
    
    auto& app = Application::instance();
    auto& resources = app.resources();
    
    // 加载背景纹理
    auto bgTex = resources.loadTexture("assets/images/start.jpg");
    if (bgTex) {
        auto background = Sprite::create(bgTex);
        background->setAnchor(0.0f, 0.0f);
        addChild(background);
    }
    
    // 加载音效图标纹理
    auto soundOn = resources.loadTexture("assets/images/soundon.png");
    auto soundOff = resources.loadTexture("assets/images/soundoff.png");
    if (soundOn && soundOff) {
        soundIcon_ = Sprite::create(g_SoundOpen ? soundOn : soundOff);
        addChild(soundIcon_);
    }
    
    // 加载字体
    font_ = resources.loadFont("assets/font.ttf", 28, true);
    
    // 创建按钮...
}
```

## 最佳实践

1. **预加载资源** - 在场景 `onEnter()` 中加载所需资源
2. **检查资源有效性** - 始终检查加载结果是否为 nullptr
3. **复用资源** - 多次使用同一资源时保存指针，避免重复加载
4. **合理设置字号** - 字体加载时会生成对应字号的图集

## 下一步

- [05. 输入处理](./05_Input_Handling.md) - 学习输入处理
- [06. 碰撞检测](./06_Collision_Detection.md) - 学习碰撞检测系统
