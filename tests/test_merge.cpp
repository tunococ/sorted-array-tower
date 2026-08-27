#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
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

#define INT_TYPES_TO_TEST size_t

TEST_SUITE_BEGIN("merge");

TEST_CASE_TEMPLATE("merge two non-empty lists", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  vector<T> expected{1, 2, 4, 5, 7, 8};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("merge with empty first list", T, INT_TYPES_TO_TEST) {
  vector<T> v1{};
  vector<T> v2{2, 5, 8};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  vector<T> expected{2, 5, 8};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("merge with empty second list", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  vector<T> expected{1, 4, 7};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("merge both empty lists", T, INT_TYPES_TO_TEST) {
  vector<T> v1{};
  vector<T> v2{};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  REQUIRE(result.empty());
}

TEST_CASE_TEMPLATE("merge with single elements", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1};
  vector<T> v2{2};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  vector<T> expected{1, 2};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("merge with reversed order using custom comparator", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{5, 4, 3};
  vector<T> v2{8, 7, 6};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a > *b; });

  vector<T> expected{8, 7, 6, 5, 4, 3};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("merge with duplicates", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 2, 2, 4};
  vector<T> v2{2, 3, 4};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  vector<T> expected{1, 2, 2, 2, 3, 4, 4};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("merging identical lists", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3, 5};
  vector<T> v2{1, 3, 5};

  vector<T> result;
  sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                            [&result](auto it) { result.push_back(*it); },
                            [](auto a, auto b) { return *a < *b; });

  vector<T> expected{1, 1, 3, 3, 5, 5};
  REQUIRE(result == expected);
}

TEST_CASE_TEMPLATE("fuzz test against std::merge", T, INT_TYPES_TO_TEST) {
  mt19937_64 rand_gen{12345};

  constexpr size_t NUM_REPS = 100;
  constexpr size_t MAX_LIST_SIZE = 20;

  for (size_t rep = 0; rep < NUM_REPS; ++rep) {
    size_t list_size_1 = rand_gen() % MAX_LIST_SIZE;
    size_t list_size_2 = rand_gen() % MAX_LIST_SIZE;

    vector<T> v1;
    v1.reserve(list_size_1);
    for (size_t i = 0; i < list_size_1; ++i) {
      v1.push_back(T(rand_gen() % 100));
    }
    sort(v1.begin(), v1.end());

    vector<T> v2;
    v2.reserve(list_size_2);
    for (size_t i = 0; i < list_size_2; ++i) {
      v2.push_back(T(rand_gen() % 100));
    }
    sort(v2.begin(), v2.end());

    vector<T> expected(list_size_1 + list_size_2);
    merge(v1.begin(), v1.end(), v2.begin(), v2.end(), expected.begin());

    vector<T> result;
    result.reserve(list_size_1 + list_size_2);
    sorted_array_tower::merge(v1.begin(), v1.end(), v2.begin(), v2.end(),
                              [&result](auto it) { result.push_back(*it); },
                              [](auto a, auto b) { return *a < *b; });

    REQUIRE(result == expected);
  }
}

TEST_SUITE_END();
