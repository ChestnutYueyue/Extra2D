-- ==============================================
-- PC 平台工具链定义 (Windows/Linux/macOS)
-- ==============================================

function define_pc_toolchain()
    toolchain("pc")
        set_kind("standalone")
        set_description("PC Platform Toolchain (Windows/Linux/macOS)")

        on_load(function (toolchain)
            -- ==============================================
            -- 平台检测与配置
            -- ==============================================
            
            if is_host("windows") then
                -- Windows: 使用 MinGW
                toolchain:set("toolset", "cc", "gcc")
                toolchain:set("toolset", "cxx", "g++")
                toolchain:set("toolset", "ld", "g++")
                toolchain:set("toolset", "ar", "ar")
            else
                -- Linux/macOS: 使用 GCC/Clang
                toolchain:set("toolset", "cc", "gcc")
                toolchain:set("toolset", "cxx", "g++")
                toolchain:set("toolset", "ld", "g++")
                toolchain:set("toolset", "ar", "ar")
            end

            -- ==============================================
            -- PC 平台宏定义
            -- ==============================================
            toolchain:add("defines", "__PC__")
            
            if is_host("windows") then
                toolchain:add("defines", "_WIN32", "NOMINMAX", "WIN32_LEAN_AND_MEAN")
            elseif is_host("linux") then
                toolchain:add("defines", "__linux__")
            elseif is_host("macosx") then
                toolchain:add("defines", "__APPLE__", "__MACH__")
            end

            -- SimpleIni 配置
            toolchain:add("defines", "SI_NO_CONVERSION")

            -- ==============================================
            -- OpenGL 配置
            -- ==============================================
            
            if is_host("windows") then
                -- Windows: 使用标准 OpenGL
                toolchain:add("links", "opengl32")
            else
                -- Linux/macOS: 使用 Mesa OpenGL
                toolchain:add("links", "GL")
            end

            -- ==============================================
            -- vcpkg 依赖配置
            -- ==============================================
            
            -- 获取 vcpkg 路径
            local vcpkg_root = os.getenv("VCPKG_ROOT")
            if vcpkg_root then
                local triplet = is_arch("x64") and "x64-windows" or "x86-windows"
                local vcpkg_installed = path.join(vcpkg_root, "installed", triplet)
                if os.isdir(vcpkg_installed) then
                    -- 添加头文件路径
                    toolchain:add("includedirs", path.join(vcpkg_installed, "include"))
                    toolchain:add("includedirs", path.join(vcpkg_installed, "include", "SDL2"))
                    -- 添加库路径
                    toolchain:add("linkdirs", path.join(vcpkg_installed, "lib"))
                    print("vcpkg packages: " .. vcpkg_installed)
                end
            end
            
            -- ==============================================
            -- 链接库
            -- ==============================================
            
            -- SDL2 及其扩展
            toolchain:add("links", "SDL2_mixer", "SDL2")
            
            -- 音频编解码库 (vcpkg 中可用的)
            toolchain:add("links", "ogg")
            
            -- OpenGL (Windows 使用标准 OpenGL)
            toolchain:add("links", "opengl32")
            
            -- 系统库
            if is_host("windows") then
                toolchain:add("syslinks", "gdi32", "user32", "shell32", "winmm", "imm32", "ole32", "oleaut32", "version", "uuid", "advapi32", "setupapi")
            else
                toolchain:add("syslinks", "m", "dl", "pthread")
                if is_host("linux") then
                    toolchain:add("syslinks", "X11", "Xext")
                end
            end

            -- ==============================================
            -- 编译器标志 (MinGW GCC)
            -- ==============================================
            toolchain:add("cxflags", "-Wall", "-Wextra", {force = true})
            toolchain:add("cxflags", "-Wno-unused-parameter", "-Wno-unused-variable", {force = true})
            
            -- Windows 控制台应用程序
            toolchain:add("ldflags", "-mconsole", {force = true})
            
            if is_mode("debug") then
                toolchain:add("defines", "E2D_DEBUG", "_DEBUG")
                toolchain:add("cxflags", "-O0", "-g", {force = true})
            else
                toolchain:add("defines", "NDEBUG")
                toolchain:add("cxflags", "-O2", {force = true})
            end
        end)
end

-- 获取 PC 平台包含路径
function get_pc_includedirs()
    local dirs = {}
    
    -- Angle
    local angle_dir = os.getenv("ANGLE_DIR") or "third_party/angle"
    if os.isdir(angle_dir) then
        table.insert(dirs, path.join(angle_dir, "include"))
    end
    
    return dirs
end

-- 获取 PC 平台库路径
function get_pc_linkdirs()
    local dirs = {}
    
    -- Angle
    local angle_dir = os.getenv("ANGLE_DIR") or "third_party/angle"
    if os.isdir(angle_dir) then
        table.insert(dirs, path.join(angle_dir, "lib"))
    end
    
    return dirs
end

-- 获取 PC 平台系统链接库
function get_pc_syslinks()
    return {
        "SDL2_mixer", "SDL2",
        "opengl32"
    }
end
