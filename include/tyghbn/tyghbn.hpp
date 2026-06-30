/**
 * @mainpage Tyghbn
 * 
 * @brief Entry point: @ref tyghbn.hpp
 * 
 * This is a template for a C++ library project.
 * It exposes multiple libraries, which are called *submodules* here.
 * 
 * There are two ways to libraries this project:
 * - Legacy: including header files for submodules.
 * - C++ modules: importing submodules.
 * 
 * The umbrella module that includes all submodules is called `tyghbn`.
 * You can use it with
 * ```
 * #include <tyghbn/tyghbn.hpp>      // Legacy way
 * ```
 * or
 * ```
 * import tyghbn;                    // C++ module way
 * ```
 * to include all submodules.
 *
 * If you want to include only a specific submodule, you can do so by including
 * the corresponding header or importing the corresponding submodule.
 * Here's an example of how to pull in only the submodule named `add_one`:
 * ```
 * #include <tyghbn/add_one.hpp>     // Legacy way
 * ```
 * or
 * ```
 * import tyghbn.add_one;            // C++ module way
 * ```
 * 
 * The umbrella module that includes all submodules is called `tyghbn`.
 * You can use it with
 * ```
 * #include <tyghbn/tyghbn.hpp>      // Legacy way
 * ```
 * or
 * ```
 * import tyghbn;                    // C++ module way
 * ```
 * to include all submodules.
 */

/**
 * @file
 * 
 * @brief [module tyghbn](module__tyghbn.html)
 * 
 * This is an umbrella header file that includes all submodules.
 */

#pragma once

#include "add_one.hpp"
#include "or_else.hpp"
