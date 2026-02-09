# Easy2D Nintendo Switch 编译指南

## 概述

本文档说明如何使用 xmake 为 Nintendo Switch 编译 Easy2D 引擎及其示例程序。

## 前置条件

### 1. 必需工具

- **devkitPro** - Nintendo Switch 开发工具包
- **xmake** - 跨平台构建系统（v3.0.6+）
- **devkitA64** - ARM64编译器工具链（devkitPro 的一部分）

### 2. 安装 devkitPro

#### Windows

1. 从 https://devkitpro.org/wiki/Getting_Started/devkitPro_installer 下载 devkitPro 安装程序
2. 运行安装程序，选择以下组件：
   - devkitA64 (ARM64)
   - libnx (Nintendo Switch库)
   - mesa (OpenGL ES)
   - tools (nacptool, elf2nro 等)
3. 默认安装路径：`C:\devkitPro`

#### Linux/macOS

请参考官方文档：https://devkitpro.org/wiki/Getting_Started

### 3. 验证安装

```bash
# 检查devkitPro是否正确安装
$env:DEVKITPRO = "C:\devkitPro"  # Windows PowerShell
export DEVKITPRO=/opt/devkitpro  # Linux/macOS

# 检查工具链
aarch64-none-elf-gcc --version  # 应该显示 GCC 版本

# 检查xmake
xmake --version  # 应该显示 v3.0.6 或更高
```

## 编译步骤

### 1. 配置项目

```bash
cd C:\Users\soulcoco\Desktop\Easy2D\Easy2D-dev

# 配置编译（使用Switch工具链）
xmake config -p switch -a arm64

# 或者使用默认配置
xmake config
```

### 2. 编译核心库

编译 Easy2D 静态库：

```bash
xmake build easy2d
```

**输出：**
- Release: `build/switch/libeasy2d.a`
- Debug: `build/switch/libeasy2dd.a`

### 3. 编译示例程序

#### 编译音频演示

```bash
xmake build switch_audio_demo
```

**输出：**
- ELF: `build/switch/switch_audio_demo`
- NACP: `build/switch/switch_audio_demo.nacp`
- NRO: `build/switch/switch_audio_demo.nro` (Switch可执行文件)

#### 编译动画演示

```bash
xmake build switch_animation_demo
```

**输出：**
- NRO: `build/switch/switch_animation_demo.nro`

### 4. 一次编译所有目标

```bash
xmake build -a
```

## 项目结构

```
Easy2D-dev/
├── xmake.lua                    # 构建配置
├── Easy2D/
│   ├── include/                 # 头文件
│   │   ├── easy2d/              # 引擎头文件
│   │   │   ├── app/             # 应用系统
│   │   │   ├── platform/        # 平台层
│   │   │   ├── graphics/        # 图形系统
│   │   │   ├── audio/           # 音频系统
│   │   │   ├── scene/           # 场景管理
│   │   │   ├── resource/        # 资源管理
│   │   │   └── utils/           # 工具类
│   │   ├── glm/                 # GLM数学库
│   │   ├── stb/                 # STB图像库
│   │   └── pfd/                 # 文件对话框库
│   ├── src/                     # 实现文件
│   │   ├── app/                 # 应用实现
│   │   ├── platform/            # 平台实现（Switch优化）
│   │   │   └── switch/          # Switch特定代码
│   │   ├── graphics/            # 图形实现
│   │   ├── audio/               # 音频实现
│   │   └── ...
│   └── examples/                # 示例程序
│       ├── switch_audio_demo/   # Switch音频演示
│       ├── switch_animation_demo/ # Switch动画演示
│       └── ...
└── squirrel/                    # Squirrel脚本引擎
```

## 编译配置详解

### xmake.lua 中的关键配置

#### 1. Switch 工具链定义 (行 15-51)

```lua
toolchain("switch")
    set_kind("standalone")
    set_description("Nintendo Switch devkitA64 toolchain")
    
    local devkitPro = "C:/devkitPro"
    local devkitA64 = path.join(devkitPro, "devkitA64")
    
    -- 编译器
    set_toolset("cc", path.join(devkitA64, "bin/aarch64-none-elf-gcc.exe"))
    set_toolset("cxx", path.join(devkitA64, "bin/aarch64-none-elf-g++.exe"))
    
    -- 架构标志
    local arch_flags = "-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"
    
    -- 链接 EGL 和 OpenGL ES (mesa)
    add_syslinks("EGL", "GLESv2", "glapi", "drm_nouveau")
```

#### 2. Easy2D 静态库配置 (行 60-134)

```lua
target("easy2d")
    set_kind("static")
    set_plat("switch")      -- 平台
    set_arch("arm64")       -- 架构
    set_toolchains("switch") -- 工具链
    
    add_files(SRC_DIR .. "/**.cpp")        -- 源文件
    add_files("squirrel/squirrel/*.cpp")   -- 脚本引擎
    add_includedirs(INC_DIR)               -- 头文件目录
```

#### 3. Switch 示例程序配置 (行 139-257)

xmake 自动处理：
- **编译** ELF 文件
- **生成 NACP** 应用元数据
- **转换为 NRO** (Switch 可执行格式)
- **打包 RomFS** (资源文件系统)

## 故障排除

### 编译错误

#### 错误：找不到 Switch 工具链

```
Error: toolchain 'switch' not found
```

**解决方案：**
1. 确认 `DEVKITPRO` 环境变量已设置
2. 检查 devkitPro 安装路径：`C:\devkitPro` (Windows) 或 `/opt/devkitpro` (Linux)
3. 验证 `devkitA64/bin` 下的编译器存在

#### 错误：找不到 libnx 库

```
Error: cannot find -lnx
```

**解决方案：**
1. 验证 devkitPro 安装了 libnx 包
2. 检查 `DEVKITPRO/libnx` 目录是否存在

#### 错误：OpenGL ES 头文件缺失

```
Error: GL/gl.h: No such file or directory
```

**解决方案：**
1. 验证 mesa 已安装：`DEVKITPRO/portlibs/switch/include`
2. 检查 xmake.lua 中的包含目录配置

### 链接错误

#### 未定义的引用到 EGL 函数

```
undefined reference to 'eglInitialize'
```

**解决方案：**
- 确保 EGL 库链接顺序正确（xmake.lua 第 93 行）

### 运行时错误

#### NRO 文件无法在 Switch 上运行

**检查清单：**
1. 确认 `DEVKITPRO` 环境变量设置正确
2. 验证 RomFS 资源已正确打包（如果需要）
3. 检查应用元数据（NACP 文件）是否正确生成

## Switch 开发资源

- **官方文档**: https://switchbrew.org/wiki/Main_Page
- **libnx 文档**: https://libnx.readthedocs.io/
- **devkitPro 论坛**: https://devkitpro.org/
- **Easy2D 文档**: https://github.com/easy2d/Easy2D

## 编译选项

### 编译模式

```bash
# Debug 模式（包含调试符号）
xmake config -m debug
xmake build easy2d

# Release 模式（优化编译）
xmake config -m release
xmake build easy2d
```

### 并行编译

```bash
# 使用 8 个线程编译
xmake build -j 8
```

### 清理构建

```bash
# 清理所有构建文件
xmake clean

# 仅清理目标
xmake clean easy2d
```

## 下一步

1. **修改示例程序** - 编辑 `Easy2D/examples/switch_*_demo/main.cpp`
2. **添加资源** - 将资源放在 `assets/` 目录
3. **优化性能** - 使用 Release 模式编译
4. **部署到 Switch** - 将 NRO 文件复制到 Switch SD 卡

## 常见问题

### Q: 如何调试 Switch 应用？

A: 可以使用以下方法：
- 使用 `nxlink` 输出日志到 PC
- 在应用中使用 `E2D_LOG_INFO()` 宏输出调试信息
- 使用支持 Switch 的调试器（如 GDB with nxlink）

### Q: 如何部署资源到 Switch？

A: 
1. 将资源放在 `examples/switch_*_demo/assets/` 目录
2. xmake 会自动将资源打包到 RomFS
3. 在代码中使用 `romfs:/` 前缀访问资源

### Q: 支持哪些音频格式？

A: miniaudio 支持：
- WAV
- FLAC
- MP3
- VORBIS

## 许可证

Easy2D 采用 MIT 许可证。详见 LICENSE 文件。

---

**最后更新**: 2026年2月9日
**Easy2D 版本**: 3.1.0
**Switch 工具链**: devkitA64 (devkitPro)
