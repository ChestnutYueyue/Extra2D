# Extra2D 构建系统文档

本文档详细介绍 Extra2D 引擎的构建系统配置和使用方法。

## 构建工具

Extra2D 使用 [xmake](https://xmake.io/) 作为构建工具，支持多平台构建。

### 安装 xmake

```bash
# Windows (使用 PowerShell)
Invoke-Expression (Invoke-WebRequest 'https://xmake.io/psget.text' -UseBasicParsing).Content

# macOS
brew install xmake

# Linux
sudo add-apt-repository ppa:xmake-io/xmake
sudo apt update
sudo apt install xmake
```

## 平台支持

| 平台 | 目标 | 说明 |
|------|------|------|
| Windows | `mingw` | MinGW-w64 工具链 |
| Nintendo Switch | `switch` | devkitPro 工具链 |

## 构建配置

### Windows (MinGW)

```bash
# 配置构建
xmake f -p mingw --mode=release

# 安装依赖
xmake require -y

# 构建引擎
xmake

# 构建示例
xmake -g examples

# 运行示例
xmake run hello_world
```

### Nintendo Switch

```bash
# 配置构建
xmake f -p switch --mode=release

# 构建引擎
xmake

# 构建示例
xmake -g examples

# 打包 NSP
xmake package push_box
```

## 构建选项

### 配置参数

```bash
# 设置构建模式
xmake f --mode=debug      # 调试模式
xmake f --mode=release    # 发布模式

# 设置目标平台
xmake f -p mingw          # Windows
xmake f -p switch         # Nintendo Switch
```

### 构建目标

```bash
# 构建所有目标
xmake

# 构建特定目标
xmake -t extra2d          # 引擎库
xmake -t hello_world      # Hello World 示例
xmake -t push_box         # 推箱子游戏
xmake -t collision_demo   # 碰撞检测演示
xmake -t spatial_index_demo # 空间索引演示

# 构建示例组
xmake -g examples
```

## 项目结构

```
Extra2D/
├── xmake.lua              # 主构建配置
├── xmake/
│   ├── engine.lua         # 引擎构建规则
│   └── toolchains/        # 工具链定义
├── Extra2D/               # 引擎源码
│   ├── include/           # 头文件
│   └── src/               # 源文件
└── examples/              # 示例程序
    ├── hello_world/
    ├── push_box/
    ├── collision_demo/
    └── spatial_index_demo/
```

## 添加新示例

创建新的示例程序：

1. 在 `examples/` 下创建目录
2. 添加 `main.cpp` 和 `xmake.lua`

### 示例 xmake.lua

```lua
-- examples/my_demo/xmake.lua
target("my_demo")
    set_kind("binary")
    add_deps("extra2d")
    add_files("*.cpp")
    add_packages("spdlog", "glm")
    
    -- 资源文件
    add_files("romfs/**", {install = true})
    
    -- Switch 特定配置
    if is_plat("switch") then
        add_rules("switch.nro")
        add_files("icon.jpg")
    end
```

## 常见问题

### 依赖安装失败

```bash
# 强制重新安装依赖
xmake require -f -y

# 清理构建缓存
xmake clean
xmake f -c
```

### Switch 构建失败

确保已安装 devkitPro：

```bash
# 安装 Switch 开发工具链
pacman -S switch-dev switch-portlibs

# 设置环境变量
$env:DEVKITPRO = "C:\devkitPro"
$env:DEVKITA64 = "C:\devkitPro\devkitA64"
```

### 运行时找不到资源

确保资源文件已正确配置：

```lua
-- 在 xmake.lua 中添加资源
add_files("romfs/**", {install = true})
```

## 高级配置

### 自定义编译选项

```lua
-- 添加编译选项
add_cxxflags("-O3", "-ffast-math")

-- 添加宏定义
add_defines("E2D_ENABLE_PROFILING")

-- 添加包含路径
add_includedirs("third_party/include")

-- 添加链接库
add_links("pthread")
```

### 条件编译

```lua
if is_plat("windows") then
    add_defines("E2D_PLATFORM_WINDOWS")
elseif is_plat("switch") then
    add_defines("E2D_PLATFORM_SWITCH")
end

if is_mode("debug") then
    add_defines("E2D_DEBUG")
    add_cxxflags("-g", "-O0")
else
    add_defines("E2D_RELEASE")
    add_cxxflags("-O3")
end
```

## 参考链接

- [xmake 官方文档](https://xmake.io/#/zh-cn/)
- [devkitPro 官网](https://devkitpro.org/)
