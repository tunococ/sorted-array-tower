/**
 * @file
 * 
 * @brief [module sorted_array_tower.add_one](module__sorted_array_tower_8add__one.html)
 */

#pragma once

#include <string>

namespace sorted_array_tower {

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

} // namespace sorted_array_tower
