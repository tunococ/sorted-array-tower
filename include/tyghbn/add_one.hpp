/**
 * @file
 * 
 * @brief [module tyghbn.add_one](module__tyghbn_8add__one.html)
 */

#pragma once

#include <string>

namespace tyghbn {

/**
 * @brief Adds 1 to the input value.
 *
 * @tparam T The type of the input and output. Must support addition with itself.
 * @param x The value to increment.
 * @return `x` incremented by 1.
 */
template <typename T>
T add_one(T x) {
    return x + T(1);
}

/// @cond
template <>
std::string add_one(std::string x);
/// @endcond

} // namespace tyghbn
