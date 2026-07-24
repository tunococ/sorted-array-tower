#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <memory_resource>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/sorted_array_tower.hpp>
#endif

using namespace sorted_array_tower;
using namespace std;

#define INT_TYPES_TO_TEST int, size_t

TEST_SUITE_BEGIN("bounded_vector");

TEST_CASE_TEMPLATE("constructors", T, INT_TYPES_TO_TEST) {
  SUBCASE("default") {
    BoundedVector<T> s;
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
    REQUIRE(s.capacity() == 0);
  }

  SUBCASE("capacity only") {
    BoundedVector<T> s(0);
    REQUIRE(s.empty());
    REQUIRE(s.capacity() == 0);
    BoundedVector<T> s2(5);
    REQUIRE(s2.empty());
    REQUIRE(s2.capacity() == 5);
  }

  SUBCASE("capacity and count") {
    BoundedVector<T> s(5, 3);
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    for (T v : s) {
      REQUIRE(v == T());
    }
  }

  SUBCASE("capacity, count and value") {
    BoundedVector<T> s(5, 3, T(7));
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    for (T v : s) {
      REQUIRE(v == T(7));
    }
  }

  SUBCASE("range") {
    vector<T> v = {T(1), T(2), T(3)};
    BoundedVector<T> s(5, v.begin(), v.end());
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    REQUIRE(equal(s.begin(), s.end(), v.begin()));
  }

  SUBCASE("initializer_list") {
    BoundedVector<T> s(5, {T(1), T(2), T(3)});
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    REQUIRE(s[0] == T(1));
    REQUIRE(s[1] == T(2));
    REQUIRE(s[2] == T(3));
  }

  SUBCASE("copy") {
    BoundedVector<T> s1(5, {T(1), T(2), T(3)});
    BoundedVector<T> s2 = s1;
    REQUIRE(equal(s1.begin(), s1.end(), s2.begin()));
    REQUIRE(s1.capacity() == s2.capacity());
  }

  SUBCASE("move") {
    BoundedVector<T> s1(5, {T(1), T(2), T(3)});
    BoundedVector<T> s2 = move(s1);
    REQUIRE(s2.size() == 3);
    REQUIRE(s1.size() == 0);
    REQUIRE(s1.capacity() == 0);
  }
}

TEST_CASE_TEMPLATE("push_pop", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> s(4);

  SUBCASE("push_back") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.push_back(T(3));
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == T(1));
    REQUIRE(s[1] == T(2));
    REQUIRE(s[2] == T(3));
    REQUIRE(s.front() == T(1));
    REQUIRE(s.back() == T(3));
  }

  SUBCASE("pop_back") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.pop_back();
    REQUIRE(s.size() == 1);
    REQUIRE(s[0] == T(1));
  }

  SUBCASE("full then push throws") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.push_back(T(3));
    s.push_back(T(4));
    REQUIRE(s.full());
    CHECK_THROWS_AS(s.push_back(T(5)), length_error);
    CHECK_THROWS_AS(s.emplace_back(T(5)), length_error);
  }
}

TEST_CASE_TEMPLATE("emplace", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> s(4);
  s.emplace_back(T(1));
  REQUIRE(s[0] == T(1));
}

TEST_CASE_TEMPLATE("set_capacity", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> s(8);
  for (T i = 0; i < 5; ++i) {
    s.push_back(i);
  }

  SUBCASE("grow") {
    s.set_capacity(10);
    REQUIRE(s.capacity() == 10);
    REQUIRE(s.size() == 5);
    for (T i = 0; i < 5; ++i) {
      REQUIRE(s[i] == i);
    }
  }

  SUBCASE("shrink keeping elements") {
    s.set_capacity(6);
    REQUIRE(s.capacity() == 6);
    REQUIRE(s.size() == 5);
    for (T i = 0; i < 5; ++i) {
      REQUIRE(s[i] == i);
    }
  }

  SUBCASE("shrink discarding back") {
    s.set_capacity(3);
    REQUIRE(s.capacity() == 3);
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == T(0));
    REQUIRE(s[1] == T(1));
    REQUIRE(s[2] == T(2));
  }

  SUBCASE("shrink to zero") {
    s.set_capacity(0);
    REQUIRE(s.empty());
    REQUIRE(s.capacity() == 0);
  }
}

TEST_CASE_TEMPLATE("access", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> s(5, {T(1), T(2), T(3)});
  SUBCASE("operator[]") {
    REQUIRE(s[0] == T(1));
    REQUIRE(s[2] == T(3));
    s[1] = T(9);
    REQUIRE(s[1] == T(9));
  }
  SUBCASE("at in range") {
    REQUIRE(s.at(1) == T(2));
  }
  SUBCASE("at out of range") {
    CHECK_THROWS_AS(s.at(3), out_of_range);
  }
}

TEST_CASE_TEMPLATE("clear", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> s(5, {T(1), T(2), T(3)});
  s.clear();
  REQUIRE(s.empty());
  REQUIRE(s.capacity() == 5);
  s.push_back(T(7));
  REQUIRE(s.size() == 1);
  REQUIRE(s[0] == T(7));
}

TEST_CASE_TEMPLATE("comparison", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> a(5, {T(1), T(2), T(3)});
  BoundedVector<T> b(5, {T(1), T(2), T(3)});
  BoundedVector<T> c(5, {T(1), T(2), T(4)});
  BoundedVector<T> d(5, {T(1), T(2)});
  REQUIRE(a == b);
  REQUIRE(a != c);
  REQUIRE(a < c);
  REQUIRE(d < a);
  REQUIRE(!(a == d));
}

TEST_CASE_TEMPLATE("fuzz against deque", T, INT_TYPES_TO_TEST) {
  mt19937 rng(12345);
  uniform_int_distribution<int> op_dist(0, 1);
  uniform_int_distribution<int> val_dist(0, 100);

  BoundedVector<T> s(64);
  deque<T> ref;

  for (int iter = 0; iter < 5000; ++iter) {
    int op = op_dist(rng);
    switch (op) {
      case 0:  // push_back
        if (ref.size() < 64) {
          T v = val_dist(rng);
          s.push_back(v);
          ref.push_back(v);
        }
        break;
      case 1:  // pop_back
        if (!ref.empty()) {
          s.pop_back();
          ref.pop_back();
        }
        break;
    }
    REQUIRE(s.size() == ref.size());
    REQUIRE(equal(s.begin(), s.end(), ref.begin()));
  }
}

TEST_CASE_TEMPLATE("resize", T, INT_TYPES_TO_TEST) {
  BoundedVector<T> s(8);
  for (T i = 0; i < 3; ++i) {
    s.push_back(i);
  }

  SUBCASE("grow with default") {
    s.resize(5);
    REQUIRE(s.size() == 5);
    REQUIRE(s[0] == T(0));
    REQUIRE(s[1] == T(1));
    REQUIRE(s[2] == T(2));
    REQUIRE(s[3] == T());
    REQUIRE(s[4] == T());
  }

  SUBCASE("grow with value") {
    s.resize(5, T(9));
    REQUIRE(s.size() == 5);
    REQUIRE(s[3] == T(9));
    REQUIRE(s[4] == T(9));
  }

  SUBCASE("shrink") {
    s.resize(1);
    REQUIRE(s.size() == 1);
    REQUIRE(s[0] == T(0));
  }

  SUBCASE("no-op") {
    s.resize(3);
    REQUIRE(s.size() == 3);
  }

  SUBCASE("exceeds capacity throws") {
    CHECK_THROWS_AS(s.resize(9), length_error);
    CHECK_THROWS_AS(s.resize(9, T(1)), length_error);
  }
}

TEST_CASE_TEMPLATE("assignment", T, INT_TYPES_TO_TEST) {
  SUBCASE("copy assignment") {
    BoundedVector<T> a(5, {T(1), T(2), T(3)});
    BoundedVector<T> b(2, {T(9)});
    b = a;
    REQUIRE(b.size() == 3);
    REQUIRE(b.capacity() == 5);
    REQUIRE(equal(a.begin(), a.end(), b.begin()));
    b[0] = T(42);
    REQUIRE(a[0] == T(1));
  }

  SUBCASE("copy assignment self") {
    BoundedVector<T> a(5, {T(1), T(2), T(3)});
    a = a;
    REQUIRE(a.size() == 3);
    REQUIRE(a[0] == T(1));
    REQUIRE(a[1] == T(2));
    REQUIRE(a[2] == T(3));
  }

  SUBCASE("move assignment") {
    BoundedVector<T> a(5, {T(1), T(2), T(3)});
    BoundedVector<T> b(2, {T(9)});
    b = move(a);
    REQUIRE(b.size() == 3);
    REQUIRE(b.capacity() == 5);
    REQUIRE(b[0] == T(1));
    REQUIRE(b[1] == T(2));
    REQUIRE(b[2] == T(3));
    REQUIRE(a.size() == 0);
    REQUIRE(a.capacity() == 0);
  }

  SUBCASE("move assignment self") {
    BoundedVector<T> a(5, {T(1), T(2), T(3)});
    a = move(a);
    REQUIRE(a.size() == 3);
    REQUIRE(a[0] == T(1));
  }

  SUBCASE("initializer_list assignment") {
    BoundedVector<T> a(5, {T(1), T(2), T(3)});
    a = {T(7), T(8)};
    REQUIRE(a.size() == 2);
    REQUIRE(a.capacity() == 5);
    REQUIRE(a[0] == T(7));
    REQUIRE(a[1] == T(8));
  }

  SUBCASE("initializer_list assignment exceeds capacity throws") {
    BoundedVector<T> a(2);
    CHECK_THROWS_AS((a = {T(1), T(2), T(3)}), length_error);
  }
}

TEST_CASE_TEMPLATE("assign", T, INT_TYPES_TO_TEST) {
  SUBCASE("count and value") {
    BoundedVector<T> s(3);
    s.assign(2, T(7));
    REQUIRE(s.size() == 2);
    REQUIRE(s.capacity() == 3);
    REQUIRE(s[0] == T(7));
    REQUIRE(s[1] == T(7));
  }

  SUBCASE("range") {
    BoundedVector<T> s(3);
    vector<T> v = {T(1), T(2)};
    s.assign(v.begin(), v.end());
    REQUIRE(s.size() == 2);
    REQUIRE(s.capacity() == 3);
    REQUIRE(s[0] == T(1));
    REQUIRE(s[1] == T(2));
  }

  SUBCASE("initializer_list") {
    BoundedVector<T> s(3);
    s.assign({T(1), T(2)});
    REQUIRE(s.size() == 2);
    REQUIRE(s.capacity() == 3);
    REQUIRE(s[0] == T(1));
    REQUIRE(s[1] == T(2));
  }

  SUBCASE("grows capacity when needed") {
    BoundedVector<T> s(2);
    s.assign(3, T(9));
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 3);
    REQUIRE(s[0] == T(9));
    REQUIRE(s[1] == T(9));
    REQUIRE(s[2] == T(9));
  }

  SUBCASE("clears previous contents") {
    BoundedVector<T> s(5, {T(1), T(2), T(3)});
    s.assign(2, T(8));
    REQUIRE(s.size() == 2);
    REQUIRE(s[0] == T(8));
    REQUIRE(s[1] == T(8));
  }

  SUBCASE("replaces with fewer elements") {
    BoundedVector<T> s(5, {T(1), T(2), T(3), T(4)});
    s.assign(2, T(5));
    REQUIRE(s.size() == 2);
    REQUIRE(s.capacity() == 5);
    REQUIRE(s[0] == T(5));
    REQUIRE(s[1] == T(5));
  }
}

struct Tracked {
  static inline int alive = 0;
  static constexpr int kDestroyed = -424242;

  int value;

  Tracked() : value(0) {
    ++alive;
  }
  Tracked(int v) : value(v) {
    ++alive;
  }
  Tracked(Tracked const& other) : value(other.value) {
    ++alive;
  }
  Tracked(Tracked&& other) noexcept : value(other.value) {
    other.value = -1;
    ++alive;
  }
  Tracked& operator=(Tracked const& other) {
    value = other.value;
    return *this;
  }
  Tracked& operator=(Tracked&& other) noexcept {
    value = other.value;
    other.value = -1;
    return *this;
  }
  ~Tracked() {
    REQUIRE(value != kDestroyed);
    REQUIRE(alive > 0);
    value = kDestroyed;
    --alive;
  }

  static void reset() {
    alive = 0;
  }

  auto operator<=>(Tracked const& other) const = default;
};

TEST_CASE("lifetime: no leaks or double-free with non-trivial type") {
  Tracked::reset();

  SUBCASE("construct and destroy") {
    {
      BoundedVector<Tracked> s(8);
      s.push_back(Tracked(1));
      s.push_back(Tracked(2));
      s.emplace_back(3);
      REQUIRE(Tracked::alive == 3);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("clear destroys exactly the live elements") {
    {
      BoundedVector<Tracked> s(8);
      for (int i = 0; i < 5; ++i) {
        s.push_back(Tracked(i));
      }
      REQUIRE(Tracked::alive == 5);
      s.clear();
      REQUIRE(Tracked::alive == 0);
      REQUIRE(s.capacity() == 8);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("pop does not leave stale live objects") {
    {
      BoundedVector<Tracked> s(8);
      for (int i = 0; i < 5; ++i) {
        s.push_back(Tracked(i));
      }
      s.pop_back();
      REQUIRE(Tracked::alive == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("copy construction") {
    {
      BoundedVector<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedVector<Tracked> b = a;
      REQUIRE(Tracked::alive == 8);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("move construction") {
    {
      BoundedVector<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedVector<Tracked> b = move(a);
      REQUIRE(Tracked::alive == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("copy assignment") {
    {
      BoundedVector<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedVector<Tracked> b(8);
      for (int i = 0; i < 6; ++i) {
        b.push_back(Tracked(100 + i));
      }
      b = a;
      REQUIRE(Tracked::alive == 8);
      REQUIRE(b.size() == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("move assignment") {
    {
      BoundedVector<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedVector<Tracked> b(8);
      for (int i = 0; i < 6; ++i) {
        b.push_back(Tracked(100 + i));
      }
      b = move(a);
      REQUIRE(Tracked::alive == 4);
      REQUIRE(b.size() == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("set_capacity grow") {
    {
      BoundedVector<Tracked> s(4);
      for (int i = 0; i < 3; ++i) {
        s.push_back(Tracked(i));
      }
      s.set_capacity(10);
      REQUIRE(Tracked::alive == 3);
      REQUIRE(s.capacity() == 10);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("set_capacity shrink discarding back") {
    {
      BoundedVector<Tracked> s(8);
      for (int i = 0; i < 6; ++i) {
        s.push_back(Tracked(i));
      }
      s.set_capacity(3);
      REQUIRE(Tracked::alive == 3);
      REQUIRE(s.size() == 3);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("set_capacity to zero") {
    {
      BoundedVector<Tracked> s(8);
      for (int i = 0; i < 4; ++i) {
        s.push_back(Tracked(i));
      }
      s.set_capacity(0);
      REQUIRE(Tracked::alive == 0);
      REQUIRE(s.capacity() == 0);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("resize shrink and grow") {
    {
      BoundedVector<Tracked> s(8);
      for (int i = 0; i < 5; ++i) {
        s.push_back(Tracked(i));
      }
      s.resize(2);
      REQUIRE(Tracked::alive == 2);
      s.resize(6, Tracked(9));
      REQUIRE(Tracked::alive == 6);
    }
    REQUIRE(Tracked::alive == 0);
  }
}

TEST_CASE("string elements exercise element lifetimes") {
  BoundedVector<string> s(4);
  s.push_back("a string long enough to require heap allocation here");
  s.emplace_back("another heap allocated string with plenty of characters ok");
  s.emplace_back("third heap allocated string with plenty of characters");
  REQUIRE(s.size() == 3);

  BoundedVector<string> copy = s;
  REQUIRE(copy == s);

  BoundedVector<string> moved = move(s);
  REQUIRE(moved.size() == 3);

  BoundedVector<string> assigned(1);
  assigned = copy;
  REQUIRE(assigned == copy);

  assigned = move(moved);
  REQUIRE(assigned.size() == 3);

  assigned.set_capacity(10);
  assigned.set_capacity(1);
  REQUIRE(assigned.size() == 1);
  assigned.pop_back();
  REQUIRE(assigned.empty());
}

// A type that counts live instances and throws once a configured number of
// constructions has happened, used to exercise exception safety of the
// fill loops (constructors and copy assignment).
struct Throwing {
  static inline int alive = 0;
  static inline int construct_budget = 0;  // -1 means "never throw"

  int value;

  static void reset(int budget) {
    alive = 0;
    construct_budget = budget;
  }

  static void maybe_throw() {
    if (construct_budget == 0) {
      throw std::runtime_error("Throwing budget exhausted");
    }
    if (construct_budget > 0) {
      --construct_budget;
    }
  }

  Throwing() : value(0) {
    maybe_throw();
    ++alive;
  }
  Throwing(int v) : value(v) {
    maybe_throw();
    ++alive;
  }
  Throwing(Throwing const& other) : value(other.value) {
    maybe_throw();
    ++alive;
  }
  Throwing& operator=(Throwing const& other) = default;
  ~Throwing() {
    --alive;
  }
};

TEST_CASE("exception safety: throwing element constructor does not leak") {
  Throwing::reset(3);
  CHECK_THROWS_AS((BoundedVector<Throwing>(10, 6)), runtime_error);
  REQUIRE(Throwing::alive == 0);

  Throwing::reset(-1);
  Throwing proto(7);
  REQUIRE(Throwing::alive == 1);
  Throwing::construct_budget = 2;
  CHECK_THROWS_AS((BoundedVector<Throwing>(10, 6, proto)), runtime_error);
  REQUIRE(Throwing::alive == 1);

  Throwing::reset(-1);
  BoundedVector<Throwing> src(10);
  for (int i = 0; i < 5; ++i) {
    src.emplace_back(i);
  }
  REQUIRE(Throwing::alive == 5);
  Throwing::construct_budget = 2;
  CHECK_THROWS_AS((BoundedVector<Throwing>(src)), runtime_error);
  REQUIRE(Throwing::alive == 5);

  Throwing::reset(-1);
  BoundedVector<Throwing> src2(10);
  for (int i = 0; i < 5; ++i) {
    src2.emplace_back(i);
  }
  REQUIRE(Throwing::alive == 5);
  Throwing::construct_budget = 2;
  CHECK_THROWS_AS((BoundedVector<Throwing>(src2)), runtime_error);
  REQUIRE(Throwing::alive == 5);

  Throwing::reset(-1);
  BoundedVector<Throwing> src3(10);
  for (int i = 0; i < 5; ++i) {
    src3.emplace_back(i);
  }
  BoundedVector<Throwing> dst(10);
  for (int i = 0; i < 3; ++i) {
    dst.emplace_back(100 + i);
  }
  REQUIRE(Throwing::alive == 8);
  Throwing::construct_budget = 2;
  CHECK_THROWS_AS((dst = src3), runtime_error);
  REQUIRE(Throwing::alive == 5);
}

TEST_CASE_TEMPLATE("pointer types", T, INT_TYPES_TO_TEST) {
  using MemoryResource = std::pmr::unsynchronized_pool_resource;
  MemoryResource memory_resource;
  using Allocator = std::pmr::polymorphic_allocator<T>;
  Allocator allocator(&memory_resource);

  SUBCASE("pointer points to value_type, not cell_type") {
    using Vector = BoundedVector<T, Allocator>;
    static_assert(std::is_same_v<typename Vector::pointer, T*>);
    static_assert(std::is_same_v<typename Vector::const_pointer, T const*>);
  }

  SUBCASE("iterator operator-> returns value_type pointer") {
    std::vector<T> v{1, 2, 3};
    BoundedVector<T, Allocator> s(10, v.begin(), v.end(), allocator);
    auto i = s.begin();
    static_assert(std::is_same_v<decltype(i.operator->()), T*>);
    REQUIRE(*i.operator->() == T(1));
    REQUIRE(*i == T(1));

    auto const& cs = s;
    auto ci = cs.begin();
    static_assert(std::is_same_v<decltype(ci.operator->()), T const*>);
    REQUIRE(*ci.operator->() == T(1));
  }
}

TEST_SUITE_END();
