-- ==============================================
-- Extra2D 引擎库构建目标
-- 支持平台: Nintendo Switch, Windows, Linux, macOS
-- ==============================================

-- 核心路径定义
local SRC_DIR = "Extra2D/src"
local INC_DIR = "Extra2D/include"

-- 定义 Extra2D 引擎库目标
function define_extra2d_target()
    target("extra2d")
        set_kind("static")
        set_basename(is_mode("debug") and "libeasy2dd" or "libeasy2d")

        -- ==============================================
        -- 源文件配置
        -- ==============================================
        
        -- 引擎源文件
        add_files(path.join(SRC_DIR, "**.cpp"))
        add_files(path.join(SRC_DIR, "glad/glad.c"))

        -- Squirrel 3.2 源文件
        add_files("squirrel/squirrel/*.cpp")
        add_files("squirrel/sqstdlib/*.cpp")

        -- ==============================================
        -- 头文件路径配置
        -- ==============================================
        
        -- 公开头文件目录
        add_includedirs(INC_DIR, {public = true})

        -- 第三方头文件目录
        add_includedirs("squirrel/include", {public = true})

        -- 平台兼容性头文件路径
        add_includedirs(path.join(INC_DIR, "extra2d/platform"), {public = true})

        -- ==============================================
        -- 平台特定配置
        -- ==============================================
        
        if is_plat("switch") then
            -- ==============================================
            -- Nintendo Switch 平台配置
            -- ==============================================
            set_plat("switch")
            set_arch("arm64")
            set_toolchains("switch")
            
            -- devkitPro mesa 路径（EGL + OpenGL ES）
            local devkitPro = "C:/devkitPro"
            add_includedirs(path.join(devkitPro, "portlibs/switch/include"), {public = true})
            add_linkdirs(path.join(devkitPro, "portlibs/switch/lib"))

            -- 链接 EGL、OpenGL ES 3.0（mesa）和 SDL2 音频
            -- 注意：链接顺序很重要！被依赖的库必须放在后面
            add_syslinks("SDL2_mixer", "SDL2",
                         "opusfile", "opus", "vorbisidec", "ogg",
                         "modplug", "mpg123", "FLAC",
                         "GLESv2",
                         "EGL",
                         "glapi",
                         "drm_nouveau",
                         {public = true})
        else
            -- ==============================================
            -- PC 平台配置 (Windows/Linux/macOS)
            -- ==============================================
            set_toolchains("pc")
            
            -- PC 平台使用标准 OpenGL (通过 MinGW)
            -- 依赖库在 pc.lua 工具链中配置 (使用 vcpkg)
            add_syslinks("SDL2_mixer", "SDL2", "opengl32", {public = true})
        end

        -- ==============================================
        -- 编译器配置
        -- ==============================================
        
        -- ==============================================
        -- 编译器标志 (MinGW GCC)
        -- ==============================================
        add_cxflags("-Wall", "-Wextra", {force = true})
        add_cxflags("-Wno-unused-variable", "-Wno-unused-function", {force = true})
        add_cxflags("-Wno-unused-parameter", {force = true})
        
        -- Squirrel 第三方库警告抑制
        add_cxflags("-Wno-deprecated-copy", "-Wno-strict-aliasing", "-Wno-implicit-fallthrough", "-Wno-class-memaccess", {force = true})
        
        -- 调试/发布模式配置
        if is_mode("debug") then
            add_defines("E2D_DEBUG", "_DEBUG", {public = true})
            add_cxxflags("-O0", "-g", {force = true})
        else
            add_defines("NDEBUG", {public = true})
            add_cxxflags("-O2", {force = true})
        end

        -- ==============================================
        -- 头文件安装配置
        -- ==============================================
        add_headerfiles(path.join(INC_DIR, "extra2d/**.h"), {prefixdir = "extra2d"})
        add_headerfiles(path.join(INC_DIR, "stb/**.h"), {prefixdir = "stb"})
        add_headerfiles(path.join(INC_DIR, "simpleini/**.h"), {prefixdir = "simpleini"})
    target_end()
end
