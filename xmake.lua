-- ==============================================
-- Extra2D - Xmake Build Script
-- Purpose: Build Extra2D static library and demo programs
-- Platform: Nintendo Switch (ARM64) / PC (Windows/Linux/macOS)
-- Graphics: OpenGL ES 3.2 via Mesa/Angle
-- Audio: SDL2_mixer
-- ==============================================

set_project("Extra2D")
set_version("3.1.0")
set_languages("c++17")
set_encodings("utf-8")
add_rules("mode.debug", "mode.release")

-- ==============================================
-- 平台检测与配置 (必须在 includes 之前)
-- ==============================================

-- 检测目标平台 - 优先级：命令行 > 环境变量 > 默认值
local target_platform = "switch"  -- 默认 Switch 平台

-- 方式1: 检查命令行传入的平台配置 (最高优先级)
local plat_config = get_config("plat")
if plat_config and plat_config ~= "" then
    if plat_config == "windows" or plat_config == "linux" or plat_config == "macosx" then
        target_platform = "pc"
    elseif plat_config == "switch" then
        target_platform = "switch"
    end
-- 方式2: 通过环境变量检测 PC 平台
elseif os.getenv("E2D_PLATFORM") == "pc" then
    target_platform = "pc"
-- 方式3: 检查 platform 配置
elseif has_config("platform") then
    local platform_val = get_config("platform")
    if platform_val == "pc" then
        target_platform = "pc"
    end
end

-- 设置默认平台和架构 (必须在 includes 之前调用)
if target_platform == "switch" then
    set_config("plat", "switch")
    set_config("arch", "arm64")
else
    -- PC 平台：根据主机自动选择
    if is_host("windows") then
        set_config("plat", "windows")
        set_config("arch", "x64")
    elseif is_host("linux") then
        set_config("plat", "linux")
    elseif is_host("macosx") then
        set_config("plat", "macosx")
    end
end

print("Extra2D Build Configuration:")
print("  Platform: " .. target_platform)
print("  Mode: " .. (is_mode("debug") and "debug" or "release"))

-- ==============================================
-- 包含子模块配置
-- ==============================================

-- 包含工具链定义
includes("xmake/toolchains/switch.lua")
includes("xmake/toolchains/pc.lua")

-- 根据平台定义工具链
if target_platform == "switch" then
    define_switch_toolchain()
else
    define_pc_toolchain()
end

-- 包含目标定义
includes("xmake/targets/extra2d.lua")
includes("xmake/targets/examples.lua")

-- ==============================================
-- 定义构建目标
-- ==============================================

-- Extra2D 引擎库
define_extra2d_target()

-- 示例程序
define_example_targets()
