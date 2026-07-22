set_project("sorted_array_tower")
set_version("1.0.0")
set_xmakever("2.8.7")

add_moduledirs("scripts/xmake")

set_languages("cxx20")

add_rules(
    "mode.debug",
    "mode.release",
    "mode.releasedbg",
    "mode.minsizerel",
    "mode.coverage",
    "mode.profile",
    "mode.check",
    "mode.valgrind"
)

option("use_modules")
    set_default(true)
    set_showmenu(true)
    set_description("Enable C++20 module support")
option_end()

option("pic")
    set_default(true)
    set_showmenu(true)
    set_description("Enable Position Independent Code (-fPIC)")
option_end()

if has_config("pic") then
    add_cxflags("-fPIC")
end

option("has_linux_version_h")
    add_cincludes("linux/version.h")
option_end()

-- Generate compile_commands.json for tools like LSP
add_rules("plugin.compile_commands.autoupdate", {
    outputdir = "build",
    lsp = "clangd"
})

-- Dependency declarations
-- =======================

-- Test library: doctest
-- We need to set `cmake = false` to avoid invoking CMake when it's actually
-- not needed. doctest can work without CMake.
add_requires("doctest >=2.5.2 <3.0.0", {configs = {cmake = false}})

-- Benchmark library: nanobench
add_requires("nanobench >=4.3.11 <5.0.0", {
    configs = {
        cxflags =
            not has_config("has_linux_version_h") and
            "-DANKERL_NANOBENCH_DISABLE_PERF_COUNTERS=1" or
            nil
    }
})

-- Target declarations
-- ===================

target("skip_array")
    if has_config("use_modules") then
        set_kind("static")
        -- .cppm interface files must be made public.
        add_files("modules/skip_array.cppm", { public = true })
    else
        set_kind("headeronly")
    end
    add_headerfiles("include/(sorted_array_tower/skip_array.hpp)")
    add_includedirs("include", { public = true })

target("bounded_array")
    if has_config("use_modules") then
        set_kind("static")
        -- .cppm interface files must be made public.
        add_files("modules/bounded_array.cppm", { public = true })
    else
        set_kind("headeronly")
    end
    add_headerfiles("include/(sorted_array_tower/bounded_array.hpp)")
    add_includedirs("include", { public = true })

target("bounded_vector")
    if has_config("use_modules") then
        set_kind("static")
        -- .cppm interface files must be made public.
        add_files("modules/bounded_vector.cppm", { public = true })
    else
        set_kind("headeronly")
    end
    add_headerfiles("include/(sorted_array_tower/bounded_vector.hpp)")
    add_includedirs("include", { public = true })

target("sorted_array_tower")
    if has_config("use_modules") then
        set_kind("static")
        -- .cppm interface files must be made public.
        add_files("modules/sorted_array_tower.cppm", { public = true })
    else
        set_kind("headeronly")
    end
    add_deps(
        "skip_array",
        "bounded_array",
        "bounded_vector"
    )
    add_headerfiles("include/(sorted_array_tower/sorted_array_tower.hpp)")
    add_includedirs("include", { public = true })

target("tests")
    set_kind("binary")
    add_packages("doctest")
    add_files(
        "tests/test_main.cpp",
        "tests/test_skip_array.cpp",
        "tests/test_bounded_array.cpp",
        "tests/test_bounded_vector.cpp"
    )
    add_deps("sorted_array_tower")
    if has_config("use_modules") then
        add_defines("SORTED_ARRAY_TOWER_USE_MODULES=1")
    end

    -- Uses the project root as the working directory
    set_rundir("$(projectdir)")

    add_tests("default", { realtime_output = true })

target("benchmarks")
    set_kind("binary")
    add_packages("nanobench")
    add_files(
        "benchmarks/benchmarks_main.cpp",
        "benchmarks/benchmark_bounded_array.cpp"
    )
    add_headerfiles(
        "benchmarks/benchmarks.hpp"
    )
    add_deps("sorted_array_tower")
    if has_config("use_modules") then
        add_defines("SORTED_ARRAY_TOWER_USE_MODULES=1")
    end
    if is_plat("linux") and not has_config("has_linux_version_h") then
        add_defines("ANKERL_NANOBENCH_DISABLE_PERF_COUNTERS=1")
    end

-- Task declarations
-- =================

task("test-report")
    set_menu({
        usage = "xmake test-report",
        description =
            "Run tests and generate test reports\n" ..
            "- JUnit format: build/test-report.xml\n" ..
            "- Text summary: build/test-report.txt"
        ,
    })
    on_run(function ()
        import("core.project.config")
        config.load()

        os.exec("xmake -y")

        local output_dir = config.builddir()
        local junit_results_path = path.join(output_dir, "test-report.xml")
        os.rm(junit_results_path)
        local test_status = os.execv(
            "xmake",
            {
                "run",
                "tests",
                "--reporters=junit",
                "--out=" .. junit_results_path,
            },
            {
                try = true,
                stdout = os.nuldev(),
                stderr = os.nuldev(),
            }
        )

        import("doctest_helpers")
        local text_results_path = path.join(output_dir, "test-report.txt")
        local stats = doctest_helpers.convert_junit_to_text(
            junit_results_path,
            text_results_path
        )

        io.write(io.readfile(text_results_path) .. "\n")

        if stats.num_failures ~= 0 then
            raise(stats.num_failures .. " test cases failed")
        end
    end)

task("coverage-report")
    set_menu({
        usage = "xmake coverage-report",
        description = "Run tests and generate an HTML coverage report"
    })
    on_run(function ()
        import("lib.detect.find_tool")
        if not find_tool("gcovr") then
            raise("Error: 'gcovr' not found. Please install it first.")
        end

        import("core.project.config")
        config.load()
        if config.get("mode") ~= "coverage" then
            raise("Error: This task requires coverage mode." ..
                " Please run:\n" ..
                "    xmake f -m coverage\n" ..
                "    xmake coverage-report")
        end

        os.exec("xmake test")

        local output_dir = path.join(config.builddir(), "coverage_report")
        local output_file = path.join(output_dir, "index.html")
        os.rm(output_dir)
        os.mkdir(output_dir)

        local gcov_executable = "gcov"
        local toolchain = config.get("toolchain") or ""
        if toolchain:find("clang") then
            gcov_executable = "llvm-cov gcov"
        end
        print("Using gcov executable: %s", gcov_executable)

        print("Generating coverage report at: %s", output_file)
        os.execv("gcovr", {
            "--root", ".",
            "--filter", "src/",
            "--filter", "include/",
            "--filter", "modules/",
            "--gcov-executable", gcov_executable,
            "--txt", path.join(output_dir, "report.txt"),
            "--html-details", path.join(output_dir, "index.html"),
            "--cobertura", path.join(output_dir, "cobertura.xml"),
            "--markdown", path.join(output_dir, "summary.md"),
            "--txt-summary"
        }, {
            stdout = path.join(output_dir, "summary.txt")
        })
        print("Coverage report generated. Open %s to view.", output_file)
    end)

task("clean-all")
    set_menu({
        usage = "xmake clean-all",
        description = "Clean all build artifacts and configuration"
    })
    on_run(function()
        os.exec("rm -rf build .xmake")
    end)

task("reports")
    set_menu({
        usage = "xmake reports",
        description = "Run tests and generate test and coverage reports"
    })
    on_run(function()
        -- Configure with coverage mode
        os.execv("xmake", {
            "config",
            "--mode=coverage",
            "--policies=build.sanitizer.address,build.sanitizer.undefined",
            "--cxflags=-fno-sanitize-recover=all",
            "--use_modules=n",
            "-y",
        })
        -- Note: We set use_modules=n here as a workaround for a bug in Xmake
        -- as build.sanitizer.undefined does not work with GCC 15 when
        -- use_modules=y, but this same configuration works fine with CMake.
        -- If you use a newer version of GCC or a newer version Xmake, you
        -- should try to change use_modules=y and see if it works or not.

        try
        {
            function()
                os.exec("xmake test-report")
                os.exec("xmake coverage-report")
            end,
            catch
            {
                function()
                    raise("coverage report not generated")
                end,
            },
        }
    end)

task("check-builds")
    set_menu({
        usage = "xmake check-builds",
        description = "Build in coverage mode with compiler and module configurations"
    })
    on_run(function()
        local configurations = {
            { toolchain = "gcc", use_modules = "y" },
            { toolchain = "gcc", use_modules = "n" },
            { toolchain = "clang", use_modules = "y" },
            { toolchain = "clang", use_modules = "n" },
        }

        for _, configuration in ipairs(configurations) do
            os.exec("xmake clean -y")

            -- Note: We disable build.sanitizer.undefined as a workaround as
            -- it does not work with GCC 15 when use_modules=y. However, this
            -- same configuration works fine with CMake, so this is likely a
            -- bug in Xmake. If you use a newer version of GCC or a newer
            -- version Xmake, you should try to enable
            -- build.sanitizer.undefined for all configurations.
            local policies = "build.sanitizer.address"
            if configuration.use_modules ~= "y" or
                configuration.toolchain ~= "gcc" then
                policies = policies .. ",build.sanitizer.undefined"
            end

            local label = string.format(
                "toolchain=%s, use_modules=%s, policies=%s",
                configuration.toolchain,
                configuration.use_modules,
                policies
            )

            print("Checking build: %s", label)

            os.execv("xmake", {
                "config",
                "--mode=debug",
                "--toolchain=" .. configuration.toolchain,
                "--use_modules=" .. configuration.use_modules,
                "--policies=" .. policies,
                "--cxflags=-fno-sanitize-recover=all",
                "-y",
            })

            os.exec("xmake test-report")

            print("Build passed: %s", label)
        end
    end)

