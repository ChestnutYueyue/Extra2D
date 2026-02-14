-- ==============================================
-- Extra2D 引擎库共享配置
-- 被主项目和示例共享使用
-- ==============================================

-- 获取当前平台
local function get_current_plat()
    return get_config("plat") or os.host()
end

-- 获取后端配置
local function get_backend()
    return get_config("backend") or "sdl2"
end

-- 定义 Extra2D 引擎库目标
function define_extra2d_engine()
    target("extra2d")
        set_kind("static")

        -- 引擎核心源文件
        add_files("Extra2D/src/**.cpp")
        add_files("Extra2D/src/glad/glad.c")

        -- 平台后端源文件
        local plat = get_current_plat()
        local backend = get_backend()
        
        if plat == "switch" then
            add_files("Extra2D/src/platform/backends/switch/*.cpp")
            add_defines("E2D_BACKEND_SWITCH")
        elseif backend == "sdl2" then
            add_files("Extra2D/src/platform/backends/sdl2/*.cpp")
            add_defines("E2D_BACKEND_SDL2")
        elseif backend == "glfw" then
            add_files("Extra2D/src/platform/backends/glfw/*.cpp")
            add_defines("E2D_BACKEND_GLFW")
        end

        -- 头文件路径
        add_includedirs("Extra2D/include", {public = true})
        add_includedirs("Extra2D/include/extra2d/platform", {public = true})

        -- 平台配置
        if plat == "switch" then
            local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
            add_includedirs(devkitPro .. "/portlibs/switch/include", {public = true})
            add_linkdirs(devkitPro .. "/portlibs/switch/lib")
            add_syslinks("SDL2", "GLESv2", "EGL", "glapi", "drm_nouveau",
                         {public = true})
        elseif plat == "mingw" then
            add_packages("glm", "nlohmann_json", {public = true})
            
            if backend == "sdl2" then
                add_packages("libsdl2", {public = true})
            elseif backend == "glfw" then
                add_packages("glfw", {public = true})
            end
            
            add_syslinks("opengl32", "glu32", "winmm", "imm32", "version", "setupapi", {public = true})
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
