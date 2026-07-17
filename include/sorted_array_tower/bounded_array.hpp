/**
 * @file
 *
 * @brief
 * [sorted_array_tower.skip_array](module__sorted_array_tower_8skip_array.html)
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <variant>
#include <vector>

namespace sorted_array_tower {

/**
 * @brief An index-based list that can grow and shrink, but has a fixed
 * maximum capacity.
 */
template <typename T, typename Allocator = std::allocator<T>>
class BoundedArray {
 public:
  using value_type = T;
  using reference_type = value_type&;
  using const_reference_type = value_type const&;
  using this_type = BoundedArray;

  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<value_type>;
  using size_type = std::allocator_traits<allocator_type>::size_type;
  using difference_type =
      std::allocator_traits<allocator_type>::difference_type;
  using pointer = std::allocator_traits<allocator_type>::pointer;
  using const_pointer = std::allocator_traits<allocator_type>::const_pointer;

 private:
  std::unique_ptr<T[]> data_;
  size_type front_index_;
  size_type capacity_;
  size_type size_;

 public:
  
};

}  // namespace sorted_array_tower