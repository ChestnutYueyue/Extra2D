-- ==============================================
-- Extra2D - 2D Game Engine
-- Build System: Xmake
-- Platforms: MinGW (Windows), Nintendo Switch
-- ==============================================

-- 项目元信息
set_project("Extra2D")
set_version("1.1.0")
set_license("MIT")

-- 语言和编码设置
set_languages("c++17")
set_encodings("utf-8")

-- 构建模式
add_rules("mode.debug", "mode.release")

-- ==============================================
-- 构建选项
-- ==============================================

option("examples")
    set_default(false)
    set_showmenu(true)
    set_description("Build example programs")
option_end()

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
local supported_plats = {mingw = true, switch = true}

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

if target_plat == "switch" then
    includes("xmake/toolchains/switch.lua")
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
-- 加载构建目标
-- ==============================================

-- 加载引擎库定义
includes("xmake/engine.lua")

-- 定义引擎库
define_extra2d_engine()

-- 示例程序目标（作为子项目）
if is_config("examples","true") then
    includes("examples/hello_world", {rootdir = "examples/hello_world"})
    includes("examples/spatial_index_demo", {rootdir = "examples/spatial_index_demo"})
    includes("examples/collision_demo", {rootdir = "examples/collision_demo"})
    includes("examples/push_box", {rootdir = "examples/push_box"})
    includes("examples/flappy_bird", {rootdir = "examples/flappy_bird"})
end

-- ==============================================
-- 项目信息输出
-- ==============================================

print("========================================")
print("Extra2D Build Configuration")
print("========================================")
print("Platform: " .. target_plat)
print("Architecture: " .. (get_config("arch") or "auto"))
print("Mode: " .. (is_mode("debug") and "debug" or "release"))
print("Examples: " .. (has_config("examples") and "enabled" or "disabled"))
print("========================================")
