cmake_minimum_required(VERSION 4.0.2)
project(sorted_array_tower
  VERSION 1.0.0
  DESCRIPTION "A C++ utility library providing simple template functions"
  LANGUAGES CXX
)

# Option for building with modules (C++20 modules support)
option(SORTED_ARRAY_TOWER_USE_MODULES "Use C++20 modules instead of headers" OFF)

if(SORTED_ARRAY_TOWER_USE_MODULES)
  if(NOT CMAKE_GENERATOR STREQUAL "Ninja" AND
    NOT CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" AND
    NOT CMAKE_GENERATOR MATCHES "Visual Studio")
    message(WARNING "${CMAKE_GENERATOR} does not support C++ modules")
    message(WARNING "Configuring for building ${PROJECT_NAME} without C++ modules")
    set(SORTED_ARRAY_TOWER_USE_MODULES OFF CACHE BOOL "Use C++20 modules instead of headers" FORCE)
  else()
    message(STATUS "Configuring for building ${PROJECT_NAME} with C++ modules")
  endif()
else()
  message(STATUS "Configuring for building ${PROJECT_NAME} without C++ modules")
endif()

# Include Conan generated files
if(EXISTS "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
  include("${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
endif()

# Prepare compile_commands.json for development tools such as documentation and
# LSP.
if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  # Automatically resolve the correct path regardless of generator type
  if(CMAKE_CONFIGURATION_TYPES)
    # Multi-Config outputs directly to the top-level build directory
    set(CDB_PATH "${CMAKE_BINARY_DIR}")
  else()
    # Single-Config outputs to the active Conan variant folder (Release/Debug)
    set(CDB_PATH "${CMAKE_CURRENT_BINARY_DIR}")
  endif()

  message("compile_commands.json path: ${CDB_PATH}")
endif()

### TEMPLATE_BLOCK: Library with no cpp files
#
# To provide a consist interface for module and non-module versions, start with
# a base library which is a non-module version.
#
# Note that the type of the library target must be INTERFACE here.
add_library(or_else-legacy INTERFACE)
target_sources(or_else-legacy
  INTERFACE
    FILE_SET HEADERS
      FILES include/sorted_array_tower/or_else.hpp
      BASE_DIRS include
)

if(NOT SORTED_ARRAY_TOWER_USE_MODULES)
  # For the non-C++-module version, we simply define a passthrough library
  # because or_else-legacy is already a valid library.
  add_library(or_else INTERFACE)
  target_link_libraries(or_else
    INTERFACE
      or_else-legacy
  )
else()
  # For the C++-module version, we add the module interface files on top of
  # the base library.
  add_library(or_else)
  target_compile_features(or_else PUBLIC cxx_std_20)
  target_link_libraries(or_else
    PUBLIC
      or_else-legacy
  )
  target_sources(or_else
    PUBLIC
      FILE_SET CXX_MODULES
        FILES modules/or_else.cppm
  )
endif()
### TEMPLATE_BLOCK_END: Library with no cpp files


### TEMPLATE_BLOCK: Library with cpp files
#
# To provide a consist interface for module and non-module versions, start with
# a base library which is a non-module version.
add_library(add_one-legacy)
target_sources(add_one-legacy
  PUBLIC
    FILE_SET HEADERS
      FILES include/sorted_array_tower/add_one.hpp
      BASE_DIRS include
  PRIVATE
    src/add_one.cpp
)

if(NOT SORTED_ARRAY_TOWER_USE_MODULES)
  # For the non-C++-module version, we simply define a passthrough library
  # because add_one-legacy is already a valid library.
  add_library(add_one INTERFACE)
  target_link_libraries(add_one
    INTERFACE
      add_one-legacy
  )
else()
  # For the C++-module version, we add the module interface files on top of
  # the base library.
  add_library(add_one)
  target_compile_features(add_one PUBLIC cxx_std_20)
  target_link_libraries(add_one
    PUBLIC
      add_one-legacy
  )
  target_sources(add_one
    PUBLIC
      FILE_SET CXX_MODULES
        FILES modules/add_one.cppm
  )
endif()
### TEMPLATE_BLOCK_END: Library with cpp files


### TEMPLATE_BLOCK: Umbrella library
#
# "All" target that includes all libraries above.
add_library(sorted_array_tower-legacy INTERFACE)
target_sources(sorted_array_tower-legacy
  INTERFACE
    FILE_SET HEADERS
      FILES include/sorted_array_tower/sorted_array_tower.hpp
      BASE_DIRS include
)
# Put all the libraries to include.
target_link_libraries(sorted_array_tower-legacy
  INTERFACE
    or_else
    add_one
)

if(NOT SORTED_ARRAY_TOWER_USE_MODULES)
  # For the non-C++-module version, we simply define a passthrough library
  # because sorted_array_tower-legacy is already a valid library.
  add_library(sorted_array_tower INTERFACE)
  target_link_libraries(sorted_array_tower
    INTERFACE
      sorted_array_tower-legacy
  )
else()
  # For the C++-module version, we add the module interface file that
  # re-exports all of its submodules.
  add_library(sorted_array_tower)
  target_compile_features(sorted_array_tower PUBLIC cxx_std_20)
  target_link_libraries(sorted_array_tower
    PUBLIC
      sorted_array_tower-legacy
  )
  # Add an extra cppm file that provides a better way to include submodules via
  # `export import`.
  target_sources(sorted_array_tower
    PUBLIC
      FILE_SET CXX_MODULES
        FILES modules/sorted_array_tower.cppm
  )
  # Note that this umbrella module does not really need sorted_array_tower.hpp if it is
  # only used via `import` as sorted_array_tower.cppm does not include sorted_array_tower.hpp, but we
  # include sorted_array_tower.hpp (via linking with sorted_array_tower-legacy) to make this library
  # support both the header-style usage and the import-style usage.
  # Additionally, it simplifies CMake code as sorted_array_tower-legacy is itself a valid
  # library for the non-C++-module case.
endif()
### TEMPLATE_BLOCK_END: Umbrella library


# Testing enabled only when the project is top-level and BUILD_TESTING is not
# explicitly defined as OFF.
if(PROJECT_IS_TOP_LEVEL AND NOT (DEFINED BUILD_TESTING AND NOT BUILD_TESTING))

  ### TEMPLATE_BLOCK: Test library initialization
  #
  # Find doctest ONLY via the configuration of this project.
  find_package(doctest CONFIG QUIET)

  if(NOT doctest_FOUND)
    message(WARNING "doctest not found - tests will not be built")
  else()
    # Include CMake scripts from doctest 
    list(APPEND CMAKE_MODULE_PATH ${doctest_CMAKE_MODULE_PATH})
    include(doctest)
    ### TEMPLATE_BLOCK_END: Test library initialization

    # Enable testing
    enable_testing()

    # Create a test executable target
    add_executable(test_sorted_array_tower)

    # Pass the SORTED_ARRAY_TOWER_USE_MODULES flag to the test code
    if(SORTED_ARRAY_TOWER_USE_MODULES)
      target_compile_definitions(test_sorted_array_tower
        PRIVATE
          SORTED_ARRAY_TOWER_USE_MODULES=1
      )
    endif()

    # Sanitizer configuration
    if(DEFINED SORTED_ARRAY_TOWER_SANITIZE AND SORTED_ARRAY_TOWER_SANITIZE)
      if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(test_sorted_array_tower
          PRIVATE
            -fsanitize=${SORTED_ARRAY_TOWER_SANITIZE}
            -fno-omit-frame-pointer
            -Og -g
            -fno-sanitize-recover=all
        )
        target_link_options(test_sorted_array_tower
          PRIVATE
            -fsanitize=${SORTED_ARRAY_TOWER_SANITIZE}
        )
        message("${SANITIZER_OPTIONS}")
      else()
        message(WARNING "Sanitizers are not supported with ${CMAKE_CXX_COMPILER_ID}")
      endif()
    endif()

    # Add test source files
    add_subdirectory(tests)

    ### TEMPLATE_BLOCK: Test library finalization
    #
    # Link the test executable to the test library
    target_link_libraries(test_sorted_array_tower
      PRIVATE
        doctest::doctest
    )

    # Create test targets
    doctest_discover_tests(test_sorted_array_tower)
    ### TEMPLATE_BLOCK_END: Test library finalization


    # Code coverage report generation
    option(SORTED_ARRAY_TOWER_ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
    if(SORTED_ARRAY_TOWER_ENABLE_COVERAGE)
      if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(test_sorted_array_tower PRIVATE --coverage -O0 -g)
        target_link_options(test_sorted_array_tower PRIVATE --coverage)

        find_program(GCOVR gcovr)

        if(GCOVR)
          if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            find_program(SORTED_ARRAY_TOWER_LLVM_COV llvm-cov)
            if(SORTED_ARRAY_TOWER_LLVM_COV)
              set(SORTED_ARRAY_TOWER_GCOV_EXECUTABLE "${SORTED_ARRAY_TOWER_LLVM_COV} gcov")
            else()
              message(WARNING "llvm-cov not found — coverage target will not be created")
              set(SORTED_ARRAY_TOWER_ENABLE_COVERAGE OFF)
            endif()
          else()
            set(SORTED_ARRAY_TOWER_GCOV_EXECUTABLE gcov)
          endif()

          if(SORTED_ARRAY_TOWER_ENABLE_COVERAGE)
            set(SORTED_ARRAY_TOWER_BUILD_DIR "$<TARGET_FILE_DIR:test_sorted_array_tower>")
            add_custom_target(coverage
              COMMAND $<TARGET_FILE:test_sorted_array_tower>
              COMMAND ${CMAKE_COMMAND} -E make_directory "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report"
              COMMAND ${GCOVR} --root ${CMAKE_SOURCE_DIR}
                --txt
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/report.txt"
                --html-details
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/index.html"
                --cobertura
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/cobertura.xml"
                --markdown
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/summary.md"
                --exclude "${CMAKE_SOURCE_DIR}/tests/"
                --gcov-executable "${SORTED_ARRAY_TOWER_GCOV_EXECUTABLE}"
                --txt-summary >
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/summary.txt"
              WORKING_DIRECTORY "${SORTED_ARRAY_TOWER_BUILD_DIR}"
              COMMENT "Generating code coverage report in ${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report"
            )
          endif()
        else()
          message(WARNING "gcovr not found — coverage target will not be created")
        endif()
      elseif(MSVC)
        message(WARNING "Code coverage is not supported with MSVC")
      else()
        message(WARNING "Code coverage is not supported with ${CMAKE_CXX_COMPILER_ID}")
      endif()
    endif()

  endif()

  ### TEMPLATE_BLOCK: Benchmarks
  find_package(nanobench CONFIG QUIET)

  if(NOT nanobench_FOUND)
    message(WARNING "nanobench not found - benchmarks will not be built")
  else()
    add_executable(benchmark_sorted_array_tower)

    if(SORTED_ARRAY_TOWER_USE_MODULES)
      target_compile_definitions(benchmark_sorted_array_tower
        PRIVATE
          SORTED_ARRAY_TOWER_USE_MODULES=1
      )
    endif()

    # nanobench relies on platform-specific functionality to provide more
    # details. If the dev library is not present on the system, we fall back
    # to a more generic level of details.
    if(LINUX)
      include(CheckIncludeFiles)
      check_include_files("linux/version.h" HAS_LINUX_VERSION_H)
      if(NOT HAS_LINUX_VERSION_H)
        target_compile_definitions(benchmark_sorted_array_tower
          PRIVATE
            ANKERL_NANOBENCH_DISABLE_PERF_COUNTERS=1
        )
        message(WARNING
          "linux/perf_event.h or linux/version.h not found. "
          "Performance counters will be disabled."
        )
      endif()
    endif()

    target_sources(benchmark_sorted_array_tower
      PRIVATE
        benchmarks/benchmark_main.cpp
    )

    target_link_libraries(benchmark_sorted_array_tower
      PRIVATE
        sorted_array_tower
        nanobench::nanobench
    )
  endif()
  ### TEMPLATE_BLOCK_END: Benchmarks

else()
  message(WARNING "Not building tests")
  message(WARNING "BUILD_TESTING is ${BUILD_TESTING}")
endif()

# Installation rules
if(SORTED_ARRAY_TOWER_USE_MODULES)
  install(TARGETS or_else add_one sorted_array_tower
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    FILE_SET CXX_MODULES
    DESTINATION modules
    FILE_SET HEADERS
    DESTINATION include
  )
else()
  install(TARGETS or_else add_one sorted_array_tower
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    FILE_SET HEADERS
    DESTINATION include
  )
endif()

