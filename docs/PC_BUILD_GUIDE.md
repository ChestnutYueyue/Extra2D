# Easy2D PC 端编译指南

本文档说明如何在 PC 端（Windows/Linux/macOS）编译和运行 Easy2D 引擎。

## 概述

Easy2D 现在支持多平台：
- **Nintendo Switch** (ARM64) - 原始平台
- **Windows** (x64) - 新增
- **Linux** (x64) - 新增
- **macOS** (x64/ARM64) - 新增

PC 端使用 OpenGL ES 3.2（通过 Angle 或 Mesa）保持与 Switch 的代码兼容性。

## 前置条件

### Windows

1. **Visual Studio 2019/2022** - 安装 C++ 桌面开发工作负载
2. **xmake** - 构建系统 (https://xmake.io)
3. **SDL2** - 可以通过以下方式安装：
   - vcpkg: `vcpkg install sdl2 sdl2-mixer`
   - 手动下载：https://www.libsdl.org/download-2.0.php
4. **Angle** (可选) - Google 的 GLES 实现
   - 下载 Angle 二进制文件或从 Chromium 构建
   - 放置到 `third_party/angle/` 目录

### Linux (Ubuntu/Debian)

```bash
# 安装依赖
sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y libsdl2-dev libsdl2-mixer-dev
sudo apt-get install -y libgles2-mesa-dev libegl1-mesa-dev

# 安装 xmake
sudo add-apt-repository ppa:xmake-io/xmake
sudo apt-get update
sudo apt-get install xmake
```

### macOS

```bash
# 安装 Homebrew (如果还没有)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装依赖
brew install sdl2 sdl2_mixer
brew install angle  # 或 mesa

# 安装 xmake
brew install xmake
```

## 编译步骤

### 1. 配置项目

```bash
# 进入项目目录
cd c:\Users\soulcoco\Desktop\Easy2D\Extra2D

# 配置 PC 平台构建
xmake config -p pc -m release

# 或使用环境变量
set E2D_PLATFORM=pc  # Windows
export E2D_PLATFORM=pc  # Linux/macOS
xmake config
```

### 2. 编译引擎库

```bash
# 编译 Easy2D 静态库
xmake build extra2d
```

### 3. 编译示例程序

```bash
# 编译 Hello World 示例
xmake build hello_world

# 编译所有示例
xmake build -a
```

### 4. 运行示例

```bash
# 运行 Hello World
xmake run hello_world

# 或手动运行
./build/windows/hello_world.exe  # Windows
./build/linux/hello_world        # Linux
./build/macos/hello_world        # macOS
```

## 项目结构

```
Extra2D/
├── xmake.lua                    # 主构建配置
├── xmake/
│   ├── toolchains/
│   │   ├── switch.lua           # Switch 工具链
│   │   └── pc.lua               # PC 工具链 (新增)
│   └── targets/
│       ├── extra2d.lua          # 引擎库配置
│       └── examples.lua         # 示例程序配置
├── Extra2D/
│   ├── include/
│   │   └── extra2d/
│   │       └── platform/
│   │           ├── platform_compat.h   # 跨平台兼容性 (新增)
│   │           ├── file_system.h       # 文件系统工具 (新增)
│   │           ├── window.h            # 窗口系统 (修改)
│   │           └── input.h             # 输入系统 (修改)
│   └── src/
│       └── platform/
│           ├── window.cpp              # (修改)
│           ├── input.cpp               # (修改)
│           └── file_system.cpp         # (新增)
└── examples/
    └── hello_world/
        └── main.cpp              # (修改，支持多平台)
```

## 平台差异

### 窗口系统

| 功能 | Switch | PC |
|------|--------|-----|
| 窗口模式 | 始终全屏 (1280x720) | 可配置全屏/窗口 |
| 窗口大小 | 固定 | 可调整 |
| DPI 缩放 | 1.0 | 自动检测 |
| 鼠标光标 | 无 | 支持多种形状 |

### 输入系统

| 功能 | Switch | PC |
|------|--------|-----|
| 键盘 | 映射到手柄 | 原生支持 |
| 鼠标 | 映射到触摸 | 原生支持 |
| 手柄 | SDL GameController | SDL GameController |
| 触摸屏 | 原生 | 可选支持 |

### 文件系统

| 功能 | Switch | PC |
|------|--------|-----|
| 资源路径 | `romfs:/assets/` | `./assets/` 或 exe 目录 |
| 文件访问 | RomFS | 标准文件系统 |

## 开发指南

### 编写跨平台代码

```cpp
#include <extra2d/platform/platform_compat.h>
#include <extra2d/platform/file_system.h>

// 检查平台
if (platform::isSwitch()) {
    // Switch 特定代码
} else if (platform::isPC()) {
    // PC 特定代码
}

// 解析资源路径（自动处理平台差异）
std::string fontPath = FileSystem::resolvePath("assets/font.ttf");
// Switch: "romfs:/assets/font.ttf"
// PC: "./assets/font.ttf"

// 条件编译
#ifdef PLATFORM_SWITCH
    // Switch 代码
#elif defined(PLATFORM_PC)
    // PC 代码
#endif
```

### 输入处理

```cpp
// 跨平台输入（推荐）
auto& input = Application::instance().input();

// 键盘（PC 原生，Switch 映射到手柄）
if (input.isKeyPressed(Key::Space)) {
    // 处理空格键
}

// 手柄（所有平台）
if (input.isButtonPressed(SDL_CONTROLLER_BUTTON_A)) {
    // 处理 A 按钮
}

// 鼠标（PC 原生，Switch 映射到触摸）
if (input.isMouseDown(MouseButton::Left)) {
    // 处理鼠标左键
}
```

## 故障排除

### 找不到 SDL2

**Windows:**
```bash
# 设置 SDL2 路径
set SDL2_DIR=C:\SDL2-2.28.0
xmake config
```

**Linux:**
```bash
# 安装 SDL2 开发库
sudo apt-get install libsdl2-dev libsdl2-mixer-dev
```

### 找不到 GLES

**Windows (Angle):**
```bash
# 下载 Angle 并设置路径
set ANGLE_DIR=C:\angle
xmake config
```

**Linux (Mesa):**
```bash
# 安装 Mesa GLES
sudo apt-get install libgles2-mesa-dev libegl1-mesa-dev
```

### 编译错误

1. 确保使用 C++17 或更高版本
2. 检查 xmake 版本是否最新：`xmake --version`
3. 清理构建缓存：`xmake clean -a`

## 下一步

1. 尝试编译和运行 `hello_world` 示例
2. 阅读 `platform_compat.h` 了解平台 API
3. 使用 `FileSystem` 类处理跨平台文件路径
4. 参考示例代码编写跨平台应用

## 许可证

Easy2D 采用 MIT 许可证。详见 LICENSE 文件。

---

**最后更新**: 2026年2月10日
**Easy2D 版本**: 3.1.0
