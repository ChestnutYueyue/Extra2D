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
"""

import os
import sys
import argparse
from pathlib import Path


class ProjectCreator:
    """Extra2D 项目创建器"""

    def __init__(self, project_name: str, output_path: str = None, author: str = "Extra2D Team"):
        self.project_name = project_name
        self.author = author
        self.script_dir = Path(__file__).parent.resolve()
        self.engine_root = self.script_dir.parent
        self.output_path = Path(output_path) if output_path else self.engine_root / "projects"
        self.project_path = self.output_path / project_name

    def create(self) -> bool:
        """创建项目"""
        print(f"正在创建 Extra2D 项目: {self.project_name}")
        print(f"项目路径: {self.project_path}")

        if self.project_path.exists():
            print(f"错误: 项目目录已存在: {self.project_path}")
            return False

        try:
            self._create_directories()
            self._create_main_cpp()
            self._create_xmake_lua()
            self._create_gitignore()
            self._create_readme()

            print(f"\n项目创建成功!")
            print(f"\n后续步骤:")
            print(f"  1. cd {self.project_path}")
            print(f"  2. xmake config")
            print(f"  3. xmake build")
            print(f"  4. xmake run {self.project_name}")

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
        """创建 xmake.lua 文件（使用远程包引入方式）"""
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
-- 依赖包（通过 xmake 远程包管理）
-- ==============================================

add_requires("extra2d")

-- ==============================================
-- 目标定义
-- ==============================================

target("{self.project_name}")
    set_kind("binary")
    add_files("src/**.cpp")
    add_packages("extra2d")

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

### 项目结构

```
{self.project_name}/
├── src/               # 源代码目录
│   └── main.cpp       # 主程序入口
├── romfs/             # 资源文件目录
│   └── assets/
│       ├── images/    # 图片资源
│       └── audio/     # 音频资源
├── xmake.lua          # 构建配置
└── README.md          # 项目说明
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


def main():
    parser = argparse.ArgumentParser(
        description="Extra2D 项目脚手架工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
    python create_project.py my_game
    python create_project.py my_game --path ./games
    python create_project.py my_game --author "Your Name"
        """
    )

    parser.add_argument(
        "name",
        help="项目名称（只能包含字母、数字和下划线）"
    )

    parser.add_argument(
        "--path", "-p",
        help="项目创建路径（默认为引擎根目录下的 projects 文件夹）"
    )

    parser.add_argument(
        "--author", "-a",
        default="Extra2D Team",
        help="项目作者（默认: Extra2D Team）"
    )

    args = parser.parse_args()

    # 验证项目名称
    if not args.name.replace("_", "").isalnum():
        print("错误: 项目名称只能包含字母、数字和下划线")
        sys.exit(1)

    # 创建项目
    creator = ProjectCreator(
        project_name=args.name,
        output_path=args.path,
        author=args.author
    )

    if not creator.create():
        sys.exit(1)


if __name__ == "__main__":
    main()
