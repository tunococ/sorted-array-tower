#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <random>
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

TEST_SUITE_END();
