# Extra2D API 参考文档

## 目录

- [核心系统](#核心系统)
- [应用管理](#应用管理)
- [场景系统](#场景系统)
- [节点系统](#节点系统)
- [输入系统](#输入系统)
- [资源管理](#资源管理)
- [动画系统](#动画系统)
- [音频系统](#音频系统)
- [文件系统](#文件系统)

---

## 核心系统

### 类型定义

```cpp
namespace extra2d {

// 基本类型
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

// 智能指针
template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T, typename... Args>
Ptr<T> makePtr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace extra2d
```

### 颜色类

```cpp
struct Color {
    float r, g, b, a;
    
    Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
    
    // 预定义颜色
    static Color White;
    static Color Black;
    static Color Red;
    static Color Green;
    static Color Blue;
    static Color Yellow;
    static Color Transparent;
};
```

### 矩形类

```cpp
struct Rect {
    float x, y, width, height;
    
    Rect(float x = 0, float y = 0, float w = 0, float h = 0);
    
    bool contains(const Vec2& point) const;
    bool intersects(const Rect& other) const;
    
    float left() const { return x; }
    float right() const { return x + width; }
    float top() const { return y; }
    float bottom() const { return y + height; }
    Vec2 center() const { return Vec2(x + width/2, y + height/2); }
};
```

---

## 应用管理

### AppConfig

应用配置结构体。

```cpp
struct AppConfig {
    String title = "Extra2D Application";
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
    int fpsLimit = 0;  // 0 = 不限制
    BackendType renderBackend = BackendType::OpenGL;
    int msaaSamples = 0;
};
```

### Application

应用主类，单例模式。

```cpp
class Application {
public:
    // 获取单例实例
    static Application& instance();
    
    // 初始化应用
    bool init(const AppConfig& config);
    
    // 运行主循环
    void run();
    
    // 退出应用
    void quit();
    
    // 进入场景
    void enterScene(Ptr<Scene> scene);
    void enterScene(Ptr<Scene> scene, Ptr<Transition> transition);
    
    // 获取子系统
    Input& input();
    AudioEngine& audio();
    ResourceManager& resources();
    RenderBackend& renderer();
    
    // 获取配置
    const AppConfig& getConfig() const;
    
    // 获取当前 FPS
    float fps() const;
    
    // Switch 特定：检测是否连接底座
    bool isDocked() const;
};
```

**使用示例：**

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

int main() {
    // 初始化日志
    Logger::init();
    Logger::setLevel(LogLevel::Debug);
    
    // 配置应用
    AppConfig config;
    config.title = "My Game";
    config.width = 1280;
    config.height = 720;
    config.vsync = true;
    
    // 初始化应用
    auto& app = Application::instance();
    if (!app.init(config)) {
        E2D_LOG_ERROR("应用初始化失败！");
        return -1;
    }
    
    // 进入场景
    app.enterScene(makePtr<MyScene>());
    
    // 运行主循环
    app.run();
    
    return 0;
}
```

---

## 场景系统

### Scene

场景类，作为游戏内容的容器。

```cpp
class Scene : public Node {
public:
    // 构造函数
    Scene();
    
    // 场景生命周期回调
    virtual void onEnter();      // 进入场景时调用
    virtual void onExit();       // 退出场景时调用
    virtual void onUpdate(float dt);  // 每帧更新
    virtual void onRender(RenderBackend& renderer);  // 渲染
    
    // 设置背景颜色
    void setBackgroundColor(const Color& color);
    
    // 空间索引
    void setSpatialIndexingEnabled(bool enabled);
    bool isSpatialIndexingEnabled() const;
    
    // 查询碰撞
    std::vector<std::pair<Node*, Node*>> queryCollisions();
    
    // 获取空间管理器
    SpatialManager& getSpatialManager();
};
```

### Transition

场景过渡动画基类。

```cpp
class Transition {
public:
    explicit Transition(float duration);
    
    virtual void update(float dt) = 0;
    virtual void render(RenderBackend& renderer, 
                       Ptr<Scene> fromScene, 
                       Ptr<Scene> toScene) = 0;
    
    bool isFinished() const;
};

// 内置过渡效果
class FadeTransition : public Transition {
public:
    FadeTransition(float duration, const Color& color = Color::Black);
};

class SlideTransition : public Transition {
public:
    SlideTransition(float duration, Direction direction);
};
```

---

## 节点系统

### Node

所有场景对象的基类。

```cpp
class Node {
public:
    Node();
    virtual ~Node();
    
    // 变换
    void setPosition(const Vec2& pos);
    Vec2 getPosition() const;
    
    void setRotation(float degrees);
    float getRotation() const;
    
    void setScale(const Vec2& scale);
    Vec2 getScale() const;
    
    void setAnchor(const Vec2& anchor);
    Vec2 getAnchor() const;
    
    // 层级
    void addChild(Ptr<Node> child);
    void removeChild(Ptr<Node> child);
    void removeFromParent();
    
    // 可见性
    void setVisible(bool visible);
    bool isVisible() const;
    
    // 动作
    void runAction(Ptr<Action> action);
    void stopAllActions();
    
    // 渲染
    virtual void onRender(RenderBackend& renderer);
    
    // 更新
    virtual void onUpdate(float dt);
    
    // 边界框（用于碰撞检测）
    virtual Rect getBoundingBox() const;
    
    // 空间索引
    void setSpatialIndexed(bool indexed);
    bool isSpatialIndexed() const;
};
```

### Sprite

精灵节点，用于显示2D图像。

```cpp
class Sprite : public Node {
public:
    // 创建方法
    static Ptr<Sprite> create(Ptr<Texture> texture);
    static Ptr<Sprite> create(const std::string& texturePath);
    
    // 设置纹理
    void setTexture(Ptr<Texture> texture);
    Ptr<Texture> getTexture() const;
    
    // 设置纹理矩形（用于精灵表）
    void setTextureRect(const Rect& rect);
    
    // 设置颜色调制
    void setColor(const Color& color);
    Color getColor() const;
    
    // 翻转
    void setFlippedX(bool flipped);
    void setFlippedY(bool flipped);
};
```

### Text

文本节点。

```cpp
class Text : public Node {
public:
    static Ptr<Text> create(const std::string& text = "");
    
    // 文本内容
    void setText(const std::string& text);
    std::string getText() const;
    
    // 字体
    void setFont(Ptr<FontAtlas> font);
    void setFontSize(int size);
    
    // 颜色
    void setTextColor(const Color& color);
    Color getTextColor() const;
    
    // 对齐
    void setHorizontalAlignment(Alignment align);
    void setVerticalAlignment(Alignment align);
};
```

---

## 输入系统

### Input

输入管理类。

```cpp
class Input {
public:
    // 键盘
    bool isKeyDown(int keyCode) const;      // 按键是否按下
    bool isKeyPressed(int keyCode) const;   // 按键是否刚按下
    bool isKeyReleased(int keyCode) const;  // 按键是否刚释放
    
    // 手柄按钮
    bool isButtonDown(int button) const;
    bool isButtonPressed(int button) const;
    bool isButtonReleased(int button) const;
    
    // 摇杆
    Vec2 getLeftStick() const;
    Vec2 getRightStick() const;
    
    // 鼠标（PC端）
    Vec2 getMousePosition() const;
    bool isMouseDown(int button) const;
    bool isMousePressed(int button) const;
    
    // 触摸（Switch/移动端）
    bool isTouching() const;
    Vec2 getTouchPosition() const;
    int getTouchCount() const;
};
```

### 按键码 (Key)

```cpp
namespace Key {
    // 字母
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // 数字
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
    
    // 功能键
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // 方向键
    Up, Down, Left, Right,
    
    // 特殊键
    Escape, Enter, Tab, Backspace, Space,
    Insert, Delete, Home, End, PageUp, PageDown,
    LeftShift, LeftControl, LeftAlt, LeftSuper,
    RightShift, RightControl, RightAlt, RightSuper
}
```

**使用示例：**

```cpp
void MyScene::onUpdate(float dt) {
    auto& input = Application::instance().input();
    
    // 键盘输入
    if (input.isKeyPressed(Key::Space)) {
        // 空格键按下
        jump();
    }
    
    if (input.isKeyDown(Key::Left)) {
        // 左方向键按住
        moveLeft();
    }
    
    // 手柄输入
    if (input.isButtonPressed(GamepadButton::A)) {
        // A 按钮按下
        jump();
    }
    
    // 摇杆输入
    Vec2 leftStick = input.getLeftStick();
    move(leftStick.x * speed, leftStick.y * speed);
}
```

---

## 资源管理

### ResourceManager

资源管理类，负责加载和管理游戏资源。

```cpp
class ResourceManager {
public:
    // 纹理
    Ptr<Texture> loadTexture(const std::string& path);
    Ptr<Texture> getTexture(const std::string& path);
    void unloadTexture(const std::string& path);
    
    // 字体
    Ptr<FontAtlas> loadFont(const std::string& path, int size, bool useSDF = false);
    Ptr<FontAtlas> loadFontWithFallbacks(const std::vector<std::string>& paths, 
                                          int size, bool useSDF = false);
    
    // 音频
    Ptr<Sound> loadSound(const std::string& path);
    Ptr<Music> loadMusic(const std::string& path);
    
    // 精灵表
    Ptr<SpriteFrameCache> loadSpriteSheet(const std::string& path);
    
    // 清理
    void unloadAll();
    void unloadUnused();
};
```

### Texture

纹理类。

```cpp
class Texture {
public:
    // 获取尺寸
    int getWidth() const;
    int getHeight() const;
    Vec2 getSize() const;
    
    // 获取 OpenGL 纹理 ID
    GLuint getTextureID() const;
};
```

### FontAtlas

字体图集类。

```cpp
class FontAtlas {
public:
    // 获取字体信息
    int getFontSize() const;
    bool isSDF() const;
    
    // 获取字符信息
    const Glyph& getGlyph(char32_t charCode) const;
    
    // 获取行高
    float getLineHeight() const;
    
    // 获取纹理
    Ptr<Texture> getTexture() const;
};
```

**使用示例：**

```cpp
void MyScene::onEnter() {
    auto& resources = Application::instance().resources();
    
    // 加载纹理
    auto playerTexture = resources.loadTexture("player.png");
    auto enemyTexture = resources.loadTexture("enemies/slime.png");
    
    // 加载字体
    auto font = resources.loadFont("font.ttf", 24, true);
    
    // 创建精灵
    auto player = Sprite::create(playerTexture);
    addChild(player);
    
    // 创建文本
    auto text = Text::create("Hello World");
    text->setFont(font);
    addChild(text);
}
```

---

## 动画系统

### Action

动作基类。

```cpp
class Action {
public:
    virtual void update(float dt) = 0;
    virtual bool isFinished() const = 0;
    void reset();
};
```

### 基础动作

```cpp
// 移动
class MoveTo : public Action {
public:
    MoveTo(float duration, const Vec2& position);
};

class MoveBy : public Action {
public:
    MoveBy(float duration, const Vec2& delta);
};

// 缩放
class ScaleTo : public Action {
public:
    ScaleTo(float duration, const Vec2& scale);
    ScaleTo(float duration, float scale);
};

class ScaleBy : public Action {
public:
    ScaleBy(float duration, const Vec2& scale);
    ScaleBy(float duration, float scale);
};

// 旋转
class RotateTo : public Action {
public:
    RotateTo(float duration, float degrees);
};

class RotateBy : public Action {
public:
    RotateBy(float duration, float degrees);
};

// 淡入淡出
class FadeTo : public Action {
public:
    FadeTo(float duration, float opacity);
};

class FadeIn : public Action {
public:
    FadeIn(float duration);
};

class FadeOut : public Action {
public:
    FadeOut(float duration);
};

// 延迟
class Delay : public Action {
public:
    Delay(float duration);
};

// 回调
class CallFunc : public Action {
public:
    CallFunc(std::function<void()> callback);
};
```

### 组合动作

```cpp
// 顺序执行
class Sequence : public Action {
public:
    Sequence(std::vector<Ptr<Action>> actions);
    
    template<typename... Args>
    static Ptr<Sequence> create(Args&&... args) {
        return makePtr<Sequence>(std::vector<Ptr<Action>>{std::forward<Args>(args)...});
    }
};

// 同时执行
class Spawn : public Action {
public:
    Spawn(std::vector<Ptr<Action>> actions);
};

// 重复
class Repeat : public Action {
public:
    Repeat(Ptr<Action> action, int times = -1);  // -1 = 无限重复
};

// 反向
class Reverse : public Action {
public:
    Reverse(Ptr<Action> action);
};
```

### 缓动函数

```cpp
namespace Ease {
    // 线性
    float linear(float t);
    
    // 二次
    float inQuad(float t);
    float outQuad(float t);
    float inOutQuad(float t);
    
    // 三次
    float inCubic(float t);
    float outCubic(float t);
    float inOutCubic(float t);
    
    // 弹性
    float inElastic(float t);
    float outElastic(float t);
    
    // 弹跳
    float inBounce(float t);
    float outBounce(float t);
    
    // 回退
    float inBack(float t);
    float outBack(float t);
}
```

**使用示例：**

```cpp
// 创建精灵
auto sprite = Sprite::create("player.png");
sprite->setPosition(Vec2(100, 100));
addChild(sprite);

// 简单动作
sprite->runAction(makePtr<MoveTo>(1.0f, Vec2(300, 200)));

// 组合动作
sprite->runAction(makePtr<Sequence>(
    makePtr<ScaleTo>(0.5f, Vec2(1.5f, 1.5f)),
    makePtr<Delay>(0.2f),
    makePtr<ScaleTo>(0.5f, Vec2(1.0f, 1.0f)),
    makePtr<CallFunc>([]() {
        E2D_LOG_INFO("动画完成！");
    })
));

// 重复动画
sprite->runAction(makePtr<Repeat>(
    makePtr<Sequence>(
        makePtr<RotateBy>(1.0f, 360.0f),
        makePtr<Delay>(0.5f)
    )
));
```

---

## 音频系统

### AudioEngine

音频引擎类。

```cpp
class AudioEngine {
public:
    // 音量控制 (0.0 - 1.0)
    void setMasterVolume(float volume);
    float getMasterVolume() const;
    
    void setBGMVolume(float volume);
    void setSFXVolume(float volume);
    
    // 播放控制
    void pauseAll();
    void resumeAll();
    void stopAll();
};
```

### Sound

音效类（短音频，适合音效）。

```cpp
class Sound {
public:
    void play(int loops = 0);  // loops: 0=播放1次, -1=无限循环
    void stop();
    void pause();
    void resume();
    
    void setVolume(float volume);
    bool isPlaying() const;
};
```

### Music

音乐类（长音频，适合背景音乐）。

```cpp
class Music {
public:
    void play(int loops = -1);
    void stop();
    void pause();
    void resume();
    
    void setVolume(float volume);
    bool isPlaying() const;
    
    void setLoopPoints(double start, double end);
};
```

**使用示例：**

```cpp
void MyScene::onEnter() {
    auto& audio = Application::instance().audio();
    auto& resources = Application::instance().resources();
    
    // 加载并播放背景音乐
    auto bgm = resources.loadMusic("bgm/level1.ogg");
    bgm->play(-1);  // 无限循环
    bgm->setVolume(0.7f);
    
    // 加载音效
    jumpSound_ = resources.loadSound("sfx/jump.wav");
    coinSound_ = resources.loadSound("sfx/coin.wav");
}

void MyScene::jump() {
    jumpSound_->play();
}

void MyScene::collectCoin() {
    coinSound_->play();
}
```

---

## 文件系统

### FileSystem

跨平台文件系统工具类。

```cpp
class FileSystem {
public:
    // 路径解析（自动处理平台差异）
    // PC: "assets/font.ttf" -> "C:/.../assets/font.ttf"
    // Switch: "assets/font.ttf" -> "romfs:/assets/font.ttf"
    static std::string resolvePath(const std::string& relativePath);
    
    // 获取资源根目录
    static std::string getResourceRoot();
    
    // 获取可执行文件目录
    static std::string getExecutableDirectory();
    
    // 获取当前工作目录
    static std::string getCurrentWorkingDirectory();
    
    // 路径组合
    static std::string combinePath(const std::string& path1, 
                                    const std::string& path2);
    
    // 文件/目录检查
    static bool fileExists(const std::string& path);
    static bool directoryExists(const std::string& path);
    
    // 读取文件内容
    static std::vector<uint8_t> readFile(const std::string& path);
    static std::string readTextFile(const std::string& path);
    
    // 写入文件
    static bool writeFile(const std::string& path, 
                         const std::vector<uint8_t>& data);
    static bool writeTextFile(const std::string& path, 
                             const std::string& content);
};
```

**使用示例：**

```cpp
// 解析资源路径（跨平台）
std::string fontPath = FileSystem::resolvePath("fonts/main.ttf");
// PC: "C:/.../assets/fonts/main.ttf"
// Switch: "romfs:/assets/fonts/main.ttf"

// 检查文件是否存在
if (FileSystem::fileExists(fontPath)) {
    auto font = resources.loadFont(fontPath, 24);
}

// 读取配置文件
std::string configPath = FileSystem::resolvePath("config/game.json");
std::string jsonContent = FileSystem::readTextFile(configPath);
```

---

## 日志系统

### Logger

日志系统。

```cpp
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void init();
    static void shutdown();
    
    static void setLevel(LogLevel level);
    
    // 日志宏
    #define E2D_LOG_DEBUG(...)  // 调试日志
    #define E2D_LOG_INFO(...)   // 信息日志
    #define E2D_LOG_WARN(...)   // 警告日志
    #define E2D_LOG_ERROR(...)  // 错误日志
};
```

**使用示例：**

```cpp
void MyClass::doSomething() {
    E2D_LOG_DEBUG("进入函数 doSomething");
    
    if (!loadData()) {
        E2D_LOG_ERROR("加载数据失败！");
        return;
    }
    
    E2D_LOG_INFO("成功加载 {} 条记录", recordCount);
}
```

---

## 平台兼容性

### 平台检测

```cpp
// 编译时检测
#ifdef PLATFORM_SWITCH
    // Switch 代码
#elif defined(PLATFORM_PC)
    // PC 代码
    #ifdef PLATFORM_WINDOWS
        // Windows 代码
    #elif defined(PLATFORM_LINUX)
        // Linux 代码
    #elif defined(PLATFORM_MACOS)
        // macOS 代码
    #endif
#endif

// 运行时检测
namespace platform {
    bool isSwitch();
    bool isPC();
    bool isWindows();
    bool isLinux();
    bool isMacOS();
    const char* getPlatformName();
}
```

---

## 更多文档

- [Switch 构建指南](./SWITCH_BUILD_GUIDE.md)
- [PC 构建指南](./PC_BUILD_GUIDE.md)
- [迁移完成记录](./SWITCH_MIGRATION_COMPLETE.md)

---

**最后更新**: 2026年2月10日  
**Extra2D 版本**: 3.1.0
