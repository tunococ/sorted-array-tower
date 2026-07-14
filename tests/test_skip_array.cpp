#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iterator>
#include <memory_resource>
#include <random>
#include <vector>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/sorted_array_tower.hpp>
#endif

using namespace sorted_array_tower;
using namespace std;

#define INT_TYPES_TO_TEST size_t

TEST_SUITE_BEGIN("skip_array");

TEST_CASE_TEMPLATE("constructors", T, INT_TYPES_TO_TEST) {
  SUBCASE("default") {
    SkipArray<T> s;
    REQUIRE(s.empty());
    REQUIRE(s.size() == 0);
  }

  SUBCASE("fill") {
    {
      SkipArray<T> s(0);
      vector<T> v(0);
      REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
      REQUIRE(s.size() == v.size());
      REQUIRE(s.empty() == v.empty());
    }
    {
      SkipArray<T> s(5);
      vector<T> v(5);
      REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
      REQUIRE(s.size() == v.size());
      REQUIRE(s.empty() == v.empty());
    }
    {
      SkipArray<T> s(5, T(1));
      vector<T> v(5, T(1));
      REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
      REQUIRE(s.size() == v.size());
      REQUIRE(s.empty() == v.empty());
    }
    {
      vector<T> v(5, T(1));
      SkipArray<T> s(v.begin(), v.end());
      REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
      REQUIRE(s.size() == v.size());
      REQUIRE(s.empty() == v.empty());
    }
  }

  SUBCASE("copy") {
    {
      SkipArray<T> s_1(0);
      SkipArray<T> s_2 = s_1;
      REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
    }
    {
      SkipArray<T> s_1(5);
      SkipArray<T> s_2 = s_1;
      REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
    }
    {
      SkipArray<T> s_1(5, T(1));
      SkipArray<T> s_2 = s_1;
      REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
    }
  }

  SUBCASE("move") {
    {
      vector<T> v(0);
      SkipArray<T> s_1(v.begin(), v.end());
      SkipArray<T> s_2 = std::move(s_1);
      REQUIRE(std::equal(s_2.begin(), s_2.end(), v.begin()));
      REQUIRE(s_1.empty());
    }
    {
      vector<T> v(5, T(1));
      SkipArray<T> s_1(v.begin(), v.end());
      SkipArray<T> s_2 = std::move(s_1);
      REQUIRE(std::equal(s_2.begin(), s_2.end(), v.begin()));
      REQUIRE(s_1.empty());
    }
  }

  SUBCASE("with allocator") {
    using MemoryResource = std::pmr::unsynchronized_pool_resource;
    MemoryResource memory_resource;
    using Allocator = std::pmr::polymorphic_allocator<T>;
    Allocator allocator(&memory_resource);

    SUBCASE("default") {
      SkipArray<T, Allocator> s{allocator};
      REQUIRE(s.size() == 0);
      REQUIRE(s.empty());
    }

    SUBCASE("fill") {
      {
        SkipArray<T, Allocator> s(0, allocator);
        vector<T> v(0);
        REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
        REQUIRE(s.size() == v.size());
        REQUIRE(s.empty() == v.empty());
      }
      {
        SkipArray<T, Allocator> s(5, allocator);
        vector<T> v(5);
        REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
        REQUIRE(s.size() == v.size());
        REQUIRE(s.empty() == v.empty());
      }
      {
        SkipArray<T, Allocator> s(5, T(1), allocator);
        vector<T> v(5, T(1));
        REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
        REQUIRE(s.size() == v.size());
        REQUIRE(s.empty() == v.empty());
      }
      {
        vector<T> v(5, T(1));
        SkipArray<T, Allocator> s(v.begin(), v.end(), allocator);
        REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
        REQUIRE(s.size() == v.size());
        REQUIRE(s.empty() == v.empty());
      }
    }

    SUBCASE("copy") {
      {
        SkipArray<T, Allocator> s_1(0);
        SkipArray<T, Allocator> s_2(s_1, allocator);
        REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
      }
      {
        SkipArray<T, Allocator> s_1(5, allocator);
        SkipArray<T, Allocator> s_2(s_1, allocator);
        REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
      }
      {
        SkipArray<T, Allocator> s_1(5, T(1));
        SkipArray<T, Allocator> s_2(s_1, allocator);
        REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
      }
    }

    SUBCASE("move") {
      {
        vector<T> v(0);
        SkipArray<T, Allocator> s_1(v.begin(), v.end());
        SkipArray<T, Allocator> s_2(std::move(s_1), allocator);
        REQUIRE(std::equal(s_2.begin(), s_2.end(), v.begin()));
        REQUIRE(s_1.empty());
      }
      {
        vector<T> v(5, T(1));
        SkipArray<T, Allocator> s_1(v.begin(), v.end());
        SkipArray<T, Allocator> s_2(std::move(s_1), allocator);
        REQUIRE(std::equal(s_2.begin(), s_2.end(), v.begin()));
        REQUIRE(s_1.empty());
      }
    }
  }
}

TEST_CASE_TEMPLATE("assignments", T, INT_TYPES_TO_TEST) {
  using MemoryResource = std::pmr::unsynchronized_pool_resource;
  MemoryResource memory_resource;
  using Allocator = std::pmr::polymorphic_allocator<T>;
  Allocator allocator(&memory_resource);

  SUBCASE("copy") {
    {
      SkipArray<T, Allocator> s_1(5, allocator);
      SkipArray<T, Allocator> s_2(allocator);
      s_2 = s_1;
      REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
    }
    {
      SkipArray<T, Allocator> s_1(5, allocator);
      SkipArray<T, Allocator> s_2(10, allocator);
      s_2 = s_1;
      REQUIRE(std::equal(s_1.begin(), s_1.end(), s_2.begin()));
    }
  }

  SUBCASE("move") {
    {
      vector<T> v(5, T(1));
      SkipArray<T, Allocator> s_1(v.begin(), v.end());
      SkipArray<T, Allocator> s_2(10, allocator);
      s_2 = std::move(s_1);
      REQUIRE(std::equal(s_2.begin(), s_2.end(), v.begin()));
      REQUIRE(s_1.empty());
    }
    {
      vector<T> v(5, T(1));
      SkipArray<T, Allocator> s_1(v.begin(), v.end(), allocator);
      SkipArray<T, Allocator> s_2(10);
      s_2 = std::move(s_1);
      REQUIRE(std::equal(s_2.begin(), s_2.end(), v.begin()));
      REQUIRE(s_1.empty());
    }
  }
}

using Value = size_t;

template <template <typename, typename> typename BaseArray>
using SkipArrayTemplate = SkipArray<Value, allocator<Value>, BaseArray>;

#define CONTAINER_TYPES_TO_TEST vector, deque

TEST_CASE_TEMPLATE("push_back and pop", SkipArrayType,
                   SkipArrayTemplate<vector>, SkipArrayTemplate<deque>) {
  SUBCASE("emplace_back") {
    vector<Value> v{1, 2, 3, 4, 5};
    SkipArrayType s;
    for (Value const& value : v) {
      auto& inserted_value = s.emplace_back(value);
      CHECK_EQ(inserted_value, value);
    }
    REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
  }

  SUBCASE("push_back") {
    vector<Value> v{1, 2, 3, 4, 5};
    SkipArrayType s;
    for (Value const& value : v) {
      s.push_back(value);
    }
    REQUIRE(std::equal(s.begin(), s.end(), v.begin()));
  }

  if constexpr (requires { declval<SkipArrayType>().emplace_front(0); }) {
    SUBCASE("emplace_front") {
      vector<Value> v{1, 2, 3, 4, 5};
      SkipArrayType s;
      for (Value const& value : v) {
        auto& inserted_value = s.emplace_front(value);
        CHECK_EQ(inserted_value, value);
      }
      REQUIRE(std::equal(s.begin(), s.end(), v.rbegin()));
    }

    SUBCASE("push_front") {
      vector<Value> v{1, 2, 3, 4, 5};
      SkipArrayType s;
      for (Value const& value : v) {
        s.push_front(value);
      }
      REQUIRE(std::equal(s.begin(), s.end(), v.rbegin()));
    }
  }

  SUBCASE("erase") {
    vector<Value> v{1, 2, 3, 4, 5};
    SkipArrayType s(v.begin(), v.end());
    REQUIRE(s == v);

    {
      v.erase(v.begin());
      auto i = s.erase(s.begin());
      REQUIRE(s == v);
      REQUIRE(s.size() == v.size());
      REQUIRE(i == s.begin());
    }

    {
      v.erase(-- --v.end());
      auto i = s.erase(-- --s.end());
      REQUIRE(s == v);
      REQUIRE(s.size() == v.size());
      REQUIRE(i == std::prev(s.end()));
    }

    v.erase(-- --v.end());
    s.erase(-- --s.end());
    REQUIRE(s == v);
    REQUIRE(s.size() == v.size());

    v.erase(-- --v.end());
    s.erase(-- --s.end());
    REQUIRE(s == v);
    REQUIRE(s.size() == v.size());

    {
      v.erase(--v.end());
      auto i = s.erase(--s.end());
      REQUIRE(s == v);
      REQUIRE(s.size() == v.size());
      REQUIRE(s.empty());
      REQUIRE(i == s.end());
    }
  }

  SUBCASE("random edits") {
    deque<Value> v;
    SkipArrayType s;
    mt19937_64 rand_gen(12345);

    constexpr size_t max_size = 100;
    for (size_t i = 0; i < 1000; ++i) {
      REQUIRE(s == v);
      if (v.size() <= size_t(rand_gen() % max_size)) {
        if constexpr (requires { declval<SkipArrayType>().emplace_front(0); }) {
          if (rand_gen() % 2 == 0) {
            v.emplace_front(i);
            s.emplace_front(i);
            continue;
          }
        }
        v.emplace_back(i);
        s.emplace_back(i);
        continue;
      }
      size_t j = rand_gen() % v.size();
      auto next_v = v.erase(v.begin() + j);
      auto next_s = s.erase(std::next(s.begin(), j));
      if (next_v == v.end()) {
        REQUIRE(next_s == s.end());
      } else {
        REQUIRE(*next_v == *next_s);
      }
    }
    REQUIRE(s == v);
  }
}

TEST_CASE_TEMPLATE("searches", SkipArrayType, SkipArrayTemplate<vector>,
                   SkipArrayTemplate<deque>) {
  SUBCASE("simple searches") {
    SkipArrayType s = {2, 2, 2, 4, 4, 4, 6, 6, 6};

    // lower_bound
    {
      auto i = s.lower_bound(1);
      CHECK(*i == 2);
      CHECK(std::distance(s.begin(), i) == 0);
      CHECK(i == s.lower_bound(2));
    }
    {
      auto i = s.lower_bound(3);
      CHECK(*i == 4);
      CHECK(std::distance(s.begin(), i) == 3);
      CHECK(i == s.lower_bound(4));
    }
    {
      auto i = s.lower_bound(5);
      CHECK(*i == 6);
      CHECK(std::distance(s.begin(), i) == 6);
      CHECK(i == s.lower_bound(6));
    }
    {
      auto i = s.lower_bound(7);
      CHECK(i == s.end());
    }

    // upper_bound
    {
      auto i = s.upper_bound(1);
      CHECK(*i == 2);
      CHECK(std::distance(s.begin(), i) == 0);
    }
    {
      auto i = s.upper_bound(2);
      CHECK(*i == 4);
      CHECK(std::distance(s.begin(), i) == 3);
      CHECK(i == s.upper_bound(3));
    }
    {
      auto i = s.upper_bound(4);
      CHECK(*i == 6);
      CHECK(std::distance(s.begin(), i) == 6);
      CHECK(i == s.upper_bound(5));
    }
    {
      auto i = s.upper_bound(6);
      CHECK(i == s.end());
    }

    // find
    {
      CHECK(*s.find(2) == 2);
      CHECK(*s.find(4) == 4);
      CHECK(*s.find(6) == 6);
      CHECK(s.find(1) == s.end());
      CHECK(s.find(3) == s.end());
      CHECK(s.find(5) == s.end());
      CHECK(s.find(7) == s.end());
    }

    // count
    {
      CHECK(s.count(1) == 0);
      CHECK(s.count(2) == 3);
      CHECK(s.count(3) == 0);
      CHECK(s.count(4) == 3);
      CHECK(s.count(5) == 0);
      CHECK(s.count(6) == 3);
      CHECK(s.count(7) == 0);
    }
  }

  SUBCASE("searches with gaps") {
    constexpr size_t num_iterations = 200;
    constexpr size_t num_values = 10;
    constexpr size_t num_dupes = 4;

    mt19937_64 rand_gen(12345);
    for (size_t iter = 0; iter < num_iterations; ++iter) {
      vector<Value> v;
      SkipArrayType s;
      for (size_t i = 0; i < num_values; ++i) {
        for (size_t j = 0; j < num_dupes; ++j) {
          v.emplace_back(1 + (i * 2));
          s.emplace_back(1 + (i * 2));
        }
      }

      while (!v.empty()) {
        size_t index = rand_gen() % v.size();
        v.erase(v.begin() + index);
        s.erase(std::next(s.begin(), index));

        for (Value value = 0; value <= num_values * 2; ++value) {
          auto [v_lb, v_ub] = std::equal_range(v.begin(), v.end(), value);
          auto [s_lb, s_ub] = s.equal_range(value);
          CHECK(distance(v.begin(), v_lb) == distance(s.begin(), s_lb));
          CHECK(distance(v.begin(), v_ub) == distance(s.begin(), s_ub));
          auto v_f = v_lb == v_ub ? v.end() : v_lb;
          auto s_f = s.find(value);
          CHECK(distance(v.begin(), v_f) == distance(s.begin(), s_f));
        }
      }
    }
  }

  SUBCASE("random edits") {
    deque<Value> v;
    SkipArrayType s;
    mt19937_64 rand_gen(12345);

    constexpr size_t max_size = 100;
    constexpr size_t num_searches = 50;
    constexpr size_t num_edits = 1000;
    // Every time an element is added, offset might increase with
    // probability 1/density.
    constexpr size_t density = 20;
    constexpr Value start_value{num_edits * 2};
    Value offset{0};
    for (size_t i = 0; i < num_edits; ++i) {
      if (!v.empty()) {
        for (size_t j = 0; j < num_searches; ++j) {
          Value min_value = start_value - offset - offset / 10;
          Value value_range = 2 * offset + offset / 5 + 1;
          Value search_value = Value{rand_gen()} % value_range + min_value;

          {
            auto [v_lb, v_ub] =
                std::equal_range(v.begin(), v.end(), search_value);
            auto [s_lb, s_ub] = s.equal_range(search_value);
            CHECK(std::distance(v.begin(), v_lb) ==
                  std::distance(s.begin(), s_lb));
            CHECK(std::distance(v.begin(), v_ub) ==
                  std::distance(s.begin(), s_ub));
            auto v_f = v_lb == v_ub ? v.end() : v_lb;
            auto s_f = s.find(search_value);
            CHECK(std::distance(v.begin(), v_f) ==
                  std::distance(s.begin(), s_f));
          }
        }
      }
      if (v.size() <= size_t(rand_gen() % max_size)) {
        if (rand_gen() % num_edits < num_edits / density) {
          ++offset;
        }
        if constexpr (requires {
                        declval<SkipArrayType>().emplace_front(start_value);
                      }) {
          if (rand_gen() % 2 == 0) {
            v.emplace_front(start_value - offset);
            s.emplace_front(start_value - offset);
            continue;
          }
        }
        v.emplace_back(start_value + offset);
        s.emplace_back(start_value + offset);
        continue;
      }
      size_t j = rand_gen() % v.size();
      auto next_v = v.erase(v.begin() + j);
      auto next_s = s.erase(std::next(s.begin(), j));
      if (next_v == v.end()) {
        REQUIRE(next_s == s.end());
      } else {
        REQUIRE(*next_v == *next_s);
      }
    }
  }
}

TEST_SUITE_END();
