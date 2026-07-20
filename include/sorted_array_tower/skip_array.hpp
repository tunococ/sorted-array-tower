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

/**
 * @brief A cell of a skip array.
 *
 * A skip array cell is either a stored value of type `T`, or a
 * `SkipArrayDisplacement` that records how far to jump to reach the next or
 * previous cell that may hold a value.
 *
 * @tparam T The value type stored in the array.
 * @tparam SizeType The type used for displacement values.
 */
template <typename T, typename SizeType = std::size_t>
using SkipArrayCell = std::variant<T, SkipArrayDisplacement<SizeType>>;

/**
 * @brief A sorted sequence container backed by a single underlying array.
 *
 * A `SkipArray` stores its elements in a contiguous `BaseArray` (which defaults
 * to `std::vector`). Some positions in the underlying array are left empty;
 * instead of a value they contain a `SkipArrayDisplacement` recording how many
 * cells to skip to reach the next and previous cells that may hold a value.
 * This keeps the elements in sorted order by their positions while still being
 * able to remove elements in amortized constant time: erasing simply marks a
 * cell as empty and patches up the surrounding displacements.
 *
 * The displacements are maintained lazily and compressed with path-halving,
 * giving fast bidirectional traversal and binary-search-like lookups that skip
 * over empty cells.
 *
 * @tparam T The type of the elements stored.
 * @tparam Allocator An allocator used to allocate cells.
 * @tparam BaseArray The underlying sequence container used to store cells. It
 *   must expose an interface compatible with `std::vector` (including, when
 *   `emplace_front` / `push_front` are used, a front-insertion operation).
 */
template <typename T, typename Allocator = std::allocator<T>,
          template <typename, typename> class BaseArray = std::vector>
class SkipArray {
 public:
  /// The type of the elements stored in the array.
  using value_type = T;
  /// A reference to an element.
  using reference = value_type&;
  /// A constant reference to an element.
  using const_reference = value_type const&;
  /// The type of this `SkipArray` specialization.
  using this_type = SkipArray<value_type, Allocator, BaseArray>;

  /// The type of a single cell (a value or a displacement).
  using cell_type = SkipArrayCell<T>;
  /// The allocator type used to allocate cells.
  using cell_allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<cell_type>;
  /// The allocator type used to allocate elements.
  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<value_type>;

  /// An unsigned integer type used for sizes and displacements.
  using size_type = std::allocator_traits<cell_allocator_type>::size_type;
  /// A two-element array storing forward and backward displacements.
  using displacement_type = SkipArrayDisplacement<size_type>;
  /// A signed integer type used for differences between iterators.
  using difference_type =
      std::allocator_traits<cell_allocator_type>::difference_type;
  /// A pointer to an element.
  using pointer = std::allocator_traits<allocator_type>::pointer;
  /// A constant pointer to an element.
  using const_pointer = std::allocator_traits<allocator_type>::const_pointer;

  /// The underlying container type that stores the cells.
  using array_type = BaseArray<cell_type, cell_allocator_type>;

 private:
  cell_allocator_type allocator_;
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

    constexpr Iterator() noexcept = default;
    constexpr Iterator(Iterator const&) = default;
    constexpr Iterator(Iterator&&) = default;
    constexpr Iterator& operator=(Iterator const&) = default;
    constexpr Iterator& operator=(Iterator&&) = default;
    constexpr Iterator(Iterator<false> const& other) noexcept(
        noexcept(size_type(index_)))
      requires(IsConst)
        : index_(other.index_), skip_array_(other.skip_array_) {}
    constexpr Iterator(Iterator<false>&& other) noexcept(
        noexcept(size_type(std::move(index_))))
      requires(IsConst)
        : index_(std::move(other.index_)), skip_array_(other.skip_array_) {}

    constexpr Iterator(SkipArray const* skip_array,
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
      return std::pointer_traits<pointer>::pointer_to(this->operator*());
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
                                 noexcept(cell_allocator_type()))
      : allocator_(cell_allocator_type()),
        base_array_(allocator_),
        front_index_(0),
        size_(0) {}

  /// @brief Copy-constructs a `SkipArray`, copying all cells and metadata.
  constexpr SkipArray(SkipArray const&) noexcept(
      noexcept(array_type(base_array_)) &&
      noexcept(cell_allocator_type())) = default;

  /// @brief Move-constructs a `SkipArray`, leaving the moved-from array empty.
  constexpr SkipArray(SkipArray&& other) noexcept(
      noexcept(array_type(std::move(base_array_))) &&
      noexcept(cell_allocator_type(std::move(allocator_))))
      : allocator_(std::move(other.allocator_)),
        base_array_(std::move(other.base_array_), allocator_),
        front_index_(other.front_index_),
        size_(other.size_) {
    other.base_array_.clear();
    other.front_index_ = 0;
    other.size_ = 0;
  }

  /// @brief Copy-assigns from `other`, replacing the contents of this array.
  constexpr SkipArray& operator=(SkipArray const& other) noexcept(
      noexcept(base_array_ = base_array_)) {
    base_array_ = other.base_array_;
    if constexpr (std::allocator_traits<cell_allocator_type>::
                      propagate_on_container_copy_assignment::value) {
      allocator_ = other.allocator_;
    }
    front_index_ = other.front_index_;
    size_ = other.size_;
    return *this;
  }

  /// @brief Move-assigns from `other`, leaving the moved-from array empty.
  constexpr SkipArray& operator=(SkipArray&& other) noexcept(
      noexcept(base_array_ = std::move(base_array_))) {
    base_array_ = std::move(other.base_array_);
    if constexpr (std::allocator_traits<cell_allocator_type>::
                      propagate_on_container_move_assignment::value) {
      allocator_ = std::move(other.allocator_);
    }
    front_index_ = other.front_index_;
    size_ = other.size_;
    other.base_array_.clear();
    other.front_index_ = 0;
    other.size_ = 0;
    return *this;
  }

  /// @brief Constructs an empty `SkipArray` using the supplied allocator.
  constexpr explicit SkipArray(Allocator const& allocator) noexcept(
      noexcept(array_type(allocator_)) && noexcept(cell_allocator_type(allocator_)))
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
                                noexcept(cell_allocator_type(allocator_)))
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
                                noexcept(cell_allocator_type(allocator_)))
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
                                noexcept(cell_allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(InputCellIterator(first), InputCellIterator(last),
                    allocator_),
        front_index_(0),
        size_(capacity()) {}

  constexpr SkipArray(
      SkipArray const& other,
      std::type_identity_t<Allocator> const&
          allocator) noexcept(noexcept(array_type(base_array_, allocator_)) &&
                              noexcept(cell_allocator_type(allocator_)))
      : allocator_(allocator),
        base_array_(other.base_array_, allocator),
        front_index_(other.front_index_),
        size_(other.size_) {}

  constexpr SkipArray(
      SkipArray&& other,
      std::type_identity_t<Allocator> const&
          allocator) noexcept(noexcept(array_type(std::move(base_array_),
                                                  allocator_)) &&
                              noexcept(cell_allocator_type(allocator_)))
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

  /// @brief Returns the number of cells in the underlying array.
  ///
  /// @return The total number of cells allocated in the underlying array,
  ///   including empty (displacement) cells.
  constexpr size_type capacity() const noexcept(noexcept(base_array_.size())) {
    return base_array_.size();
  }

  /// @brief Returns the number of elements stored in the array.
  ///
  /// @return The number of non-empty elements.
  constexpr size_type size() const noexcept {
    return size_;
  }

  /// @brief Checks whether the array contains no elements.
  ///
  /// @return `true` if the array holds no elements, `false` otherwise.
  constexpr bool empty() const noexcept {
    return size_ == 0;
  }

  /// @brief Replaces the contents with `count` copies of `value`.
  constexpr void assign(size_type count, value_type const& value) noexcept(
      noexcept(base_array_.assign(count,
                                  cell_type(std::in_place_index<0>, value)))) {
    base_array_.assign(count, cell_type(std::in_place_index<0>, value));
    size_ = count;
    front_index_ = 0;
  }

  /// @brief Replaces the contents with copies of the elements in the range
  ///   `[first, last)`.
  template <typename InputIterator>
  constexpr void assign(InputIterator first, InputIterator last) noexcept(
      noexcept(base_array_.assign(InputCellIterator(first),
                                  InputCellIterator(last))) &&
      noexcept(capacity())) {
    base_array_.assign(InputCellIterator(first), InputCellIterator(last));
    size_ = capacity();
    front_index_ = 0;
  }

  /// @brief Replaces the contents with the elements of an initializer list.
  constexpr void assign(std::initializer_list<value_type> init) noexcept(
      noexcept(assign(init.begin(), init.end()))) {
    assign(init.begin(), init.end());
  }

  /// @brief Returns a copy of the allocator associated with the array.
  constexpr cell_allocator_type get_allocator() const noexcept {
    return allocator_;
  }

  /// @brief Returns an iterator to the first element.
  constexpr iterator begin() noexcept(noexcept(iterator(this, front_index_))) {
    return iterator(this, front_index_);
  }

  /// @brief Returns a constant iterator to the first element.
  constexpr const_iterator begin() const noexcept(noexcept(cbegin())) {
    return cbegin();
  }

  /// @brief Returns a constant iterator to the first element.
  constexpr const_iterator cbegin() const
      noexcept(noexcept(const_iterator(this, front_index_))) {
    return const_iterator(this, front_index_);
  }

  /// @brief Returns an iterator to the element following the last element.
  constexpr iterator end() noexcept(noexcept(iterator(this, capacity()))) {
    return iterator(this, capacity());
  }

  /// @brief Returns a constant iterator to the element following the last
  ///   element.
  constexpr const_iterator end() const noexcept(noexcept(cend())) {
    return cend();
  }

  /// @brief Returns a constant iterator to the element following the last
  ///   element.
  constexpr const_iterator cend() const
      noexcept(noexcept(const_iterator(this, capacity()))) {
    return const_iterator(this, capacity());
  }

  /// @brief Returns a reverse iterator to the first element of the reversed
  ///   array.
  constexpr iterator rbegin() noexcept(
      noexcept(std::make_reverse_iterator(end()))) {
    return std::make_reverse_iterator(end());
  }

  /// @brief Returns a constant reverse iterator to the first element of the
  ///   reversed array.
  constexpr const_iterator rbegin() const noexcept(noexcept(crbegin())) {
    return crbegin();
  }

  /// @brief Returns a constant reverse iterator to the first element of the
  ///   reversed array.
  constexpr const_iterator crbegin() const
      noexcept(noexcept(std::make_reverse_iterator(cend()))) {
    return std::make_reverse_iterator(cend());
  }

  /// @brief Returns a reverse iterator to the element following the last
  ///   element of the reversed array.
  constexpr iterator rend() noexcept(
      noexcept(std::make_reverse_iterator(begin()))) {
    return std::make_reverse_iterator(begin());
  }

  /// @brief Returns a constant reverse iterator to the element following the
  ///   last element of the reversed array.
  constexpr const_iterator rend() const noexcept(noexcept(crend())) {
    return crend();
  }

  /// @brief Returns a constant reverse iterator to the element following the
  ///   last element of the reversed array.
  constexpr const_iterator crend() const
      noexcept(noexcept(std::make_reverse_iterator(cbegin()))) {
    return std::make_reverse_iterator(cbegin());
  }

  /// @brief Returns a reference to the first element.
  constexpr reference front() {
    assert(is_valid_index(front_index_));
    return std::get<0>(base_array_[front_index_]);
  }

  /// @brief Returns a constant reference to the first element.
  constexpr const_reference front() const {
    assert(is_valid_index(front_index_));
    return std::get<0>(base_array_[front_index_]);
  }

  /// @brief Returns a reference to the last element.
  constexpr reference back() {
    assert(is_valid_index(back_index()));
    return std::get<0>(base_array_[back_index()]);
  }

  /// @brief Returns a constant reference to the last element.
  constexpr const_reference back() const {
    assert(is_valid_index(back_index()));
    return std::get<0>(base_array_[back_index()]);
  }

  /// @brief Constructs an element in place at the front of the array.
  ///
  /// @tparam Args The types of the arguments forwarded to the element's
  ///   constructor.
  /// @param args The arguments used to construct the new element.
  /// @return A reference to the newly constructed element.
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

  /// @brief Inserts a copy of `value` at the front of the array.
  constexpr void push_front(value_type const& value)
    requires requires {
      std::declval<array_type>().emplace_front(std::declval<cell_type>());
    }
  {
    emplace_front(value);
  }

  /// @brief Inserts `value` at the front of the array by moving it.
  constexpr void push_front(value_type&& value)
    requires requires {
      std::declval<array_type>().emplace_front(std::declval<cell_type>());
    }
  {
    emplace_front(std::move(value));
  }

  /// @brief Constructs an element in place at the back of the array.
  ///
  /// @tparam Args The types of the arguments forwarded to the element's
  ///   constructor.
  /// @param args The arguments used to construct the new element.
  /// @return A reference to the newly constructed element.
  template <typename... Args>
  constexpr reference emplace_back(Args&&... args)
    requires requires {
      std::declval<array_type>().emplace_back(std::declval<cell_type>());
    }
  {
    cell_type& cell = base_array_.emplace_back(std::in_place_index<0>,
                                               std::forward<Args>(args)...);
    ++size_;
    return std::get<0>(cell);
  }

  /// @brief Appends a copy of `value` to the back of the array.
  constexpr void push_back(value_type const& value)
    requires requires {
      std::declval<array_type>().emplace_back(std::declval<cell_type>());
    }
  {
    emplace_back(value);
  }

  /// @brief Appends `value` to the back of the array by moving it.
  constexpr void push_back(value_type&& value)
    requires requires {
      std::declval<array_type>().emplace_back(std::declval<cell_type>());
    }
  {
    emplace_back(std::move(value));
  }

  /// @brief Removes the element at the given position.
  ///
  /// @param pos Iterator to the element to remove. Must be valid.
  /// @return An iterator following the removed element.
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

  /// @brief Removes the first element of the array.
  constexpr void pop_front() {
    erase(begin());
  }

  /// @brief Removes the last element of the array.
  constexpr void pop_back() {
    erase(--end());
  }

  /// @brief Removes all elements from the array, leaving it empty.
  constexpr void clear() {
    size_ = 0;
    front_index_ = 0;
    base_array_.resize(0);
  }

  /// @brief Rewrites the underlying array so that all stored elements are
  ///   packed contiguously and all deleted (empty) cells are removed.
  ///
  /// Repeated erasures leave empty cells in the underlying array that are only
  /// represented by displacements. This rewrites the array so that it contains
  /// exactly the stored elements, in order, with no gaps, reducing `capacity()`
  /// to `size()` and resetting the front index to `0`.
  ///
  /// This does not change the number of elements stored nor their order.
  constexpr void compact() {
    if (size_ == capacity()) {
      return;
    }
    size_type new_index = 0;
    size_type old_index = front_index_;
    for (; new_index < size_; ++new_index) {
      assert(is_valid_index(old_index));
      base_array_[new_index] = std::move(base_array_[old_index]);
      old_index = first_valid_right_index(old_index + 1);
    }

    base_array_.resize(size_);
    front_index_ = 0;
  }

  /// @brief Returns an iterator to the first element not less than `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return An iterator to the first element that is not less than `key`,
  ///   or `end()` if no such element exists.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr iterator lower_bound(Key const& key, Compare comp = Compare()) {
    return iterator(this, lower_bound_index(0, capacity(), key,
                                            std::forward<Compare>(comp)));
  }

  /// @brief Returns a constant iterator to the first element not less than
  ///   `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return A constant iterator to the first element that is not less than
  ///   `key`, or `end()` if no such element exists.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr const_iterator lower_bound(Key const& key,
                                       Compare comp = Compare()) const {
    return const_iterator(this, lower_bound_index(0, capacity(), key,
                                                  std::forward<Compare>(comp)));
  }

  /// @brief Returns an iterator to the first element greater than `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return An iterator to the first element greater than `key`, or `end()`
  ///   if no such element exists.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr iterator upper_bound(Key const& key, Compare comp = Compare()) {
    return iterator(this, upper_bound_index(0, capacity(), key,
                                            std::forward<Compare>(comp)));
  }

  /// @brief Returns a constant iterator to the first element greater than
  ///   `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return A constant iterator to the first element greater than `key`, or
  ///   `end()` if no such element exists.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr const_iterator upper_bound(Key const& key,
                                       Compare comp = Compare()) const {
    return const_iterator(this, upper_bound_index(0, capacity(), key,
                                                  std::forward<Compare>(comp)));
  }

  /// @brief Returns a range of iterators covering all elements equal to
  ///   `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return A pair of iterators `[lower_bound(key), upper_bound(key))`.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr std::pair<iterator, iterator> equal_range(
      Key const& key, Compare comp = Compare()) {
    return std::make_pair(lower_bound(key, comp), upper_bound(key, comp));
  }

  /// @brief Returns a constant range of iterators covering all elements equal
  ///   to `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return A pair of constant iterators `[lower_bound(key),
  ///   upper_bound(key))`.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr std::pair<const_iterator, const_iterator> equal_range(
      Key const& key, Compare comp = Compare()) const {
    return std::make_pair(lower_bound(key, comp), upper_bound(key, comp));
  }

  /// @brief Returns the number of elements equal to `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return The number of elements that compare equal to `key`.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr size_type count(Key const& key, Compare comp = Compare()) const {
    size_type matches = 0;
    for (auto l = lower_bound(key, comp), u = upper_bound(key, comp); l != u;
         ++l) {
      ++matches;
    };
    return matches;
  }

  /// @brief Finds the first element equal to `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return An iterator to the first element equal to `key`, or `end()` if no
  ///   such element exists.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr iterator find(Key const& key, Compare comp = Compare()) {
    auto l = lower_bound(key, comp);
    if (l == end() || comp(key, *l)) {
      return end();
    }
    return l;
  }

  /// @brief Finds the first element equal to `key`.
  ///
  /// @tparam Key The type of the search key.
  /// @tparam Compare The comparison functor used to order elements.
  /// @param key The value to compare elements against.
  /// @param comp The comparison object.
  /// @return A constant iterator to the first element equal to `key`, or
  ///   `end()` if no such element exists.
  template <typename Key, typename Compare = std::less<value_type>>
  constexpr const_iterator find(Key const& key,
                                Compare comp = Compare()) const {
    auto l = lower_bound(key, comp);
    if (l == end() || comp(key, *l)) {
      return end();
    }
    return l;
  }

  /// @brief Compares the contents of this array with another range for
  ///   equality.
  ///
  /// @param other The range to compare against.
  /// @return `true` if both ranges have the same number of elements and
  ///   corresponding elements compare equal, `false` otherwise.
  constexpr bool operator==(std::ranges::range auto const& other) const {
    return std::equal(begin(), end(), other.begin());
  }

  /// @brief Lexicographically compares the contents of this array with
  ///   another range.
  ///
  /// @param other The range to compare against.
  /// @return A `std::strong_ordering` value describing the lexicographical
  ///   ordering of the two ranges.
  constexpr std::strong_ordering operator<=>(
      std::ranges::range auto const& other) const {
    return std::lexicographical_compare_three_way(begin(), end(), other.begin(),
                                                  other.end());
  }

  constexpr array_type& base_array() noexcept {
    return base_array_;
  }

  constexpr array_type const& base_array() const noexcept {
    return base_array_;
  }
};

}  // namespace sorted_array_tower
