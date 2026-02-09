-- ==============================================
-- Nintendo Switch 工具链定义
-- ==============================================

function define_switch_toolchain()
    toolchain("switch")
        set_kind("standalone")
        set_description("Nintendo Switch devkitA64 toolchain")

        -- 检查 DEVKITPRO 环境变量（Windows 上使用 C:/devkitPro）
        local devkitPro = "C:/devkitPro"
        local devkitA64 = path.join(devkitPro, "devkitA64")

        -- 设置工具链路径
        set_toolset("cc", path.join(devkitA64, "bin/aarch64-none-elf-gcc.exe"))
        set_toolset("cxx", path.join(devkitA64, "bin/aarch64-none-elf-g++.exe"))
        set_toolset("ld", path.join(devkitA64, "bin/aarch64-none-elf-g++.exe"))
        set_toolset("ar", path.join(devkitA64, "bin/aarch64-none-elf-gcc-ar.exe"))
        set_toolset("strip", path.join(devkitA64, "bin/aarch64-none-elf-strip.exe"))

        -- 架构标志
        local arch_flags = "-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE"
        add_cxflags(arch_flags)
        -- 使用 devkitPro 提供的 switch.specs 文件
        add_ldflags("-specs=" .. path.join(devkitPro, "libnx/switch.specs"), "-g", arch_flags)

        -- 定义 Switch 平台宏
        add_defines("__SWITCH__", "__NX__", "MA_SWITCH", "PFD_SWITCH")
        
        -- SimpleIni 配置：不使用 Windows API
        add_defines("SI_NO_CONVERSION")
        
        -- libnx 路径 - 必须在工具链级别添加
        add_includedirs(path.join(devkitPro, "libnx/include"))
        add_linkdirs(path.join(devkitPro, "libnx/lib"))
        
        -- portlibs 路径（EGL + 桌面 OpenGL + SDL2）
        add_includedirs(path.join(devkitPro, "portlibs/switch/include"))
        add_includedirs(path.join(devkitPro, "portlibs/switch/include/SDL2"))
        add_linkdirs(path.join(devkitPro, "portlibs/switch/lib"))
        
        add_syslinks("nx", "m")
end

-- 获取 devkitPro 路径
function get_devkitpro_path()
    return "C:/devkitPro"
end

-- 获取 Switch 平台包含路径
function get_switch_includedirs()
    local devkitPro = get_devkitpro_path()
    return {
        path.join(devkitPro, "libnx/include"),
        path.join(devkitPro, "portlibs/switch/include"),
        path.join(devkitPro, "portlibs/switch/include/SDL2")
    }
end

-- 获取 Switch 平台库路径
function get_switch_linkdirs()
    local devkitPro = get_devkitpro_path()
    return {
        path.join(devkitPro, "libnx/lib"),
        path.join(devkitPro, "portlibs/switch/lib")
    }
end

-- 获取 Switch 平台系统链接库
function get_switch_syslinks()
    -- 注意：链接顺序很重要！被依赖的库必须放在后面
    -- 依赖链：SDL2 -> EGL -> drm_nouveau
    --          GLESv2 -> glapi -> drm_nouveau
    return {
        "SDL2_mixer", "SDL2",
        "opusfile", "opus", "vorbisidec", "ogg",
        "modplug", "mpg123", "FLAC",
        "GLESv2",
        "EGL",
        "glapi",
        "drm_nouveau"
    }
end
