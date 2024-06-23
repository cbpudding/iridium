-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_requires("allegro", "cglm", "luajit", "physfs", "stb")
add_rules("mode.debug", "mode.release")

-- This only exists because ISO/IEC 9899:2024 has not been standardized yet. ~ahill
rule("xxd")
    on_build_file(function(target, source, opt)
        import("utils.progress")
        os.mkdir(target:dependir())
        local output = path.join(target:dependir(), path.basename(source) .. path.extension(source) .. ".h")
        os.vrunv("xxd", {"-i", source, output})
        progress.show(opt.progress, "${color.build.object}xxd %s", source)
    end)

target("iridium")
    set_kind("binary")
    set_languages("clatest")
    set_warnings("everything")
    add_packages("allegro", "cglm", "luajit", "physfs", "stb")
    if is_plat("windows") or is_plat("mingw") then
        add_syslinks("opengl32")
    elseif is_plat("linux") then
        add_syslinks("GL")
    end
    add_rules("xxd")
    if is_mode("debug") then
        add_includedirs("$(buildir)/.deps/iridium/linux/x86_64/debug")
    else
        add_includedirs("$(buildir)/.deps/iridium/linux/x86_64/release")
    end
    add_files("src/**.glsl", {rule = "xxd"})
    add_files("src/**.lua", {rule = "xxd"})
    add_files("src/**.c")
