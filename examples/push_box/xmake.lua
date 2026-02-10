-- ==============================================
-- Push Box 示例 - Xmake 构建脚本
-- 支持平台: MinGW (Windows), Nintendo Switch
-- ==============================================

-- 获取当前平台
local host_plat = os.host()
local target_plat = get_config("plat") or host_plat

-- 可执行文件目标
target("push_box")
    set_kind("binary")
    add_files("*.cpp")
    add_includedirs("../../Extra2D/include")
    add_deps("extra2d")

    if target_plat == "switch" then
        set_plat("switch")
        set_arch("arm64")
        set_toolchains("switch")
        set_targetdir("build/switch")

        after_build(function (target)
            local devkitPro = os.getenv("DEVKITPRO") or "C:/devkitPro"
            local elf_file = target:targetfile()
            local output_dir = path.directory(elf_file)
            local nacp_file = path.join(output_dir, "push_box.nacp")
            local nro_file = path.join(output_dir, "push_box.nro")
            local nacptool = path.join(devkitPro, "tools/bin/nacptool.exe")
            local elf2nro = path.join(devkitPro, "tools/bin/elf2nro.exe")

            if os.isfile(nacptool) and os.isfile(elf2nro) then
                os.vrunv(nacptool, {"--create", "Push Box", "Extra2D Team", "1.0.0", nacp_file})
                local romfs = path.absolute("romfs")
                if os.isdir(romfs) then
                    os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file, "--romfsdir=" .. romfs})
                else
                    os.vrunv(elf2nro, {elf_file, nro_file, "--nacp=" .. nacp_file})
                end
            end
        end)

    elseif target_plat == "mingw" then
        set_plat("mingw")
        set_arch("x86_64")
        set_targetdir("build/mingw")
        add_ldflags("-mwindows", {force = true})

        after_build(function (target)
            local romfs = path.absolute("romfs")
            if os.isdir(romfs) then
                local target_dir = path.directory(target:targetfile())
                local assets_dir = path.join(target_dir, "assets")
                if not os.isdir(assets_dir) then
                    os.mkdir(assets_dir)
                end
                os.cp(path.join(romfs, "assets/*"), assets_dir)
            end
        end)
    end
target_end()
