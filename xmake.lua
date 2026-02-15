-- ==============================================
-- Extra2D - 2D Game Engine
-- Build System: Xmake
-- 
-- 支持平台:
-- - Windows (MinGW)
-- - Linux
-- - macOS
-- - Nintendo Switch
-- 
-- 窗口后端: SDL2 (统一)
-- ==============================================

-- 项目元信息
set_project("Extra2D")
set_version("1.2.0")
set_license("MIT")

-- 语言和编码设置
set_languages("c++17")
set_encodings("utf-8")

-- 构建模式
add_rules("mode.debug", "mode.release")

-- ==============================================
-- 构建选项
-- ==============================================

option("debug_logs")
    set_default(false)
    set_showmenu(true)
    set_description("Enable debug logging")
option_end()

-- ==============================================
-- 平台检测与配置
-- ==============================================

local host_plat = os.host()
local target_plat = get_config("plat") or host_plat
local supported_plats = {mingw = true, windows = true, linux = true, macosx = true, switch = true}

-- 自动选择平台
if not supported_plats[target_plat] then
    if host_plat == "windows" then
        target_plat = "mingw"
    elseif host_plat == "linux" then
        target_plat = "linux"
    elseif host_plat == "macosx" then
        target_plat = "macosx"
    else
        error("Unsupported platform: " .. target_plat .. ". Supported platforms: mingw, windows, linux, macosx, switch")
    end
end

set_plat(target_plat)

-- 设置架构
if target_plat == "switch" then
    set_arch("arm64")
elseif target_plat == "mingw" or target_plat == "windows" then
    set_arch("x86_64")
elseif target_plat == "linux" then
    set_arch("x86_64")
elseif target_plat == "macosx" then
    set_arch("x86_64")
end

-- ==============================================
-- 加载工具链配置
-- ==============================================

if target_plat == "switch" then
    includes("xmake/toolchains/switch.lua")
    set_toolchains("switch")
elseif target_plat == "mingw" then
    set_toolchains("mingw")
end

-- ==============================================
-- 添加依赖包
-- ==============================================

if target_plat ~= "switch" then
    add_requires("glm")
    add_requires("nlohmann_json")
    local sdl2_configs = {
        configs = {
            wayland = false
        }
    }
    if target_plat == "linux" then
        local is_wayland = os.getenv("XDG_SESSION_TYPE") == "wayland"
        if is_wayland then
            sdl2_configs.configs.wayland = true
        end
    end
    add_requires("libsdl2", sdl2_configs)
end


-- ==============================================
-- 加载构建目标
-- ==============================================

-- 加载引擎库定义
includes("xmake/engine.lua")

-- 定义引擎库
define_extra2d_engine()

-- ==============================================
-- Shader文件安装函数
-- ==============================================

function install_shaders(target)
    local plat = get_config("plat") or os.host()
    local targetdir = target:targetdir()
    
    local shader_src = "Extra2D/shaders"
    
    if plat == "switch" then
        local romfs_dir = path.join(targetdir, "romfs")
        local shader_dest = path.join(romfs_dir, "shaders")
        
        os.rm(shader_dest)
        os.cp(shader_src, shader_dest)
        print("Shaders installed to romfs: " .. shader_dest)
    else
        local shader_dest = path.join(targetdir, "shaders")
        
        os.rm(shader_dest)
        os.cp(shader_src, shader_dest)
        print("Shaders installed to: " .. shader_dest)
    end
end

-- ==============================================
-- 示例程序
-- ==============================================

-- 基础示例
target("demo_basic")
    set_kind("binary")
    set_default(false)
    
    add_deps("extra2d")
    add_files("examples/basic/main.cpp")
    
    -- 平台配置
    local plat = get_config("plat") or os.host()
    if plat == "mingw" or plat == "windows" then
        add_packages("glm", "nlohmann_json", "libsdl2")
        add_syslinks("opengl32", "glu32", "winmm", "imm32", "version", "setupapi")
    elseif plat == "linux" then
        add_packages("glm", "nlohmann_json", "libsdl2")
        add_syslinks("GL", "dl", "pthread")
    elseif plat == "macosx" then
        add_packages("glm", "nlohmann_json", "libsdl2")
        add_frameworks("OpenGL", "Cocoa", "IOKit", "CoreVideo")
    end
    
    -- 构建后安装Shader文件
    after_build(install_shaders)
target_end()
