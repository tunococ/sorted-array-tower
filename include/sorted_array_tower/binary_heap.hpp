#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

namespace sorted_array_tower {

template <typename T, typename SizeType = std::size_t>
struct BinaryHeapEntry {
  using size_type = SizeType;
  using value_type = T;
  using this_type = BinaryHeapEntry;

  size_type id;
  value_type value;

  BinaryHeapEntry(BinaryHeapEntry const&) = default;
  BinaryHeapEntry(BinaryHeapEntry&&) = default;

  BinaryHeapEntry& operator=(BinaryHeapEntry const&) = default;
  BinaryHeapEntry& operator=(BinaryHeapEntry&&) = default;

  template <typename... Args>
  BinaryHeapEntry(size_type id, Args... args)
      : id(id), value(std::forward<Args>(args)...) {}
};

/**
 * @breif Min-heap that doesn't support deletion and has a fixed capacity.
 *
 * This heap supports adding and modifying values, but not deleting them.
 * Deletion can be simulated by setting the value to the largest value.
 *
 * An insertion method (`emplace` or `push`) returns an id, which can be used
 * in other operations such as `operator[]`, `increase_key`, `decrease_key`,
 * and `modify_key`.
 */
template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<BinaryHeapEntry<T>>>
class BinaryHeap {
 public:
  using value_type = T;
  using key_type = T;
  using value_compare = Compare;
  using key_compare = Compare;
  using size_type = std::allocator_traits<Allocator>::size_type;
  using entry_type = BinaryHeapEntry<value_type, size_type>;
  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<entry_type>;
  using index_allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<size_type>;
  using reference = value_type&;
  using const_reference = value_type const&;

 private:
  [[no_unique_address]] key_compare compare_;
  std::vector<entry_type, allocator_type> entries_;
  std::vector<size_type, index_allocator_type> id_to_index_;
  constexpr static size_type INVALID_INDEX =
      std::numeric_limits<size_type>::max();

  constexpr size_type get_index(size_type id) const {
    assert(id < id_to_index_.size());
    assert(id_to_index_[id] < entries_.size());
    return id_to_index_[id];
  }

  template <typename U>
  constexpr void increase_key_at_index(size_type index, U&& value) {
    if (compare_(value, entries_[index].value)) {
      return;
    }
    entries_[index].value = std::forward<U>(value);
    bubble_down(index);
    return;
  }

  template <typename U>
  constexpr void decrease_key_at_index(size_type index, U&& value) {
    if (compare_(entries_[index].value, value)) {
      return;
    }
    entries_[index].value = std::forward<U>(value);
    bubble_up(index);
    return;
  }

  template <typename U>
  constexpr void modify_key_at_index(size_type index, U&& value) {
    if (compare_(entries_[index].value, value)) {
      entries_[index].value = std::forward<U>(value);
      bubble_down(index);
      return;
    }
    entries_[index].value = std::forward<U>(value);
    bubble_up(index);
  }

 public:
  BinaryHeap() : BinaryHeap(key_compare()) {}
  explicit BinaryHeap(key_compare const& comp,
                      Allocator const& alloc = Allocator())
      : compare_(comp),
        entries_(allocator_type(alloc)),
        id_to_index_(index_allocator_type(alloc)) {}
  explicit BinaryHeap(Allocator const& alloc)
      : compare_(key_compare()),
        entries_(allocator_type(alloc)),
        id_to_index_(index_allocator_type(alloc)) {}

  BinaryHeap(BinaryHeap const&) = default;
  BinaryHeap(BinaryHeap&&) = default;

  BinaryHeap& operator=(BinaryHeap const&) = default;
  BinaryHeap& operator=(BinaryHeap&&) = default;

  constexpr size_type size() const noexcept {
    return entries_.size();
  }

  constexpr bool empty() const noexcept {
    return entries_.empty();
  }

  constexpr void reserve(size_type capacity) {
    entries_.reserve(capacity);
    id_to_index_.reserve(capacity);
  }

  constexpr void bubble_up(size_type index) {
    if (index == 0) {
      return;
    }
    size_type parent = (index - 1) / 2;
    entry_type& parent_entry = entries_[parent];
    if (!compare_(entries_[index].value, parent_entry.value)) {
      return;
    }

    // Save the starting entry.
    value_type value = std::move(entries_[index].value);
    size_type id = entries_[index].id;

    // Shift the parent entry down.
    entries_[index].value = std::move(parent_entry.value);
    entries_[index].id = parent_entry.id;
    id_to_index_[parent_entry.id] = index;

    while (true) {
      index = parent;
      if (index == 0) {
        break;
      }
      parent = (index - 1) / 2;
      entry_type& parent_entry = entries_[parent];
      if (!compare_(value, parent_entry.value)) {
        break;
      }

      // Shift the parent entry down.
      entries_[index].value = std::move(parent_entry.value);
      entries_[index].id = parent_entry.id;
      id_to_index_[parent_entry.id] = index;
    }

    // Put in the starting entry.
    entries_[index].value = std::move(value);
    entries_[index].id = id;
    id_to_index_[id] = index;
  }

  constexpr void bubble_down(size_type index) {
    size_type child = index * 2 + 1;
    if (child >= entries_.size()) {
      return;
    }
    if (child + 1 < entries_.size() &&
        compare_(entries_[child + 1].value, entries_[child].value)) {
      ++child;
    }
    entry_type& child_entry = entries_[child];
    if (!compare_(child_entry.value, entries_[index].value)) {
      return;
    }

    // Save the starting entry.
    value_type value = std::move(entries_[index].value);
    size_type id = entries_[index].id;

    // Shift the child entry up.
    entries_[index].value = std::move(child_entry.value);
    entries_[index].id = child_entry.id;
    id_to_index_[child_entry.id] = index;

    while (true) {
      index = child;
      child = index * 2 + 1;
      if (child >= entries_.size()) {
        break;
      }
      if (child + 1 < entries_.size() &&
          compare_(entries_[child + 1].value, entries_[child].value)) {
        ++child;
      }
      entry_type& child_entry = entries_[child];
      if (!compare_(child_entry.value, value)) {
        break;
      }

      // Shift the child entry up.
      entries_[index].value = std::move(child_entry.value);
      entries_[index].id = child_entry.id;
      id_to_index_[child_entry.id] = index;
    }

    // Put in the starting entry.
    entries_[index].value = std::move(value);
    entries_[index].id = id;
    id_to_index_[id] = index;
  }

  template <typename... Args>
  constexpr size_type emplace(Args... args) {
    size_type id = entries_.size();
    entries_.emplace_back(id, std::forward<Args>(args)...);
    while (id_to_index_.size() <= id) {
      id_to_index_.emplace_back(INVALID_INDEX);
    }
    id_to_index_[id] = id;
    bubble_up(id);
    return id;
  }

  template <typename U>
  constexpr size_type push(U&& u) {
    return emplace(std::forward<U>(u));
  }

  constexpr const_reference top() const {
    assert(!entries_.empty());
    return entries_[0].value;
  }

  constexpr reference top() {
    assert(!entries_.empty());
    return entries_[0].value;
  }

  constexpr size_type top_id() const {
    return entries_[0].id;
  }

  constexpr const_reference at(size_type id) const {
    return entries_[get_index(id)].value;
  }

  constexpr reference at(size_type id) {
    return entries_[get_index(id)].value;
  }

  constexpr const_reference operator[](size_type id) const {
    return at(id);
  }

  constexpr reference operator[](size_type id) {
    return at(id);
  }

  template <typename U>
  constexpr void increase_key(size_type id, U&& value) {
    increase_key_at_index(get_index(id), std::forward<U>(value));
    return;
  }

  template <typename U>
  constexpr void decrease_key(size_type id, U&& value) {
    decrease_key_at_index(get_index(id), std::forward<U>(value));
    return;
  }

  template <typename U>
  constexpr void modify_key(size_type id, U&& value) {
    size_type index = get_index(id);
    modify_key_at_index(index, std::forward<U>(value));
  }

  template <typename U>
  constexpr void increase_top_key(U&& value) {
    increase_key_at_index(0, std::forward<U>(value));
    return;
  }

  template <typename U>
  constexpr void decrease_top_key(U&& value) {
    decrease_key_at_index(0, std::forward<U>(value));
    return;
  }

  template <typename U>
  constexpr void modify_top_key(U&& value) {
    modify_key_at_index(0, std::forward<U>(value));
  }
};

}  // namespace sorted_array_tower