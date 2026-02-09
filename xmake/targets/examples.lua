-- ==============================================
-- Extra2D 示例程序构建目标
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

-- 定义示例程序的通用配置
-- @param name 目标名称
-- @param options 配置选项表
local function define_example_target(name, options)
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

-- 定义所有示例程序目标
function define_example_targets()
    -- ============================================
    -- Switch 简单测试程序
    -- ============================================
    define_example_target("hello_world", {
        app_title = "Extra2D hello_world",
        app_author = "Extra2D hello_world",
        app_version = "1.0.0"
    })

    -- ============================================
    -- 引擎空间索引演示（1000个节点）
    -- ============================================
    define_example_target("spatial_index_demo", {
        app_title = "Extra2D Spatial Index Demo",
        app_version = "1.0.0",
        ldflags = "-Wl,-Map=build/switch/spatial_index_demo.map"
    })

    -- ============================================
    -- 碰撞检测演示程序
    -- ============================================
    define_example_target("collision_demo", {
        app_title = "Extra2D Collision Demo",
        app_version = "1.0.0"
    })
end
