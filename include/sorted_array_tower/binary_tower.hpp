#pragma once

#include <algorithm>
#include <memory>

#include "bounded_array.hpp"
#include "skip_array.hpp"

namespace sorted_array_tower {

template <typename T, typename Allocator = std::allocator<T>,
          std::allocator_traits<Allocator>::size_type MIN_SIZE =
              std::max(512 / sizeof(T), 2)>
class BinaryTower {
 public:
  /// The type of the elements stored in the array.
  using value_type = T;
  /// The type of this `BinaryTower` specialization.
  using this_type = BinaryTower;

  /// The allocator type used to allocate elements.
  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<value_type>;

  using allocator_traits = std::allocator_traits<allocator_type>;

  /// An unsigned integer type used for sizes and displacements.
  using size_type = allocator_traits::size_type;
  /// A signed integer type used for differences between iterators.
  using difference_type = allocator_traits::difference_type;
  /// A pointer to an element.
  using pointer = allocator_traits::pointer;
  /// A constant pointer to an element.
  using const_pointer = allocator_traits::const_pointer;
  /// A reference to an element.
  using reference = allocator_traits::reference;
  /// A constant reference to an element.
  using const_reference = allocator_traits::const_reference;

  /// The type of each layer in the tower.
  using layer_type = SkipArray<T, allocator_type, BoundedArray>;

  /// The allocator type used to allocate layers.
  using layer_allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<layer_type>;
  using layer_allocator_traits = std::allocator_traits<layer_allocator_type>;
  using layer_pointer = layer_allocator_traits::pointer;
  using layer_const_pointer = layer_allocator_traits::const_pointer;

 private:
  std::vector<layer_pointer> layers_;
};

}  // namespace sorted_array_tower
