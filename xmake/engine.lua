-- ==============================================
-- Extra2D 引擎库共享配置
-- 被主项目和示例共享使用
-- 
-- 窗口后端统一使用 SDL2，支持以下平台：
-- - Windows (MinGW)
-- - Linux
-- - macOS
-- - Nintendo Switch
-- ==============================================

-- 获取当前平台
local function get_current_plat()
    return get_config("plat") or os.host()
end

-- 定义 Extra2D 引擎库目标
function define_extra2d_engine()
    target("extra2d")
        set_kind("static")

        -- 引擎核心源文件
        add_files("Extra2D/src/**.cpp")
        add_files("Extra2D/src/glad/glad.c")

        -- SDL2 后端源文件（所有平台统一使用）
        add_files("Extra2D/src/platform/backends/sdl2/*.cpp")
        add_defines("E2D_BACKEND_SDL2")

        -- 头文件路径
        add_includedirs("Extra2D/include", {public = true})
        add_includedirs("Extra2D/include/extra2d/platform", {public = true})

        -- 平台配置
        local plat = get_current_plat()
        
        if plat == "switch" then
            -- Nintendo Switch 平台配置
            local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
            add_includedirs(devkitPro .. "/portlibs/switch/include", {public = true})
            add_linkdirs(devkitPro .. "/portlibs/switch/lib")
            add_syslinks("SDL2", "GLESv2", "EGL", "glapi", "drm_nouveau", "nx", "m",
                         {public = true})
        elseif plat == "mingw" or plat == "windows" then
            -- Windows (MinGW) 平台配置
            add_packages("glm", "nlohmann_json", {public = true})
            add_packages("libsdl2", {public = true})
            add_syslinks("opengl32", "glu32", "winmm", "imm32", "version", "setupapi", 
                         {public = true})
        elseif plat == "linux" then
            -- Linux 平台配置
            add_packages("glm", "nlohmann_json", {public = true})
            add_packages("libsdl2", {public = true})
            add_syslinks("GL", "dl", "pthread", {public = true})
        elseif plat == "macosx" then
            -- macOS 平台配置
            add_packages("glm", "nlohmann_json", {public = true})
            add_packages("libsdl2", {public = true})
            add_frameworks("OpenGL", "Cocoa", "IOKit", "CoreVideo", {public = true})
        end

        -- 编译器标志 (C 和 C++ 共用)
        add_cxflags("-Wall", "-Wextra", {force = true})
        add_cxflags("-Wno-unused-variable", "-Wno-unused-function", "-Wno-unused-parameter", {force = true})
        add_cxflags("-Wno-strict-aliasing", "-Wno-implicit-fallthrough", {force = true})
        add_cxflags("-Wno-missing-field-initializers", {force = true})

        -- C++ 专用编译器标志
        add_cxxflags("-Wno-deprecated-copy", "-Wno-class-memaccess", {force = true})

        if is_mode("debug") then
            add_defines("E2D_DEBUG", "_DEBUG", {public = true})
            add_cxxflags("-O0", "-g", {force = true})
        else
            add_defines("NDEBUG", {public = true})
            add_cxxflags("-O2", {force = true})
        end
    target_end()
end
