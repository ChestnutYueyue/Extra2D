-- ==============================================
-- Extra2D 示例程序构建目标
-- 支持平台: Nintendo Switch, Windows, Linux, macOS
-- ==============================================

-- 获取 devkitPro 路径
local function get_devkitpro_path()
    return "C:/devkitPro"
end

-- 生成 Switch NRO 文件的通用后构建函数
-- @param target_name 目标名称
-- @param app_title 应用标题
-- @param app_author 应用作者
-- @param app_version 应用版本
-- @param romfs_dir RomFS 目录路径（相对于项目根目录）
local function generate_nro_after_build(target_name, app_title, app_author, app_version, romfs_dir)
    after_build(function (target)
        local devkitPro = get_devkitpro_path()
        local elf_file = target:targetfile()
        local output_dir = path.directory(elf_file)
        local nacp_file = path.join(output_dir, target_name .. ".nacp")
        local nro_file = path.join(output_dir, target_name .. ".nro")
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
        os.vrunv(nacptool, {"--create", app_title, app_author, app_version, nacp_file})
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
end

-- 定义 Switch 示例程序的通用配置
-- @param name 目标名称
-- @param options 配置选项表
local function define_switch_example_target(name, options)
    target(name)
        set_kind("binary")
        set_plat("switch")
        set_arch("arm64")
        set_toolchains("switch")
        set_targetdir("build/switch")

        -- 添加源文件
        add_files(options.source_file or ("Extra2D/examples/" .. name .. "/main.cpp"))

        -- 添加头文件路径
        add_includedirs("Extra2D/include")

        -- 链接 extra2d 库
        add_deps("extra2d")

        -- Windows 控制台应用程序（仅 PC 平台）
        if is_plat("windows") then
            add_ldflags("-mconsole", {force = true})
        end
        
        -- 可选：添加链接器标志
        if options.ldflags then
            add_ldflags(options.ldflags, {force = true})
        end

        -- 构建后生成 .nro 文件
        generate_nro_after_build(
            name,
            options.app_title or ("Extra2D " .. name),
            options.app_author or "Extra2D Team",
            options.app_version or "1.0.0",
            options.romfs_dir or ("Extra2D/examples/" .. name .. "/romfs")
        )
    target_end()
end

-- 定义 PC 示例程序的通用配置
-- @param name 目标名称
-- @param options 配置选项表
local function define_pc_example_target(name, options)
    target(name)
        set_kind("binary")
        set_toolchains("pc")
        
        -- 设置输出目录
        if is_host("windows") then
            set_targetdir("build/windows")
        elseif is_host("linux") then
            set_targetdir("build/linux")
        elseif is_host("macosx") then
            set_targetdir("build/macos")
        else
            set_targetdir("build/pc")
        end

        -- 添加源文件
        add_files(options.source_file or ("Extra2D/examples/" .. name .. "/main.cpp"))

        -- 添加头文件路径
        add_includedirs("Extra2D/include")

        -- 链接 extra2d 库
        add_deps("extra2d")

        -- 可选：添加链接器标志
        if options.ldflags then
            add_ldflags(options.ldflags, {force = true})
        end
        
        -- PC 端构建后复制资源文件和 DLL
        after_build(function (target)
            local target_file = target:targetfile()
            local output_dir = path.directory(target_file)
            local romfs_dir = options.romfs_dir or ("Extra2D/examples/" .. name .. "/romfs")
            local romfs_absolute = path.absolute(romfs_dir)
            
            -- 复制 vcpkg DLL 到输出目录
            local vcpkg_root = os.getenv("VCPKG_ROOT")
            if vcpkg_root then
                local triplet = is_arch("x64") and "x64-windows" or "x86-windows"
                local vcpkg_bin = path.join(vcpkg_root, "installed", triplet, "bin")
                if os.isdir(vcpkg_bin) then
                    -- 复制 SDL2 相关 DLL
                    local dlls = {"SDL2.dll", "SDL2_mixer.dll", "ogg.dll", "vorbis.dll", "vorbisfile.dll", "wavpackdll.dll"}
                    for _, dll in ipairs(dlls) do
                        local dll_path = path.join(vcpkg_bin, dll)
                        if os.isfile(dll_path) then
                            os.cp(dll_path, output_dir)
                        end
                    end
                    print("Copied DLLs from: " .. vcpkg_bin)
                end
            end
            
            -- 复制资源文件到输出目录
            if os.isdir(romfs_absolute) then
                local assets_dir = path.join(output_dir, "assets")
                os.mkdir(assets_dir)
                
                -- 复制 romfs 内容到 assets 目录
                local romfs_assets = path.join(romfs_absolute, "assets")
                if os.isdir(romfs_assets) then
                    print("Copying assets from: " .. romfs_assets .. " to " .. assets_dir)
                    os.cp(romfs_assets .. "/*", assets_dir)
                end
                
                print("Built " .. path.filename(target_file) .. " (with assets)")
            else
                print("Built " .. path.filename(target_file))
            end
        end)
    target_end()
end

-- 定义所有示例程序目标
function define_example_targets()
    -- 根据平台选择示例程序定义方式
    if is_plat("switch") then
        -- ============================================
        -- Switch 示例程序
        -- ============================================
        define_switch_example_target("hello_world", {
            app_title = "Extra2D hello_world",
            app_author = "Extra2D hello_world",
            app_version = "1.0.0"
        })

        define_switch_example_target("spatial_index_demo", {
            app_title = "Extra2D Spatial Index Demo",
            app_version = "1.0.0",
            ldflags = "-Wl,-Map=build/switch/spatial_index_demo.map"
        })

        define_switch_example_target("collision_demo", {
            app_title = "Extra2D Collision Demo",
            app_version = "1.0.0"
        })
    else
        -- ============================================
        -- PC 示例程序 (Windows/Linux/macOS)
        -- ============================================
        define_pc_example_target("hello_world", {
            romfs_dir = "Extra2D/examples/hello_world/romfs"
        })

        define_pc_example_target("spatial_index_demo", {
            romfs_dir = "Extra2D/examples/spatial_index_demo/romfs"
        })

        define_pc_example_target("collision_demo", {
            romfs_dir = "Extra2D/examples/collision_demo/romfs"
        })
    end
end
