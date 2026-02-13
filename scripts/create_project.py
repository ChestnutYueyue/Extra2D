#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Extra2D 项目脚手架工具
用于快速创建新的 Extra2D 游戏项目

用法:
    python create_project.py <项目名称> [选项]

示例:
    python create_project.py my_game
    python create_project.py my_game --path ./games
    python create_project.py my_game --author "Your Name"

项目结构:
    my_game/
    ├── src/
    │   └── main.cpp
    ├── romfs/
    │   └── assets/
    ├── xmake.lua
    ├── README.md
    └── Extra2D/          # 引擎源码（自动克隆）
        ├── Extra2D/
        ├── xmake/
        └── ...
"""

import os
import sys
import argparse
import subprocess
import platform
import shutil
from pathlib import Path
from typing import Optional

ENGINE_REPO = "https://github.com/ChestnutYueyue/Extra2D.git"
DEVKITPRO_URL = "https://github.com/devkitPro/installer/releases/download/v3.0.3/devkitProUpdater-3.0.3.exe"
MINGW_URL = "https://github.com/brechtsanders/winlibs_mingw/releases/download/16.0.0-snapshot20251026posix-14.0.0-ucrt-r1/winlibs-i686-posix-dwarf-gcc-16.0.0-snapshot20251026-mingw-w64ucrt-14.0.0-r1.zip"


class DevToolsChecker:
    """开发工具检测器"""

    def __init__(self):
        self.is_windows = platform.system() == "Windows"

    def check_git(self) -> bool:
        """检查 Git 是否安装"""
        return shutil.which("git") is not None

    def check_xmake(self) -> bool:
        """检查 xmake 是否安装"""
        return shutil.which("xmake") is not None

    def check_mingw(self) -> bool:
        """检查 MinGW 是否安装"""
        if shutil.which("gcc"):
            result = subprocess.run(
                ["gcc", "-dumpmachine"],
                capture_output=True,
                text=True,
                creationflags=subprocess.CREATE_NO_WINDOW if self.is_windows else 0
            )
            return "mingw" in result.stdout.lower()
        return False

    def check_devkitpro(self) -> bool:
        """检查 devkitPro 是否安装"""
        devkitpro = os.environ.get("DEVKITPRO", "C:/devkitPro")
        devkita64 = os.path.join(devkitpro, "devkitA64")
        return os.path.isdir(devkita64)

    def get_missing_tools(self) -> dict:
        """获取缺失的工具列表"""
        missing = {}

        if not self.check_git():
            missing["git"] = "Git 版本控制工具"

        if not self.check_xmake():
            missing["xmake"] = "xmake 构建工具"

        return missing

    def get_missing_dev_tools(self) -> dict:
        """获取缺失的开发工具链"""
        missing = {}

        if not self.check_mingw():
            missing["mingw"] = MINGW_URL

        if not self.check_devkitpro():
            missing["devkitpro"] = DEVKITPRO_URL

        return missing

    def print_tool_status(self):
        """打印工具状态"""
        print("\n========================================")
        print("开发环境检测")
        print("========================================")

        tools = [
            ("Git", self.check_git()),
            ("xmake", self.check_xmake()),
            ("MinGW (Windows开发)", self.check_mingw()),
            ("devkitPro (Switch开发)", self.check_devkitpro()),
        ]

        for name, installed in tools:
            status = "✓ 已安装" if installed else "✗ 未安装"
            print(f"  {name}: {status}")

        print("========================================\n")


class ProjectCreator:
    """Extra2D 项目创建器"""

    def __init__(self, project_name: str, output_path: str = None, author: str = "Extra2D Team"):
        self.project_name = project_name
        self.author = author
        self.output_path = Path(output_path) if output_path else Path.cwd()
        self.project_path = self.output_path / project_name
        self.engine_path = self.project_path / "Extra2D"

    def create(self) -> bool:
        """创建项目"""
        print(f"\n正在创建 Extra2D 项目: {self.project_name}")
        print(f"项目路径: {self.project_path}")

        if self.project_path.exists():
            print(f"错误: 项目目录已存在: {self.project_path}")
            return False

        try:
            self._create_directories()
            self._copy_system_font()
            self._create_main_cpp()
            self._create_xmake_lua()
            self._create_gitignore()
            self._create_readme()

            print(f"\n✓ 项目创建成功!")

            if self._clone_engine():
                self._print_next_steps()
            else:
                print("\n请手动克隆引擎源码:")
                print(f"  cd {self.project_path}")
                print(f"  git clone {ENGINE_REPO} Extra2D")

            return True
        except Exception as e:
            print(f"创建项目时出错: {e}")
            return False

    def _create_directories(self):
        """创建项目目录结构"""
        print("创建目录结构...")
        self.project_path.mkdir(parents=True, exist_ok=True)
        (self.project_path / "romfs" / "assets" / "images").mkdir(parents=True, exist_ok=True)
        (self.project_path / "romfs" / "assets" / "audio").mkdir(parents=True, exist_ok=True)
        (self.project_path / "src").mkdir(parents=True, exist_ok=True)

    def _copy_system_font(self):
        """复制系统字体到项目 assets 目录"""
        print("复制系统字体...")
        font_dest = self.project_path / "romfs" / "assets" / "font.ttf"

        if platform.system() == "Windows":
            font_sources = [
                Path("C:/Windows/Fonts/msyh.ttc"),      # 微软雅黑
                Path("C:/Windows/Fonts/msyh.ttf"),      # 微软雅黑 (部分系统)
                Path("C:/Windows/Fonts/simhei.ttf"),    # 黑体
                Path("C:/Windows/Fonts/simsun.ttc"),    # 宋体
            ]
        else:
            font_sources = [
                Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
                Path("/usr/share/fonts/TTF/DejaVuSans.ttf"),
                Path("/System/Library/Fonts/PingFang.ttc"),  # macOS
            ]

        for font_src in font_sources:
            if font_src.exists():
                try:
                    shutil.copy2(str(font_src), str(font_dest))
                    print(f"✓ 已复制字体: {font_src.name}")
                    return True
                except Exception as e:
                    print(f"✗ 复制字体失败: {e}")
                    continue

        print("⚠ 未找到系统字体，请手动添加字体文件到 romfs/assets/font.ttf")
        return False

    def _clone_engine(self) -> bool:
        """克隆引擎源码到项目目录内的 Extra2D 子目录"""
        print(f"\n正在克隆 Extra2D 引擎源码...")
        print(f"仓库地址: {ENGINE_REPO}")
        print(f"目标路径: {self.engine_path}")

        try:
            result = subprocess.run(
                ["git", "clone", ENGINE_REPO, str(self.engine_path)],
                cwd=str(self.project_path),
                creationflags=subprocess.CREATE_NO_WINDOW if platform.system() == "Windows" else 0
            )

            if result.returncode == 0:
                print("✓ 引擎源码克隆成功!")
                return True
            else:
                print("✗ 引擎源码克隆失败!")
                return False
        except Exception as e:
            print(f"✗ 克隆过程中出错: {e}")
            return False

    def _create_main_cpp(self):
        """创建 main.cpp 文件（放在 src 目录下）"""
        print("创建 src/main.cpp...")
        content = f'''#include <extra2d/extra2d.h>

using namespace extra2d;

// ============================================================================
// 主场景
// ============================================================================

/**
 * @brief 主游戏场景
 */
class MainScene : public Scene {{
public:
    /**
     * @brief 场景进入时调用
     */
    void onEnter() override {{
        E2D_LOG_INFO("MainScene::onEnter - 进入场景");

        // 设置背景颜色
        setBackgroundColor(Color(0.1f, 0.1f, 0.2f, 1.0f));

        // 加载字体（请确保 assets 目录下有 font.ttf 文件）
        auto &resources = Application::instance().resources();
        font_ = resources.loadFont("assets/font.ttf", 32, true);

        if (!font_) {{
            E2D_LOG_ERROR("字体加载失败！请确保 assets 目录下有 font.ttf 文件");
            return;
        }}

        // 创建标题文本
        auto title = Text::create("{self.project_name}", font_);
        title->setCoordinateSpace(CoordinateSpace::Screen);
        title->setScreenPosition(640.0f, 200.0f);
        title->setAnchor(0.5f, 0.5f);
        title->setTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        addChild(title);

        // 创建提示文本
        auto hint = Text::create("按 START 退出", font_);
        hint->setCoordinateSpace(CoordinateSpace::Screen);
        hint->setScreenPosition(640.0f, 650.0f);
        hint->setAnchor(0.5f, 0.5f);
        hint->setTextColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
        addChild(hint);
    }}

    /**
     * @brief 每帧更新时调用
     * @param dt 时间间隔（秒）
     */
    void onUpdate(float dt) override {{
        Scene::onUpdate(dt);

        // 检查退出按键
        auto &input = Application::instance().input();
        if (input.isButtonPressed(GamepadButton::Start)) {{
            E2D_LOG_INFO("退出应用");
            Application::instance().quit();
        }}
    }}

private:
    Ptr<FontAtlas> font_;
}};

// ============================================================================
// 程序入口
// ============================================================================

int main(int argc, char **argv) {{
    // 初始化日志系统
    Logger::init();
    Logger::setLevel(LogLevel::Debug);

    E2D_LOG_INFO("========================");
    E2D_LOG_INFO("{self.project_name}");
    E2D_LOG_INFO("========================");

    // 获取应用实例
    auto &app = Application::instance();

    // 配置应用
    AppConfig config;
    config.title = "{self.project_name}";
    config.width = 1280;
    config.height = 720;
    config.vsync = true;
    config.fpsLimit = 60;

    // 初始化应用
    if (!app.init(config)) {{
        E2D_LOG_ERROR("应用初始化失败！");
        return -1;
    }}

    // 进入主场景
    app.enterScene(makePtr<MainScene>());

    E2D_LOG_INFO("开始主循环...");

    // 运行应用
    app.run();

    E2D_LOG_INFO("应用结束");

    return 0;
}}
'''
        with open(self.project_path / "src" / "main.cpp", "w", encoding="utf-8") as f:
            f.write(content)

    def _create_xmake_lua(self):
        """创建 xmake.lua 文件（使用项目内的引擎源码）"""
        print("创建 xmake.lua...")
        content = f'''-- ==============================================
-- {self.project_name} - Extra2D 游戏项目
-- 构建系统: Xmake
-- 支持平台: MinGW (Windows), Nintendo Switch
-- ==============================================

-- 项目元信息
set_project("{self.project_name}")
set_version("1.0.0")
set_license("MIT")

-- 语言设置
set_languages("c++17")
set_encodings("utf-8")

-- 构建模式
add_rules("mode.debug", "mode.release")

-- ==============================================
-- 平台检测与配置
-- ==============================================

local host_plat = os.host()
local target_plat = get_config("plat") or host_plat
local supported_plats = {{mingw = true, switch = true}}

if not supported_plats[target_plat] then
    if host_plat == "windows" then
        target_plat = "mingw"
    else
        error("Unsupported platform: " .. target_plat .. ". Supported platforms: mingw, switch")
    end
end

set_plat(target_plat)

if target_plat == "switch" then
    set_arch("arm64")
elseif target_plat == "mingw" then
    set_arch("x86_64")
end

-- ==============================================
-- 加载工具链配置
-- ==============================================

-- 引擎目录（克隆的仓库目录）
local engine_repo = "Extra2D"
-- 引擎源码目录（仓库内的 Extra2D 子目录）
local engine_src = path.join(engine_repo, "Extra2D")

if target_plat == "switch" then
    includes(path.join(engine_repo, "xmake/toolchains/switch.lua"))
    set_toolchains("switch")
elseif target_plat == "mingw" then
    set_toolchains("mingw")
end

-- ==============================================
-- 添加依赖包 (MinGW)
-- ==============================================

if target_plat == "mingw" then
    add_requires("glm", "libsdl2", "libsdl2_mixer")
end

-- ==============================================
-- 引擎库定义
-- ==============================================

target("extra2d")
    set_kind("static")

    -- 引擎源文件
    add_files(path.join(engine_src, "src/**.cpp"))
    add_files(path.join(engine_src, "src/glad/glad.c"))

    -- 头文件路径
    add_includedirs(path.join(engine_src, "include"), {{public = true}})
    add_includedirs(path.join(engine_src, "include/extra2d/platform"), {{public = true}})

    -- 平台配置
    if target_plat == "switch" then
        local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
        add_includedirs(devkitPro .. "/portlibs/switch/include", {{public = true}})
        add_linkdirs(devkitPro .. "/portlibs/switch/lib")
        add_syslinks("SDL2_mixer", "SDL2", "opusfile", "opus", "vorbisidec", "ogg",
                     "modplug", "mpg123", "FLAC", "GLESv2", "EGL", "glapi", "drm_nouveau",
                     {{public = true}})
    elseif target_plat == "mingw" then
        add_packages("glm", "libsdl2", "libsdl2_mixer", {{public = true}})
        add_syslinks("opengl32", "glu32", "winmm", "imm32", "version", "setupapi", {{public = true}})
    end

    -- 编译器标志
    add_cxflags("-Wall", "-Wextra", {{force = true}})
    add_cxflags("-Wno-unused-variable", "-Wno-unused-function", "-Wno-unused-parameter", {{force = true}})
    add_cxflags("-Wno-strict-aliasing", "-Wno-implicit-fallthrough", {{force = true}})
    add_cxflags("-Wno-missing-field-initializers", {{force = true}})
    add_cxxflags("-Wno-deprecated-copy", "-Wno-class-memaccess", {{force = true}})

    if is_mode("debug") then
        add_defines("E2D_DEBUG", "_DEBUG", {{public = true}})
        add_cxxflags("-O0", "-g", {{force = true}})
    else
        add_defines("NDEBUG", {{public = true}})
        add_cxxflags("-O2", {{force = true}})
    end
target_end()

-- ==============================================
-- 目标定义
-- ==============================================

target("{self.project_name}")
    set_kind("binary")
    add_files("src/**.cpp")
    add_deps("extra2d")

    -- Nintendo Switch 平台配置
    if is_plat("switch") then
        set_targetdir("build/switch")

        -- 构建后生成 NRO 文件
        after_build(function (target)
            local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
            local elf_file = target:targetfile()
            local output_dir = path.directory(elf_file)
            local nacp_file = path.join(output_dir, "{self.project_name}.nacp")
            local nro_file = path.join(output_dir, "{self.project_name}.nro")
            local nacptool = path.join(devkitPro, "tools/bin/nacptool.exe")
            local elf2nro = path.join(devkitPro, "tools/bin/elf2nro.exe")

            if os.isfile(nacptool) and os.isfile(elf2nro) then
                os.vrunv(nacptool, {{"--create", "{self.project_name}", "{self.author}", "1.0.0", nacp_file}})
                local project_dir = os.scriptdir()
                local romfs = path.join(project_dir, "romfs")
                if os.isdir(romfs) then
                    os.vrunv(elf2nro, {{elf_file, nro_file, "--nacp=" .. nacp_file, "--romfsdir=" .. romfs}})
                else
                    os.vrunv(elf2nro, {{elf_file, nro_file, "--nacp=" .. nacp_file}})
                end
                print("Generated NRO: " .. nro_file)
            end
        end)

        -- 打包时将 NRO 文件复制到 package 目录
        after_package(function (target)
            local nro_file = path.join(target:targetdir(), "{self.project_name}.nro")
            local package_dir = target:packagedir()
            if os.isfile(nro_file) and package_dir then
                os.cp(nro_file, package_dir)
                print("Copied NRO to package: " .. package_dir)
            end
        end)

    -- Windows 平台配置
    elseif is_plat("mingw", "windows") then
        set_targetdir("build/windows")
        add_ldflags("-mwindows", {{force = true}})

        -- 复制资源到输出目录
        after_build(function (target)
            local project_dir = os.scriptdir()
            local romfs = path.join(project_dir, "romfs")
            if os.isdir(romfs) then
                local target_dir = path.directory(target:targetfile())
                local assets_dir = path.join(target_dir, "assets")

                if not os.isdir(assets_dir) then
                    os.mkdir(assets_dir)
                end

                os.cp(path.join(romfs, "assets/**"), assets_dir)
                print("Copied assets to: " .. assets_dir)
            end
        end)
    end
target_end()

-- ==============================================
-- 项目信息输出
-- ==============================================

print("========================================")
print("{self.project_name} Build Configuration")
print("========================================")
print("Platform: " .. target_plat)
print("Architecture: " .. (get_config("arch") or "auto"))
print("Mode: " .. (is_mode("debug") and "debug" or "release"))
print("========================================")
'''
        with open(self.project_path / "xmake.lua", "w", encoding="utf-8") as f:
            f.write(content)

    def _create_gitignore(self):
        """创建 .gitignore 文件"""
        print("创建 .gitignore...")
        content = '''# 构建产物
.build/
build/
.xmake/

# IDE 配置
.vscode/
.idea/
*.swp
*.swo

# 操作系统文件
.DS_Store
Thumbs.db

# 临时文件
*.tmp
*.temp
*.log
'''
        with open(self.project_path / ".gitignore", "w", encoding="utf-8") as f:
            f.write(content)

    def _create_readme(self):
        """创建 README.md 文件"""
        print("创建 README.md...")
        content = f'''# {self.project_name}

使用 Extra2D 游戏引擎开发的游戏项目。

## 支持平台

- MinGW (Windows)
- Nintendo Switch

## 项目结构

```
{self.project_name}/
├── src/               # 源代码目录
│   └── main.cpp       # 主程序入口
├── romfs/             # 资源文件目录
│   └── assets/
│       ├── images/    # 图片资源
│       └── audio/     # 音频资源
├── Extra2D/           # 引擎源码
├── xmake.lua          # 构建配置
└── README.md          # 项目说明
```

## 构建说明

### 前置要求

- [xmake](https://xmake.io/) 构建工具
- MinGW-w64 工具链 (Windows) 或 devkitPro (Switch)

### 构建步骤

#### Windows 平台

```bash
# 配置项目
xmake config -p mingw

# 构建项目
xmake build

# 运行项目
xmake run {self.project_name}
```

#### Nintendo Switch 平台

```bash
# 配置项目
xmake config -p switch

# 构建项目
xmake build

# 生成的 NRO 文件位于 build/switch/ 目录
```

## 资源文件

请将游戏所需的资源文件放入 `romfs/assets/` 目录：

- `images/` - 图片资源（PNG, JPG 等）
- `audio/` - 音频资源（WAV, OGG 等）
- `font.ttf` - 字体文件（需要手动添加）

## 作者

{self.author}

## 许可证

MIT License
'''
        with open(self.project_path / "README.md", "w", encoding="utf-8") as f:
            f.write(content)

    def _print_next_steps(self):
        """打印后续步骤"""
        print(f"\n后续步骤:")
        print(f"  1. cd {self.project_path}")
        print(f"  2. xmake config -p mingw")
        print(f"  3. xmake build")
        print(f"  4. xmake run {self.project_name}")


def prompt_download_tools(dev_tools: dict):
    """
    提示用户下载缺失的开发工具链
    
    @param dev_tools: 缺失的开发工具链
    """
    if not dev_tools:
        return

    print("\n========================================")
    print("检测到以下开发工具链未安装:")
    print("========================================")

    if "mingw" in dev_tools:
        print("  - MinGW-w64 (Windows 开发)")
    if "devkitpro" in dev_tools:
        print("  - devkitPro (Switch 开发)")

    print("\n是否需要下载这些工具?")
    print("  1. 下载 MinGW-w64 (Windows 开发)")
    print("  2. 下载 devkitPro (Switch 开发)")
    print("  3. 下载全部")
    print("  4. 跳过下载")

    try:
        choice = input("\n请选择 (1-4): ").strip()

        import webbrowser

        if choice == "1" and "mingw" in dev_tools:
            print(f"\n正在打开 MinGW 下载页面...")
            print(f"下载地址: {MINGW_URL}")
            webbrowser.open(MINGW_URL)
        elif choice == "2" and "devkitpro" in dev_tools:
            print(f"\n正在打开 devkitPro 下载页面...")
            print(f"下载地址: {DEVKITPRO_URL}")
            webbrowser.open(DEVKITPRO_URL)
        elif choice == "3":
            if "mingw" in dev_tools:
                print(f"\n正在打开 MinGW 下载页面...")
                print(f"下载地址: {MINGW_URL}")
                webbrowser.open(MINGW_URL)
            if "devkitpro" in dev_tools:
                print(f"\n正在打开 devkitPro 下载页面...")
                print(f"下载地址: {DEVKITPRO_URL}")
                webbrowser.open(DEVKITPRO_URL)
        else:
            print("\n已跳过下载。")

        if dev_tools:
            print("\n安装提示:")
            if "mingw" in dev_tools:
                print("  MinGW: 解压后将 bin 目录添加到系统 PATH 环境变量")
            if "devkitpro" in dev_tools:
                print("  devkitPro: 运行安装程序，按提示完成安装")

    except KeyboardInterrupt:
        print("\n已取消。")


def main():
    parser = argparse.ArgumentParser(
        description="Extra2D 项目脚手架工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
    python create_project.py my_game
    python create_project.py my_game --path ./games
    python create_project.py my_game --author "Your Name"

项目结构:
    my_game/
    ├── src/
    │   └── main.cpp
    ├── romfs/
    │   └── assets/
    ├── xmake.lua
    ├── README.md
    └── Extra2D/          # 引擎源码（自动克隆）
        """
    )

    parser.add_argument(
        "name",
        help="项目名称（只能包含字母、数字和下划线）"
    )

    parser.add_argument(
        "--path", "-p",
        help="项目创建路径（默认为当前目录）"
    )

    parser.add_argument(
        "--author", "-a",
        default="Extra2D Team",
        help="项目作者（默认: Extra2D Team）"
    )

    args = parser.parse_args()

    checker = DevToolsChecker()
    checker.print_tool_status()

    missing_tools = checker.get_missing_tools()
    if missing_tools:
        print("错误: 缺少必要的工具:")
        for tool, desc in missing_tools.items():
            print(f"  - {desc}")
        print("\n请先安装这些工具后再创建项目。")
        sys.exit(1)

    dev_tools = checker.get_missing_dev_tools()
    if dev_tools:
        prompt_download_tools(dev_tools)

    if not args.name.replace("_", "").isalnum():
        print("错误: 项目名称只能包含字母、数字和下划线")
        sys.exit(1)

    creator = ProjectCreator(
        project_name=args.name,
        output_path=args.path,
        author=args.author
    )

    if not creator.create():
        sys.exit(1)


if __name__ == "__main__":
    main()
