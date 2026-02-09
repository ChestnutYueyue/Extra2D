-- ==============================================
-- Extra2D for Nintendo Switch - Xmake Build Script
-- Purpose: Build Extra2D static library and Switch demo programs
-- Platform: Nintendo Switch (ARM64)
-- Graphics: Desktop OpenGL 3.3+ via Mesa EGL
-- Audio: SDL2_mixer
-- ==============================================

set_project("Extra2D")
set_version("3.1.0")
set_languages("c++17")
set_encodings("utf-8")
add_rules("mode.debug", "mode.release")

-- ==============================================
-- Nintendo Switch 工具链定义
-- ==============================================
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
    -- 使用修复后的 switch_fix.specs 文件（使用 Windows 路径）
    add_ldflags("-specs=switch_fix.specs", "-g", arch_flags)

    -- 定义 Switch 平台宏
    add_defines("__SWITCH__", "__NX__", "MA_SWITCH", "PFD_SWITCH")
    
    -- SimpleIni 配置：不使用 Windows API
    add_defines("SI_NO_CONVERSION")
    
    -- OpenGL 配置：使用标准 GLES3.2
    add_defines("GL_GLES_PROTOTYPES")

    -- libnx 路径 - 必须在工具链级别添加
    add_includedirs(path.join(devkitPro, "libnx/include"))
    add_linkdirs(path.join(devkitPro, "libnx/lib"))
    
    -- portlibs 路径（EGL + 桌面 OpenGL + SDL2）
    add_includedirs(path.join(devkitPro, "portlibs/switch/include"))
    add_includedirs(path.join(devkitPro, "portlibs/switch/include/SDL2"))
    add_linkdirs(path.join(devkitPro, "portlibs/switch/lib"))
    
    add_syslinks("nx", "m")

-- 核心路径定义
local SRC_DIR = "Extra2D/src"
local INC_DIR = "Extra2D/include"

-- ==============================================
-- 1. Extra2D 静态库 (Switch 专用)
-- ==============================================
target("extra2d")
    set_kind("static")
    set_plat("switch")
    set_arch("arm64")
    set_toolchains("switch")
    set_basename(is_mode("debug") and "libeasy2dd" or "libeasy2d")

    -- 引擎源文件
    add_files(path.join(SRC_DIR, "**.cpp"))

    -- Squirrel 3.2 源文件
    add_files("squirrel/squirrel/*.cpp")
    add_files("squirrel/sqstdlib/*.cpp")

    -- 公开头文件目录
    add_includedirs(INC_DIR, {public = true})

    -- 第三方头文件目录
    add_includedirs("squirrel/include", {public = true})

    -- ==============================================
    -- Nintendo Switch 平台配置
    -- ==============================================

    -- devkitPro mesa 路径（EGL + 桌面 OpenGL）
    local devkitPro = "C:/devkitPro"
    add_includedirs(path.join(devkitPro, "portlibs/switch/include"), {public = true})
    add_linkdirs(path.join(devkitPro, "portlibs/switch/lib"))

    -- 使用系统 GLES3.2 头文件 (位于 devkitPro/portlibs/switch/include)

    -- 链接 EGL、OpenGL ES 3.0（mesa）和 SDL2 音频
    -- 注意：链接顺序很重要！被依赖的库必须放在后面
    -- 依赖链：SDL2 -> EGL -> drm_nouveau
    --          GLESv2 -> glapi -> drm_nouveau
    add_syslinks("SDL2_mixer", "SDL2",
                 "opusfile", "opus", "vorbisidec", "ogg",
                 "modplug", "mpg123", "FLAC",
                 "GLESv2",
                 "EGL",
                 "glapi",
                 "drm_nouveau",
                 {public = true})

    -- 注意：pfd (portable-file-dialogs) 暂时禁用，需要进一步修复
    -- add_files(path.join(INC_DIR, "pfd/pfd_switch.cpp"))

    -- 添加 Switch 兼容性头文件路径
    add_includedirs(path.join(INC_DIR, "extra2d/platform"), {public = true})

    -- Switch 特定编译标志
    -- 注意：Squirrel 脚本绑定使用 dynamic_cast，需要 RTTI 支持
    -- add_cxflags("-fno-rtti", {force = true})
    add_cxflags("-Wno-unused-variable", "-Wno-unused-function", {force = true})
    
    -- Squirrel 第三方库警告抑制
    add_cxflags("-Wno-deprecated-copy", "-Wno-strict-aliasing", "-Wno-implicit-fallthrough", "-Wno-class-memaccess", {force = true})

    -- 使用 switch 工具链
    set_toolchains("switch")

    -- ==============================================
    -- 头文件安装配置
    -- ==============================================
    add_headerfiles(path.join(INC_DIR, "extra2d/**.h"), {prefixdir = "extra2d"})
    -- 使用 devkitPro 的 switch-glm 替代项目自带的 GLM
    -- add_headerfiles(path.join(INC_DIR, "glm/**.hpp"), {prefixdir = "glm"})
    add_headerfiles(path.join(INC_DIR, "stb/**.h"), {prefixdir = "stb"})
    add_headerfiles(path.join(INC_DIR, "simpleini/**.h"), {prefixdir = "simpleini"})

    -- 编译器配置
    add_cxxflags("-Wall", "-Wextra", {force = true})
    add_cxxflags("-Wno-unused-parameter", {force = true})
    if is_mode("debug") then
        add_defines("E2D_DEBUG", "_DEBUG", {public = true})
        add_cxxflags("-O0", "-g", {force = true})
    else
        add_defines("NDEBUG", {public = true})
        add_cxxflags("-O2", {force = true})
    end
target_end()

-- ==============================================
-- 2. Nintendo Switch 音频演示
-- ==============================================
target("switch_audio_demo")
    set_kind("binary")
    set_plat("switch")
    set_arch("arm64")
    set_toolchains("switch")
    
    add_files("Extra2D/examples/push_box/src/**.cpp")
    add_deps("extra2d")
    set_targetdir("$(builddir)/switch")
    
    -- 链接 EGL、OpenGL ES 3.0 和 SDL2 音频库
    -- 注意：链接顺序很重要！被依赖的库必须放在后面
    -- 依赖链：SDL2 -> EGL -> drm_nouveau
    --          GLESv2 -> glapi -> drm_nouveau
    add_syslinks("SDL2_mixer", "SDL2",
                 "opusfile", "opus", "vorbisidec", "ogg",
                 "modplug", "mpg123", "FLAC",
                 "GLESv2",
                 "EGL",
                 "glapi",
                 "drm_nouveau")

    local appTitle = "Extra2D Switch Audio Demo"
    local appAuthor = "Extra2D Switch Audio Demo"
    local appVersion = "1.0.0"
    
    after_build(function (target)
        -- 强制使用 Windows 路径
        local devkitPro = "C:/devkitPro"
        local elf_file = target:targetfile()
        local output_dir = path.directory(elf_file)
        local nacp_file = path.join(output_dir, "switch_audio_demo.nacp")
        local nro_file = path.join(output_dir, "switch_audio_demo.nro")
        local romfs_dir = "Extra2D/examples/push_box/src/romfs"
        local nacptool = path.join(devkitPro, "tools/bin/nacptool.exe")
        local elf2nro = path.join(devkitPro, "tools/bin/elf2nro.exe")

        if not os.isfile(nacptool) then
            print("Warning: nacptool not found at " .. nacptool)
            return
        end
        if not os.isfile(elf2nro) then
            print("Warning: elf2nro not found at " .. elf2nro)
            return
        end

        -- 生成 .nacp 文件
        os.vrunv(nacptool, {"--create", appTitle, appAuthor, appVersion, nacp_file})
        print("Built " .. path.filename(nacp_file))

        -- 生成 .nro 文件（包含 RomFS）
        local romfs_absolute = path.absolute(romfs_dir)
        if os.isdir(romfs_absolute) then
            print("Packing RomFS from: " .. romfs_absolute)
            os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file, "--romfsdir=" .. romfs_absolute})
            print("Built " .. path.filename(nro_file) .. " (with RomFS)")
        else
            os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file})
            print("Built " .. path.filename(nro_file))
        end
    end)
target_end()

-- ============================================
-- Switch 简单测试程序
-- ============================================
target("switch_simple_test")
    set_kind("binary")
    set_targetdir("build/switch")
    
    -- 应用信息
    local appTitle = "Extra2D Simple Test"
    local appAuthor = "Extra2D Team"
    local appVersion = "1.0.0"
    
    -- 添加源文件
    add_files("Extra2D/examples/switch_simple_test/main.cpp")
    
    -- 添加头文件路径
    add_includedirs("Extra2D/include")
    
    -- 链接 extra2d 库
    add_deps("extra2d")
    

    -- 构建后生成 .nro 文件（包含 RomFS）
    after_build(function (target)
        local devkitPro = "C:/devkitPro"
        local elf_file = target:targetfile()
        local output_dir = path.directory(elf_file)
        local nacp_file = path.join(output_dir, "switch_simple_test.nacp")
        local nro_file = path.join(output_dir, "switch_simple_test.nro")
        local romfs_dir = "Extra2D/examples/switch_simple_test/romfs"
        local nacptool = path.join(devkitPro, "tools/bin/nacptool.exe")
        local elf2nro = path.join(devkitPro, "tools/bin/elf2nro.exe")
        
        if not os.isfile(nacptool) then
            print("Warning: nacptool not found at " .. nacptool)
            return
        end
        if not os.isfile(elf2nro) then
            print("Warning: elf2nro not found at " .. elf2nro)
            return
        end
        
        -- 生成 .nacp 文件
        os.vrunv(nacptool, {"--create", appTitle, appAuthor, appVersion, nacp_file})
        print("Built " .. path.filename(nacp_file))
        
        -- 生成 .nro 文件（包含 RomFS）
        local romfs_absolute = path.absolute(romfs_dir)
        if os.isdir(romfs_absolute) then
            print("Packing RomFS from: " .. romfs_absolute)
            os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file, "--romfsdir=" .. romfs_absolute})
            print("Built " .. path.filename(nro_file) .. " (with RomFS)")
        else
            os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file})
            print("Built " .. path.filename(nro_file))
        end
    end)
target_end()

