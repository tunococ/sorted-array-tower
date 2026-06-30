/**
 * @file
 * 
 * @brief [sorted_array_tower.or_else](module__sorted_array_tower_8or__else.html)
 */

#pragma once

namespace sorted_array_tower {

/**
 * @brief Returns @p x if it is non-zero; otherwise returns @p y.
 *
 * @tparam T The element type. Must be equality-comparable with `T(0)`.
 * @param x The value to test.
 * @param y The fallback value.
 * @return x if x != T(0), otherwise y.
 */
template <typename T>
T or_else(T x, T y) {
    return x == T(0) ? y : x;
}

} // namespace sorted_array_tower
