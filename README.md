# tyghbn

Repository template for a C++ library package

## Features

This repository contains example code for a C++ library package project
with the following features:

- Two build systems are supported:

  - CMake + Conan
  - Xmake + Xrepo

- The project contains simple code examples for 3 types of libraries:

  - An ordinary (static) library.
  - A header-only library.
  - An umbrella library that re-exports other libraries.

- Code can be built with or without C++ modules.

  - The choice to support C++ modules can be made during the *configuration
    stage*.

- Test code that integrates with [doctest](https://github.com/doctest/doctest)
  is provided.

- Benchmark code that integrates with
  [nanobench](https://nanobench.ankerl.com/) is provided.

- Shorthand scripts for common tasks are provided via
  [`just`](https://just.systems/) (and `xmake`).

  - Scripts to generate test and code coverage reports are provided.
  - Scripts to build and run tests with sanitizers are provided.

- Dockerfiles for testing in the CI environment are provided.

- Code documentation can be generated with Doxygen.

- GitHub Actions workflows are provided.

  - Pull request checks use Dockerfiles in the [ci](ci) subdirectory.
  - Post-merge checks will automatically create a GitHub issue on failure.

## How to use this template

This repository provides a working example of a C++ library package named
`Tyghbn`.
The main steps to use this template properly are:

1. Pick your build system.
2. Rename `Tyghbn`, `tyghbn`, and `TYGHBN` to be your project name.
3. Replace the example code with your own code.

### Build systems

This repository supports 2 build systems:

1. CMake, with Conan as the package manager.
2. Xmake, with Xrepo as the package manager.

You can choose to use one of them or both.
If you do not need both build systems, you should delete files and references
that are specific to the build system you do not need.

#### 1. CMake+Conan specific files and references

- [`.pr`](.pr) subdirectory
- [`cmake.just`](cmake.just)
- [`conanfile.py`](conanfile.py)
- [`CMakeLists.txt`](CMakeLists.txt)
- [`ci/Dockerfile.alpine`](ci/Dockerfile.alpine) and
  [`ci/Dockerfile.ubuntu`](ci/Dockerfile.ubuntu)
- [`.github/workflows/cmake-pull-request.yaml`](.github/workflows/cmake-pull-request.yaml)
  and
  [`.github/workflows/cmake-check-builds.yaml`](.github/workflows/cmake-check-builds.yaml)
- The job named `cmake-check-builds` in
  [`.github/workflows/post-merge.yaml`](.github/workflows/post-merge.yaml).
  - If you are not using CMake, you will need to remove the whole block of
    `cmake-check-builds` as well as its references.
    See all the `TODO:` in comments inside
    [`.github/workflows/post-merge.yaml`](.github/workflows/post-merge.yaml)
    for more information.

Shell command for removing CMake+Conan specific files:

```bash
rm -rf .pr cmake.just conanfile.py CMakeLists.txt \
    ci/Dockerfile.alpine ci/Dockerfile.ubuntu \
    .github/workflows/cmake-pull-request.yaml \
    .github/workflows/cmake-check-builds.yaml
```

Remember to clean up all the `TODO:` in
[`.github/workflows/post-merge.yaml`](.github/workflows/post-merge.yaml)
accordingly.

#### 2. Xmake+Xrepo specific files and references

- [`scripts/xmake`](scripts/xmake) subdirectory
- [`xmake.lua`](xmake.lua)
- [`ci/Dockerfile.alpine-xmake`](ci/Dockerfile.alpine-xmake) and
  [`ci/Dockerfile.ubuntu-xmake`](ci/Dockerfile.ubuntu-xmake)
- [`.github/workflows/xmake-pull-request.yaml`](.github/workflows/xmake-pull-request.yaml)
  and
  [`.github/workflows/xmake-check-builds.yaml`](.github/workflows/xmake-check-builds.yaml)
- The job named `xmake-check-builds` in
  [`.github/workflows/post-merge.yaml`](.github/workflows/post-merge.yaml).
  - If you are not using Xmake, you will need to remove the whole block of
    `xmake-check-builds` as well as its references.
    See comments in
    [`.github/workflows/post-merge.yaml`](.github/workflows/post-merge.yaml)
    for more information.

Shell command for removing Xmake+Xrepo specific files:

```bash
rm -rf scripts/xmake xmake.lua \
    ci/Dockerfile.alpine-xmake ci/Dockerfile.ubuntu-xmake \
    .github/workflows/xmake-pull-request.yaml \
    .github/workflows/xmake-check-builds.yaml
```

Remember to clean up all the `TODO:` in
[`.github/workflows/post-merge.yaml`](.github/workflows/post-merge.yaml)
accordingly.

### Renaming `tyghbn`

This template is for a C++ library package named `Tyghbn`.
This name was chosen to be easy to type and unique enough to find and replace.
There are 3 variants of this name that occur in files in this repository:

- `Tyghbn`: Used in [`conanfile.py`](conanfile.py) as a Python class name and
  in Doxygen comment for the [main page](include/tyghbn/tyghbn.hpp).
- `TYGHBN`: Used in variable names in scripts and C++ macros.
- `tyghbn`: Used in C++ code as a namespace name, module names, and header file
  names.

You should rename these strings with proper variants of your project name in
all files, as well as rename all files and subdirectories with `tyghbn` in
their names.

### Replacing example code with your code

The example code consists of 3 libraries:

- `or_else`: a binary-less (header-only) library.
- `add_one`: a normal library.
- `tyghbn`: an umbrella library that re-exports `or_else` and `add_one`.

The files for these libraries live in 3 subdirectories:

- [`include`](include): Header files that expose the public interface.
- [`src`](src): `.cpp` files that implement non-template entities.
- [`modules`](modules): `.cppm` files that expose C++ modules as the public
  interface.

To support both the header style and the C++ module style, the main code logic
must be independent of C++ modules. That means the main implementation must be
in the traditional C++ style: header files in [`include`](include) and
implementation files in [`src`](src).
The C++ module interfaces, defined in [`modules`](modules), simply export
entities from header files in [`include`](include).

`.cppm` files in [`modules`](modules) define C++ module interfaces from header
files:

- [`modules/or_else.cppm`](modules/or_else.cppm) defines
  `module tyghbn.or_else` that exposes names from
  [`include/tyghbn/or_else.hpp`](include/tyghbn/or_else.hpp).
- [`modules/add_one.cppm`](modules/add_one.cppm) defines
  `module tyghbn.add_one` that exposes names from
  [`include/tyghbn/add_one.hpp`](include/tyghbn/add_one.hpp).
- [`modules/tyghbn.cppm`](modules/tyghbn.cppm) re-exports names from
  `module tyghbn.or_else` and `module tyghbn.add_one` under a new module name:
  `tyghbn`.

Note that [`modules/tyghbn.cppm`](modules/tyghbn.cppm), which is the umbrella
C++ module interface, does not include
[`tyghbn.hpp`](include/tyghbn/tyghbn.hpp). Instead, it `imports` the other 2
`.cppm` files, similar to how [`tyghbn.hpp`](include/tyghbn/tyghbn.hpp)
includes the other 2 `.hpp` files.

#### Test code

Test code lives in [`tests`](tests).
The test library used here is [doctest](https://github.com/doctest/doctest).

You can change the test library by modifying the dependency in
[`conanfile.py`](conanfile.py), [`CMakeLists.txt`](CMakeLists.txt), and/or
[`xmake.lua`](xmake.lua), as well as fixing doctest-specific code.

#### Benchmark code

Benchmark code lives in [`benchmarks`](benchmarks).
The benchmark library used here is
[nanobench](https://nanobench.ankerl.com/).

You can change the benchmark library by modifying the dependency in
[`conanfile.py`](conanfile.py), [`CMakeLists.txt`](CMakeLists.txt), and/or
[`xmake.lua`](xmake.lua), as well as fixing nanobench-specific code.

## Environment setup

You will need to have certain applications installed on your system to build
and run code in this repository.
The set of required applications varies based on your configuration choices.

- **For compilation**
  - [GCC](https://gcc.gnu.org/) version 15 or newer, and/or
    [Clang](https://clang.llvm.org/) version 18 or newer.

- **For CMake + Conan**
  - [CMake](https://cmake.org/) version 4.0.2 or newer.
  - [Conan](https://conan.io/) version 2.0 or newer.
  - [Ninja](https://ninja-build.org/) (version 1.11.0 or newer).
    - Note: [GNU Make](https://www.gnu.org/software/make/) could be used
      instead, but because it doesn't support C++ modules, it is not
      recommended.
  - [`just`](https://github.com/casey/just) version 1.49.0 or newer.
    - `just` is used to provide ergonomic commands for common tasks.
      If you install `just`, you can use [`just` commands](#just-scripts)
      defined in [`justfile`](justfile).
  
- **For Xmake + Xrepo**
  - [Xmake](https://xmake.io/) version 3.0.7 or newer.
    This comes with Xrepo.

- **For Clang's C++ module support**
  - [Extra Clang tools](https://clang.llvm.org/extra/).
    - This provides `clang-scan-deps`, which is needed by `clang` to support
      C++ modules.
    - The version must match `clang`'s version.
    - Note: The necessity of `clang-scan-deps` might be dropped in some future
      versions of Clang.

- **For coverage information**
  - GCC needs `gcov` to produce coverage information. `gcov` is usually
    included with `gcc`, so when you install GCC, you should already have
    `gcov`.
    - The code in [`CMakeLists.txt`](CMakeLists.txt) requires `gcov` to be
      available at the command line.
      If you had to alias some versioned `gcc` executable (for example, from
      `gcc-15` to `gcc`), please make sure that you also have `gcov` available,
      and that it has the same version as `gcc`.
  - [`llvm-cov`](https://llvm.org/docs/CommandGuide/llvm-cov.html) is needed to
    generate coverage information when Clang is the compiler.
    It is usually *not* included with a common Clang installation, but it is
    included in [`llvm`](https://llvm.org/).
    - Similar to the `gcov` situation above, you need to make sure that
      `llvm-cov` is available at the command line, and that its version is the
      same as `clang`.
  - [`gcovr`](https://gcovr.com/en/stable/installation.html) is needed to
    generate coverage reports.

- **For development in a Docker container**
  - [Docker](https://www.docker.com/).
  - [`just`](https://github.com/casey/just) version 1.49.0 or newer.
    - [`justfile`](justfile) provides shorthand commands for some
      Docker-related tasks.
    - You can choose not to install `just` if you do not need those shorthand
      commands.
      You can use `docker` directly with the provided
      [`compose.yaml`](ci/compose.yaml) and `Dockerfile`s in the [`ci`](ci)
      subdirectory.

- **For Doxygen documentation**
  - [Doxygen](https://www.doxygen.nl/) version 1.9.8 or newer.
  - [Graphviz](https://graphviz.org/). This is the default graph visualization
    tool used by Doxygen to generate dependency graphs.
  - [`just`](https://github.com/casey/just) version 1.49.0 or newer.
    - [`justfile`](justfile) provides shorthand commands for some
      Doxygen-related tasks.
    - You can choose not to install `just` if you do not need those shorthand
      commands.
      Using `doxygen` directly is not very complicated.

You can use Dockerfiles in the [ci](ci) subdirectory as guidelines for your
environment setup. Note, however, that not all applications are included in
those Dockerfiles as they are meant for only one build system at a time
(CMake or XMake), and they do not contain Docker and Doxygen.

*Developer's note: This repository is being developed on Ubuntu 24.04.4 and
Penguin Linux (Debian Forky/SID) with the following software:*

- [GCC](https://gcc.gnu.org/) 15
- [Clang](https://clang.llvm.org/) 18
  - [Extra Clang Tools](https://clang.llvm.org/extra/index.html) 18
  - [LLVM](https://llvm.org/) 18
- [Python3](https://www.python.org/) 3.11.2
- [CMake](https://cmake.org/) 4.0.2
- [Conan](https://conan.io/) 2.28.1
- [Xmake](https://xmake.io/) 3.0.7
- [Gcovr](https://gcovr.com/) 8.6
- [Docker](https://www.docker.com/) 29.5.3
- [Just](https://just.systems/) 1.49.0
- [Doxygen](https://www.doxygen.nl/) 1.18.0
  - [Graphviz](https://graphviz.org/) 2.43.0

## Using CMake + Conan

### CMakeLists.txt

[`CMakeLists.txt`](CMakeLists.txt) defines 3 main library targets:

- `add_one`
- `or_else`
- `tyghbn`

Each library
[target](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Key%20Concepts.html#targets)
has one header file in [`include/tyghbn`](include/tyghbn)
and one module file in [`modules`](modules) of the same name.
The header file exposes a traditional interface, while the module file exposes
a C++ module interface. Note that the directory structure of
[`modules`](modules) is simpler than [`include/tyghbn`](include/tyghbn) as it
does not need a nested `tyghbn` inside for disambiguation.

#### Base `-legacy` library targets

For each of the main library targets, we also have a base `-legacy` library.
For the non-C++-module case (when `TYGHBN_USE_MODULES` is not `ON`), this is
exactly the same as the main library target. For the C++-module case, a `.cppm`
file is added on top of the base `-legacy` library.

The type of the CMake library target of `add_one-legacy` is not the
same as `or_else-legacy` because `or_else-legacy` is a header-only library,
but `add_one-legacy` is not. The `if` conditional blocks attempt to unify this
divergence, which is arguably a historical quirk of CMake.

- When you are making a library without `.cpp` files, you can use the CMake
  code in the `TEMPLATE_BLOCK: Library without cpp files` as your guide.

- When you are making a library with `.cpp` files, you can use the CMake code
  in the `TEMPLATE_BLOCK: Library with cpp files` as your guide.

#### Umbrella library target

The base `-legacy` umbrella target is an `INTERFACE` library because it does
not add any code on top of its submodules.
The place you will need to modify is the list of submodules inside
`target_link_libraries(tyghbn-legacy ...)`.

#### Test library

This project uses [`doctest`](https://github.com/doctest/doctest) as the test
library, but you can switch to a different library by modifying the following
things:

- The function `Tyghbn.requirements()` in [conanfile.py](conanfile.py).
- CMake code in `TEMPLATE_BLOCK: Test library initialization` and
  `TEMPLATE_BLOCK: Test library finalization` in
  [CMakeLists.txt](CMakeLists.txt).

#### `TYGHBN_USE_MODULES` macro

To support choosing between compiling for C++ modules or not, a variable named
`TYGHBN_USE_MODULES` in `CMakeLists.txt` can be set during the
[CMake configure stage](#2-cmake-configure-stage), and it will be passed as a
macro named `TYGHBN_USE_MODULES` with value `1` C++ code.

When you call `conan install`, you can specify a Conan option named
`use_modules`, which will translate to `TYGHBN_USE_MODULES` in CMake.
This logic lives in `Tyghbn.generate` in [`conanfile.py`](conanfile.py).

### CMake+Conan development workflow

*TL;DR - use `just detect id` to initialize the build system, and
use `just bd td` to build and run tests.*

The development workflow can be split into the following stages:

1. [Conan install](#1-conan-install-stage)

    Download necessary dependencies and prepare them for CMake.

2. [CMake configure](#2-cmake-configure-stage)

    Configure CMake: define
    [CMake targets](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Key%20Concepts.html#targets).

3. [CMake build](#3-cmake-build-stage)

    Build code, i.e., CMake build targets.

4. [CTest](#4-ctest-stage)

    Run tests.

### 1. Conan install stage

> **Prerequisite**: You need to have a Conan profile that contains your system
> configuration. This is generally done by calling
>
> ```bash
> conan profile detect --force
> ```

This stage prepares the `build` subdirectory for building.
The command to run is

```bash
conan install . --build=missing [...args]
```

`[...args]` specifies choices to be made at this stage, which are:

- Build type: There are 4 build types that CMake recognizes:

  - `Debug`
  - `Release`
  - `RelWithDebInfo`
  - `MinSizeRel`

  You choose the build type by appending `-s build_type=...` to the
  `conan install` command.
  If you omit `-s`, the default is `Release`.

- Compiler: Your default profile contains a default compiler, but you can
  override it.

  - You will actually need to override the C++ standard version in your
    default profile because our code needs C++20, but `conan profile detect`
    usually creates a profile with an older C++ standard.

  - This repository provides the following Conan profiles for compiler choices:
    - [`.pr/gcc`](.pr/gcc): The GCC version is also fixed to >= 15.
    - [`.pr/clang`](.pr/clang): The Clang version is also fixed to >= 18.
  
  You can choose the compiler by appending `-pr .pr/...` to the
  `conan install` command.

- Build generator: Your default generator is dependent on your operating
  system, but you can override it.

  - There are 2 types of *build generators*: single-config, and multi-config.
    The difference between these 2 types does not affect the workflow except
    in [step 2](#2-cmake-configure-stage) when you call
    `cmake --preset conan-...`.

  - This repository provides the following Conan profiles to override your
    default generator with [Ninja](https://ninja-build.org/):
    - [`.pr/ninja`](.pr/ninja): `Ninja`
    - [`.pr/ninja-multi`](.pr/ninja-multi): `Ninja Multi-Config`
  
  You can choose the generator by appending `-pr .pr/...` to the
  `conan install` command.

- C++ module support: This repository supports building the package with and
  without C++ module interfaces. Appending one of the following options to
  the `conan install` command to make the choice:

  - `-o '&:use_modules=True'`: build with C++ module support
  - `-o '&:use_modules=False'`: build without C++ module support
  
  If not specified, `use_modules` defaults to `True`.

#### Examples

- ```bash
  conan install . --build=missing -pr .pr/gcc -s build_type=Debug
  ```

  - Build type: `Debug`
  - Build generator: OS-provided
  - Compiler: GCC
  - C++ module support: enabled

- ```bash
  conan install . --build=missing -pr .pr/clang -pr .pr/ninja-multi -o '&:use_modules=False'
  ```

  - Build type: `Release`
  - Build generator: Ninja multi-config
  - Compiler: Clang
  - C++ module support: disabled

- ```bash
  conan install . --build=missing -pr .pr/gcc -pr .pr/ninja -o '&:use_modules=True' -s build_type=RelWithDebInfo
  ```

  - Build type: `RelWithDebInfo`
  - Build generator: Ninja
  - Compiler: GCC
  - C++ module support: enabled

You can run `conan install` multiple times with different build types to
prepare multiple build types at once, but you should not vary other options.
You will have to remove the `build` subdirectory and `CMakeUserPresets.json` if
you want to vary other options.

### 2. CMake configure stage

This is where the difference between a single-config generator and a
multi-config generator matters.

- Single-config:

  ```bash
  cmake --preset conan-<build_type>
  ```

  This has to be executed for each build type that you have previously
  prepared with `conan install` in [step 1](#1-conan-install-stage),
  and `build_type` is a lowercase version of the build type.

  For example, if you want to use `Debug` and `MinSizeRel`, you will need to
  run 2 commands:

  ```bash
  cmake --preset conan-debug
  cmake --preset conan-minsizerel
  ```

  Note that `debug` and `minsizerel` are in lowercase.

- Multi-config:

  ```bash
  cmake --preset conan-default
  ```

  You only need to run this once.

#### Coverage information

If you want to generate coverage reports, you must append
`-DTYGHBN_ENABLE_COVERAGE=ON` to the `cmake --preset` command.
See the section [Generating code coverage reports](#generating-code-coverage-reports)
below for more information.

### 3. CMake build stage

To build the code, execute

```bash
cmake --build --preset conan-<build_type>
```

for each `<build_type>` that you have prepared earlier.
For example,

```bash
cmake --build --preset conan-debug
```

will compile and build the package for the `Debug` build.

### 4. CTest stage

After you have successfully compiled the code with `cmake --build`, you can run
tests by issuing a `ctest` command. The format is similar to `cmake --build`:

```bash
ctest --preset conan-<build_type> [...args]
```

For example,

```bash
ctest --preset conan-debug
```

will run tests for the `Debug` build.

### Generating code coverage reports

**Prerequisites**: To generate coverage reports, you need to

- have this project as the top-level project.
- have [`gcovr`](https://gcovr.com/en/stable/installation.html) installed.
- (for GCC) have `gcov` command available, and with the same version as `gcc`.
- (for Clang) have `llvm-cov` command available, and with the same version as
  `clang`.

Provided that all the requirements above are met,
code coverage instrumentation can be generated by adding
`-DTYGHBN_ENABLE_COVERAGE=ON` to the CMake configure command in the
[CMake configure stage](#2-cmake-configure-stage).
Remember that there is a difference at this stage between using a single-config
generator and using a multi-config generator.

#### Example: Configure step for coverage information

- Single-config generator

  ```bash
  cmake --preset conan-debug -DTYGHBN_ENABLE_COVERAGE=ON
  ```

  prepares coverage instrumentation for the `Debug` build.

- Multi-config generator

  ```bash
  cmake --preset conan-default -DTYGHBN_ENABLE_COVERAGE=ON
  ```
  
  prepares coverage instrumentation for all build types.

After configuring, the coverage reports can be made by building the `coverage`
target, i.e., passing `--target coverage` to the `cmake --build` command in
[stage 3](#3-cmake-build-stage).

#### Example: Build step for coverage information

- ```bash
  cmake --build --preset conan-debug --target coverage
  ```

  runs tests in the `Debug` mode and generates a coverage report.

If this runs successfully, the coverage report will be generated in the
directory `build/<BuildType>/coverage_report/` in 5 formats:

- `report.txt`: Plaintext detailed report
- `summary.txt`: Plaintext summary
- `summary.md`: Markdown summary
- `cobertura.xml`: Cobertura XML
- `index.html`: HTML page

### Sanitizers

Sanitizers such as AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan)
can be enabled during testing by defining the CMake variable `TYGHBN_SANITIZE`
during the [CMake configure stage](#2-cmake-configure-stage).

For example:

- ```bash
  cmake --preset conan-debug -DTYGHBN_SANITIZE=address,undefined
  ```

This configures the project to compile tests with `-fsanitize=address,undefined`.
The value of `TYGHBN_SANITIZE` is passed directly to the compiler's
`-fsanitize` flag, so you can specify any sanitizer supported by your compiler.

#### Example: Configure step for sanitizers

- Single-config generator

  ```bash
  cmake --preset conan-debug -DTYGHBN_SANITIZE=address,undefined
  ```

  prepares the `Debug` build with ASan and UBSan enabled.

- Multi-config generator

  ```bash
  cmake --preset conan-default -DTYGHBN_SANITIZE=address,undefined
  ```

  prepares all build types with ASan and UBSan enabled.

After configuring, run tests normally with `ctest` or `just test`.
The sanitized test binary will be executed, and any sanitizer diagnostics will
be printed to the console.

### Troubleshooting

- In order to use Clang with C++ modules, you will need `clang-scan-deps` to
  be available. It is a part of [Extra Clang Tools](
    https://clang.llvm.org/extra/index.html).
- In order to generate coverage information with `gcc`, `gcov` must be
  accessible and have the same version as `gcc`.
  - If `gcc` fails to compile, check if its version is at least 15.
- In order to generate coverage information with `clang`, `llvm-cov` must be
  accessible and have the same version as `clang`.

### `just` scripts

`just` commands are provided for convenience.
To use them, install [just](https://github.com/casey/just) version 1.49.0 or
newer, then type `just --list` to see available commands.
Also, make sure that you have Ninja installed as these commands rely on
Ninja-based profiles.

Below is a summary of `just` commands available in [`justfile`](justfile):

- ```bash
  just clean
  ```

  Removes the [`build`](build) directory and
  [`CMakeUserPresets.json`](CMakeUserPresets.json).

- ```bash
  just init [<build_type> [<compiler> [<coverage> [<modules> [<sanitize>]]]]]
  ```

  Does [step 1](#1-conan-install-stage) and [step 2](#2-cmake-configure-stage)
  with a given build type (`debug`, `release`, `relwithdebinfo`, or
  `minsizerel`), a given compiler (default to `gcc`), and Ninja single-config
  as the generator.
  Note that `<build_type>` is in lowercase.

  If the third argument from the end contains `cov` as a substring, the code
  coverage report generation will be enabled. (If absent, it defaults to
  `cov`.)
  If the second last argument contains `mod` as a substring, the code will be
  compiled for C++ modules. (If absent, it defaults to `mod`.)
  If the last argument is a non-empty string, it is passed as
  `-DTYGHBN_SANITIZE=<sanitize>` to the CMake configure command. This enables
  sanitizers during testing.

  Examples:

  - ```bash
    just init debug clang with-coverage -
    ```

    Prepares for the debug build, using Clang as the compiler, with the
    `coverage` CMake target. The library will be built for classic header
    `#include`.
  
  - ```bash
    just init debug gcc cov mod address,undefined
    ```

    Prepares for the debug build, using GCC as the compiler, with the
    `coverage` CMake target. The library will be built as a C++ module.
    AddressSanitizer and UndefinedBehaviorSanitizer will be enabled during
    tests.

- ```bash
  just init-single [<compiler> [<coverage> [<sanitize>]]]
  ```

  Initializes all build types for the given compiler, coverage option, and
  optional sanitizer.
  This simply calls `just init` 4 times, once for each build type.

- ```bash
  just init-multi [<compiler> [<coverage> [<sanitize>]]]
  ```

  Does [step 1](#1-conan-install-stage) and [step 2](#2-cmake-configure-stage)
  with a given compiler (default to `gcc`) and Ninja multi-config as the
  generator.
  If the second last argument contains `cov` as a substring, the code coverage
  report generation will be enabled. If the last argument is a non-empty
  string, it is passed as `-DTYGHBN_SANITIZE=<sanitize>` to the CMake configure
  command.

  Examples:

  - ```bash
    just init-multi clang - with-coverage - address
    ```

    Uses Clang as the compiler and Ninja multi-config as the generator.
    Code coverage report generation will be disabled.
    The AddressSanitizer will be enabled.
  
  - ```bash
    just init-multi
    ```

    Uses GCC as the compiler and Ninja multi-config as the generator.
    Code coverage report generation will be enabled.

- ```bash
  just build <build_type> [...args]
  ```

  Builds the code for the given build type. This simply calls

  ```bash
  cmake --build --preset conan-<build_type> ...args
  ```

- ```bash
  just test <build_type> [...args]
  ```

  Runs tests for the given build type.
  This is similar to

  ```bash
  ctest --preset conan-<build_type> ...args
  ```

  with additional options to output the test result in 2 formats:

  - `build/<BuildType>/test-report.xml`: JUnit XML
  - `build/<BuildType>/test-report.txt`: Plaintext
  
  where `BuildType` is the capitalized PascalCase version of the specified
  build type.

  Note that the code must have been built before running tests.

- ```bash
  just benchmark [<build_type>] [...args]
  ```

  Runs the benchmark executable for the given build type.
  Defaults to `release`.

- ```bash
  just build-cov <build_type>
  ```

  Builds the code coverage report for the given build type.
  This will automatically build and run tests, and generate the coverage report
  in the directory `build/<BuildType>/coverage_report`, where `<BuildType>`
  is the capitalized PascalCase version of the specified build type.

  **Prerequisites:**
  - The build system must have been initialized with coverage information
    enabled.
  - Relevant programs must be installed. See
    [Generating code coverage reports](#generating-code-coverage-reports)
    for more information.

- ```bash
  just show-cov <build_type> [<port>]
  ```

  Shows the code coverage report for the given build type as a web page at
  `http://localhost:<port>`. The default port is 8070.
  
  This command requires `python3` to be available at the command line.
  The coverage report must have been generated before calling `just show-cov`.

There are also `just` shortcuts that assume some default arguments:

- `just id` ⇒ `just init debug`.
- `just ir` ⇒ `just init release`.
- `just is` ⇒ `just init-single`.
- `just im` ⇒ `just init-multi`.
- `just ihd` ⇒ `just init debug gcc cov -`.
- `just ihr` ⇒ `just init release gcc cov -`.
- `just ihs` ⇒ `just init-single gcc cov -`.
- `just ihm` ⇒ `just init-multi gcc cov -`.
- `just bd` ⇒ `just build debug`.
- `just br` ⇒ `just build release`.
- `just ba` ⇒ `just bd; just br`.
- `just td` ⇒ `just test debug`.
- `just tr` ⇒ `just test release`.
- `just ta` ⇒ `just td; just tr`.
- `just bmd` ⇒ `just benchmark debug`.
- `just bmr` ⇒ `just benchmark release`.
- `just bmrd` ⇒ `just benchmark relwithdebinfo`.
- `just bmm` ⇒ `just benchmark minsizerel`.
- `just bcd` ⇒ `just build-cov debug`.
- `just bcr` ⇒ `just build-cov release`.
- `just bca` ⇒ `just bcd; just bcr`.
- `just scd` ⇒ `just show-cov debug`.
- `just scr` ⇒ `just show-cov release`.

These shortcuts support only `Debug` and `Release` build types,
except for the benchmark shortcuts (`bmd`, `bmr`, `bmrd`, `bmm`)
which support all build types.

#### One-line command

`just` commands that don't have arguments can be combined in a single line.
For example,

- ```bash
  just clean is bc sc
  ```

  will clean the build directory, initialize the build system, build the
  code coverage report, and display it.

Note that a single line call with multiple `just` commands will not execute
a command that is listed more than once, so
`just clean id bd td clean im bd td` will not work properly because
`just clean` will be executed only once.

#### Composite `just` commands

- ```bash
  just fresh-build [<build_type> [<compiler> [<coverage> [<modules> [<sanitize>]]]]]
  ```

  Cleans the [build](build) directory, initializes the build system with the
  given options (with `just init <build_type> <compiler> <coverage> <modules> <sanitize>`
  ), then builds the code (with `just build <build_type>`).

- ```bash
  just fresh-test [<build_type> [<compiler> [<coverage> [<modules> [<sanitize>]]]]]
  ```

  Does `just fresh-build`, followed by `just test <build_type>`.

- ```bash
  just fresh-cov [<build_type> [<compiler> [<modules> [<sanitize>]]]]
  ```

  Does `just fresh-build` with `coverage=cov`, followed by
  `just build-cov <build_type>`.

- ```bash
  just check-builds [<sanitize>]
  ```

  Calls `just fresh-test` for the debug build for the following 4
  configurations:
  - GCC with C++ modules
  - GCC with headers
  - Clang with C++ modules
  - Clang with headers

  If `<sanitize>` is not provided, it defaults to `address,undefined`.

- ```bash
  just make-reports [<compiler> [<modules> [<sanitize>]]]
  ```

  Cleans, builds, runs tests to generate a test report and a coverage report.
  The test results are stored in
  [`build/Debug/test-report.xml`](build/Debug/test-report.xml) and
  [`build/Debug/test-report.txt`](build/Debug/test-report.txt).
  The coverage report is stored in
  [`build/Debug/coverage_report`](build/Debug/coverage_report).

  If `<sanitize>` is not provided, it defaults to `address,undefined`.

## Using Xmake + Xrepo

### xmake.lua

In [`xmake.lua`](xmake.lua), you will see two configuration options for the
`tyghbn` project:

- `use_modules`: Whether to support C++ modules or not.
- `pic`: Whether to build code to be position independent or not.

When you call `xmake f` (short for `xmake config`), you can choose the values
for these options.

There are 4 build targets in [`xmake.lua`](xmake.lua), each one is defined by
the `target(...)` command:

- `add_one`
- `or_else`
- `tyghbn`
- `tests`

The first three targets are library targets, while the last target (`tests`) is
a binary target.

Each library target
has one header file in [`include/tyghbn`](include/tyghbn)
and one module file in [`modules`](modules) of the same name.
The header file exposes a traditional interface, while the module file exposes
a C++ module interface. Note that the directory structure of
[`modules`](modules) is simpler than [`include/tyghbn`](include/tyghbn) as it
does not need a nested `tyghbn` inside for disambiguation.

The command `add_files` is used to add both the implementation files (`.cpp`)
and module interface files (`.cppm`).
For module interface files, they must be explicitly marked as `public` to be
available to the consumer.
This is in contrast with `add_headerfiles`, which makes files public by
default.

Note that the Lua code for `add_one` is slightly different from `or_else`
because `or_else` does not contain a `.cpp` file but `add_one` contains a
`.cpp` file. This difference is only important when `use_modules` is `false`,
which is when we need to set the *kind* of `or_else` to `headeronly` instead
of `static`. When `use_modules` is `true`, both `add_one` and `or_else` can
have the same kind (`static` here).

#### Umbrella library target

The target named `tyghbn` is an umbrella target that reexports public entities
from `add_one` and `or_else`.
The *kind* of `tyghbn` is defined with the same logic as `or_else` because it
only adds non-implementation files on top of its dependencies.

#### Test library

This project uses [`doctest`](https://github.com/doctest/doctest) as the test
library, but you can switch to a different library by modifying the following
things:

- The `add_requires("doctest", {configs = {cmake = false}})` statement in the
  `Dependency declarations` section.
- The call to `add_packages("doctest")` in the `target("tests")` block.
- The task `test-report`, which supplies doctest-specific arguments to
  the target named `tests`, and parses the JUnit output by calling
  a function from
  [`scripts/xmake/doctest_helpers.lua`](scripts/xmake/doctest_helpers.lua).

You do not need to explicitly tell Xmake that the test library is a "test-only"
library. When you create a package, you can specify which targets you want to
export, and Xmake will know which dependencies are used only by the targets you
are exporting.

#### `TYGHBN_USE_MODULES` macro

To support choosing between compiling for C++ modules or not, a package
configuration option named `use_modules` is provided in `xmake.lua`.
It can be set in the
[XMake configure stage](#1-xmake-configure-stage), and it will be passed as a
macro named `TYGHBN_USE_MODULES` with value `1` in C++ code.

### Xmake+Xrepo development workflow

*TL;DR - use `xmake test` to build and run tests.*

The workflow for using Xmake as the build system in your development can be
separated into 3 stages:

1. [Xmake configure](#1-xmake-configure-stage)

   Configure build options and prepare dependencies.

2. [Xmake build](#2-xmake-build-stage)

   Build libraries and test code.

3. [Xmake test](#3-xmake-test-stage)

   Run tests.

### 1. Xmake configure stage

Before compiling code, you need to initialize the build environment by choosing
configuration options as you run `xmake config [...args]`.
Common options and their corresponding arguments are:

- Build mode: `--mode=<mode>`
  
  Common values for `<mode>` are:

  - `debug`
  - `release`
  - `releasedbg`: release with debug symbols
  - `minsizerel`: release with minimized size
  - `coverage`: debug with coverage information
  - `profile`: debug with profiling information
  - `check`: debug with extra checks (like asan and trapv)

  *Note: This list is far from exhaustive. See Xmake's official
  documentation for more information.*

- Toolchain: `--toolchain=<toolchain>`

  Common values for `<toolchain>` are:

  - `gcc`
  - `clang`

- Policies: `--policies=<policies>`

  Xmake supports many types of build-in
  [policies](https://xmake.io/api/description/builtin-policies.html).
  Here are the two common policies related to sanitizers:

  - `build.sanitizer.address`: enables AddressSanitizer (ASan).
  - `build.sanitizer.undefined`: enables UndefinedBehaviorSanitizer (UBSan).

  It is recommended to enable these sanitizers during development.
  (They are not enabled by default.)

  **Note: There is a bug in Xmake causing the compilation to fail when
  `build.sanitizer.undefined` is enabled, GCC 15 is used, and `use_modules` is
  `true`. If you are using a newer version of Xmake or a newer version of GCC,
  you should try to see if `build.sanitizer.undefined` can be enabled.**

- Project config `use_modules`: `--use_modules={y|n}`

  Possible values:

  - `y`: the project will support C++ modules. This is the default option.
  - `n`: the project will not support C++ modules.

- Project config `pic`: `--pic={y|n}`

  Possible values:

  - `y`: the compiled binary code will be position-independent code (PIC).
    This is the default option.
  - `n`: the compiled binary code will not be position-independent.

  *Note: Unless you really know what you are doing, there's no need to touch
  this option. Position-independent code is the modern standard.*

#### Examples

- ```bash
  xmake config
  ```

  Configures Xmake to use your system's default compiler, build in the
  release mode, and support C++ modules.

- ```bash
  xmake config --toolchain=clang --mode=debug --use_modules=n \
    --policies=build.sanitizer.address,build.sanitizer.undefined
  ```

  Configures Xmake to use Clang as the compiler, build in the debug mode with
  AddressSanitizer and UndefinedBehaviorSanitizer available, and omit support
  for C++ modules.

#### Managing Xmake configurations

Xmake stores all your configurations inside the [`.xmake`](.xmake)
subdirectory.
That means multiple configurations can exist at the same time, but only the
*active*, i.e., the most recently configured one, will be used for subsequent
`xmake` commands.
You can simply call `xmake config` to switch your active configuration.

##### Cleaning Xmake configurations

You can call `xmake f -c` (short for `xmake config --clean`) to clean all
configurations.

Note that configuration files are separate from build artifacts, which will be
in the [`build`](build) subdirectory.
If you want to clean both the configuration files and the build artifacts, run
`xmake clean-all`.

### 2. Xmake build stage

To compile the code, simply execute

```bash
xmake
```

You can skip this command and go directly to the test stage if you also want to
run tests right after compiling.

### 3. Xmake test stage

To run tests, execute

```bash
xmake run tests
```

This will run the `tests` executable target defined in
[`xmake.lua`](xmake.lua).

To run benchmarks with Xmake, execute

```bash
xmake run benchmarks
```

This will run the `benchmarks` executable target defined in
[`xmake.lua`](xmake.lua).

### Composite Xmake tasks

You can also run *tasks* that do more than just running tests:

- ```bash
  xmake test-report
  ```

  Runs tests and creates a test report.
  The report will be stored in 2 files:

  - [`build/test-report.xml`](build/test-report.xml):
    JUnit XML format
  - [`build/test-report.txt`](build/test-report.txt):
    plaintext summary

  *Note: This test report generation relies on doctest.
  If you use a different test library, the provided code will not work.*

- ```bash
  xmake coverage-report
  ```

  Runs tests and creates a coverage report.
  This will only work if your project active configuration has mode `coverage`.

  The report will be stored in the directory
  [`build/coverage_report`](build/coverage_report).
  There are 5 formats of the report in this directory:

  - [`build/coverage_report/report.txt`](build/coverage_report/report.txt):
    plaintext detailed report
  - [`build/coverage_report/cobertura.xml`](build/coverage_report/cobertura.xml):
    Cobertura XML
  - [`build/coverage_report/index.html`](build/coverage_report/index.html):
    static HTML page
  - [`build/coverage_report/summary.md`](build/coverage_report/summary.md):
    markdown summary
  - [`build/coverage_report/summary.txt`](build/coverage_report/summary.txt):
    plaintext summary

- ```bash
  xmake clean-all
  ```

  Cleans all the project configurations and build artifacts.

- ```bash
  xmake reports
  ```

  Cleans the project configurations, configure with the `coverage` mode and
  with PIC, ASan and UBSan enabled, then create both a test report and a
  coverage report by calling `xmake test-report` and `xmake coverage-report`.

  **Note: The configuration has `--use_modules=n` because there is a bug in
  Xmake causing GCC 15 to fail to build if `use_modules=y` and UBSan is
  enabled. If you happen to use a newer version of Xmake or a newer version of
  GCC, you should try to see if flipping `use_modules` to `y` works.**

- ```bash
  xmake check-builds
  ```

  Builds and runs tests for the following combinations of configurations:

  - `toolchain=gcc` and `toolchain=clang`
  - `use_modules=y` and `use_modules=n`

  PIC, ASan and UBSan will be enabled.

  **Note: There is a bug in Xmake causing GCC 15 to fail to build if
  `use_modules=y` and UBSan is enabled. The current code in `xmake.lua` has a
  workaround to disable `build.sanitizer.undefined` for this specific
  configuration.
  If you happen to use a newer version of XMake or a newer version of
  GCC, you should try to see if `build.sanitizer.undefined` can stay enabled
  throughout all the configurations.**

You can see how these tasks are defined in [`xmake.lua`](xmake.lua) under the
section `Task declarations`.

### Troubleshooting

- There is a bug in Xmake causing GCC 15 to fail to build if `use_modules=y`
  and UBSan is enabled. This same configuration works fine with Clang.
  - Because of this, the tasks `xmake reports` and `xmake check-builds` skip
    this problematic configuration.
  - If you happen to use a newer version of XMake or a
    newer version of GCC, you should try to see if `build.sanitizer.undefined`
    can be enabled without breaking the build.

## Docker

Some Dockerfiles for development are provided in the [ci](ci) subdirectory.
You can use the provided [`ci/compose.yaml`](ci/compose.yaml) to simplify
building docker images and running containers locally. Some `just` commands are
also provided for convenience.

- ```bash
  just run-docker [<variant> [<stage> [...args]]]
  ```

  Builds an ephemeral Docker container from the image at the specified `stage`
  in `Dockerfile.<variant>`, and runs the command specified in `...args`.
  Files in this repository will be copied into the directory `/workspace`
  inside the container before the command is run.

  - `variant`: A suffix of a Dockerfile in the [ci](ci) subdirectory.
    For example, putting `ubuntu` will use
    [ci/Dockerfile.ubuntu](ci/Dockerfile.ubuntu).
    If `variant` is not specified, it defaults to `alpine`.
  - `stage`: A *stage* in the Dockerfile. This should be `gcc` or `full`.
    The `gcc` stage has a smaller image than `full` as it does not have
    Clang and related tools.
    If `stage` is not specified, it defaults to `full`.
  - `args`: The command to run on the container. If not specified, it defaults
    to `sh`, which effectively brings up an interactive shell of the container.
    (The container will be destroyed after the shell session ends.)

- ```bash
  just create-docker [<variant> [<stage> [<name> [...args]]]]
  ```

  Creates a Docker container with the specified `name` from the image at the
  specified `stage` in `Dockerfile.<variant>`, and run the command specified in
  `...args` in detached mode.
  Files in this repository will be copied into the directory `/workspace`
  inside the container before the command is run.

  - `variant`: A suffix of a Dockerfile in the [ci](ci) subdirectory.
    For example, putting `ubuntu` will use
    [ci/Dockerfile.ubuntu](ci/Dockerfile.ubuntu).
    If `variant` is not specified, it defaults to `alpine`.
  - `stage`: A *stage* in the Dockerfile. This should be `gcc` or `full`.
    The `gcc` stage has a smaller image than `full` as it does not have
    Clang and related tools.
    If `stage` is not specified, it defaults to `full`.
  - `name`: The name of the container. If not specified, it defaults to
    `<variant>-<stage>`. If a container with the specified name already exists,
    it will be stopped and deleted first.
  - `args`: The command to run on the container. If not specified, it defaults
    to `tail -f /dev/null`, which effectively does nothing except keeping the
    container alive.

- ```bash
  just clean-docker-images [<prefix>]
  ```

  Calls `docker rmi` on all images whose tags have the specified `prefix`.
  If `prefix` is not specified, it defaults to `tyghbn-`, which is the prefix of
  `name` in [compose.yaml](ci/compose.yaml).

Example:

- ```bash
  just run-docker ubuntu full just check-builds
  ```

## Code documentation

**Prerequisite: [Doxygen](https://www.doxygen.nl/) version 1.9.8 or newer**

- ```bash
  just doc
  ```

  Create the code documentation in `build/doc`.

- ```bash
  just clean-doc
  ```

  Remove `build/doc`.

- ```bash
  just show-doc [<port>]
  ```

  Show the doc as a webpage at `http://localhost:<port>`.
  The default `port` is 8060.

  This command requires `python3` to be available at the command line.
  The documentation must have been generated before calling `just show-doc`.

- ```bash
  just docx [<port>]
  ```

  Same as `just clean-doc && just doc && just show-doc <port>`.
  The default `port` is 8060.

## GitHub Actions

### Pull request actions

- [`cmake-pull-request.yaml`](.github/workflows/cmake-pull-request.yaml)

  - Triggers when a PR is opened, updated, or reopened.
  - Runs `just make-reports` inside a Docker container based on the image in
    [`Dockerfile.alpine`](ci/Dockerfile.alpine), then uploads test and
    coverage reports as artifacts.
  - *Remove this file if you are not using CMake.*

- [`xmake-pull-request.yaml`](.github/workflows/xmake-pull-request.yaml)

  - Triggers when a PR is opened, updated, or reopened.
  - Runs `xmake reports` inside a Docker container based on the image in
    [`Dockerfile.alpine-xmake`](ci/Dockerfile.alpine-xmake), then uploads test
    and coverage reports as artifacts.
  - *Remove this file if you are not using Xmake.*

### Post-merge actions

- [`post-merge.yaml`](.github/workflows/post-merge.yaml)

  - Triggers when there is a `push` action to the `main` branch.
  - Runs workflows in `cmake-check-builds.yaml` and `xmake-check-builds.yaml`.
    If they both succeed, the `stable` tag will be updated to point to the
    tip of the `main` branch. Otherwise, a new issue will be created.
  - *If you do not use both CMake and Xmake, you should edit this file.*

- [`cmake-check-builds.yaml`](.github/workflows/cmake-check-builds.yaml)

  - Triggers only when explicitly called by `post-merge.yaml`.
  - Runs `just check-builds` inside Docker containers based on the images in
    [`Dockerfile.alpine`](ci/Dockerfile.alpine) and
    [`Dockerfile.ubuntu`](ci/Dockerfile.ubuntu).
  - *Remove this file if you are not using CMake.*

- [`xmake-check-builds.yaml`](.github/workflows/xmake-check-builds.yaml)

  - Triggers only when explicitly called by `post-merge.yaml`.
  - Runs `xmake check-builds` inside Docker containers based on the images in
    [`Dockerfile.alpine-xmake`](ci/Dockerfile.alpine-xmake) and
    [`Dockerfile.ubuntu-xmake`](ci/Dockerfile.ubuntu-xmake).
  - *Remove this file if you are not using Xmake.*
