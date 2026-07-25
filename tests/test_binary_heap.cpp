#include <doctest/doctest.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <memory_resource>
#include <random>
#include <vector>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/binary_heap.hpp>
#endif

using namespace sorted_array_tower;
using namespace std;

TEST_SUITE_BEGIN("binary_heap");

TEST_CASE_TEMPLATE("constructors", T, int, size_t) {
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
}

TEST_CASE_TEMPLATE("push and top", T, int, size_t) {
  BinaryHeap<T> h;

  SUBCASE("single element") {
    auto id = h.push(T(5));
    REQUIRE(h.size() == 1);
    REQUIRE(h.top() == T(5));
    REQUIRE(h.top_id() == id);
  }

  SUBCASE("multiple elements") {
    vector<T> values{T(3), T(1), T(4), T(1), T(5)};
    vector<size_t> ids;
    for (T v : values) {
      ids.push_back(h.push(v));
    }
    REQUIRE(h.size() == 5);
    REQUIRE(h.top() == T(1));
    REQUIRE((h.top_id() == ids[1] || h.top_id() == ids[3]));
  }

  SUBCASE("emplace") {
    auto id = h.emplace(T(7));
    REQUIRE(h.size() == 1);
    REQUIRE(h.top() == T(7));
    REQUIRE(h.top_id() == id);
  }
}

TEST_CASE_TEMPLATE("at and operator[]", T, int, size_t) {
  BinaryHeap<T> h;
  auto id0 = h.push(T(10));
  auto id1 = h.push(T(20));
  auto id2 = h.push(T(30));

  SUBCASE("operator[]") {
    REQUIRE(h[id0] == T(10));
    REQUIRE(h[id1] == T(20));
    REQUIRE(h[id2] == T(30));
  }

  SUBCASE("at") {
    REQUIRE(h.at(id0) == T(10));
    REQUIRE(h.at(id1) == T(20));
    REQUIRE(h.at(id2) == T(30));
  }

  SUBCASE("mutable access") {
    h[id1] = T(99);
    REQUIRE(h[id1] == T(99));
  }
}

TEST_CASE_TEMPLATE("increase_key", T, int, size_t) {
  BinaryHeap<T> h;
  auto id = h.push(T(1));
  h.push(T(2));
  h.push(T(3));

  SUBCASE("increase to larger value") {
    h.increase_key(id, T(10));
    REQUIRE(h[id] == T(10));
    REQUIRE(h.top() == T(2));
  }

  SUBCASE("increase to same value") {
    h.increase_key(id, T(1));
    REQUIRE(h[id] == T(1));
    REQUIRE(h.top() == T(1));
  }

  SUBCASE("increase to smaller value does nothing") {
    h.increase_key(id, T(0));
    REQUIRE(h[id] == T(1));
    REQUIRE(h.top() == T(1));
  }

  SUBCASE("increase top key") {
    REQUIRE(h.top() == T(1));
    h.increase_top_key(T(5));
    REQUIRE(h.top() == T(2));
  }
}

TEST_CASE_TEMPLATE("decrease_key", T, int, size_t) {
  BinaryHeap<T> h;
  auto id = h.push(T(5));
  h.push(T(6));
  h.push(T(7));

  SUBCASE("decrease to smaller value") {
    h.decrease_key(id, T(1));
    REQUIRE(h[id] == T(1));
    REQUIRE(h.top() == T(1));
  }

  SUBCASE("decrease to same value") {
    h.decrease_key(id, T(5));
    REQUIRE(h[id] == T(5));
    REQUIRE(h.top() == T(5));
  }

  SUBCASE("decrease to larger value does nothing") {
    h.decrease_key(id, T(10));
    REQUIRE(h[id] == T(5));
    REQUIRE(h.top() == T(5));
  }

  SUBCASE("decrease top key") {
    REQUIRE(h.top() == T(5));
    h.decrease_top_key(T(2));
    REQUIRE(h.top() == T(2));
  }
}

TEST_CASE_TEMPLATE("modify_key", T, int, size_t) {
  BinaryHeap<T> h;
  auto id = h.push(T(5));
  h.push(T(6));
  h.push(T(7));

  SUBCASE("modify to larger value") {
    h.modify_key(id, T(10));
    REQUIRE(h[id] == T(10));
    REQUIRE(h.top() == T(6));
  }

  SUBCASE("modify to smaller value") {
    h.modify_key(id, T(1));
    REQUIRE(h[id] == T(1));
    REQUIRE(h.top() == T(1));
  }

  SUBCASE("modify top key larger") {
    REQUIRE(h.top() == T(5));
    h.modify_top_key(T(10));
    REQUIRE(h.top() == T(6));
  }

  SUBCASE("modify top key smaller") {
    REQUIRE(h.top() == T(5));
    h.modify_top_key(T(1));
    REQUIRE(h.top() == T(1));
  }
}

TEST_CASE_TEMPLATE("bubble_up and bubble_down", T, int, size_t) {
  BinaryHeap<T> h;
  auto id0 = h.push(T(10));
  auto id1 = h.push(T(20));
  auto id2 = h.push(T(30));
  auto id3 = h.push(T(40));

  SUBCASE("increase key triggers bubble_down") {
    h.modify_key(id0, T(50));
    REQUIRE(h.top() == T(20));
  }

  SUBCASE("decrease key triggers bubble_up") {
    h.modify_key(id3, T(5));
    REQUIRE(h.top() == T(5));
  }
}

TEST_CASE_TEMPLATE("reserve", T, int, size_t) {
  BinaryHeap<T> h;
  h.reserve(100);
  REQUIRE(h.size() == 0);
  for (int i = 0; i < 50; ++i) {
    h.push(T(i));
  }
  REQUIRE(h.size() == 50);
}

TEST_CASE("max heap behavior") {
  BinaryHeap<int, greater<int>> h;

  h.push(1);
  h.push(3);
  h.push(2);
  h.push(5);
  h.push(4);

  REQUIRE(h.size() == 5);
  REQUIRE(h.top() == 5);
  REQUIRE(h.top_id() >= 0);
  REQUIRE(h.top_id() < 5);

  vector<int> sorted;
  for (size_t i = 0; i < h.size(); ++i) {
    sorted.push_back(h.top());
    h.modify_top_key(numeric_limits<int>::min());
  }
  REQUIRE(sorted == vector<int>{5, 4, 3, 2, 1});
}

TEST_CASE("fuzz: push and modify") {
  mt19937 rng(12345);
  uniform_int_distribution<int> val_dist(0, 1000);
  uniform_int_distribution<int> op_dist(0, 2);

  BinaryHeap<int> h;

  vector<pair<size_t, int>> entries;
  for (int iter = 0; iter < 5000; ++iter) {
    int op = op_dist(rng);
    switch (op) {
      case 0:  // push
        if (h.size() < 100) {
          int v = val_dist(rng);
          size_t id = h.push(v);
          entries.push_back({id, v});
        }
        break;
      case 1:  // modify random existing key
        if (!entries.empty()) {
          size_t idx = rng() % entries.size();
          int new_val = val_dist(rng);
          h.modify_key(entries[idx].first, new_val);
          entries[idx].second = new_val;
        }
        break;
      case 2:  // bubble down top by setting to a large value
        if (!h.empty()) {
          size_t top_id = h.top_id();
          int top_val = h.top();
          h.increase_top_key(numeric_limits<int>::max());
          auto it = find_if(entries.begin(), entries.end(),
                            [top_id](auto const& e) { return e.first == top_id; });
          if (it != entries.end()) {
            it->second = numeric_limits<int>::max();
          }
        }
        break;
    }

    if (!h.empty()) {
      int top = h.top();
      for (auto const& e : entries) {
        REQUIRE(h[e.first] == e.second);
      }
      int min_val = (*min_element(entries.begin(), entries.end(),
                                 [](auto const& a, auto const& b) {
                                   return a.second < b.second;
                                 })).second;
      REQUIRE(top == min_val);
    }
  }
}

struct LifetimeTracked {
  static inline int alive = 0;
  static constexpr int kDestroyed = -424242;

  int value;

  LifetimeTracked() : value(0) {
    ++alive;
  }
  LifetimeTracked(int v) : value(v) {
    ++alive;
  }
  LifetimeTracked(LifetimeTracked const& other) : value(other.value) {
    ++alive;
  }
  LifetimeTracked(LifetimeTracked&& other) noexcept : value(other.value) {
    other.value = -1;
    ++alive;
  }
  LifetimeTracked& operator=(LifetimeTracked const& other) {
    value = other.value;
    return *this;
  }
  LifetimeTracked& operator=(LifetimeTracked&& other) noexcept {
    value = other.value;
    other.value = -1;
    return *this;
  }
  ~LifetimeTracked() {
    assert(value != kDestroyed);
    assert(alive > 0);
    value = kDestroyed;
    --alive;
  }

  static void reset() {
    alive = 0;
  }

  auto operator<=>(LifetimeTracked const& other) const = default;
};

TEST_CASE("lifetime: no leaks or double-free") {
  LifetimeTracked::reset();

  SUBCASE("push and modify") {
    {
      BinaryHeap<LifetimeTracked> h;
      h.push(LifetimeTracked(1));
      h.push(LifetimeTracked(2));
      h.push(LifetimeTracked(3));
      REQUIRE(LifetimeTracked::alive == 3);
      auto id = h.top_id();
      h.modify_key(id, LifetimeTracked(10));
      REQUIRE(LifetimeTracked::alive == 3);
    }
    REQUIRE(LifetimeTracked::alive == 0);
  }
}

TEST_CASE_TEMPLATE("copy and move", T, int, size_t) {
  SUBCASE("copy") {
    BinaryHeap<T> h;
    h.push(T(1));
    h.push(T(2));
    h.push(T(3));

    BinaryHeap<T> h2 = h;
    REQUIRE(h2.size() == h.size());
    REQUIRE(h2.top() == h.top());
  }

  SUBCASE("move") {
    BinaryHeap<T> h;
    h.push(T(1));
    h.push(T(2));
    h.push(T(3));

    BinaryHeap<T> h2 = move(h);
    REQUIRE(h2.size() == 3);
    REQUIRE(h.size() == 0);
  }
}

TEST_CASE("large heap operations") {
  BinaryHeap<int> h;
  vector<int> values;
  mt19937 rng(42);
  uniform_int_distribution<int> dist(0, 10000);

  for (int i = 0; i < 1000; ++i) {
    int v = dist(rng);
    values.push_back(v);
    h.push(v);
  }

  REQUIRE(h.size() == 1000);
  int expected_min = *min_element(values.begin(), values.end());
  REQUIRE(h.top() == expected_min);

  vector<int> tops;
  for (size_t i = 0; i < values.size(); ++i) {
    tops.push_back(h.top());
    h.increase_top_key(numeric_limits<int>::max());
  }
  for (size_t i = 1; i < tops.size(); ++i) {
    REQUIRE(tops[i] >= tops[i - 1]);
  }
}

TEST_SUITE_END();
