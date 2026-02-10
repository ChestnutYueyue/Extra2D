# Extra2D 构建系统文档

## 概述

Extra2D 使用 **Xmake** 作为构建系统，支持 **MinGW (Windows)** 和 **Nintendo Switch** 两个平台。

## 项目结构

```
Extra2D/
├── xmake.lua                 # 主构建脚本
├── xmake/
│   ├── engine.lua           # 引擎库定义
│   └── toolchains/
│       └── switch.lua       # Switch 工具链定义
├── Extra2D/
│   ├── src/                 # 引擎源码
│   └── include/             # 引擎头文件
├── squirrel/                # Squirrel 脚本引擎
└── examples/                # 示例程序
    ├── hello_world/
    ├── collision_demo/
    ├── push_box/
    └── spatial_index_demo/
```

## 环境准备

### MinGW (Windows) 平台

1. **安装 MinGW-w64**
   - 下载地址: https://www.mingw-w64.org/downloads/
   - 或使用 MSYS2: `pacman -S mingw-w64-x86_64-toolchain`

2. **安装 Xmake**
   - 下载地址: https://xmake.io/#/zh-cn/guide/installation

3. **安装依赖包**
   ```bash
   xmake require -y
   ```

### Nintendo Switch 平台

1. **安装 devkitPro**
   - 下载地址: https://devkitpro.org/wiki/Getting_Started
   - Windows 安装程序会自动设置环境变量

2. **设置环境变量**
   ```powershell
   # PowerShell
   $env:DEVKITPRO="C:/devkitPro"
   
   # 或永久设置（系统属性 -> 环境变量）
   [Environment]::SetEnvironmentVariable("DEVKITPRO", "C:/devkitPro", "User")
   ```

3. **在 MSYS2 中安装 Switch 库**
   ```bash
   # 打开 MSYS2 (devkitPro 提供的)
   pacman -S switch-sdl2 switch-sdl2_mixer switch-glm
   
   # 或安装所有 Switch 开发库
   pacman -S $(pacman -Slq dkp-libs | grep switch-)
   ```

4. **验证安装**
   ```bash
   ls $DEVKITPRO/portlibs/switch/include/SDL2
   ls $DEVKITPRO/portlibs/switch/include/glm
   ```

## 主构建脚本 (xmake.lua)

### 项目元信息
- **项目名称**: Extra2D
- **版本**: 3.1.0
- **许可证**: MIT
- **语言标准**: C++17
- **编码**: UTF-8

### 构建选项

| 选项 | 默认值 | 描述 |
|------|--------|------|
| `examples` | true | 构建示例程序 |
| `debug_logs` | false | 启用调试日志 |

### 平台检测逻辑

1. 获取主机平台: `os.host()`
2. 获取目标平台: `get_config("plat")` 或主机平台
3. 平台回退: 如果不支持，Windows 回退到 `mingw`
4. 设置平台: `set_plat(target_plat)`
5. 设置架构:
   - Switch: `arm64`
   - MinGW: `x86_64`

### 依赖包 (MinGW 平台)

```lua
add_requires("glm", "libsdl2", "libsdl2_mixer")
```

## 引擎库定义 (xmake/engine.lua)

### 目标: extra2d
- **类型**: 静态库 (`static`)
- **源文件**:
  - `Extra2D/src/**.cpp`
  - `Extra2D/src/glad/glad.c`
  - `squirrel/squirrel/*.cpp`
  - `squirrel/sqstdlib/*.cpp`

### 头文件路径
- `Extra2D/include` (public)
- `squirrel/include` (public)
- `Extra2D/include/extra2d/platform` (public)

### 平台配置

#### Switch 平台
```lua
add_includedirs(devkitPro .. "/portlibs/switch/include")
add_linkdirs(devkitPro .. "/portlibs/switch/lib")
add_syslinks("SDL2_mixer", "SDL2", "opusfile", "opus", ...)
```

#### MinGW 平台
```lua
add_packages("glm", "libsdl2", "libsdl2_mixer")
add_syslinks("opengl32", "glu32", "winmm", "imm32", "version", "setupapi")
```

### 编译器标志
- `-Wall`, `-Wextra`
- `-Wno-unused-variable`, `-Wno-unused-function`
- `-Wno-deprecated-copy`, `-Wno-class-memaccess`

### 构建模式
- **Debug**: `-O0`, `-g`, 定义 `E2D_DEBUG`, `_DEBUG`
- **Release**: `-O2`, 定义 `NDEBUG`

## Switch 工具链 (xmake/toolchains/switch.lua)

### 工具链: switch
- **类型**: standalone
- **描述**: Nintendo Switch devkitA64 工具链

### 工具路径
- **CC**: `aarch64-none-elf-gcc.exe`
- **CXX**: `aarch64-none-elf-g++.exe`
- **LD**: `aarch64-none-elf-g++.exe`
- **AR**: `aarch64-none-elf-gcc-ar.exe`

### 架构标志
```
-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
```

### 链接标志
```
-specs=switch.specs -g
```

### 预定义宏
- `__SWITCH__`
- `__NX__`
- `MA_SWITCH`
- `PFD_SWITCH`

### 系统库
- `nx` (libnx)
- `m` (math)

## 示例程序构建脚本

### 通用结构

```lua
-- 使用与主项目相同的平台配置
if is_plat("switch") then
    -- Switch 平台配置
elseif is_plat("mingw") then
    -- MinGW 平台配置
end
```

### Switch 平台配置
- 设置平台: `set_plat("switch")`
- 设置架构: `set_arch("arm64")`
- 设置工具链: `set_toolchains("switch")`
- 设置输出目录: `set_targetdir("../../build/examples/xxx")`
- 构建后生成 NRO 文件

### MinGW 平台配置
- 设置平台: `set_plat("mingw")`
- 设置架构: `set_arch("x86_64")`
- 设置输出目录: `set_targetdir("../../build/examples/xxx")`
- 链接标志: `-mwindows`
- 构建后复制资源文件

## 构建命令

### 配置项目

```bash
# 默认配置 (MinGW)
xmake f -c

# 指定平台 (使用 -p 参数)
xmake f -c -p mingw
xmake f -c -p switch

# 指定 MinGW 路径（如果不在默认位置）
xmake f -c -p mingw --mingw=C:\mingw

# 禁用示例
xmake f --examples=n

# 启用调试日志
xmake f --debug_logs=y
```

### 安装依赖 (MinGW)
```bash
xmake require -y
```

### 构建项目
```bash
# 构建所有目标
xmake

# 构建特定目标
xmake -r extra2d
xmake -r push_box

# 并行构建
xmake -j4
```

### 运行程序
```bash
# 运行示例
xmake run push_box
xmake run hello_world
```

### 清理构建
```bash
xmake clean
xmake f -c  # 重新配置
```

## 输出目录结构

```
build/
├── examples/
│   ├── hello_world/
│   │   ├── hello_world.exe    # MinGW
│   │   ├── hello_world.nro    # Switch
│   │   └── assets/            # 资源文件
│   ├── push_box/
│   ├── collision_demo/
│   └── spatial_index_demo/
└── ...
```

## 关键设计决策

### 1. 平台检测
- 使用 `is_plat()` 而不是手动检测，确保与主项目一致
- 示例脚本继承主项目的平台配置

### 2. 资源处理
- **Switch**: 使用 romfs 嵌入 NRO 文件
- **MinGW**: 构建后复制到输出目录

### 3. 依赖管理
- **MinGW**: 使用 Xmake 包管理器 (`add_requires`)
- **Switch**: 使用 devkitPro 提供的库

### 4. 工具链隔离
- Switch 工具链定义在单独文件中
- 通过 `set_toolchains("switch")` 切换

## 常见问题

### 1. 依赖包找不到
```bash
xmake repo -u
xmake require -y
```

### 2. Switch 工具链找不到
- 确保 DEVKITPRO 环境变量设置正确
- 默认路径: `C:/devkitPro`

### 3. 平台配置不匹配
- 使用 `xmake show` 查看当前配置
- 使用 `xmake f -c` 重新配置

### 4. MinGW 路径问题
如果 MinGW 安装在非默认位置，使用 `--mingw` 参数指定：
```bash
xmake f -c -p mingw --mingw=D:\Tools\mingw64
```

### 5. Switch 库找不到
确保在 MSYS2 中安装了必要的库：
```bash
pacman -S switch-sdl2 switch-sdl2_mixer switch-glm
```

## 扩展指南

### 添加新示例
1. 在 `examples/` 下创建新目录
2. 创建 `xmake.lua` 构建脚本
3. 在 `xmake.lua` 中添加 `includes("examples/new_example")`

### 添加新平台
1. 在 `xmake/toolchains/` 下创建工具链定义
2. 在 `xmake.lua` 中添加平台检测逻辑
3. 在 `xmake/engine.lua` 中添加平台配置