-- ==============================================
-- Extra2D 引擎库共享配置
-- 被主项目和示例共享使用
-- ==============================================

-- 获取当前平台
local function get_current_plat()
    return get_config("plat") or os.host()
end

-- 定义 Extra2D 引擎库目标
function define_extra2d_engine()
    target("extra2d")
        set_kind("static")

        -- 引擎源文件
        add_files("Extra2D/src/**.cpp")
        add_files("Extra2D/src/glad/glad.c")
        add_files("squirrel/squirrel/*.cpp")
        add_files("squirrel/sqstdlib/*.cpp")

        -- 头文件路径
        add_includedirs("Extra2D/include", {public = true})
        add_includedirs("squirrel/include", {public = true})
        add_includedirs("Extra2D/include/extra2d/platform", {public = true})

        -- 平台配置
        local plat = get_current_plat()
        if plat == "switch" then
            local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
            add_includedirs(devkitPro .. "/portlibs/switch/include", {public = true})
            add_linkdirs(devkitPro .. "/portlibs/switch/lib")
            add_syslinks("SDL2_mixer", "SDL2", "opusfile", "opus", "vorbisidec", "ogg",
                         "modplug", "mpg123", "FLAC", "GLESv2", "EGL", "glapi", "drm_nouveau",
                         {public = true})
        elseif plat == "mingw" then
            add_packages("glm", "libsdl2", "libsdl2_mixer", {public = true})
            add_syslinks("opengl32", "glu32", "winmm", "imm32", "version", "setupapi", {public = true})
        end

        -- 编译器标志
        add_cxflags("-Wall", "-Wextra", {force = true})
        add_cxflags("-Wno-unused-variable", "-Wno-unused-function", "-Wno-unused-parameter", {force = true})
        add_cxflags("-Wno-deprecated-copy", "-Wno-strict-aliasing", "-Wno-implicit-fallthrough", "-Wno-class-memaccess", {force = true})

        if is_mode("debug") then
            add_defines("E2D_DEBUG", "_DEBUG", {public = true})
            add_cxxflags("-O0", "-g", {force = true})
        else
            add_defines("NDEBUG", {public = true})
            add_cxxflags("-O2", {force = true})
        end
    target_end()
end
