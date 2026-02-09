-- ==============================================
-- Extra2D 引擎库构建目标
-- ==============================================

-- 核心路径定义
local SRC_DIR = "Extra2D/src"
local INC_DIR = "Extra2D/include"

-- 定义 Extra2D 引擎库目标
function define_extra2d_target()
    target("extra2d")
        set_kind("static")
        set_plat("switch")
        set_arch("arm64")
        set_toolchains("switch")
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

        -- ==============================================
        -- 编译器配置
        -- ==============================================
        
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

        -- 编译器通用配置
        add_cxxflags("-Wall", "-Wextra", {force = true})
        add_cxxflags("-Wno-unused-parameter", {force = true})
        
        -- 调试/发布模式配置
        if is_mode("debug") then
            add_defines("E2D_DEBUG", "_DEBUG", {public = true})
            add_cxxflags("-O0", "-g", {force = true})
        else
            add_defines("NDEBUG", {public = true})
            add_cxxflags("-O2", {force = true})
        end
    target_end()
end
