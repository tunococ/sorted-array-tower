/**
 * @file
 *
 * @brief
 * [sorted_array_tower.bounded_vector](module__sorted_array_tower_8bounded__vector.html)
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
 * @brief An index-based vector that can grow and shrink, but has a fixed
 * maximum capacity.
 *
 * A `BoundedVector` is like `std::vector` in that it supports efficient
 * insertion and removal at the back, but unlike a vector it has an explicit
 * maximum capacity that can be set through `set_capacity`. When the capacity is
 * exceeded by a push or an emplace, the operation throws `std::length_error`.
 *
 * Internally the elements are stored in a contiguous buffer `data_`. The element
 * at index `i` of the `BoundedVector` is the element at index `i` in `data_`.
 *
 * @tparam T The type of the elements stored.
 * @tparam Allocator An allocator used to allocate the underlying buffer.
 */
template <typename T, typename Allocator = std::allocator<T>>
class BoundedVector {
 public:
  using value_type = T;
  using reference = value_type&;
  using const_reference = value_type const&;
  using this_type = BoundedVector;

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
    using value_type = BoundedVector::value_type;
    using difference_type = BoundedVector::difference_type;
    using pointer = std::conditional_t<IsConst, BoundedVector::const_pointer,
                                       BoundedVector::pointer>;
    using reference = std::conditional_t<IsConst, BoundedVector::const_reference,
                                         BoundedVector::reference>;

   private:
    using vector_pointer =
        std::conditional_t<IsConst, BoundedVector const*, BoundedVector*>;
    vector_pointer vector_;
    size_type index_;

    friend class Iterator<false>;
    friend class Iterator<true>;

   public:
    constexpr Iterator() noexcept = default;

    constexpr Iterator(Iterator const&) = default;
    constexpr Iterator(Iterator&&) = default;
    constexpr Iterator& operator=(Iterator const&) = default;
    constexpr Iterator& operator=(Iterator&&) = default;

    constexpr Iterator(vector_pointer vector, size_type index) noexcept
        : vector_(vector), index_(index) {}

    constexpr Iterator(Iterator<false> const& other) noexcept
      requires(IsConst)
        : vector_(other.vector_), index_(other.index_) {}

    constexpr reference operator*() const {
      return (*vector_)[index_];
    }

    constexpr pointer operator->() const {
      return std::pointer_traits<pointer>::pointer_to((*vector_)[index_]);
    }

    constexpr reference operator[](difference_type n) const {
      return (*vector_)[index_ + n];
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
    destroy_range(0, size_);
    deallocate_storage(data_, capacity_);
  }

  /// @brief Destroys `count` elements starting at index `first`.
  constexpr void destroy_range(size_type first, size_type count) {
    for (size_type n = 0; n < count; ++n) {
      std::allocator_traits<allocator_type>::destroy(allocator_,
                                                     &data_[first + n]);
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
      std::allocator_traits<allocator_type>::construct(
          allocator_, &new_data[i], std::move(data_[i]));
      std::allocator_traits<allocator_type>::destroy(allocator_,
                                                     &data_[i]);
    }
    deallocate_storage(data_, capacity_);
    data_ = new_data;
    capacity_ = new_capacity;
  }

 public:
  /// @brief Constructs an empty `BoundedVector` with zero capacity.
  constexpr BoundedVector() noexcept(noexcept(allocator_type()))
      : allocator_(allocator_type()),
        data_(nullptr),
        capacity_(0),
        size_(0) {}

  /// @brief Constructs an empty `BoundedVector` using the supplied allocator.
  constexpr explicit BoundedVector(Allocator const& allocator) noexcept(
      noexcept(allocator_type(allocator)))
      : allocator_(allocator),
        data_(nullptr),
        capacity_(0),
        size_(0) {}

  /// @brief Constructs a `BoundedVector` with the given capacity, holding no
  ///   elements.
  constexpr explicit BoundedVector(size_type capacity,
                                   Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        capacity_(capacity),
        size_(0) {
    data_ = allocate_storage(capacity);
  }

  /// @brief Constructs a `BoundedVector` with the given capacity, holding
  ///   `count` default-constructed elements.
  constexpr BoundedVector(size_type capacity, size_type count,
                          Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        capacity_(capacity),
        size_(0) {
    if (count > capacity_) {
      throw std::length_error("BoundedVector count exceeds capacity");
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

  /// @brief Constructs a `BoundedVector` with the given capacity, holding
  ///   `count` copies of `value`.
  constexpr BoundedVector(size_type capacity, size_type count,
                          value_type const& value,
                          Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        capacity_(capacity),
        size_(0) {
    if (count > capacity_) {
      throw std::length_error("BoundedVector count exceeds capacity");
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

  /// @brief Constructs a `BoundedVector` from a range of elements with the
  ///   given capacity.
  template <typename InputIterator>
    requires(!std::is_convertible_v<InputIterator, size_type>)
  constexpr BoundedVector(size_type capacity, InputIterator first,
                         InputIterator last,
                         Allocator const& allocator = Allocator())
      : allocator_(allocator),
        data_(nullptr),
        capacity_(capacity),
        size_(0) {
    data_ = allocate_storage(capacity);
    try {
      for (; first != last; ++first) {
        if (size_ == capacity_) {
          throw std::length_error("BoundedVector range exceeds capacity");
        }
        raw_construct(size_, *first);
        ++size_;
      }
    } catch (...) {
      destroy_and_deallocate_partial();
      throw;
    }
  }

  /// @brief Constructs a `BoundedVector` from an initializer list with the given
  ///   capacity.
  constexpr BoundedVector(size_type capacity,
                          std::initializer_list<value_type> init,
                          Allocator const& allocator = Allocator())
      : BoundedVector(capacity, init.begin(), init.end(), allocator) {}

  /// @brief Copy-constructs a `BoundedVector`.
  constexpr BoundedVector(BoundedVector const& other)
      : allocator_(std::allocator_traits<allocator_type>::
                        select_on_container_copy_construction(other.allocator_)),
        data_(nullptr),
        capacity_(other.capacity_),
        size_(0) {
    data_ = allocate_storage(other.capacity_);
    try {
      for (size_type i = 0; i < other.size_; ++i) {
        raw_construct(i, other[i]);
        ++size_;
      }
    } catch (...) {
      destroy_and_deallocate_partial();
      throw;
    }
  }

  /// @brief Move-constructs a `BoundedVector`, leaving the moved-from vector
  ///   empty with zero capacity.
  constexpr BoundedVector(BoundedVector&& other) noexcept
      : allocator_(std::move(other.allocator_)),
        data_(other.data_),
        capacity_(other.capacity_),
        size_(other.size_) {
    other.data_ = nullptr;
    other.capacity_ = 0;
    other.size_ = 0;
  }

  /// @brief Copy-assigns from `other`, replacing the contents of this vector.
  constexpr BoundedVector& operator=(BoundedVector const& other) {
    if (this == &other) {
      return *this;
    }
    if (std::allocator_traits<
            allocator_type>::propagate_on_container_copy_assignment::value) {
      allocator_ = other.allocator_;
    }
    clear();
    deallocate_storage(data_, capacity_);
    data_ = nullptr;
    capacity_ = 0;
    size_ = 0;
    data_ = allocate_storage(other.capacity_);
    capacity_ = other.capacity_;
    try {
      for (size_type i = 0; i < other.size_; ++i) {
        raw_construct(i, other[i]);
        ++size_;
      }
    } catch (...) {
      destroy_range(0, size_);
      deallocate_storage(data_, capacity_);
      data_ = nullptr;
      capacity_ = 0;
      size_ = 0;
      throw;
    }
    return *this;
  }

  /// @brief Move-assigns from `other`, leaving the moved-from vector empty with
  ///   zero capacity.
  constexpr BoundedVector& operator=(BoundedVector&& other) noexcept(
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
    capacity_ = other.capacity_;
    size_ = other.size_;
    other.data_ = nullptr;
    other.capacity_ = 0;
    other.size_ = 0;
    return *this;
  }

  /// @brief Replaces the contents with those of an initializer list, provided
  ///   its size does not exceed capacity.
  constexpr BoundedVector& operator=(std::initializer_list<value_type> init) {
    if (init.size() > capacity_) {
      throw std::length_error("BoundedVector initializer list exceeds capacity");
    }
    clear();
    for (auto const& v : init) {
      push_back(v);
    }
    return *this;
  }

  /// @brief Destructor. Destroys all stored elements and frees the storage.
  constexpr ~BoundedVector() {
    destroy_range(0, size_);
    deallocate_storage(data_, capacity_);
  }

  /// @brief Returns a copy of the allocator associated with the vector.
  constexpr allocator_type get_allocator() const noexcept {
    return allocator_;
  }

  /// @brief Returns the maximum number of elements the vector can hold.
  constexpr size_type capacity() const noexcept {
    return capacity_;
  }

  /// @brief Returns the number of elements stored in the vector.
  constexpr size_type size() const noexcept {
    return size_;
  }

  /// @brief Checks whether the vector contains no elements.
  constexpr bool empty() const noexcept {
    return size_ == 0;
  }

  /// @brief Checks whether the vector has reached its maximum capacity.
  constexpr bool full() const noexcept {
    return size_ == capacity_;
  }

  /// @brief Sets the maximum capacity of the vector.
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
        std::allocator_traits<allocator_type>::construct(
            allocator_, &new_data[i], std::move(data_[i]));
        std::allocator_traits<allocator_type>::destroy(allocator_,
                                                       &data_[i]);
      }
      deallocate_storage(data_, capacity_);
      data_ = new_data;
      capacity_ = new_capacity;
      return;
    }
    if (new_capacity < size_) {
      size_type remove_count = size_ - new_capacity;
      for (size_type i = 0; i < remove_count; ++i) {
        std::allocator_traits<allocator_type>::destroy(allocator_,
                                                       &data_[size_ - 1 - i]);
      }
      size_ = new_capacity;
    }
    reallocate(new_capacity);
  }

  /// @brief Accesses the element at the given logical index.
  constexpr reference operator[](size_type index) {
    return data_[index];
  }

  /// @brief Accesses the element at the given logical index.
  constexpr const_reference operator[](size_type index) const {
    return data_[index];
  }

  /// @brief Accesses the element at the given logical index, with bounds
  ///   checking.
  constexpr reference at(size_type index) {
    if (index >= size_) {
      throw std::out_of_range("BoundedVector index out of range");
    }
    return (*this)[index];
  }

  /// @brief Accesses the element at the given logical index, with bounds
  ///   checking.
  constexpr const_reference at(size_type index) const {
    if (index >= size_) {
      throw std::out_of_range("BoundedVector index out of range");
    }
    return (*this)[index];
  }

  /// @brief Returns a reference to the first element.
  constexpr reference front() {
    assert(size_ > 0);
    return data_[0];
  }

  /// @brief Returns a constant reference to the first element.
  constexpr const_reference front() const {
    assert(size_ > 0);
    return data_[0];
  }

  /// @brief Returns a reference to the last element.
  constexpr reference back() {
    assert(size_ > 0);
    return data_[size_ - 1];
  }

  /// @brief Returns a constant reference to the last element.
  constexpr const_reference back() const {
    assert(size_ > 0);
    return data_[size_ - 1];
  }

  /// @brief Constructs an element in place at the back of the vector.
  ///
  /// @tparam Args The types of the arguments forwarded to the element's
  ///   constructor.
  /// @param args The arguments used to construct the new element.
  /// @return A reference to the newly constructed element.
  /// @throw std::length_error If the vector is already full.
  template <typename... Args>
  constexpr reference emplace_back(Args&&... args) {
    if (size_ == capacity_) {
      throw std::length_error("BoundedVector is full");
    }
    raw_construct(size_, std::forward<Args>(args)...);
    ++size_;
    return data_[size_ - 1];
  }

  /// @brief Appends a copy of `value` to the back of the vector.
  constexpr void push_back(value_type const& value) {
    emplace_back(value);
  }

  /// @brief Appends `value` to the back of the vector by moving it.
  constexpr void push_back(value_type&& value) {
    emplace_back(std::move(value));
  }

  /// @brief Removes the last element of the vector.
  ///
  /// @pre The vector is not empty.
  constexpr void pop_back() {
    assert(size_ > 0);
    std::allocator_traits<allocator_type>::destroy(allocator_,
                                                   &data_[size_ - 1]);
    --size_;
  }

  /// @brief Removes all elements from the vector, leaving it empty.
  ///
  /// The capacity is preserved.
  constexpr void clear() {
    destroy_range(0, size_);
    size_ = 0;
  }

  /// @brief Resizes the vector to contain `count` elements.
  ///
  /// If `count` is greater than the current size, appends default-constructed
  /// elements at the back. If `count` is less than the current size, the
  /// excess elements at the back are removed.
  ///
  /// @param count The new size.
  /// @throw std::length_error If `count` is greater than the capacity.
  constexpr void resize(size_type count) {
    if (count > capacity_) {
      throw std::length_error("BoundedVector resize exceeds capacity");
    }
    if (count > size_) {
      while (size_ < count) {
        raw_construct(size_, value_type());
        ++size_;
      }
    } else {
      while (size_ > count) {
        pop_back();
      }
    }
  }

  /// @brief Resizes the vector to contain `count` elements, appending copies of
  ///   `value` if growing.
  ///
  /// @param count The new size.
  /// @param value The value used to initialize appended elements.
  /// @throw std::length_error If `count` is greater than the capacity.
  constexpr void resize(size_type count, value_type const& value) {
    if (count > capacity_) {
      throw std::length_error("BoundedVector resize exceeds capacity");
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
  ///   vector.
  constexpr reverse_iterator rbegin() noexcept {
    return std::make_reverse_iterator(end());
  }

  /// @brief Returns a constant reverse iterator to the first element of the
  ///   reversed vector.
  constexpr const_reverse_iterator rbegin() const noexcept {
    return crbegin();
  }

  /// @brief Returns a constant reverse iterator to the first element of the
  ///   reversed vector.
  constexpr const_reverse_iterator crbegin() const noexcept {
    return std::make_reverse_iterator(cend());
  }

  /// @brief Returns a reverse iterator to the element following the last
  ///   element of the reversed vector.
  constexpr reverse_iterator rend() noexcept {
    return std::make_reverse_iterator(begin());
  }

  /// @brief Returns a constant reverse iterator to the element following the
  ///   last element of the reversed vector.
  constexpr const_reverse_iterator rend() const noexcept {
    return crend();
  }

  /// @brief Returns a constant reverse iterator to the element following the
  ///   last element of the reversed vector.
  constexpr const_reverse_iterator crend() const noexcept {
    return std::make_reverse_iterator(cbegin());
  }

  /// @brief Compares the contents of two vectors for equality.
  constexpr bool operator==(BoundedVector const& other) const {
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

  /// @brief Lexicographically compares the contents of two vectors.
  constexpr std::strong_ordering operator<=>(BoundedVector const& other) const {
    size_type common = std::min(size_, other.size_);
    for (size_type i = 0; i < common; ++i) {
      if (auto cmp = this->operator[](i) <=> other[i]; cmp != 0) {
        return cmp;
      }
    }
    return size_ <=> other.size_;
  }

  /// @brief Compares the contents of this vector with another range for
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

  /// @brief Lexicographically compares the contents of this vector with another
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
