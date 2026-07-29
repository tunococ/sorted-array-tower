#include <doctest/doctest.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <memory_resource>
#include <random>
#include <set>
#include <vector>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/binary_heap.hpp>
#endif

using namespace sorted_array_tower;
using namespace std;

template <typename T, typename Compare, typename Allocator>
vector<T, typename std::allocator_traits<Allocator>::template rebind_alloc<T>>
extract_list(BinaryHeap<T, Compare, Allocator>& heap) {
  vector<T, typename std::allocator_traits<Allocator>::template rebind_alloc<T>>
      v(heap.get_allocator());
  v.reserve(heap.size());
  while (v.size() < heap.size()) {
    v.emplace_back(heap.top());
    heap.set_top(numeric_limits<T>::max());
  }
  return v;
}

#define INT_TYPES_TO_TEST size_t

TEST_SUITE_BEGIN("binary_heap");

TEST_CASE_TEMPLATE("constructors", T, INT_TYPES_TO_TEST) {
  SUBCASE("default") {
    BinaryHeap<T> h;
    REQUIRE(h.empty());
    REQUIRE(h.size() == 0);
  }

  SUBCASE("with comparator") {
    BinaryHeap<T, greater<T>> h{greater<T>()};
    REQUIRE(h.empty());
    h.push(T(1));
    h.push(T(3));
    REQUIRE(h.top() == T(3));
  }

  SUBCASE("with allocator") {
    using MemoryResource = std::pmr::unsynchronized_pool_resource;
    MemoryResource memory_resource;
    using Allocator = std::pmr::polymorphic_allocator<T>;
    Allocator allocator(&memory_resource);

    BinaryHeap<T, less<T>, Allocator> h(allocator);
    REQUIRE(h.empty());
    REQUIRE(h.size() == 0);
  }

  SUBCASE("with comparator and allocator") {
    using MemoryResource = std::pmr::unsynchronized_pool_resource;
    MemoryResource memory_resource;
    using Allocator = std::pmr::polymorphic_allocator<T>;
    Allocator allocator(&memory_resource);

    BinaryHeap<T, greater<T>, Allocator> h(greater<T>(), allocator);
    REQUIRE(h.empty());
  }

  SUBCASE("assign") {
    vector<T> v{T(5), T(4), T(3), T(2), T(1)};
    BinaryHeap<T> h{v.begin(), v.end()};
    sort(v.begin(), v.end());
    REQUIRE(extract_list(h) == v);
  }

  SUBCASE("assign with allocator") {
    using MemoryResource = std::pmr::unsynchronized_pool_resource;
    MemoryResource memory_resource;
    using Allocator = std::pmr::polymorphic_allocator<T>;
    Allocator allocator(&memory_resource);

    vector<T, Allocator> v{{T(5), T(4), T(3), T(2), T(1)}, allocator};
    BinaryHeap<T, less<T>, Allocator> h(v.begin(), v.end(), allocator);
    sort(v.begin(), v.end());
    REQUIRE(extract_list(h) == v);
  }

  SUBCASE("copy") {
    vector<T> v{T(5), T(4), T(3), T(2), T(1)};

    BinaryHeap<T> h;
    for (auto const& value : v) {
      h.push(value);
    }
    sort(v.begin(), v.end());

    BinaryHeap<T> h_2 = h;
    auto l = extract_list(h);
    auto l_2 = extract_list(h_2);
    REQUIRE(l_2 == l);
    REQUIRE(l_2 == v);
  }

  SUBCASE("move") {
    vector<T> v{T(5), T(4), T(3), T(2), T(1)};

    BinaryHeap<T> h;
    for (auto const& value : v) {
      h.push(value);
    }
    sort(v.begin(), v.end());

    BinaryHeap<T> h_2 = std::move(h);
    REQUIRE(extract_list(h_2) == v);
    REQUIRE(h.empty());
  }
}

TEST_CASE_TEMPLATE("push and top", T, INT_TYPES_TO_TEST) {
  BinaryHeap<T> h;

  h.push(T(3));
  REQUIRE(h.top() == T(3));
  REQUIRE(h.top_id() == 0);
  h.push(T(5));
  REQUIRE(h.top() == T(3));
  REQUIRE(h.top_id() == 0);
  h.push(T(2));
  REQUIRE(h.top() == T(2));
  REQUIRE(h.top_id() == 2);
  h.push(T(1));
  REQUIRE(h.top() == T(1));
  REQUIRE(h.top_id() == 3);
  h.push(T(4));
  REQUIRE(h.top() == T(1));
  REQUIRE(h.top_id() == 3);
}

TEST_CASE_TEMPLATE("at and operator[]", T, INT_TYPES_TO_TEST) {
  BinaryHeap<T> h;
  auto id0 = h.push(T(20));
  REQUIRE(h.top() == h[h.top_id()]);
  auto id1 = h.push(T(10));
  REQUIRE(h.top() == h[h.top_id()]);
  auto id2 = h.push(T(30));
  REQUIRE(h.top() == h[h.top_id()]);

  REQUIRE(h[id0] == T(20));
  REQUIRE(h[id1] == T(10));
  REQUIRE(h[id2] == T(30));
  REQUIRE(h.at(id0) == T(20));
  REQUIRE(h.at(id1) == T(10));
  REQUIRE(h.at(id2) == T(30));

  h[id2] = T(99);
  REQUIRE(h[id2] == T(99));
}

TEST_CASE_TEMPLATE("assignment", T, INT_TYPES_TO_TEST) {
  SUBCASE("copy") {
    vector<T> v{T(5), T(4), T(3), T(2), T(1)};

    BinaryHeap<T> h;
    BinaryHeap<T> h_2;
    for (auto const& value : v) {
      h.push(value);
    }
    sort(v.begin(), v.end());

    h_2 = h;
    REQUIRE(h_2.size() == h.size());
    REQUIRE(h_2.top() == h.top());

    auto l = extract_list(h);
    auto l_2 = extract_list(h_2);
    REQUIRE(l_2 == v);
    REQUIRE(l_2 == l);
  }

  SUBCASE("move") {
    vector<T> v{T(5), T(4), T(3), T(2), T(1)};

    BinaryHeap<T> h;
    BinaryHeap<T> h_2;
    for (auto const& value : v) {
      h.push(value);
    }
    sort(v.begin(), v.end());

    h_2 = std::move(h);
    REQUIRE(extract_list(h_2) == v);
    REQUIRE(h.empty());
  }
}

TEST_CASE_TEMPLATE("clear", T, INT_TYPES_TO_TEST) {
  BinaryHeap<T> h;
  h.push(T(1));
  h.push(T(2));
  h.push(T(3));
  REQUIRE(h.size() == 3);

  h.clear();
  REQUIRE(h.empty());
  REQUIRE(h.size() == 0);
}

TEST_CASE_TEMPLATE("key_comp and value_comp", T, INT_TYPES_TO_TEST) {
  BinaryHeap<T> h;
  BinaryHeap<T, greater<T>> h_greater;

  REQUIRE(h.key_comp()(T(1), T(2)));
  REQUIRE_FALSE(h.key_comp()(T(2), T(1)));
  REQUIRE(h.value_comp()(T(1), T(2)));
  REQUIRE_FALSE(h.value_comp()(T(2), T(1)));

  REQUIRE_FALSE(h_greater.key_comp()(T(1), T(2)));
  REQUIRE(h_greater.key_comp()(T(2), T(1)));
  REQUIRE_FALSE(h_greater.value_comp()(T(1), T(2)));
  REQUIRE(h_greater.value_comp()(T(2), T(1)));
}

TEST_CASE_TEMPLATE("build_heap", T, INT_TYPES_TO_TEST) {
  vector<T> v{T(50), T(20), T(10), T(40), T(30)};

  SUBCASE("in constructor") {
    BinaryHeap<T> h(v.begin(), v.end());
    sort(v.begin(), v.end());
    REQUIRE(extract_list(h) == v);
  }

  SUBCASE("in assign") {
    BinaryHeap<T> h;
    h.assign(v.begin(), v.end());
    sort(v.begin(), v.end());
    REQUIRE(extract_list(h) == v);
  }
}


TEST_CASE_TEMPLATE("fuzz test", T, INT_TYPES_TO_TEST) {
  mt19937_64 rand_gen{12345};

  SUBCASE("build_heap") {
    constexpr size_t NUM_REPS = 4;
    constexpr size_t MAX_SIZE = 33;
    for (size_t size = 0; size <= MAX_SIZE; ++size) {
      for (size_t r = 0; r < NUM_REPS; ++r) {
        vector<T> v(size);
        for (size_t i = 0; i < size; ++i) {
          v[i] = T(rand_gen() % size);
        }
        BinaryHeap<T> h_1(v.begin(), v.end());
        BinaryHeap<T> h_2;
        h_2.assign(v.begin(), v.end());

        sort(v.begin(), v.end());
        REQUIRE(extract_list(h_1) == v);
        REQUIRE(extract_list(h_2) == v);
      }
    }
  }

  SUBCASE("parallel multiset") {
    constexpr size_t SIZE = 33;
    constexpr size_t NUM_REPS = 10000;

    BinaryHeap<T> h;
    multiset<T> s;
    vector<typename multiset<T>::iterator> s_index;
    for (size_t i = 0; i < SIZE; ++i) {
      T value = rand_gen() % SIZE;
      h.emplace(value);
      s_index.emplace_back(s.emplace(value));
      REQUIRE(h.top() == *s.begin());
    }
    for (size_t r = 0; r < NUM_REPS; ++r) {
      {
        size_t id = rand_gen() % SIZE;
        T value = rand_gen() % SIZE;
        h.set(id, value);
        s.erase(s_index[id]);
        s_index[id] = s.emplace(value);
        REQUIRE(h.top() == *s.begin());
      }
      {
        size_t id = h.top_id();
        T value = rand_gen() % SIZE;
        h.set_top(value);
        s.erase(s_index[id]);
        s_index[id] = s.emplace(value);
        REQUIRE(h.top() == *s.begin());
      }
    }
    while (!s.empty()) {
      REQUIRE(h.top() == *s.begin());
      h.set_top(SIZE);
      s.erase(s.begin());
    }
  }
}


TEST_SUITE_END();
