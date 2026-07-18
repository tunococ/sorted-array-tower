#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iterator>
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

TEST_SUITE_BEGIN("bounded_array");

TEST_CASE_TEMPLATE("constructors", T, INT_TYPES_TO_TEST) {
  SUBCASE("default") {
    BoundedArray<T> s;
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
    REQUIRE(s.capacity() == 0);
  }

  SUBCASE("capacity only") {
    BoundedArray<T> s(0);
    REQUIRE(s.empty());
    REQUIRE(s.capacity() == 0);
    BoundedArray<T> s2(5);
    REQUIRE(s2.empty());
    REQUIRE(s2.capacity() == 5);
  }

  SUBCASE("capacity and count") {
    BoundedArray<T> s(5, 3);
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    for (T v : s) {
      REQUIRE(v == T());
    }
  }

  SUBCASE("capacity, count and value") {
    BoundedArray<T> s(5, 3, T(7));
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    for (T v : s) {
      REQUIRE(v == T(7));
    }
  }

  SUBCASE("range") {
    vector<T> v = {T(1), T(2), T(3)};
    BoundedArray<T> s(5, v.begin(), v.end());
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    REQUIRE(equal(s.begin(), s.end(), v.begin()));
  }

  SUBCASE("initializer_list") {
    BoundedArray<T> s(5, {T(1), T(2), T(3)});
    REQUIRE(s.size() == 3);
    REQUIRE(s.capacity() == 5);
    REQUIRE(s[0] == T(1));
    REQUIRE(s[1] == T(2));
    REQUIRE(s[2] == T(3));
  }

  SUBCASE("copy") {
    BoundedArray<T> s1(5, {T(1), T(2), T(3)});
    BoundedArray<T> s2 = s1;
    REQUIRE(equal(s1.begin(), s1.end(), s2.begin()));
    REQUIRE(s1.capacity() == s2.capacity());
  }

  SUBCASE("move") {
    BoundedArray<T> s1(5, {T(1), T(2), T(3)});
    BoundedArray<T> s2 = move(s1);
    REQUIRE(s2.size() == 3);
    REQUIRE(s1.size() == 0);
    REQUIRE(s1.capacity() == 0);
  }
}

TEST_CASE_TEMPLATE("push_pop", T, INT_TYPES_TO_TEST) {
  BoundedArray<T> s(4);

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

  SUBCASE("push_front") {
    s.push_front(T(1));
    s.push_front(T(2));
    s.push_front(T(3));
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == T(3));
    REQUIRE(s[1] == T(2));
    REQUIRE(s[2] == T(1));
    REQUIRE(s.front() == T(3));
    REQUIRE(s.back() == T(1));
  }

  SUBCASE("mixed") {
    s.push_back(T(1));
    s.push_front(T(2));
    s.push_back(T(3));
    s.push_front(T(4));
    REQUIRE(s.size() == 4);
    REQUIRE(s[0] == T(4));
    REQUIRE(s[1] == T(2));
    REQUIRE(s[2] == T(1));
    REQUIRE(s[3] == T(3));
  }

  SUBCASE("pop_back") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.pop_back();
    REQUIRE(s.size() == 1);
    REQUIRE(s[0] == T(1));
  }

  SUBCASE("pop_front") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.pop_front();
    REQUIRE(s.size() == 1);
    REQUIRE(s[0] == T(2));
  }

  SUBCASE("full then push throws") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.push_back(T(3));
    s.push_back(T(4));
    REQUIRE(s.full());
    CHECK_THROWS_AS(s.push_back(T(5)), length_error);
    CHECK_THROWS_AS(s.push_front(T(5)), length_error);
    CHECK_THROWS_AS(s.emplace_back(T(5)), length_error);
    CHECK_THROWS_AS(s.emplace_front(T(5)), length_error);
  }

  SUBCASE("wrap around") {
    s.push_back(T(1));
    s.push_back(T(2));
    s.push_back(T(3));
    s.pop_front();
    s.push_back(T(4));
    s.pop_front();
    s.push_back(T(5));
    REQUIRE(s.size() == 3);
    REQUIRE(s[0] == T(3));
    REQUIRE(s[1] == T(4));
    REQUIRE(s[2] == T(5));
  }
}

TEST_CASE_TEMPLATE("emplace", T, INT_TYPES_TO_TEST) {
  BoundedArray<T> s(4);
  s.emplace_back(T(1));
  s.emplace_front(T(2));
  REQUIRE(s[0] == T(2));
  REQUIRE(s[1] == T(1));
}

TEST_CASE_TEMPLATE("set_capacity", T, INT_TYPES_TO_TEST) {
  BoundedArray<T> s(8);
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

  SUBCASE("set capacity after wrap-around") {
    s.pop_front();
    s.pop_front();
    s.push_back(T(5));
    s.push_back(T(6));
    s.set_capacity(10);
    REQUIRE(s.size() == 5);
    REQUIRE(s[0] == T(2));
    REQUIRE(s[1] == T(3));
    REQUIRE(s[2] == T(4));
    REQUIRE(s[3] == T(5));
    REQUIRE(s[4] == T(6));
  }
}

TEST_CASE_TEMPLATE("access", T, INT_TYPES_TO_TEST) {
  BoundedArray<T> s(5, {T(1), T(2), T(3)});
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
  BoundedArray<T> s(5, {T(1), T(2), T(3)});
  s.clear();
  REQUIRE(s.empty());
  REQUIRE(s.capacity() == 5);
  s.push_back(T(7));
  REQUIRE(s.size() == 1);
  REQUIRE(s[0] == T(7));
}

TEST_CASE_TEMPLATE("comparison", T, INT_TYPES_TO_TEST) {
  BoundedArray<T> a(5, {T(1), T(2), T(3)});
  BoundedArray<T> b(5, {T(1), T(2), T(3)});
  BoundedArray<T> c(5, {T(1), T(2), T(4)});
  BoundedArray<T> d(5, {T(1), T(2)});
  REQUIRE(a == b);
  REQUIRE(a != c);
  REQUIRE(a < c);
  REQUIRE(d < a);
  REQUIRE(!(a == d));
}

TEST_CASE_TEMPLATE("fuzz against deque", T, INT_TYPES_TO_TEST) {
  mt19937 rng(12345);
  uniform_int_distribution<int> op_dist(0, 3);
  uniform_int_distribution<int> val_dist(0, 100);

  BoundedArray<T> s(64);
  deque<T> ref;

  for (int iter = 0; iter < 5000; ++iter) {
    int op = op_dist(rng);
    switch (op) {
      case 0:  // push_front
        if (ref.size() < 64) {
          T v = val_dist(rng);
          s.push_front(v);
          ref.push_front(v);
        }
        break;
      case 1:  // push_back
        if (ref.size() < 64) {
          T v = val_dist(rng);
          s.push_back(v);
          ref.push_back(v);
        }
        break;
      case 2:  // pop_front
        if (!ref.empty()) {
          s.pop_front();
          ref.pop_front();
        }
        break;
      case 3:  // pop_back
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
  BoundedArray<T> s(8);
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
    BoundedArray<T> a(5, {T(1), T(2), T(3)});
    BoundedArray<T> b(2, {T(9)});
    b = a;
    REQUIRE(b.size() == 3);
    REQUIRE(b.capacity() == 5);
    REQUIRE(equal(a.begin(), a.end(), b.begin()));
    // Mutating the copy must not affect the source.
    b[0] = T(42);
    REQUIRE(a[0] == T(1));
  }

  SUBCASE("copy assignment self") {
    BoundedArray<T> a(5, {T(1), T(2), T(3)});
    a = a;
    REQUIRE(a.size() == 3);
    REQUIRE(a[0] == T(1));
    REQUIRE(a[1] == T(2));
    REQUIRE(a[2] == T(3));
  }

  SUBCASE("move assignment") {
    BoundedArray<T> a(5, {T(1), T(2), T(3)});
    BoundedArray<T> b(2, {T(9)});
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
    BoundedArray<T> a(5, {T(1), T(2), T(3)});
    a = move(a);
    REQUIRE(a.size() == 3);
    REQUIRE(a[0] == T(1));
  }

  SUBCASE("initializer_list assignment") {
    BoundedArray<T> a(5, {T(1), T(2), T(3)});
    a = {T(7), T(8)};
    REQUIRE(a.size() == 2);
    REQUIRE(a.capacity() == 5);
    REQUIRE(a[0] == T(7));
    REQUIRE(a[1] == T(8));
  }

  SUBCASE("initializer_list assignment exceeds capacity throws") {
    BoundedArray<T> a(2);
    CHECK_THROWS_AS((a = {T(1), T(2), T(3)}), length_error);
  }

  SUBCASE("copy assignment after wrap-around") {
    BoundedArray<T> a(4);
    a.push_back(T(1));
    a.push_back(T(2));
    a.push_back(T(3));
    a.pop_front();
    a.push_back(T(4));  // logical: 2, 3, 4 wrapped in buffer
    BoundedArray<T> b(1);
    b = a;
    REQUIRE(b.size() == 3);
    REQUIRE(b[0] == T(2));
    REQUIRE(b[1] == T(3));
    REQUIRE(b[2] == T(4));
  }
}

// A type that tracks the number of live instances to detect leaks and
// double-destruction. `alive` is incremented by every constructor and
// decremented by the destructor; a correct container leaves it at zero and
// never drives it negative. Each object also carries a sentinel so that a
// second destruction of the same object is caught even while other instances
// remain alive.
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
    // A destroyed object is poisoned with a sentinel; seeing it again means
    // this exact object was destroyed twice, which the global `alive` count
    // alone would miss whenever other instances are still alive.
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
      BoundedArray<Tracked> s(8);
      s.push_back(Tracked(1));
      s.push_back(Tracked(2));
      s.emplace_back(3);
      s.push_front(Tracked(0));
      REQUIRE(Tracked::alive == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("clear destroys exactly the live elements") {
    {
      BoundedArray<Tracked> s(8);
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
      BoundedArray<Tracked> s(8);
      for (int i = 0; i < 5; ++i) {
        s.push_back(Tracked(i));
      }
      s.pop_front();
      s.pop_back();
      REQUIRE(Tracked::alive == 3);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("copy construction") {
    {
      BoundedArray<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedArray<Tracked> b = a;
      REQUIRE(Tracked::alive == 8);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("move construction") {
    {
      BoundedArray<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedArray<Tracked> b = move(a);
      // Move should transfer storage, not create new live objects.
      REQUIRE(Tracked::alive == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("copy assignment") {
    {
      BoundedArray<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedArray<Tracked> b(8);
      for (int i = 0; i < 6; ++i) {
        b.push_back(Tracked(100 + i));
      }
      b = a;  // b's 6 old elements must be destroyed, 4 new copied in
      REQUIRE(Tracked::alive == 8);
      REQUIRE(b.size() == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("move assignment") {
    {
      BoundedArray<Tracked> a(8);
      for (int i = 0; i < 4; ++i) {
        a.push_back(Tracked(i));
      }
      BoundedArray<Tracked> b(8);
      for (int i = 0; i < 6; ++i) {
        b.push_back(Tracked(100 + i));
      }
      b = move(a);  // b's 6 old elements destroyed, a's storage stolen
      REQUIRE(Tracked::alive == 4);
      REQUIRE(b.size() == 4);
    }
    REQUIRE(Tracked::alive == 0);
  }

  SUBCASE("set_capacity grow") {
    {
      BoundedArray<Tracked> s(4);
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
      BoundedArray<Tracked> s(8);
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
      BoundedArray<Tracked> s(8);
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
      BoundedArray<Tracked> s(8);
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

  SUBCASE("lifetime survives wrap-around and reallocation") {
    {
      BoundedArray<Tracked> s(4);
      s.push_back(Tracked(1));
      s.push_back(Tracked(2));
      s.push_back(Tracked(3));
      s.pop_front();
      s.pop_front();
      s.push_back(Tracked(4));
      s.push_back(Tracked(5));  // wrapped: logical 3,4,5
      REQUIRE(Tracked::alive == 3);
      s.set_capacity(16);  // reallocation must move+destroy exactly 3
      REQUIRE(Tracked::alive == 3);
      REQUIRE(s[0].value == 3);
      REQUIRE(s[1].value == 4);
      REQUIRE(s[2].value == 5);
    }
    REQUIRE(Tracked::alive == 0);
  }
}

TEST_CASE("string elements exercise element lifetimes") {
  // std::string owns heap memory, so a double-free or leak in element
  // management is caught by sanitizers here.
  BoundedArray<string> s(4);
  s.push_back("a string long enough to require heap allocation here");
  s.push_front("another heap allocated string of sufficient length ok");
  s.emplace_back("third heap allocated string with plenty of characters");
  REQUIRE(s.size() == 3);

  BoundedArray<string> copy = s;
  REQUIRE(copy == s);

  BoundedArray<string> moved = move(s);
  REQUIRE(moved.size() == 3);

  BoundedArray<string> assigned(1);
  assigned = copy;
  REQUIRE(assigned == copy);

  assigned = move(moved);
  REQUIRE(assigned.size() == 3);

  assigned.set_capacity(10);
  assigned.set_capacity(1);  // shrink, destroying back elements
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
  SUBCASE("count constructor") {
    Throwing::reset(3);  // allow 3 constructions, throw on the 4th
    CHECK_THROWS_AS((BoundedArray<Throwing>(10, 6)), runtime_error);
    REQUIRE(Throwing::alive == 0);
  }

  SUBCASE("value constructor") {
    Throwing::reset(-1);
    Throwing proto(7);  // construct the prototype with throwing disabled
    REQUIRE(Throwing::alive == 1);
    Throwing::construct_budget = 2;  // now allow 2 copies, throw on the 3rd
    CHECK_THROWS_AS((BoundedArray<Throwing>(10, 6, proto)), runtime_error);
    REQUIRE(Throwing::alive == 1);  // only proto remains
  }

  SUBCASE("copy constructor") {
    Throwing::reset(-1);
    BoundedArray<Throwing> src(10);
    for (int i = 0; i < 5; ++i) {
      src.emplace_back(i);
    }
    REQUIRE(Throwing::alive == 5);
    Throwing::construct_budget = 2;  // copy will throw on 3rd element
    CHECK_THROWS_AS((BoundedArray<Throwing>(src)), runtime_error);
    REQUIRE(Throwing::alive == 5);  // only src's elements remain
  }

  SUBCASE("copy assignment") {
    Throwing::reset(-1);
    BoundedArray<Throwing> src(10);
    for (int i = 0; i < 5; ++i) {
      src.emplace_back(i);
    }
    BoundedArray<Throwing> dst(10);
    for (int i = 0; i < 3; ++i) {
      dst.emplace_back(100 + i);
    }
    REQUIRE(Throwing::alive == 8);
    Throwing::construct_budget = 2;  // assignment throws on 3rd copied element
    CHECK_THROWS_AS((dst = src), runtime_error);
    // dst's old 3 elements were destroyed; the 2 partially-copied elements
    // were cleaned up; src's 5 remain.
    REQUIRE(Throwing::alive == 5);
  }
}

TEST_SUITE_END();
