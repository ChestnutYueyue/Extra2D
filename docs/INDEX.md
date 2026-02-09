# Extra2D 文档索引

欢迎来到 Extra2D 文档中心！这里包含了使用 Extra2D 游戏引擎所需的所有文档。

## 📚 快速导航

### 入门指南

| 文档 | 描述 |
|------|------|
| [README.md](../README.md) | 项目概述和快速开始 |
| [Switch 构建指南](./SWITCH_BUILD_GUIDE.md) | Nintendo Switch 平台构建教程 |
| [PC 构建指南](./PC_BUILD_GUIDE.md) | Windows/Linux/macOS 构建教程 |

### API 参考

| 文档 | 描述 |
|------|------|
| [API 参考文档](./API_REFERENCE.md) | 完整的 API 文档和示例 |

### 开发文档

| 文档 | 描述 |
|------|------|
| [迁移完成记录](./SWITCH_MIGRATION_COMPLETE.md) | 项目迁移历史记录 |
| [数据存储文档](./DataStore.md) | 数据持久化系统文档 |

---

## 🚀 快速开始

### 1. 选择平台

**Nintendo Switch:**
```bash
# 设置环境变量
$env:DEVKITPRO = "C:/devkitPro"

# 配置并构建
xmake f --plat=switch -a arm64
xmake
```

**Windows PC:**
```bash
# 设置环境变量
$env:VCPKG_ROOT = "C:\vcpkg"

# 配置并构建
xmake f --plat=windows -a x64
xmake
```

### 2. 运行示例

```bash
# Switch（生成 .nro 文件）
./build/switch/hello_world.nro

# Windows
./build/windows/hello_world.exe
```

### 3. 开始开发

```cpp
#include <extra2d/extra2d.h>

using namespace extra2d;

int main() {
    // 初始化
    Logger::init();
    
    AppConfig config;
    config.title = "My Game";
    config.width = 1280;
    config.height = 720;
    
    auto& app = Application::instance();
    app.init(config);
    
    // 创建场景
    auto scene = makePtr<Scene>();
    
    // 添加精灵
    auto sprite = Sprite::create("player.png");
    scene->addChild(sprite);
    
    // 运行动画
    sprite->runAction(makePtr<MoveTo>(1.0f, Vec2(300, 200)));
    
    // 运行
    app.enterScene(scene);
    app.run();
    
    return 0;
}
```

---

## 📖 核心概念

### 应用生命周期

```
main()
  └── Application::init()
        └── Scene::onEnter()
              └── Node::onUpdate() [每帧]
              └── Node::onRender() [每帧]
        └── Scene::onExit()
```

### 场景图结构

```
Scene (场景)
  ├── Node (节点)
  │     ├── Sprite (精灵)
  │     ├── Text (文本)
  │     └── CustomNode (自定义节点)
  └── Node
        └── ...
```

### 坐标系统

- **原点**: 左上角 (0, 0)
- **X轴**: 向右为正
- **Y轴**: 向下为正
- **单位**: 像素

---

## 🛠️ 平台差异

| 功能 | Switch | PC |
|------|--------|-----|
| 窗口 | 固定全屏 | 可调整大小 |
| 输入 | 手柄/触摸 | 键盘/鼠标/手柄 |
| 资源路径 | `romfs:/` | `./assets/` |
| 渲染 | OpenGL ES | OpenGL ES (Angle) |

---

## 💡 示例代码

### 基础示例

- [Hello World](../Extra2D/examples/hello_world/main.cpp) - 基础窗口和文本
- [Collision Demo](../Extra2D/examples/collision_demo/main.cpp) - 碰撞检测
- [Spatial Index Demo](../Extra2D/examples/spatial_index_demo/main.cpp) - 空间索引

### 常用模式

**场景切换:**
```cpp
auto newScene = makePtr<GameScene>();
auto transition = makePtr<FadeTransition>(0.5f);
app.enterScene(newScene, transition);
```

**输入处理:**
```cpp
void onUpdate(float dt) {
    auto& input = app.input();
    
    if (input.isKeyPressed(Key::Space)) {
        jump();
    }
    
    if (input.isButtonPressed(GamepadButton::A)) {
        jump();
    }
}
```

**资源加载:**
```cpp
auto& resources = app.resources();
auto texture = resources.loadTexture("player.png");
auto font = resources.loadFont("font.ttf", 24);
auto sound = resources.loadSound("jump.wav");
```

---

## 📦 项目结构

```
Extra2D/
├── docs/                   # 文档
├── Extra2D/
│   ├── include/extra2d/   # 头文件
│   ├── src/               # 源文件
│   └── examples/          # 示例程序
├── squirrel/              # Squirrel 脚本引擎
├── xmake/                 # xmake 配置
│   ├── toolchains/        # 工具链定义
│   └── targets/           # 构建目标
├── xmake.lua              # 主构建配置
└── README.md              # 项目说明
```

---

## 🔗 相关链接

- **GitHub**: https://github.com/ChestnutYueyue/extra2d
- **Issues**: https://github.com/ChestnutYueyue/extra2d/issues
- **devkitPro**: https://devkitpro.org/
- **Switch 开发**: https://switchbrew.org/

---

## 📝 许可证

Extra2D 使用 [MIT 许可证](../LICENSE)。

---

**最后更新**: 2026年2月10日  
**版本**: 3.1.0
