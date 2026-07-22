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
    set(COMPILE_COMMANDS_PATH "${CMAKE_BINARY_DIR}/compile_commands.json")
  else()
    # Single-Config outputs to the active Conan variant folder (Release/Debug)
    set(COMPILE_COMMANDS_PATH "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
  endif()

  message(STATUS "compile_commands.json path: ${COMPILE_COMMANDS_PATH}")

  add_custom_target(copy_compile_commands ALL
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      "${COMPILE_COMMANDS_PATH}"
      "${PROJECT_SOURCE_DIR}/build/compile_commands.json"
  )

  if(NOT (DEFINED BUILD_TESTING AND NOT BUILD_TESTING))
    option(SORTED_ARRAY_TOWER_ENABLE_COVERAGE "Enable code coverage instrumentation" OFF)
    if(SORTED_ARRAY_TOWER_ENABLE_COVERAGE)
      if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(--coverage -g)
        add_link_options(--coverage)
      endif()
    endif()
  endif()

endif()

add_library(skip_array-legacy INTERFACE)
target_sources(skip_array-legacy
  INTERFACE
    FILE_SET HEADERS
      FILES include/sorted_array_tower/skip_array.hpp
      BASE_DIRS include
)

if(NOT SORTED_ARRAY_TOWER_USE_MODULES)
  # For the non-C++-module version, we simply define a passthrough library
  # because skip_array-legacy is already a valid library.
  add_library(skip_array INTERFACE)
  target_link_libraries(skip_array
    INTERFACE
      skip_array-legacy
  )
else()
  # For the C++-module version, we add the module interface files on top of
  # the base library.
  add_library(skip_array)
  target_compile_features(skip_array PUBLIC cxx_std_20)
  target_link_libraries(skip_array
    PUBLIC
      skip_array-legacy
  )
  target_sources(skip_array
    PUBLIC
      FILE_SET CXX_MODULES
        FILES modules/skip_array.cppm
  )
endif()


add_library(bounded_array-legacy INTERFACE)
target_sources(bounded_array-legacy
  INTERFACE
    FILE_SET HEADERS
      FILES include/sorted_array_tower/bounded_array.hpp
      BASE_DIRS include
)

if(NOT SORTED_ARRAY_TOWER_USE_MODULES)
  add_library(bounded_array INTERFACE)
  target_link_libraries(bounded_array
    INTERFACE
      bounded_array-legacy
  )
else()
  add_library(bounded_array)
  target_compile_features(bounded_array PUBLIC cxx_std_20)
  target_link_libraries(bounded_array
    PUBLIC
      bounded_array-legacy
  )
  target_sources(bounded_array
    PUBLIC
      FILE_SET CXX_MODULES
        FILES modules/bounded_array.cppm
  )
endif()


add_library(bounded_vector-legacy INTERFACE)
target_sources(bounded_vector-legacy
  INTERFACE
    FILE_SET HEADERS
      FILES include/sorted_array_tower/bounded_vector.hpp
      BASE_DIRS include
)

if(NOT SORTED_ARRAY_TOWER_USE_MODULES)
  add_library(bounded_vector INTERFACE)
  target_link_libraries(bounded_vector
    INTERFACE
      bounded_vector-legacy
  )
else()
  add_library(bounded_vector)
  target_compile_features(bounded_vector PUBLIC cxx_std_20)
  target_link_libraries(bounded_vector
    PUBLIC
      bounded_vector-legacy
  )
  target_sources(bounded_vector
    PUBLIC
      FILE_SET CXX_MODULES
        FILES modules/bounded_vector.cppm
  )
endif()


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
    skip_array
    bounded_array
    bounded_vector
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
        add_compile_options(
          -fsanitize=${SORTED_ARRAY_TOWER_SANITIZE}
          -fno-omit-frame-pointer
          -Og -g
          -fno-sanitize-recover=all
        )
        add_link_options(
          -fsanitize=${SORTED_ARRAY_TOWER_SANITIZE}
        )
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
              COMMAND ${GCOVR}
                --root "."
                --filter "src/"
                --filter "include/"
                --filter "modules/"
                --gcov-executable "${SORTED_ARRAY_TOWER_GCOV_EXECUTABLE}"
                --txt
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/report.txt"
                --html-details
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/index.html"
                --cobertura
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/cobertura.xml"
                --markdown
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/summary.md"
                --txt-summary >
                "${SORTED_ARRAY_TOWER_BUILD_DIR}/coverage_report/summary.txt"
              WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
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
        benchmarks/benchmarks.hpp
        benchmarks/benchmarks_main.cpp
        benchmarks/benchmark_bounded_array.cpp
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
  install(TARGETS bounded_array bounded_vector sorted_array_tower
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    FILE_SET CXX_MODULES
    DESTINATION modules
    FILE_SET HEADERS
    DESTINATION include
  )
else()
  install(TARGETS bounded_array bounded_vector sorted_array_tower
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    FILE_SET HEADERS
    DESTINATION include
  )
endif()

