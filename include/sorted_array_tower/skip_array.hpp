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
 * @brief A displacement for a skip array cell.
 *
 * A SkipArray is an array where each cell may or may not contain a value.
 * If it does not contain a value, it instead contains a
 * `SkipArrayDisplacement`, which stores the number of cells to skip to reach
 * the next and previous cells that *may* contain a value.
 *
 * This `SkipArrayDisplacement` is a two-element array. The first element is
 * the number of cells to skip to reach the next potentially non-empty cell.
 * The second element is the number of cells to skip to reach the previous
 * potentially non-empty cell.
 *
 * @tparam SizeType The type used for the displacement values. Defaults to
 *   `std::size_t`.
 */
template <typename SizeType = std::size_t>
using SkipArrayDisplacement = std::array<SizeType, 2>;

template <typename T, typename SizeType = std::size_t>
using SkipArrayCell = std::variant<T, SkipArrayDisplacement<SizeType>>;

template <typename T, typename Allocator = std::allocator<T>,
          template <typename, typename> class BaseArray = std::vector>
class SkipArray {
 public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = value_type const&;
  using this_type = SkipArray<value_type, Allocator, BaseArray>;

  using cell_type = SkipArrayCell<T>;
  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<cell_type>;

  using size_type = std::allocator_traits<allocator_type>::size_type;
  using displacement_type = SkipArrayDisplacement<size_type>;
  using difference_type =
      std::allocator_traits<allocator_type>::difference_type;
  using pointer = std::allocator_traits<allocator_type>::pointer;
  using const_pointer = std::allocator_traits<allocator_type>::const_pointer;

  using array_type = BaseArray<cell_type, allocator_type>;

 private:
  allocator_type allocator_;
  mutable array_type base_array_;
  mutable size_type front_index_;
  size_type size_;

  constexpr bool is_valid_index(size_type index) const {
    return index < capacity() && base_array_[index].index() == 0;
  }

  constexpr size_type first_valid_right_index(size_type index) const {
    assert(index <= capacity());
    displacement_type* disp;
    displacement_type* next_disp;

    if (index >= capacity() || !(disp = std::get_if<1>(&base_array_[index]))) {
      return index;
    }
    while (true) {
      index += (*disp)[0];
      if (index >= capacity() ||
          !(next_disp = std::get_if<1>(&base_array_[index]))) {
        return index;
      }
      // Use path-halving.
      (*disp)[0] += (*next_disp)[0];
      disp = next_disp;
    }
  }

  constexpr size_type first_valid_left_index(size_type index) const {
    assert(index >= front_index_);
    displacement_type* disp;
    displacement_type* next_disp;

    if (index >= capacity() || !(disp = std::get_if<1>(&base_array_[index]))) {
      return index;
    }
    while (true) {
      index -= (*disp)[1];
      if (index >= capacity() ||
          !(next_disp = std::get_if<1>(&base_array_[index]))) {
        return index;
      }
      // Use path-halving.
      (*disp)[1] += (*next_disp)[1];
      disp = next_disp;
    }
  }

  constexpr size_type back_index() const {
    assert(size_ > 0);
    return first_valid_left_index(capacity() - 1);
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr size_type lower_bound_index(size_type left, size_type right,
                                        Key const& key,
                                        Compare comp = Compare()) const {
    if (left < front_index_) {
      left = front_index_;
    }
    while (true) {
      if (left == right) {
        return first_valid_right_index(left);
      }
      assert(left < right);
      size_type mid = left + (right - left) / 2;
      size_type mid_value_index = first_valid_left_index(mid);
      if (comp(std::get<0>(base_array_[mid_value_index]), key)) {
        left = first_valid_right_index(mid + 1);
      } else {
        right = mid_value_index;
      }
    }
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr size_type upper_bound_index(size_type left, size_type right,
                                        Key const& key,
                                        Compare comp = Compare()) const {
    if (left < front_index_) {
      left = front_index_;
    }
    while (true) {
      if (left == right) {
        return first_valid_right_index(left);
      }
      assert(left < right);
      size_type mid = left + (right - left) / 2;
      size_type mid_value_index = first_valid_left_index(mid);
      if (comp(key, std::get<0>(base_array_[mid_value_index]))) {
        right = mid_value_index;
      } else {
        left = first_valid_right_index(mid + 1);
      }
    }
  }

  template <bool IsConst>
  struct Iterator {
    size_type index_;
    SkipArray const* skip_array_;

    using iterator_category = std::bidirectional_iterator_tag;
    using iterator_concept = std::bidirectional_iterator_tag;
    using value_type =
        std::conditional_t<IsConst, typename SkipArray::value_type const,
                           typename SkipArray::value_type>;
    using difference_type = typename SkipArray::difference_type;
    using pointer =
        std::conditional_t<IsConst, typename SkipArray::const_pointer,
                           typename SkipArray::pointer>;
    using reference =
        std::conditional_t<IsConst, typename SkipArray::const_reference,
                           typename SkipArray::reference>;

    Iterator(Iterator const&) = default;
    Iterator(Iterator&&) = default;
    Iterator(Iterator<false> const& other) noexcept(noexcept(size_type(index_)))
      requires(IsConst)
        : index_(other.index_), skip_array_(other.skip_array_) {}
    Iterator(Iterator<false>&& other) noexcept(
        noexcept(size_type(std::move(index_))))
      requires(IsConst)
        : index_(std::move(other.index_)), skip_array_(other.skip_array_) {}

    Iterator(SkipArray const* skip_array,
             size_type index) noexcept(noexcept(size_type(index_)))
        : index_(index), skip_array_(skip_array) {}

    constexpr bool is_valid() const {
      return skip_array_->is_valid_index(index_);
    }

    constexpr reference operator*() const {
      assert(is_valid());
      return std::get<0>(skip_array_->base_array_[index_]);
    }

    constexpr pointer operator->() const {
      return &(*this);
    }

    constexpr Iterator& operator++() {
      assert(index_ < skip_array_->capacity());
      index_ = skip_array_->first_valid_right_index(index_ + 1);
      return *this;
    }

    constexpr Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    constexpr Iterator& operator--() {
      assert(index_ > skip_array_->front_index_);
      index_ = skip_array_->first_valid_left_index(index_ - 1);
      return *this;
    }

    constexpr Iterator operator--(int) {
      Iterator tmp = *this;
      --(*this);
      return tmp;
    }

    constexpr bool operator==(Iterator const& other) const
        noexcept(noexcept(index_ == index_)) = default;

    constexpr std::strong_ordering operator<=>(Iterator const& other) const
        noexcept(noexcept(index_ <=> index_)) = default;
  };

  // An input iterator transformer that takes a value and wraps it in a
  // cell_type.
  template <typename InputIterator>
  struct InputCellIterator {
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = cell_type;
    using difference_type = array_type::difference_type;
    using pointer = array_type::pointer;
    using reference = cell_type;

    InputIterator it_;

    constexpr InputCellIterator() noexcept(noexcept(InputIterator())) = default;
    constexpr InputCellIterator(InputIterator it) noexcept(
        noexcept(InputIterator(it_)))
        : it_(it) {}

    constexpr reference operator*() const
        noexcept(noexcept(cell_type(std::in_place_index<0>, *it_))) {
      return cell_type(std::in_place_index<0>, *it_);
    }

    constexpr InputCellIterator& operator++() noexcept(noexcept(++it_)) {
      ++it_;
      return *this;
    }

    constexpr InputCellIterator operator++(int) noexcept(
        noexcept(InputCellIterator(*this)) && noexcept(++(*this))) {
      InputCellIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    constexpr bool operator==(InputCellIterator const&) const = default;
  };

 public:
  constexpr SkipArray() noexcept(noexcept(array_type(allocator_)) &&
                                 noexcept(allocator_type()))
      : allocator_(allocator_type()),
        base_array_(allocator_),
        front_index_(0),
        size_(0) {}

  constexpr SkipArray(SkipArray const&) noexcept(
      noexcept(array_type(base_array_)) &&
      noexcept(allocator_type())) = default;

  constexpr SkipArray(SkipArray&& other) noexcept(
      noexcept(array_type(std::move(base_array_))) &&
      noexcept(allocator_type(std::move(allocator_))))
      : allocator_(std::move(other.allocator_)),
        base_array_(std::move(other.base_array_), allocator_),
        front_index_(other.front_index_),
        size_(other.size_) {
    other.base_array_.clear();
    other.front_index_ = 0;
    other.size_ = 0;
  }

  constexpr SkipArray& operator=(SkipArray const& other) noexcept(
      noexcept(base_array_ = base_array_)) {
    base_array_ = other.base_array_;
    front_index_ = other.front_index_;
    size_ = other.size_;
    return *this;
  }

  constexpr SkipArray& operator=(SkipArray&& other) noexcept(
      noexcept(base_array_ = std::move(base_array_))) {
    base_array_ = std::move(other.base_array_);
    front_index_ = other.front_index_;
    size_ = other.size_;
    other.base_array_.clear();
    other.front_index_ = 0;
    other.size_ = 0;
    return *this;
  }

  constexpr explicit SkipArray(Allocator const& allocator) noexcept(
      noexcept(array_type(allocator_)) && noexcept(allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(allocator_),
        front_index_(0),
        size_(0) {}

  constexpr explicit SkipArray(
      size_type count,
      Allocator const& allocator =
          Allocator()) noexcept(noexcept(base_array_, count,
                                         cell_type(std::in_place_index<0>,
                                                   value_type())) &&
                                noexcept(allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(count, cell_type(std::in_place_index<0>, value_type()),
                    allocator_),
        front_index_(0),
        size_(count) {}

  constexpr SkipArray(
      size_type count, value_type const& value,
      Allocator const& allocator =
          Allocator()) noexcept(noexcept(base_array_, count,
                                         cell_type(std::in_place_index<0>,
                                                   value)) &&
                                noexcept(allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(count, cell_type(std::in_place_index<0>, value),
                    allocator_),
        front_index_(0),
        size_(count) {}

  template <typename InputIterator>
  constexpr SkipArray(
      InputIterator first, InputIterator last,
      Allocator const& allocator =
          Allocator()) noexcept(noexcept(array_type(InputCellIterator(first),
                                                    InputCellIterator(last),
                                                    allocator_)) &&
                                noexcept(allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(InputCellIterator(first), InputCellIterator(last),
                    allocator_),
        front_index_(0),
        size_(capacity()) {}

  constexpr SkipArray(
      SkipArray const& other,
      std::type_identity_t<Allocator> const&
          allocator) noexcept(noexcept(array_type(base_array_, allocator_)) &&
                              noexcept(allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(other.base_array_, allocator),
        front_index_(other.front_index_),
        size_(other.size_) {}

  constexpr SkipArray(
      SkipArray&& other,
      std::type_identity_t<Allocator> const&
          allocator) noexcept(noexcept(array_type(std::move(base_array_),
                                                  allocator_)) &&
                              noexcept(allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(std::move(other.base_array_), allocator),
        front_index_(other.front_index_),
        size_(other.size_) {
    other.base_array_.clear();
    other.front_index_ = 0;
    other.size_ = 0;
  }

  constexpr SkipArray(std::initializer_list<value_type> init,
                      std::type_identity_t<Allocator> const& allocator =
                          Allocator()) noexcept(noexcept(SkipArray(init.begin(),
                                                                   init.end(),
                                                                   allocator_)))
      : SkipArray(init.begin(), init.end(), allocator) {}

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using reverse_const_iterator = std::reverse_iterator<const_iterator>;

  constexpr size_type capacity() const noexcept(noexcept(base_array_.size())) {
    return base_array_.size();
  }

  constexpr size_type size() const noexcept {
    return size_;
  }

  constexpr bool empty() const noexcept {
    return size_ == 0;
  }

  constexpr void assign(size_type count, value_type const& value) noexcept(
      noexcept(base_array_.assign(count,
                                  cell_type(std::in_place_index<0>, value)))) {
    base_array_.assign(count, cell_type(std::in_place_index<0>, value));
    size_ = count;
    front_index_ = 0;
  }

  template <typename InputIterator>
  constexpr void assign(InputIterator first, InputIterator last) noexcept(
      noexcept(base_array_.assign(InputCellIterator(first),
                                  InputCellIterator(last))) &&
      noexcept(capacity())) {
    base_array_.assign(InputCellIterator(first), InputCellIterator(last));
    size_ = capacity();
    front_index_ = 0;
  }

  constexpr void assign(std::initializer_list<value_type> init) noexcept(
      noexcept(assign(init.begin(), init.end()))) {
    assign(init.begin(), init.end());
  }

  constexpr allocator_type get_allocator() const noexcept {
    return allocator_;
  }

  constexpr iterator begin() noexcept(noexcept(iterator(this, front_index_))) {
    return iterator(this, front_index_);
  }

  constexpr const_iterator begin() const noexcept(noexcept(cbegin())) {
    return cbegin();
  }

  constexpr const_iterator cbegin() const
      noexcept(noexcept(const_iterator(this, front_index_))) {
    return const_iterator(this, front_index_);
  }

  constexpr iterator end() noexcept(noexcept(iterator(this, capacity()))) {
    return iterator(this, capacity());
  }

  constexpr const_iterator end() const noexcept(noexcept(cend())) {
    return cend();
  }

  constexpr const_iterator cend() const
      noexcept(noexcept(const_iterator(this, capacity()))) {
    return const_iterator(this, capacity());
  }

  constexpr iterator rbegin() noexcept(
      noexcept(std::make_reverse_iterator(end()))) {
    return std::make_reverse_iterator(end());
  }

  constexpr const_iterator rbegin() const noexcept(noexcept(crbegin())) {
    return crbegin();
  }

  constexpr const_iterator crbegin() const
      noexcept(noexcept(std::make_reverse_iterator(cend()))) {
    return std::make_reverse_iterator(cend());
  }

  constexpr iterator rend() noexcept(
      noexcept(std::make_reverse_iterator(begin()))) {
    return std::make_reverse_iterator(begin());
  }

  constexpr const_iterator rend() const noexcept(noexcept(crend())) {
    return crend();
  }

  constexpr const_iterator crend() const
      noexcept(noexcept(std::make_reverse_iterator(cbegin()))) {
    return std::make_reverse_iterator(cbegin());
  }

  constexpr reference front() {
    assert(is_valid_index(front_index_));
    return std::get<0>(base_array_[front_index_]);
  }

  constexpr const_reference front() const {
    assert(is_valid_index(front_index_));
    return std::get<0>(base_array_[front_index_]);
  }

  constexpr reference back() {
    assert(is_valid_index(back_index()));
    return std::get<0>(base_array_[back_index()]);
  }

  constexpr const_reference back() const {
    assert(is_valid_index(back_index()));
    return std::get<0>(base_array_[back_index()]);
  }

  template <typename... Args>
  constexpr reference emplace_front(Args&&... args)
    requires requires {
      std::declval<array_type>().emplace_front(std::declval<cell_type>());
    }
  {
    cell_type& cell = base_array_.emplace_front(std::in_place_index<0>,
                                                std::forward<Args>(args)...);
    ++size_;
    front_index_ = 0;
    return std::get<0>(cell);
  }

  constexpr void push_front(value_type const& value)
    requires requires {
      std::declval<array_type>().emplace_front(std::declval<cell_type>());
    }
  {
    emplace_front(value);
  }

  constexpr void push_front(value_type&& value)
    requires requires {
      std::declval<array_type>().emplace_front(std::declval<cell_type>());
    }
  {
    emplace_front(std::move(value));
  }

  template <typename... Args>
  constexpr reference emplace_back(Args&&... args) {
    cell_type& cell = base_array_.emplace_back(std::in_place_index<0>,
                                               std::forward<Args>(args)...);
    ++size_;
    return std::get<0>(cell);
  }

  constexpr void push_back(value_type const& value) {
    emplace_back(value);
  }

  constexpr void push_back(value_type&& value) {
    emplace_back(std::move(value));
  }

  constexpr iterator erase(const_iterator pos) {
    assert(pos.is_valid());
    size_type index = pos.index_;
    base_array_[index] = SkipArrayDisplacement<size_type>{1, 1};
    --size_;
    size_type next_index = first_valid_right_index(index + 1);
    if (index == front_index_) {
      front_index_ = next_index;
    }
    return iterator(this, next_index);
  }

  constexpr void pop_front() {
    erase(begin());
  }

  constexpr void pop_back() {
    erase(--end());
  }

  constexpr void clear() {
    size_ = 0;
    front_index_ = 0;
    base_array_.resize(0);
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr iterator lower_bound(Key const& key, Compare comp = Compare()) {
    return iterator(this, lower_bound_index(0, capacity(), key,
                                            std::forward<Compare>(comp)));
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr const_iterator lower_bound(Key const& key,
                                       Compare comp = Compare()) const {
    return const_iterator(this, lower_bound_index(0, capacity(), key,
                                                  std::forward<Compare>(comp)));
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr iterator upper_bound(Key const& key, Compare comp = Compare()) {
    return iterator(this, upper_bound_index(0, capacity(), key,
                                            std::forward<Compare>(comp)));
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr const_iterator upper_bound(Key const& key,
                                       Compare comp = Compare()) const {
    return const_iterator(this, upper_bound_index(0, capacity(), key,
                                                  std::forward<Compare>(comp)));
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr std::pair<iterator, iterator> equal_range(
      Key const& key, Compare comp = Compare()) {
    return std::make_pair(lower_bound(key, comp), upper_bound(key, comp));
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr std::pair<const_iterator, const_iterator> equal_range(
      Key const& key, Compare comp = Compare()) const {
    return std::make_pair(lower_bound(key, comp), upper_bound(key, comp));
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr size_type count(Key const& key, Compare comp = Compare()) const {
    size_type matches = 0;
    for (auto l = lower_bound(key, comp), u = upper_bound(key, comp); l != u;
         ++l) {
      ++matches;
    };
    return matches;
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr iterator find(Key const& key, Compare comp = Compare()) {
    auto l = lower_bound(key, comp);
    if (l == end() || comp(key, *l)) {
      return end();
    }
    return l;
  }

  template <typename Key, typename Compare = std::less<value_type>>
  constexpr const_iterator find(Key const& key,
                                Compare comp = Compare()) const {
    auto l = lower_bound(key, comp);
    if (l == end() || comp(key, *l)) {
      return end();
    }
    return l;
  }

  constexpr bool operator==(std::ranges::range auto const& other) const {
    return std::equal(begin(), end(), other.begin());
  }

  constexpr std::strong_ordering operator<=>(
      std::ranges::range auto const& other) const {
    return std::lexicographical_compare_three_way(begin(), end(), other.begin(),
                                                  other.end());
  }
};

}  // namespace sorted_array_tower
