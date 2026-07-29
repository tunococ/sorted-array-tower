#pragma once

#include <iterator>
#include <memory>
#include <vector>
#include <utility>

#include "binary_heap.hpp"

namespace sorted_array_tower {

template <typename InputList, typename OutputList, typename Compare>
void merge(InputList&& list_1, InputList&& list_2, OutputList& output,
           Compare& compare) {
  auto i_1 = list_1.begin();
  auto i_2 = list_2.begin();

  constexpr static auto push =
      [](OutputList& o, auto&& i) {
        if constexpr (std::is_rvalue_reference_v<InputList>) {
          o.emplace_back(std::move(*i));
        } else {
          o.emplace_back(*i);
        }
      };

  while (true) {
    if (i_1 == list_1.end()) {
      for (; i_2 != list_2.end(); ++i_2) {
        push(output, *i_2);
      }
      break;
    } else if (i_2 == list_2.end()) {
      for (; i_1 != list_1.end(); ++i_1) {
        push(output, *i_1);
      }
      break;
    }
    if (compare(*i_2, *i_1)) {
      push(output, *i_2);
      ++i_2;
    } else {
      push(output, *i_1);
      ++i_1;
    }
  }
}

/**
 * @brief An iterator that represents a multi-way merged list.
 *
 * This iterator takes as input a list of (begin, end) pairs of iterators of
 * sorted lists, and presents an iterator that iterates over the merged list.
 */
template <typename Iterator,
          typename Compare =
              std::less<typename std::iterator_traits<Iterator>::value_type>,
          typename Allocator = std::allocator<Iterator>>
class MergeIterator {
 public:
  using iterator = Iterator;
  using iterator_traits = std::iterator_traits<iterator>;
  using difference_type = iterator_traits::difference_type;
  using value_type = iterator_traits::value_type;
  using pointer = iterator_traits::pointer;
  using reference = iterator_traits::reference;
  using iterator_category = iterator_traits::iterator_category;

  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<iterator>;
  using size_type = std::allocator_traits<allocator_type>::size_type;
  using key_compare = Compare;
  using value_compare = key_compare;

  using location_type = std::pair<size_type, iterator>;

 protected:
  [[no_unique_address]] key_compare compare_;

  struct IteratorTriple {
    iterator begin;
    iterator end;
    iterator current;

    constexpr bool at_begin() const {
      return begin == current;
    }

    constexpr bool at_end() const {
      return end == current;
    }

    constexpr IteratorTriple() = default;
    constexpr IteratorTriple(iterator begin, iterator end, iterator current)
        : begin(begin), end(end), current(current) {}
    constexpr IteratorTriple(const IteratorTriple&) = default;
    constexpr IteratorTriple(IteratorTriple&&) = default;
    constexpr IteratorTriple& operator=(const IteratorTriple&) = default;
    constexpr IteratorTriple& operator=(IteratorTriple&&) = default;
  };

  using IteratorTripleAllocator =
      std::allocator_traits<Allocator>::template rebind_alloc<IteratorTriple>;

  std::vector<IteratorTriple, IteratorTripleAllocator> triples_;

  using ThisPointer =
      std::pointer_traits<pointer>::template rebind<MergeIterator>;

  struct CompareIndices {
    ThisPointer merge_iterator;

    constexpr bool operator()(size_type index_1, size_type index_2) const {
      IteratorTriple const& triple_1 = merge_iterator->triples_[index_1];
      IteratorTriple const& triple_2 = merge_iterator->triples_[index_2];
      if (triple_1.at_end()) {
        return false;
      }
      if (triple_2.at_end()) {
        return true;
      }
      if (merge_iterator->compare_(*triple_1.current, *triple_2.current)) {
        return true;
      }
      if (merge_iterator->compare_(*triple_2.current, *triple_1.current)) {
        return false;
      }
      return index_1 < index_2;
    }
  };

  BinaryHeap<size_type, CompareIndices> forward_heap_;

  template <typename Triples>
  constexpr MergeIterator(MergeIterator const& other, Triples&& triples)
      : compare_(other.compare_),
        triples_(std::forward<Triples>(triples)),
        forward_heap_(CompareIndices(this),
                      other.forward_heap_.get_allocator()) {
    forward_heap_.reserve(triples_.size());
    for (size_type i = 0; i < triples_.size(); ++i) {
      forward_heap_.emplace(i);
    }
  }

 public:
  constexpr MergeIterator() : MergeIterator(Allocator()) {}
  constexpr explicit MergeIterator(Compare const& comp,
                                   Allocator const& alloc = Allocator())
      : compare_(comp),
        triples_(IteratorTripleAllocator(alloc)),
        forward_heap_(CompareIndices(this), IteratorTripleAllocator(alloc)) {}
  constexpr explicit MergeIterator(Allocator const& alloc)
      : MergeIterator(Compare(), alloc) {}

  constexpr MergeIterator(MergeIterator const& other)
      : compare_(other.compare_),
        triples_(other.triples_),
        forward_heap_(other.forward_heap_) {
    forward_heap_.set_comp(CompareIndices(this));
  }

  constexpr MergeIterator(MergeIterator&& other)
      : compare_(other.compare_),
        triples_(std::move(other.triples_)),
        forward_heap_(std::move(other.forward_heap_)) {
    other.triples_.clear();
    other.forward_heap_.clear();
    forward_heap_.set_comp(CompareIndices(this));
  }

  constexpr MergeIterator& operator=(MergeIterator const& other) {
    compare_ = other.compare_;
    triples_ = other.triples_;
    forward_heap_ = other.forward_heap_;
    forward_heap_.set_comp(CompareIndices(this));
    return *this;
  }

  constexpr MergeIterator& operator=(MergeIterator&& other) {
    compare_ = other.compare_;
    triples_ = std::move(other.triples_);
    forward_heap_ = std::move(other.forward_heap_);
    forward_heap_.set_comp(CompareIndices(this));
    other.triples_.clear();
    other.forward_heap_.clear();
    return *this;
  }

  constexpr void reserve(size_type capacity) {
    triples_.reserve(capacity);
    forward_heap_.reserve(capacity);
  }

  constexpr size_type add_list(iterator begin, iterator end,
                               iterator current) {
    size_type index = triples_.size();
    triples_.emplace_back(begin, end, current);
    forward_heap_.emplace(index);
    return index;
  }

  constexpr size_type add_list(iterator begin, iterator end) {
    return add_list(begin, end, begin);
  }

  constexpr bool is_past_end() const {
    if (forward_heap_.empty()) {
      return true;
    }
    size_type index = forward_heap_.top();
    return triples_[index].at_end();
  }

  constexpr MergeIterator& move_to_begin() {
    for (auto& triple: triples_) {
      triple.current = triple.begin;
    }
    forward_heap_.build_heap();
    return *this;
  }

  constexpr MergeIterator make_begin() const {
    MergeIterator output = *this;
    output.move_to_begin();
    return output;
  }

  constexpr MergeIterator& move_to_end() {
    for (auto& triple: triples_) {
      triple.current = triple.end;
    }
    forward_heap_.build_heap();
    return *this;
  }

  constexpr MergeIterator make_end() const {
    MergeIterator output = *this;
    output.move_to_end();
    return output;
  }

  constexpr location_type location() const {
    size_type index = forward_heap_.top();
    return std::make_pair(index, triples_[index].current);
  }

  constexpr reference operator*() const {
    return *triples_[forward_heap_.top()].current;
  }

  constexpr MergeIterator& operator++() {
    size_type index = forward_heap_.top();
    ++triples_[index].current;
    forward_heap_.increase_top_key();
    return *this;
  }

  constexpr MergeIterator operator++(int) {
    MergeIterator old = *this;
    operator++();
    return old;
  }

  constexpr bool operator==(MergeIterator const& other) const {
    if (is_past_end()) {
      return other.is_past_end();
    }
    if (other.is_past_end()) {
      return false;
    }
    return location() == other.location();
  }

};

/**
 * @brief A bidirectional iterator that represents a multi-way merged list.
 *
 * This iterator extends MergeIterator with support for operator--, allowing
 * backward traversal of the merged sequence.
 */
template <typename Iterator,
          typename Compare =
              std::less<typename std::iterator_traits<Iterator>::value_type>,
          typename Allocator = std::allocator<Iterator>>
class BidirectionalMergeIterator : public MergeIterator<Iterator, Compare, Allocator> {
 public:
  using Base = MergeIterator<Iterator, Compare, Allocator>;
  using iterator = Iterator;
  using iterator_traits = std::iterator_traits<iterator>;
  using difference_type = iterator_traits::difference_type;
  using value_type = iterator_traits::value_type;
  using pointer = iterator_traits::pointer;
  using reference = iterator_traits::reference;
  using iterator_category = iterator_traits::iterator_category;

  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<iterator>;
  using size_type = std::allocator_traits<allocator_type>::size_type;
  using key_compare = Compare;
  using value_compare = key_compare;

  using location_type = std::pair<size_type, iterator>;

 protected:
  using Base::triples_;
  using Base::compare_;
  using Base::forward_heap_;

  using IteratorTriple = typename Base::IteratorTriple;

  using ThisPointer =
      std::pointer_traits<pointer>::template rebind<BidirectionalMergeIterator>;

  struct BackwardCompareIndices {
    ThisPointer merge_iterator;

    constexpr bool operator()(size_type index_1, size_type index_2) const {
      IteratorTriple const& triple_1 = merge_iterator->triples_[index_1];
      IteratorTriple const& triple_2 = merge_iterator->triples_[index_2];
      if (triple_1.at_begin()) {
        return false;
      }
      if (triple_2.at_begin()) {
        return true;
      }
      auto prev_1 = std::prev(triple_1.current);
      auto prev_2 = std::prev(triple_2.current);
      if (merge_iterator->compare_(*prev_2, *prev_1)) {
        return true;
      }
      if (merge_iterator->compare_(*prev_1, *prev_2)) {
        return false;
      }
      return index_2 < index_1;
    }
  };

  BinaryHeap<size_type, BackwardCompareIndices> backward_heap_;

 public:
  constexpr BidirectionalMergeIterator()
      : BidirectionalMergeIterator(Allocator()) {}
  constexpr explicit BidirectionalMergeIterator(
      Compare const& comp, Allocator const& alloc = Allocator())
      : Base(comp, alloc),
        backward_heap_(BackwardCompareIndices{this},
                       typename Base::IteratorTripleAllocator(alloc)) {
    for (size_type i = 0; i < triples_.size(); ++i) {
      backward_heap_.emplace(i);
    }
  }
  constexpr explicit BidirectionalMergeIterator(Allocator const& alloc)
      : BidirectionalMergeIterator(Compare(), alloc) {}

  constexpr BidirectionalMergeIterator(
      BidirectionalMergeIterator const& other)
      : Base(other),
        backward_heap_(BackwardCompareIndices{this},
                       other.backward_heap_.get_allocator()) {
    backward_heap_.reserve(triples_.size());
    for (size_type i = 0; i < triples_.size(); ++i) {
      backward_heap_.emplace(i);
    }
  }

  constexpr BidirectionalMergeIterator(
      BidirectionalMergeIterator&& other)
      : Base(std::move(other)),
        backward_heap_(std::move(other.backward_heap_)) {
    other.backward_heap_.clear();
    backward_heap_.set_comp(BackwardCompareIndices{this});
  }

  constexpr BidirectionalMergeIterator& operator=(
      BidirectionalMergeIterator const& other) {
    Base::operator=(other);
    backward_heap_ = other.backward_heap_;
    backward_heap_.set_comp(BackwardCompareIndices{this});
    return *this;
  }

  constexpr BidirectionalMergeIterator& operator=(
      BidirectionalMergeIterator&& other) {
    Base::operator=(std::move(other));
    backward_heap_ = std::move(other.backward_heap_);
    backward_heap_.set_comp(BackwardCompareIndices{this});
    other.backward_heap_.clear();
    return *this;
  }

  constexpr void reserve(size_type capacity) {
    Base::reserve(capacity);
    backward_heap_.reserve(capacity);
  }

  using Base::add_list;

  constexpr size_type add_list(iterator begin, iterator end,
                               iterator current) {
    size_type index = Base::add_list(begin, end, current);
    backward_heap_.emplace(index);
    return index;
  }

  constexpr size_type add_list(iterator begin, iterator end) {
    return add_list(begin, end, begin);
  }

  constexpr bool is_at_begin() const {
    if (backward_heap_.empty()) {
      return true;
    }
    size_type index = backward_heap_.top();
    return triples_[index].at_begin();
  }

  constexpr BidirectionalMergeIterator& move_to_begin() {
    Base::move_to_begin();
    backward_heap_.build_heap();
    return *this;
  }

  constexpr BidirectionalMergeIterator make_begin() const {
    BidirectionalMergeIterator output = *this;
    output.move_to_begin();
    return output;
  }

  constexpr BidirectionalMergeIterator& move_to_end() {
    Base::move_to_end();
    backward_heap_.build_heap();
    return *this;
  }

  constexpr BidirectionalMergeIterator make_end() const {
    BidirectionalMergeIterator output = *this;
    output.move_to_end();
    return output;
  }

  constexpr BidirectionalMergeIterator& operator++() {
    size_type index = forward_heap_.top();
    ++triples_[index].current;
    forward_heap_.increase_top_key();
    backward_heap_.modify_key(index);
    return *this;
  }

  constexpr BidirectionalMergeIterator operator++(int) {
    BidirectionalMergeIterator old = *this;
    operator++();
    return old;
  }

  constexpr BidirectionalMergeIterator& operator--() {
    size_type index = backward_heap_.top();
    --triples_[index].current;
    forward_heap_.modify_key(index);
    backward_heap_.modify_key(index);
    return *this;
  }

  constexpr BidirectionalMergeIterator operator--(int) {
    BidirectionalMergeIterator old = *this;
    operator--();
    return old;
  }
};

}  // namespace sorted_array_tower