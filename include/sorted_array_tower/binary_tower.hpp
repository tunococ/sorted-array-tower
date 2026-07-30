#pragma once

#include <algorithm>
#include <cassert>
#include <memory>

#include "bounded_array.hpp"
#include "merge_iterator.hpp"
#include "skip_array.hpp"

namespace sorted_array_tower {

template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<T>,
          std::allocator_traits<Allocator>::size_type MIN_SIZE =
              std::max(512 / sizeof(T), 1)>
class BinaryTower {
 public:
  /// Types of the elements stored in the array.
  using value_type = T;
  using key_type = T;
  /// Types of comparator.
  using key_compare = Compare;
  using value_compare = Compare;
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
  using layer_type = SkipArray<T, allocator_type, BoundedVector>;

  /// The allocator type used to allocate layers.
  using layer_allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<layer_type>;
  using layer_allocator_traits = std::allocator_traits<layer_allocator_type>;
  using layer_pointer = layer_allocator_traits::pointer;
  using layer_const_pointer = layer_allocator_traits::const_pointer;

  using inlayer_iterator = typename layer_type::iterator;
  using inlayer_const_iterator = typename layer_type::const_iterator;
  using iterator =
      BidirectionalMergeIterator<inlayer_iterator, key_compare, Allocator>;
  using const_iterator = BidirectionalMergeIterator<inlayer_const_iterator,
                                                    key_compare, Allocator>;

 protected:
  key_compare compare_{};
  layer_allocator_type layer_allocator_{};
  std::vector<layer_pointer> layers_;

 public:
  BinaryTower() : BinaryTower(key_compare()) {}
  explicit BinaryTower(key_compare const& comp,
                       allocator_type const& alloc = allocator_type())
      : compare_(comp), layer_allocator_(alloc), layers_(layer_allocator_) {}
  explicit BinaryTower(allocator_type const& alloc)
      : compare_(key_compare()),
        layer_allocator_(alloc),
        layers_(layer_allocator_) {}
};

}  // namespace sorted_array_tower
