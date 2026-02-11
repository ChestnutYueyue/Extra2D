-- ==============================================
-- Hello World 示例 - Xmake 构建脚本
-- 支持平台: MinGW (Windows), Nintendo Switch
-- ==============================================

-- 获取当前脚本所在目录（示例根目录）
local example_dir = os.scriptdir()

-- 可执行文件目标
target("hello_world")
    set_kind("binary")
    add_files("main.cpp")
    add_includedirs("../../Extra2D/include")
    add_deps("extra2d")

    -- 使用与主项目相同的平台配置
    if is_plat("switch") then
        set_targetdir("../../build/examples/hello_world")

        -- 构建后生成 NRO 文件
        after_build(function (target)
            local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
            local elf_file = target:targetfile()
            local output_dir = path.directory(elf_file)
            local nacp_file = path.join(output_dir, "hello_world.nacp")
            local nro_file = path.join(output_dir, "hello_world.nro")
            local nacptool = path.join(devkitPro, "tools/bin/nacptool.exe")
            local elf2nro = path.join(devkitPro, "tools/bin/elf2nro.exe")

            if os.isfile(nacptool) and os.isfile(elf2nro) then
                os.vrunv(nacptool, {"--create", "Hello World", "Extra2D Team", "1.0.0", nacp_file})
                local romfs = path.join(example_dir, "romfs")
                if os.isdir(romfs) then
                    os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file, "--romfsdir=" .. romfs})
                else
                    os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file})
                end
                print("Generated NRO: " .. nro_file)
            end
        end)
        
        -- 打包时将 NRO 文件复制到 package 目录
        after_package(function (target)
            local nro_file = path.join(target:targetdir(), "hello_world.nro")
            local package_dir = target:packagedir()
            if os.isfile(nro_file) and package_dir then
                os.cp(nro_file, package_dir)
                print("Copied NRO to package: " .. package_dir)
            end
        end)

    elseif is_plat("mingw") then
        set_targetdir("../../build/examples/hello_world")
        add_ldflags("-mwindows", {force = true})

        -- 复制资源到输出目录
        after_build(function (target)
            local romfs = path.join(example_dir, "romfs")
            if os.isdir(romfs) then
                local target_dir = path.directory(target:targetfile())
                local assets_dir = path.join(target_dir, "assets")
                
                -- 创建 assets 目录
                if not os.isdir(assets_dir) then
                    os.mkdir(assets_dir)
                end
                
                -- 复制所有资源文件（包括子目录）
                os.cp(path.join(romfs, "assets/**"), assets_dir)
                print("Copied assets from " .. romfs .. " to " .. assets_dir)
            else
                print("Warning: romfs directory not found at " .. romfs)
            end
        end)
        
        -- 打包时将资源复制到 package 目录
        after_package(function (target)
            local target_dir = path.directory(target:targetfile())
            local assets_dir = path.join(target_dir, "assets")
            local package_dir = target:packagedir()
            if os.isdir(assets_dir) and package_dir then
                local package_assets = path.join(package_dir, "assets")
                if not os.isdir(package_assets) then
                    os.mkdir(package_assets)
                end
                os.cp(path.join(assets_dir, "**"), package_assets)
                print("Copied assets to package: " .. package_assets)
            end
        end)
    end
target_end()
