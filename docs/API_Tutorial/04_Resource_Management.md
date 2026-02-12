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

### 异步加载

Extra2D 支持异步加载纹理，避免阻塞主线程：

```cpp
// 同步加载（默认）
auto texture = resources.loadTexture("assets/images/player.png");

// 异步加载
auto texture = resources.loadTexture("assets/images/player.png", true);

// 使用回调函数处理异步加载完成
resources.loadTextureAsync("assets/images/player.png", 
    TextureFormat::Auto,
    [](Ptr<Texture> texture, const std::string& path) {
        if (texture) {
            // 加载成功，可以安全使用
            auto sprite = Sprite::create(texture);
            // ...
        }
    });
```

### 纹理压缩格式

Extra2D 支持多种纹理压缩格式，可显著减少显存占用：

```cpp
// 支持的纹理格式
enum class TextureFormat {
    Auto,       // 自动选择最佳格式
    RGBA8,      // 32位 RGBA（无压缩）
    RGB8,       // 24位 RGB（无压缩）
    DXT1,       // DXT1 压缩（适用于不透明纹理）
    DXT5,       // DXT5 压缩（适用于透明纹理）
    ETC2,       // ETC2 压缩（移动平台）
    ASTC4x4,    // ASTC 4x4 高质量压缩
    ASTC8x8     // ASTC 8x8 高压缩率
};

// 使用压缩格式加载纹理
auto texture = resources.loadTexture("assets/images/player.png", false, TextureFormat::DXT5);

// 异步加载 + 压缩
auto texture = resources.loadTexture("assets/images/player.png", true, TextureFormat::ASTC4x4);
```

**格式选择建议：**

| 格式 | 压缩比 | 质量 | 适用场景 |
|------|--------|------|---------|
| RGBA8 | 1:1 | 最高 | 小图标、需要最高质量 |
| DXT1 | 1:8 | 高 | 不透明纹理、大背景图 |
| DXT5 | 1:4 | 高 | 透明纹理、角色精灵 |
| ETC2 | 1:4 | 高 | 移动设备、跨平台 |
| ASTC4x4 | 1:4 | 很高 | 高质量透明纹理 |
| ASTC8x8 | 1:16 | 中等 | 大纹理、远景贴图 |

### 纹理缓存

资源管理器会自动缓存已加载的纹理，多次加载同一文件会返回缓存的实例：

```cpp
// 第一次加载 - 从文件读取
auto tex1 = resources.loadTexture("assets/image.png");

// 第二次加载 - 返回缓存
auto tex2 = resources.loadTexture("assets/image.png");

// tex1 和 tex2 指向同一个纹理对象
```

### LRU 缓存机制

Extra2D 使用 LRU (Least Recently Used) 算法管理纹理缓存，自动清理最久未使用的纹理：

```cpp
// 配置纹理缓存参数
auto& resources = Application::instance().resources();

// 设置缓存参数：最大缓存大小(字节)、最大纹理数量、自动清理间隔(秒)
resources.setTextureCache(
    128 * 1024 * 1024,  // 128MB 最大缓存
    512,                 // 最多 512 个纹理
    30.0f               // 每 30 秒检查一次
);

// 在主循环中更新资源管理器（用于自动清理）
void GameScene::update(float dt) {
    // 这会触发缓存清理检查
    resources.update(dt);
}
```

#### 缓存统计

```cpp
// 获取缓存使用情况
size_t memoryUsage = resources.getTextureCacheMemoryUsage();  // 当前缓存大小（字节）
float hitRate = resources.getTextureCacheHitRate();            // 缓存命中率 (0.0 - 1.0)
size_t cacheSize = resources.getTextureCacheSize();            // 缓存中的纹理数量

// 打印详细统计信息
resources.printTextureCacheStats();
// 输出示例：
// [INFO] 纹理缓存统计:
// [INFO]   缓存纹理数: 45/512
// [INFO]   缓存大小: 32 / 128 MB
// [INFO]   缓存命中: 1024
// [INFO]   缓存未命中: 56
// [INFO]   命中率: 94.8%
```

#### 自动清理策略

LRU 缓存会自动执行以下清理策略：

1. **容量限制**：当缓存超过 `maxCacheSize` 或 `maxTextureCount` 时，自动驱逐最久未使用的纹理
2. **定时清理**：每 `unloadInterval` 秒检查一次，如果缓存超过 80%，清理到 50%
3. **访问更新**：每次访问纹理时，自动将其移到 LRU 链表头部（标记为最近使用）

```cpp
// 手动清理缓存
resources.clearTextureCache();  // 清空所有纹理缓存
resources.clearAllCaches();     // 清空所有资源缓存（纹理、字体、音效）

// 手动卸载特定纹理
resources.unloadTexture("assets/images/old_texture.png");
```

#### 缓存配置建议

| 平台 | 最大缓存大小 | 最大纹理数 | 清理间隔 | 说明 |
|------|-------------|-----------|---------|------|
| Switch 掌机模式 | 64-128 MB | 256-512 | 30s | 内存有限，保守设置 |
| Switch 主机模式 | 128-256 MB | 512-1024 | 30s | 内存充足，可以更大 |
| PC (MinGW) | 256-512 MB | 1024+ | 60s | 内存充足，可以更大 |

```cpp
// 根据平台设置不同的缓存策略
void setupCache() {
    auto& resources = Application::instance().resources();
    
#ifdef __SWITCH__
    // Switch 平台使用保守设置
    resources.setTextureCache(64 * 1024 * 1024, 256, 30.0f);
#else
    // PC 平台可以使用更大的缓存
    resources.setTextureCache(256 * 1024 * 1024, 1024, 60.0f);
#endif
}
```

### 纹理图集（Texture Atlas）

Extra2D 自动使用纹理图集优化渲染性能：

```cpp
// 获取纹理图集管理器
auto& atlasManager = resources.getTextureAtlasManager();

// 将多个纹理打包到图集（自动进行）
// 渲染时，相同图集的精灵会自动批处理

// 手动创建图集（高级用法）
auto atlas = atlasManager.createAtlas("ui_atlas", 2048, 2048);
atlas->addTexture("button", buttonTexture);
atlas->addTexture("icon", iconTexture);
atlas->pack();  // 执行打包
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
// 清理未使用的资源（清理字体和音效缓存中已失效的弱引用）
resources.purgeUnused();

// 清空特定类型的缓存
resources.clearTextureCache();  // 清空纹理缓存
resources.clearFontCache();     // 清空字体缓存
resources.clearSoundCache();    // 清空音效缓存

// 清空所有缓存（谨慎使用）
resources.clearAllCaches();

// 检查是否有正在进行的异步加载
if (resources.hasPendingAsyncLoads()) {
    // 等待所有异步加载完成
    resources.waitForAsyncLoads();
}
```

## 内存管理

### 内存池（内部自动管理）

Extra2D 使用内存池优化小对象分配，无需用户干预：

```cpp
// 内存池自动管理以下对象：
// - 场景节点
// - 渲染命令
// - 碰撞形状
// - 事件对象

// 用户代码无需特殊处理，正常使用即可
auto node = Node::create();  // 自动使用内存池
auto sprite = Sprite::create(texture);  // 自动使用内存池
```

### 批量更新（内部自动进行）

Extra2D 自动批量更新节点变换，优化性能：

```cpp
// 以下操作会自动批处理：
// - 节点变换更新
// - 渲染命令提交
// - 纹理绑定

// 用户代码无需特殊处理
for (int i = 0; i < 1000; ++i) {
    auto sprite = Sprite::create(texture);
    sprite->setPosition(i * 10, 100);
    addChild(sprite);  // 变换更新会自动批处理
}
```

## 渲染批处理

### 自动批处理

Extra2D 自动将渲染命令批处理以优化性能：

```cpp
// 以下情况会自动批处理：
// 1. 相同纹理的精灵
// 2. 相同图层的节点
// 3. 相同混合模式

// 示例：1000 个相同纹理的精灵会自动批处理为少量 draw call
for (int i = 0; i < 1000; ++i) {
    auto sprite = Sprite::create(texture);
    addChild(sprite);
}
```

### 手动控制渲染顺序

```cpp
// 设置节点的渲染层级（z-order）
sprite->setZOrder(10);  // 值越大，渲染越靠前

// 同层级的节点会自动批处理
```

## 完整示例

参考 `examples/push_box/StartScene.cpp`：

```cpp
void StartScene::onEnter() {
    Scene::onEnter();
    
    auto& app = Application::instance();
    auto& resources = app.resources();
    
    // 加载背景纹理（异步 + 压缩）
    auto bgTex = resources.loadTexture("assets/images/start.jpg", true, TextureFormat::DXT1);
    if (bgTex) {
        auto background = Sprite::create(bgTex);
        background->setAnchor(0.0f, 0.0f);
        addChild(background);
    }
    
    // 加载音效图标纹理（异步 + DXT5 压缩支持透明）
    auto soundOn = resources.loadTexture("assets/images/soundon.png", true, TextureFormat::DXT5);
    auto soundOff = resources.loadTexture("assets/images/soundoff.png", true, TextureFormat::DXT5);
    if (soundOn && soundOff) {
        soundIcon_ = Sprite::create(g_SoundOpen ? soundOn : soundOff);
        addChild(soundIcon_);
    }
    
    // 加载字体
    font_ = resources.loadFont("assets/font.ttf", 28, true);
    
    // 创建按钮...
}
```

## 性能优化建议

### 纹理优化

1. **使用纹理压缩** - 对大型纹理使用 DXT/ASTC 压缩减少显存占用
2. **使用纹理图集** - 将多个小纹理打包到图集，减少 draw call
3. **异步加载大纹理** - 避免在主线程加载大型资源造成卡顿
4. **合理设置纹理尺寸** - 避免使用过大的纹理（建议最大 2048x2048）
5. **配置合适的缓存大小** - 根据平台内存设置合理的 LRU 缓存参数
6. **监控缓存命中率** - 使用 `printTextureCacheStats()` 检查缓存效率

### 资源加载策略

```cpp
// 场景预加载
void GameScene::onEnter() {
    auto& resources = Application::instance().resources();
    
    // 预加载关键资源
    resources.loadTexture("assets/textures/player.png", true);
    resources.loadTexture("assets/textures/enemy.png", true);
    resources.loadFont("assets/fonts/main.ttf", 24, true);
}

// 异步加载非关键资源
void GameScene::loadOptionalResources() {
    resources.loadTextureAsync("assets/textures/background.jpg", 
        TextureFormat::DXT1,
        [](Ptr<Texture> tex, const std::string& path) {
            if (tex) {
                // 加载完成后创建背景
            }
        });
}
```

## 最佳实践

1. **预加载资源** - 在场景 `onEnter()` 中加载所需资源
2. **检查资源有效性** - 始终检查加载结果是否为 nullptr
3. **复用资源** - 多次使用同一资源时保存指针，避免重复加载
4. **合理设置字号** - 字体加载时会生成对应字号的图集
5. **使用异步加载** - 对大型资源使用异步加载避免卡顿
6. **选择合适的压缩格式** - 根据纹理用途选择最佳压缩格式
7. **利用自动批处理** - 相同纹理的精灵会自动批处理，无需手动优化
8. **配置 LRU 缓存** - 根据平台内存配置合适的缓存大小
9. **定期监控缓存** - 在开发阶段定期检查缓存命中率和内存使用
10. **在主循环更新资源管理器** - 确保调用 `resources.update(dt)` 以触发自动清理

## 下一步

- [05. 输入处理](./05_Input_Handling.md) - 学习输入处理
- [06. 碰撞检测](./06_Collision_Detection.md) - 学习碰撞检测系统
