-- ==============================================
-- Nintendo Switch 工具链定义
-- ==============================================

function define_switch_toolchain()
    toolchain("switch")
        set_kind("standalone")
        set_description("Nintendo Switch devkitA64 toolchain")

        -- 检查 DEVKITPRO 环境变量（Windows 上使用 C:/devkitPro）
        local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
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
    toolchain_end()
end

-- 定义工具链
define_switch_toolchain()
