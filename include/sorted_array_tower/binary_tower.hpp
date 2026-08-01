#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

#include "bounded_array.hpp"
#include "merge.hpp"
#include "merge_iterator.hpp"
#include "skip_array.hpp"

namespace sorted_array_tower {

template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<T>,
          std::allocator_traits<Allocator>::size_type MIN_SIZE =
              std::max(256 / sizeof(T), 1)>
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
  using inlayer_iterator = typename layer_type::iterator;
  using inlayer_const_iterator = typename layer_type::const_iterator;

  /// The type of the bottom layer in the tower, which stores a sorted vector.
  using bottom_layer_type = std::vector<T, allocator_type>;
  using bottom_inlayer_iterator = typename bottom_layer_type::iterator;
  using bottom_inlayer_const_iterator =
      typename bottom_layer_type::const_iterator;

  /// The allocator type used to allocate layers.
  using layer_allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<layer_type>;
  using layer_allocator_traits = std::allocator_traits<layer_allocator_type>;
  using layer_pointer = layer_allocator_traits::pointer;
  using layer_const_pointer = layer_allocator_traits::const_pointer;

  using iterator =
      BidirectionalMergeIterator<inlayer_iterator, key_compare, Allocator>;
  using const_iterator = BidirectionalMergeIterator<inlayer_const_iterator,
                                                    key_compare, Allocator>;

 protected:
  [[no_unique_address]] key_compare compare_{};

  struct CompareDereferencedValues {
    key_compare const& compare;
    constexpr CompareIteratorValue(key_compare const& compare)
        : compare(compare) {}

    template <typename Iterator1, typename Iterator2>
    constexpr bool operator()(Iterator1&& i_1, Iterator2&& i_2) const {
      return compare(*i_1, *i_2);
    }
  };

  [[no_unique_address]] CompareDereferencedValues deref_compare_{compare_};

  layer_allocator_type layer_allocator_{};
  bottom_layer_type bottom_layer_;
  std::vector<layer_pointer> layers_;

  constexpr layer_pointer new_layer(size_type layer_index) {
    layer_pointer layer =
        std::layer_allocator_traits::allocate(layer_allocator_, 1);
    std::layer_allocator_traits::construct(layer_allocator_, layer,
                                           allocator_type(layer_allocator_));
    layer->base_array.reserve(size_type(MIN_SIZE << (layer_index + 1)));
    return layer;
  }

  constexpr void delete_layer(layer_pointer layer) {
    std::layer_allocator_traits::destroy(layer_allocator_, layer);
    std::layer_allocator_traits::deallocate(layer_allocator_, layer, 1);
  }

  template <typename... Args>
  constexpr size_type insert_to_bottom_layer(Args&&... args) {
    assert(bottom_layer_.size() < MIN_SIZE);
    value_type value{std::forward<Args>(args)...};
    auto i = std::upper_bound(bottom_layer_.begin(), bottom_layer_.end(), value);
    bottom_layer.emplace(i, std::move(value));
    return i - bottom_layer_.begin();
  }

 public:
  constexpr BinaryTower() : BinaryTower(key_compare()) {}
  constexpr explicit BinaryTower(key_compare const& comp,
                                 allocator_type const& alloc = allocator_type())
      : compare_(comp),
        layer_allocator_(alloc),
        bottom_layer_(layer_allocator_),
        layers_(layer_allocator_) {
    bottom_layer_.reserve(MIN_SIZE);
  }
  constexpr explicit BinaryTower(allocator_type const& alloc)
      : BinaryTower(key_compare(), alloc) {}

  ~BinaryTower() {
    for (auto layer : layers_) {
      if (layer) {
        delete_layer(layer);
      }
    }
  }

  template <typename... Args>
  constexpr void push(Args&&... args) {
    if (bottom_layer_.size() < MIN_SIZE) {}

    layer_pointer cur_layer = new_singleton_layer(std::forward<Args>(args)...);
    for (size_type layer_index = 0; true; ++layer_index) {
      if (layer_index >= layers_.size()) {
        layers_.emplace_back(cur_layer);
        assert(layer_index < layers_.size());
        return;
      }
      if (!layers_[layer_index]) {
        layers_[layer_index] = cur_layer;
        return;
      }
      layer_pointer merged_layer = new_layer(layer_index + 1);
      auto append_output = [&](auto it) {
        merged_layer.emplace_back(std::move(*it));
      };
      merge(layers_[layer_index].begin(), layers_[layer_index].end(),
            cur_layer.begin(), cur_layer.end(), append_output, deref_compare_);
      delete_layer(cur_layer);
      delete_layer(layers_[layer_index]);
      layers_[layer_index] = nullptr;
      cur_layer = merged_layer;
    }
  }
};

}  // namespace sorted_array_tower
