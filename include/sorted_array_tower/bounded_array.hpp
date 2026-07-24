/**
 * @file
 *
 * @brief
 * [sorted_array_tower.skip_array](module__sorted_array_tower_8skip_array.html)
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace sorted_array_tower {

/**
 * @brief An index-based list that can grow and shrink, but has a fixed
 * maximum capacity.
 *
 * A `BoundedArray` is like `std::deque` in that it supports efficient
 * insertion and removal at both the front and the back, but unlike a deque it
 * has an explicit maximum capacity that can be set through `set_capacity`. When
 * the capacity is exceeded by a push or an emplace, the operation throws
 * `std::length_error`.
 *
 * Internally the elements are stored in a contiguous buffer `data_` treated as
 * a circular queue. The element at index `i` of the `BoundedArray` is the
 * element at index `(i + front_index_) % capacity_` in `data_`. Both
 * `front_index_` and the index of the back element wrap around modulo
 * `capacity_`.
 *
 * @tparam T The type of the elements stored.
 * @tparam Allocator An allocator used to allocate the underlying buffer.
 */
template <typename T, typename Allocator = std::allocator<T>>
class BoundedArray {
 public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = value_type const&;
  using this_type = BoundedArray;

  using allocator_type =
      std::allocator_traits<Allocator>::template rebind_alloc<value_type>;
  using size_type = std::allocator_traits<allocator_type>::size_type;
  using difference_type =
      std::allocator_traits<allocator_type>::difference_type;
  using pointer = std::allocator_traits<allocator_type>::pointer;
  using const_pointer = std::allocator_traits<allocator_type>::const_pointer;

  template <bool IsConst>
  class Iterator;

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  template <bool IsConst>
  class Iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = BoundedArray::value_type;
    using difference_type = BoundedArray::difference_type;
    using pointer = std::conditional_t<IsConst, BoundedArray::const_pointer,
                                       BoundedArray::pointer>;
    using reference = std::conditional_t<IsConst, BoundedArray::const_reference,
                                         BoundedArray::reference>;

   private:
    using array_pointer =
        std::conditional_t<IsConst, BoundedArray const*, BoundedArray*>;
    array_pointer array_;
    size_type index_;

    friend class Iterator<false>;
    friend class Iterator<true>;

   public:
    constexpr Iterator() noexcept = default;

    constexpr Iterator(Iterator const&) = default;
    constexpr Iterator(Iterator&&) = default;
    constexpr Iterator& operator=(Iterator const&) = default;
    constexpr Iterator& operator=(Iterator&&) = default;

    constexpr Iterator(array_pointer array, size_type index) noexcept
        : array_(array), index_(index) {}

    constexpr Iterator(Iterator<false> const& other) noexcept
      requires(IsConst)
        : array_(other.array_), index_(other.index_) {}

    constexpr reference operator*() const {
      return (*array_)[index_];
    }

    constexpr pointer operator->() const {
      return std::pointer_traits<pointer>::pointer_to((*array_)[index_]);
    }

    constexpr reference operator[](difference_type n) const {
      return (*array_)[index_ + n];
    }

    constexpr Iterator& operator++() {
      ++index_;
      return *this;
    }

    constexpr Iterator operator++(int) {
      Iterator tmp = *this;
      ++index_;
      return tmp;
    }

    constexpr Iterator& operator--() {
      --index_;
      return *this;
    }

    constexpr Iterator operator--(int) {
      Iterator tmp = *this;
      --index_;
      return tmp;
    }

    constexpr Iterator& operator+=(difference_type n) {
      index_ += n;
      return *this;
    }

    constexpr Iterator& operator-=(difference_type n) {
      index_ -= n;
      return *this;
    }

    constexpr friend Iterator operator+(Iterator it, difference_type n) {
      it.index_ += n;
      return it;
    }

    constexpr friend Iterator operator+(difference_type n, Iterator it) {
      it.index_ += n;
      return it;
    }

    constexpr friend Iterator operator-(Iterator it, difference_type n) {
      it.index_ -= n;
      return it;
    }

    constexpr friend difference_type operator-(Iterator const& a,
                                               Iterator const& b) {
      return static_cast<difference_type>(a.index_) -
             static_cast<difference_type>(b.index_);
    }

    constexpr bool operator==(Iterator const& other) const = default;

    constexpr std::strong_ordering operator<=>(Iterator const& other) const
        noexcept(noexcept(index_ <=> index_)) = default;
  };

 private:
  allocator_type allocator_;
  pointer data_;
  size_type front_index_;
  size_type capacity_;
  size_type size_;

  /// @brief Allocates raw, uninitialized storage for `count` elements.
  constexpr pointer allocate_storage(size_type count) {
    if (count == 0) {
      return nullptr;
    }
    return std::allocator_traits<allocator_type>::allocate(allocator_, count);
  }

  /// @brief Deallocates raw storage previously obtained from
  ///   `allocate_storage`. Does not destroy any elements.
  constexpr void deallocate_storage(pointer data, size_type count) {
    if (data) {
      std::allocator_traits<allocator_type>::deallocate(allocator_, data,
                                                        count);
    }
  }

  /// @brief Cleanup helper for use in a constructor's catch block.
  ///
  /// Destroys the `size_` elements that were successfully constructed at the
  /// front of `data_` and frees the buffer. Constructors need this because a
  /// throwing element constructor means the object is never fully constructed,
  /// so its destructor will not run.
  constexpr void destroy_and_deallocate_partial() {
    destroy_range(front_index_, size_);
    deallocate_storage(data_, capacity_);
  }

  constexpr size_type logical_to_physical(size_type index) const {
    assert(index < size_);
    return (index + front_index_) % (capacity_ == 0 ? 1 : capacity_);
  }

  constexpr size_type back_index() const {
    assert(size_ > 0);
    return (front_index_ + size_ - 1) % (capacity_ == 0 ? 1 : capacity_);
  }

  /// @brief Destroys `count` elements starting at physical index `first`,
  ///   wrapping around the circular buffer.
  ///
  /// Iterating by count (rather than comparing a start and end index) is
  /// required so that a completely full buffer, whose start and end physical
  /// indices coincide, still has all of its elements destroyed.
  constexpr void destroy_range(size_type first, size_type count) {
    size_type i = first;
    for (size_type n = 0; n < count; ++n) {
      std::allocator_traits<allocator_type>::destroy(allocator_, &data_[i]);
      i = (i + 1) % (capacity_ == 0 ? 1 : capacity_);
    }
  }

  constexpr void raw_construct(size_type index, value_type const& value) {
    std::allocator_traits<allocator_type>::construct(allocator_, &data_[index],
                                                     value);
  }

  constexpr void raw_construct(size_type index, value_type&& value) {
    std::allocator_traits<allocator_type>::construct(allocator_, &data_[index],
                                                     std::move(value));
  }

  template <typename... Args>
  constexpr void raw_construct(size_type index, Args&&... args) {
    std::allocator_traits<allocator_type>::construct(
        allocator_, &data_[index], std::forward<Args>(args)...);
  }

  constexpr void reallocate(size_type new_capacity) {
    if (new_capacity == capacity_) {
      return;
    }
    pointer new_data = allocate_storage(new_capacity);
    for (size_type i = 0; i < size_; ++i) {
      size_type old_physical = logical_to_physical(i);
      size_type new_physical = i % (new_capacity == 0 ? 1 : new_capacity);
      std::allocator_traits<allocator_type>::construct(
          allocator_, &new_data[new_physical], std::move(data_[old_physical]));
      std::allocator_traits<allocator_type>::destroy(allocator_,
                                                     &data_[old_physical]);
    }
    deallocate_storage(data_, capacity_);
    data_ = new_data;
    capacity_ = new_capacity;
    front_index_ = 0;
  }

 public:
  /// @brief Constructs an empty `BoundedArray` with zero capacity.
  constexpr BoundedArray() noexcept(noexcept(allocator_type()))
      : allocator_(allocator_type()),
        data_(nullptr),
        front_index_(0),
        capacity_(0),
        size_(0) {}

  /// @brief Constructs an empty `BoundedArray` using the supplied allocator.
  constexpr explicit BoundedArray(Allocator const& allocator) noexcept(
      noexcept(allocator_type(allocator)))
      : allocator_(allocator),
        data_(nullptr),
        front_index_(0),
        capacity_(0),
        size_(0) {}

  /// @brief Constructs a `BoundedArray` with the given capacity, holding no
  ///   elements.
  constexpr explicit BoundedArray(size_type capacity,
                                  Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        front_index_(0),
        capacity_(capacity),
        size_(0) {
    data_ = allocate_storage(capacity);
  }

  /// @brief Constructs a `BoundedArray` with the given capacity, holding
  ///   `count` default-constructed elements.
  constexpr BoundedArray(size_type capacity, size_type count,
                         Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        front_index_(0),
        capacity_(capacity),
        size_(0) {
    if (count > capacity_) {
      throw std::length_error("BoundedArray count exceeds capacity");
    }
    data_ = allocate_storage(capacity);
    try {
      for (size_type i = 0; i < count; ++i) {
        raw_construct(i, value_type());
        ++size_;
      }
    } catch (...) {
      destroy_and_deallocate_partial();
      throw;
    }
  }

  /// @brief Constructs a `BoundedArray` with the given capacity, holding
  ///   `count` copies of `value`.
  constexpr BoundedArray(size_type capacity, size_type count,
                         value_type const& value,
                         Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        front_index_(0),
        capacity_(capacity),
        size_(0) {
    if (count > capacity_) {
      throw std::length_error("BoundedArray count exceeds capacity");
    }
    data_ = allocate_storage(capacity);
    try {
      for (size_type i = 0; i < count; ++i) {
        raw_construct(i, value);
        ++size_;
      }
    } catch (...) {
      destroy_and_deallocate_partial();
      throw;
    }
  }

  /// @brief Constructs a `BoundedArray` from a range of elements with the
  ///   given capacity.
  template <typename InputIterator>
    requires(!std::is_convertible_v<InputIterator, size_type>)
  constexpr BoundedArray(size_type capacity, InputIterator first,
                         InputIterator last,
                         Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        front_index_(0),
        capacity_(capacity),
        size_(0) {
    data_ = allocate_storage(capacity);
    try {
      for (; first != last; ++first) {
        if (size_ == capacity_) {
          throw std::length_error("BoundedArray range exceeds capacity");
        }
        raw_construct(size_, *first);
        ++size_;
      }
    } catch (...) {
      destroy_and_deallocate_partial();
      throw;
    }
  }

  /// @brief Constructs a `BoundedArray` from an initializer list with the given
  ///   capacity.
  constexpr BoundedArray(size_type capacity,
                         std::initializer_list<value_type> init,
                         Allocator const& allocator = Allocator())
      : BoundedArray(capacity, init.begin(), init.end(), allocator) {}

  /// @brief Copy-constructs a `BoundedArray`.
  constexpr BoundedArray(BoundedArray const& other)
      : allocator_(std::allocator_traits<allocator_type>::
                       select_on_container_copy_construction(other.allocator_)),
        data_(nullptr),
        front_index_(0),
        capacity_(other.capacity_),
        size_(0) {
    data_ = allocate_storage(capacity_);
    if (other.size_ == 0) {
      return;
    }
    size_type back_count = capacity_ - other.front_index_;
    pointer src = other.data_ + other.front_index_;
    try {
      if (other.size_ <= back_count) {
        std::uninitialized_copy(src, src + other.size_, data_);
      } else {
        std::uninitialized_copy(src, src + back_count, data_);
        std::uninitialized_copy(other.data_,
                                other.data_ + other.size_ - back_count,
                                data_ + back_count);
      }
    } catch (...) {
      deallocate_storage(data_, capacity_);
      throw;
    }
    size_ = other.size_;
  }

  /// @brief Move-constructs a `BoundedArray`, leaving the moved-from array
  ///   empty with zero capacity.
  constexpr BoundedArray(BoundedArray&& other) noexcept
      : allocator_(std::move(other.allocator_)),
        data_(other.data_),
        front_index_(other.front_index_),
        capacity_(other.capacity_),
        size_(other.size_) {
    other.data_ = nullptr;
    other.front_index_ = 0;
    other.capacity_ = 0;
    other.size_ = 0;
  }

  /// @brief Copy-assigns from `other`, replacing the contents of this array.
  constexpr BoundedArray& operator=(BoundedArray const& other) {
    if (this == &other) {
      return *this;
    }
    if (std::allocator_traits<
            allocator_type>::propagate_on_container_copy_assignment::value) {
      allocator_ = other.allocator_;
    }
    clear();
    if (capacity_ != other.capacity_) {
      deallocate_storage(data_, capacity_);
      data_ = allocate_storage(other.capacity_);
      capacity_ = other.capacity_;
    }
    if (other.size_ == 0) {
      return *this;
    }
    size_type back_count = capacity_ - other.front_index_;
    pointer src = other.data_ + other.front_index_;
    if (other.size_ <= back_count) {
      std::uninitialized_copy(src, src + other.size_, data_);
    } else {
      std::uninitialized_copy(src, src + back_count, data_);
      try {
        std::uninitialized_copy(other.data_,
                                other.data_ + other.size_ - back_count,
                                data_ + back_count);
      } catch (...) {
        for (size_type i = 0; i < back_count; ++i) {
          std::allocator_traits<allocator_type>::destroy(allocator_, data_ + i);
        }
        deallocate_storage(data_, capacity_);
        data_ = nullptr;
        capacity_ = 0;
        throw;
      }
    }
    size_ = other.size_;
    return *this;
  }

  /// @brief Move-assigns from `other`, leaving the moved-from array empty with
  ///   zero capacity.
  constexpr BoundedArray& operator=(BoundedArray&& other) noexcept(
      std::allocator_traits<allocator_type>::is_always_equal::value) {
    if (this == &other) {
      return *this;
    }
    clear();
    deallocate_storage(data_, capacity_);
    data_ = nullptr;
    capacity_ = 0;
    if (std::allocator_traits<
            allocator_type>::propagate_on_container_move_assignment::value) {
      allocator_ = std::move(other.allocator_);
    }
    data_ = other.data_;
    front_index_ = other.front_index_;
    capacity_ = other.capacity_;
    size_ = other.size_;
    other.data_ = nullptr;
    other.front_index_ = 0;
    other.capacity_ = 0;
    other.size_ = 0;
    return *this;
  }

  /// @brief Replaces the contents with those of an initializer list, provided
  ///   its size does not exceed capacity.
  constexpr BoundedArray& operator=(std::initializer_list<value_type> init) {
    if (init.size() > capacity_) {
      throw std::length_error("BoundedArray initializer list exceeds capacity");
    }
    clear();
    for (auto const& v : init) {
      push_back(v);
    }
    return *this;
  }

  /// @brief Destructor. Destroys all stored elements and frees the storage.
  constexpr ~BoundedArray() {
    destroy_range(front_index_, size_);
    deallocate_storage(data_, capacity_);
  }

  /// @brief Returns a copy of the allocator associated with the array.
  constexpr allocator_type get_allocator() const noexcept {
    return allocator_;
  }

  /// @brief Returns the maximum number of elements the array can hold.
  constexpr size_type capacity() const noexcept {
    return capacity_;
  }

  /// @brief Returns the number of elements stored in the array.
  constexpr size_type size() const noexcept {
    return size_;
  }

  /// @brief Checks whether the array contains no elements.
  constexpr bool empty() const noexcept {
    return size_ == 0;
  }

  /// @brief Checks whether the array has reached its maximum capacity.
  constexpr bool full() const noexcept {
    return size_ == capacity_;
  }

  /// @brief Sets the maximum capacity of the array.
  ///
  /// If `new_capacity` is smaller than the current number of elements, the
  /// excess elements at the back are discarded. Existing elements retain their
  /// relative order and logical indices.
  ///
  /// @param new_capacity The new capacity. May be zero.
  constexpr void set_capacity(size_type new_capacity) {
    if (new_capacity >= capacity_ && size_ <= new_capacity) {
      if (new_capacity == capacity_) {
        return;
      }
      pointer new_data = allocate_storage(new_capacity);
      for (size_type i = 0; i < size_; ++i) {
        size_type old_physical = logical_to_physical(i);
        std::allocator_traits<allocator_type>::construct(
            allocator_, &new_data[i], std::move(data_[old_physical]));
        std::allocator_traits<allocator_type>::destroy(allocator_,
                                                       &data_[old_physical]);
      }
      deallocate_storage(data_, capacity_);
      data_ = new_data;
      capacity_ = new_capacity;
      front_index_ = 0;
      return;
    }
    if (new_capacity < size_) {
      size_type remove_count = size_ - new_capacity;
      for (size_type i = 0; i < remove_count; ++i) {
        size_type physical = back_index();
        std::allocator_traits<allocator_type>::destroy(allocator_,
                                                       &data_[physical]);
        --size_;
      }
    }
    reallocate(new_capacity);
  }

  /// @brief Accesses the element at the given logical index.
  constexpr reference operator[](size_type index) {
    return data_[logical_to_physical(index)];
  }

  /// @brief Accesses the element at the given logical index.
  constexpr const_reference operator[](size_type index) const {
    return data_[logical_to_physical(index)];
  }

  /// @brief Accesses the element at the given logical index, with bounds
  ///   checking.
  constexpr reference at(size_type index) {
    if (index >= size_) {
      throw std::out_of_range("BoundedArray index out of range");
    }
    return (*this)[index];
  }

  /// @brief Accesses the element at the given logical index, with bounds
  ///   checking.
  constexpr const_reference at(size_type index) const {
    if (index >= size_) {
      throw std::out_of_range("BoundedArray index out of range");
    }
    return (*this)[index];
  }

  /// @brief Returns a reference to the first element.
  constexpr reference front() {
    assert(size_ > 0);
    return data_[front_index_];
  }

  /// @brief Returns a constant reference to the first element.
  constexpr const_reference front() const {
    assert(size_ > 0);
    return data_[front_index_];
  }

  /// @brief Returns a reference to the last element.
  constexpr reference back() {
    assert(size_ > 0);
    return data_[back_index()];
  }

  /// @brief Returns a constant reference to the last element.
  constexpr const_reference back() const {
    assert(size_ > 0);
    return data_[back_index()];
  }

  /// @brief Constructs an element in place at the front of the array.
  ///
  /// @tparam Args The types of the arguments forwarded to the element's
  ///   constructor.
  /// @param args The arguments used to construct the new element.
  /// @return A reference to the newly constructed element.
  /// @throw std::length_error If the array is already full.
  template <typename... Args>
  constexpr reference emplace_front(Args&&... args) {
    if (size_ == capacity_) {
      throw std::length_error("BoundedArray is full");
    }
    front_index_ = (front_index_ + capacity_ - 1) % capacity_;
    raw_construct(front_index_, std::forward<Args>(args)...);
    ++size_;
    return data_[front_index_];
  }

  /// @brief Inserts a copy of `value` at the front of the array.
  constexpr void push_front(value_type const& value) {
    emplace_front(value);
  }

  /// @brief Inserts `value` at the front of the array by moving it.
  constexpr void push_front(value_type&& value) {
    emplace_front(std::move(value));
  }

  /// @brief Constructs an element in place at the back of the array.
  ///
  /// @tparam Args The types of the arguments forwarded to the element's
  ///   constructor.
  /// @param args The arguments used to construct the new element.
  /// @return A reference to the newly constructed element.
  /// @throw std::length_error If the array is already full.
  template <typename... Args>
  constexpr reference emplace_back(Args&&... args) {
    if (size_ == capacity_) {
      throw std::length_error("BoundedArray is full");
    }
    size_type index = (front_index_ + size_) % capacity_;
    raw_construct(index, std::forward<Args>(args)...);
    ++size_;
    return data_[index];
  }

  /// @brief Appends a copy of `value` to the back of the array.
  constexpr void push_back(value_type const& value) {
    emplace_back(value);
  }

  /// @brief Appends `value` to the back of the array by moving it.
  constexpr void push_back(value_type&& value) {
    emplace_back(std::move(value));
  }

  /// @brief Removes the first element of the array.
  ///
  /// @pre The array is not empty.
  constexpr void pop_front() {
    assert(size_ > 0);
    std::allocator_traits<allocator_type>::destroy(allocator_,
                                                   &data_[front_index_]);
    front_index_ = (front_index_ + 1) % capacity_;
    --size_;
  }

  /// @brief Removes the last element of the array.
  ///
  /// @pre The array is not empty.
  constexpr void pop_back() {
    assert(size_ > 0);
    size_type index = back_index();
    std::allocator_traits<allocator_type>::destroy(allocator_, &data_[index]);
    --size_;
  }

  /// @brief Removes all elements from the array, leaving it empty.
  ///
  /// The capacity is preserved.
  constexpr void clear() {
    destroy_range(front_index_, size_);
    front_index_ = 0;
    size_ = 0;
  }

  /// @brief Replaces the contents with `count` copies of `value`.
  constexpr void assign(size_type count, value_type const& value) {
    if (count > capacity_) {
      set_capacity(count);
    }
    clear();
    for (size_type i = 0; i < count; ++i) {
      push_back(value);
    }
  }

  /// @brief Replaces the contents with copies of the elements in the range
  ///   `[first, last)`.
  template <typename InputIterator>
    requires(!std::is_convertible_v<InputIterator, size_type>)
  constexpr void assign(InputIterator first, InputIterator last) {
    size_type count = static_cast<size_type>(std::distance(first, last));
    if (count > capacity_) {
      set_capacity(count);
    }
    clear();
    for (; first != last; ++first) {
      push_back(*first);
    }
  }

  /// @brief Replaces the contents with the elements of an initializer list.
  constexpr void assign(std::initializer_list<value_type> init) {
    if (init.size() > capacity_) {
      set_capacity(static_cast<size_type>(init.size()));
    }
    clear();
    for (auto const& v : init) {
      push_back(v);
    }
  }

  /// @brief Resizes the array to contain `count` elements.
  ///
  /// If `count` is greater than the current size, appends default-constructed
  /// elements at the back. If `count` is less than the current size, the
  /// excess elements at the back are removed.
  ///
  /// @param count The new size.
  /// @throw std::length_error If `count` is greater than the capacity.
  constexpr void resize(size_type count) {
    if (count > capacity_) {
      throw std::length_error("BoundedArray resize exceeds capacity");
    }
    if (count > size_) {
      while (size_ < count) {
        size_type index = (front_index_ + size_) % capacity_;
        raw_construct(index, value_type());
        ++size_;
      }
    } else {
      while (size_ > count) {
        pop_back();
      }
    }
  }

  /// @brief Resizes the array to contain `count` elements, appending copies of
  ///   `value` if growing.
  ///
  /// @param count The new size.
  /// @param value The value used to initialize appended elements.
  /// @throw std::length_error If `count` is greater than the capacity.
  constexpr void resize(size_type count, value_type const& value) {
    if (count > capacity_) {
      throw std::length_error("BoundedArray resize exceeds capacity");
    }
    if (count > size_) {
      while (size_ < count) {
        emplace_back(value);
      }
    } else {
      while (size_ > count) {
        pop_back();
      }
    }
  }

  /// @brief Returns an iterator to the first element.
  constexpr iterator begin() noexcept {
    return iterator(this, 0);
  }

  /// @brief Returns a constant iterator to the first element.
  constexpr const_iterator begin() const noexcept {
    return cbegin();
  }

  /// @brief Returns a constant iterator to the first element.
  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(this, 0);
  }

  /// @brief Returns an iterator to the element following the last element.
  constexpr iterator end() noexcept {
    return iterator(this, size_);
  }

  /// @brief Returns a constant iterator to the element following the last
  ///   element.
  constexpr const_iterator end() const noexcept {
    return cend();
  }

  /// @brief Returns a constant iterator to the element following the last
  ///   element.
  constexpr const_iterator cend() const noexcept {
    return const_iterator(this, size_);
  }

  /// @brief Returns a reverse iterator to the first element of the reversed
  ///   array.
  constexpr reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }

  /// @brief Returns a constant reverse iterator to the first element of the
  ///   reversed array.
  constexpr const_reverse_iterator rbegin() const noexcept {
    return crbegin();
  }

  /// @brief Returns a constant reverse iterator to the first element of the
  ///   reversed array.
  constexpr const_reverse_iterator crbegin() const noexcept {
    return std::make_reverse_iterator(cend());
  }

  /// @brief Returns a reverse iterator to the element following the last
  /// element
  ///   of the reversed array.
  constexpr reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(begin());
  }

  /// @brief Returns a constant reverse iterator to the element following the
  ///   last element of the reversed array.
  constexpr const_reverse_iterator rend() const noexcept {
    return crend();
  }

  /// @brief Returns a constant reverse iterator to the element following the
  ///   last element of the reversed array.
  constexpr const_reverse_iterator crend() const noexcept {
    return std::make_reverse_iterator(cbegin());
  }

  /// @brief Compares the contents of two arrays for equality.
  constexpr bool operator==(BoundedArray const& other) const {
    if (size_ != other.size_) {
      return false;
    }
    for (size_type i = 0; i < size_; ++i) {
      if (!(this->operator[](i) == other[i])) {
        return false;
      }
    }
    return true;
  }

  /// @brief Lexicographically compares the contents of two arrays.
  constexpr std::strong_ordering operator<=>(BoundedArray const& other) const {
    size_type common = std::min(size_, other.size_);
    for (size_type i = 0; i < common; ++i) {
      if (auto cmp = this->operator[](i) <=> other[i]; cmp != 0) {
        return cmp;
      }
    }
    return size_ <=> other.size_;
  }

  /// @brief Compares the contents of this array with another range for
  ///   equality.
  constexpr bool operator==(std::ranges::range auto const& other) const {
    if (size_ != std::ranges::size(other)) {
      return false;
    }
    auto it = std::ranges::begin(other);
    for (size_type i = 0; i < size_; ++i, ++it) {
      if (!(this->operator[](i) == *it)) {
        return false;
      }
    }
    return true;
  }

  /// @brief Lexicographically compares the contents of this array with another
  ///   range.
  constexpr auto operator<=>(std::ranges::range auto const& other) const {
    auto it = std::ranges::begin(other);
    size_type other_size = std::ranges::size(other);
    size_type common = std::min(size_, other_size);
    for (size_type i = 0; i < common; ++i, ++it) {
      if (auto cmp = this->operator[](i) <=> *it; cmp != 0) {
        return cmp;
      }
    }
    return size_ <=> other_size;
  }
};

}  // namespace sorted_array_tower