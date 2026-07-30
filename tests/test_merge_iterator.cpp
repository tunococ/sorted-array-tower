#include <doctest/doctest.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
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

TEST_SUITE_BEGIN("merge_iterator");

TEST_CASE_TEMPLATE("default constructor", T, INT_TYPES_TO_TEST) {
  MergeIterator<typename vector<T>::iterator> it;
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("add_list and iterate", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};
  vector<T> v3{3, 6, 9};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  it.add_list(v3.begin(), v3.end());

  vector<T> expected{1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("post-increment returns old value", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3, 5};
  vector<T> v2{2, 4, 6};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  auto old = it++;
  REQUIRE(*old == T(1));
  REQUIRE(*it == T(2));
}

TEST_CASE_TEMPLATE("make_begin resets iteration", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3, 5};
  vector<T> v2{2, 4, 6};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  ++it;
  ++it;
  REQUIRE(*it == T(3));

  it = it.make_begin();
  REQUIRE(*it == T(1));
}

TEST_CASE_TEMPLATE("make_end advances all to end", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3};
  vector<T> v2{2, 4};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  auto end_it = it.make_end();
  REQUIRE(end_it.is_past_end());
}

TEST_CASE_TEMPLATE("move_to_begin and move_to_end", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3};
  vector<T> v2{2, 4};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  ++it;
  it.move_to_begin();
  REQUIRE(*it == T(1));

  it.move_to_end();
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("location", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5};
  vector<T> v3{3, 6, 8, 9};

  MergeIterator<typename vector<T>::iterator> it;
  size_t idx1 = it.add_list(v1.begin(), v1.end());
  size_t idx2 = it.add_list(v2.begin(), v2.end());
  size_t idx3 = it.add_list(v3.begin(), v3.end());

  auto loc = it.location();
  REQUIRE(loc.first == idx1);
  REQUIRE(*loc.second == T(1));

  ++it;
  loc = it.location();
  REQUIRE(loc.first == idx2);
  REQUIRE(*loc.second == T(2));
}

TEST_CASE_TEMPLATE("empty lists", T, INT_TYPES_TO_TEST) {
  vector<T> v1{};
  vector<T> v2{1, 2, 3};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  vector<T> expected{1, 2, 3};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("all empty lists", T, INT_TYPES_TO_TEST) {
  vector<T> v1{};
  vector<T> v2{};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("single list", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 2, 3};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());

  for (T expected : v1) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("custom comparator", T, INT_TYPES_TO_TEST) {
  vector<T> v1{5, 4, 3};
  vector<T> v2{10, 9, 8};

  MergeIterator<typename vector<T>::iterator, greater<T>> it{greater<T>()};
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  vector<T> expected{10, 9, 8, 5, 4, 3};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("copy constructor", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;

  MergeIterator<typename vector<T>::iterator> it2 = it;
  REQUIRE(*it2 == T(2));
  ++it2;
  REQUIRE(*it2 == T(4));
}

TEST_CASE_TEMPLATE("move constructor", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;

  MergeIterator<typename vector<T>::iterator> it2 = move(it);
  REQUIRE(*it2 == T(2));
}

TEST_CASE_TEMPLATE("copy assignment", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;

  MergeIterator<typename vector<T>::iterator> it2;
  it2 = it;
  REQUIRE(*it2 == T(2));
}

TEST_CASE_TEMPLATE("move assignment", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;

  MergeIterator<typename vector<T>::iterator> it2;
  it2 = move(it);
  REQUIRE(*it2 == T(2));
}

TEST_CASE_TEMPLATE("copy constructor preserves equality", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;

  MergeIterator<typename vector<T>::iterator> it2 = it;
  REQUIRE(it == it2);
}

TEST_CASE_TEMPLATE("copy assignment preserves equality", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;

  MergeIterator<typename vector<T>::iterator> it2;
  it2 = it;
  REQUIRE(it == it2);
}

TEST_CASE_TEMPLATE("equality", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3, 5};
  vector<T> v2{2, 4, 6};

  MergeIterator<typename vector<T>::iterator> it1;
  it1.add_list(v1.begin(), v1.end());
  it1.add_list(v2.begin(), v2.end());

  MergeIterator<typename vector<T>::iterator> it2;
  it2.add_list(v1.begin(), v1.end());
  it2.add_list(v2.begin(), v2.end());

  REQUIRE(it1 == it2);

  ++it1;
  REQUIRE_FALSE(it1 == it2);

  ++it2;
  REQUIRE(it1 == it2);
}

TEST_CASE_TEMPLATE("reserve", T, INT_TYPES_TO_TEST) {
  MergeIterator<typename vector<T>::iterator> it;
  it.reserve(10);
  REQUIRE(it.is_past_end());

  vector<T> v1{1, 2};
  it.add_list(v1.begin(), v1.end());
  REQUIRE_FALSE(it.is_past_end());
}

TEST_CASE_TEMPLATE("add_list with custom current", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 2, 3, 4, 5};

  MergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end(), v1.begin() + 2);

  REQUIRE(*it == T(3));
  REQUIRE(*it == T(3));
  ++it;
  REQUIRE(*it == T(4));
}

TEST_CASE_TEMPLATE("fuzz test", T, INT_TYPES_TO_TEST) {
  mt19937_64 rand_gen{12345};

  constexpr size_t NUM_REPS = 100;
  constexpr size_t MAX_LIST_SIZE = 20;
  constexpr size_t NUM_LISTS[] = {0, 1, 10, 100};

  for (size_t num_lists : NUM_LISTS) {
    for (size_t rep = 0; rep < NUM_REPS; ++rep) {
      vector<vector<T>> lists(num_lists);
      vector<T> merged;

      for (size_t i = 0; i < num_lists; ++i) {
        size_t list_size = rand_gen() % MAX_LIST_SIZE;
        lists[i].resize(list_size);
        for (size_t j = 0; j < list_size; ++j) {
          lists[i][j] = T(rand_gen() % 100);
        }
        sort(lists[i].begin(), lists[i].end());
        merged.insert(merged.end(), lists[i].begin(), lists[i].end());
      }
      sort(merged.begin(), merged.end());

      MergeIterator<typename vector<T>::iterator> it;
      for (size_t i = 0; i < num_lists; ++i) {
        it.add_list(lists[i].begin(), lists[i].end());
      }

      for (size_t i = 0; i < merged.size(); ++i) {
        REQUIRE_FALSE(it.is_past_end());
        CHECK(*it == merged[i]);
        ++it;
      }
      REQUIRE(it.is_past_end());
    }
  }
}

TEST_CASE_TEMPLATE("bidirectional default constructor", T, INT_TYPES_TO_TEST) {
  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  REQUIRE(it.is_past_end());
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional forward and backward iteration", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};
  vector<T> v3{3, 6, 9};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  it.add_list(v3.begin(), v3.end());

  vector<T> expected{1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional post-increment and post-decrement", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3, 5};
  vector<T> v2{2, 4, 6};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  auto old = it++;
  REQUIRE(*old == T(1));
  REQUIRE(*it == T(2));

  old = it--;
  REQUIRE(*old == T(2));
  REQUIRE(*it == T(1));
}

TEST_CASE_TEMPLATE("bidirectional is_at_begin", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3};
  vector<T> v2{2, 4};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  REQUIRE(it.is_at_begin());
  ++it;
  REQUIRE_FALSE(it.is_at_begin());
  --it;
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional custom comparator", T, INT_TYPES_TO_TEST) {
  vector<T> v1{5, 4, 3};
  vector<T> v2{10, 9, 8};

  BidirectionalMergeIterator<typename vector<T>::iterator, greater<T>> it{
      greater<T>()};
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  vector<T> expected{10, 9, 8, 5, 4, 3};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional tie-breaking with equal predecessors", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 2};
  vector<T> v2{1, 2};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  size_t idx1 = it.add_list(v1.begin(), v1.end());
  size_t idx2 = it.add_list(v2.begin(), v2.end());

  ++it;
  CHECK(it.location().first == idx2);

  ++it;
  CHECK(it.location().first == idx1);

  --it;
  CHECK(it.location().first == idx2);

  ++it;
  CHECK(it.location().first == idx1);

  BidirectionalMergeIterator<typename vector<T>::iterator> it2;
  it2.add_list(v1.begin(), v1.end());
  it2.add_list(v2.begin(), v2.end());

  ++it2;
  ++it2;
  CHECK(it2.location().first == idx1);

  REQUIRE(it == it2);
}

TEST_CASE_TEMPLATE("bidirectional identity --++ and ++--", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 2, 3};
  vector<T> v2{1, 2, 3};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  for (; !it.is_past_end(); ++it) {
    auto it2 = it;
    ++it;
    REQUIRE(it != it2);
    --it;
    REQUIRE(it == it2);
  }
}

TEST_CASE_TEMPLATE("bidirectional copy and move", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;
  ++it;

  BidirectionalMergeIterator<typename vector<T>::iterator> it2 = it;
  REQUIRE(*it2 == T(4));
  --it2;
  REQUIRE(*it2 == T(2));

  BidirectionalMergeIterator<typename vector<T>::iterator> it3 = move(it);
  REQUIRE(*it3 == T(4));
  --it3;
  REQUIRE(*it3 == T(2));
}

TEST_CASE_TEMPLATE("bidirectional copy constructor preserves equality", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;
  ++it;

  BidirectionalMergeIterator<typename vector<T>::iterator> it2 = it;
  REQUIRE(it == it2);
}

TEST_CASE_TEMPLATE("bidirectional copy assignment preserves equality", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());
  ++it;
  ++it;

  BidirectionalMergeIterator<typename vector<T>::iterator> it2;
  it2 = it;
  REQUIRE(it == it2);
}

TEST_CASE_TEMPLATE("bidirectional make_end", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3};
  vector<T> v2{2, 4};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  auto end_it = it.make_end();
  REQUIRE(end_it.is_past_end());
}

TEST_CASE_TEMPLATE("bidirectional post-decrement", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 3, 5};
  vector<T> v2{2, 4, 6};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end());
  it.add_list(v2.begin(), v2.end());

  ++it;
  ++it;
  ++it;

  auto old = it--;
  REQUIRE(*old == T(4));
  REQUIRE(*it == T(3));
}

TEST_CASE_TEMPLATE("bidirectional add_list with custom current", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 2, 3, 4, 5};

  BidirectionalMergeIterator<typename vector<T>::iterator> it;
  it.add_list(v1.begin(), v1.end(), v1.begin() + 2);

  REQUIRE(*it == T(3));
  ++it;
  REQUIRE(*it == T(4));
  --it;
  REQUIRE(*it == T(3));
}

TEST_CASE_TEMPLATE("bidirectional fuzz test", T, INT_TYPES_TO_TEST) {
  mt19937_64 rand_gen{12345};

  constexpr size_t NUM_REPS = 100;
  constexpr size_t MAX_LIST_SIZE = 20;
  constexpr size_t NUM_LISTS[] = {0, 1, 10, 100};

  for (size_t num_lists : NUM_LISTS) {
    for (size_t rep = 0; rep < NUM_REPS; ++rep) {
      vector<vector<T>> lists(num_lists);
      vector<T> merged;

      for (size_t i = 0; i < num_lists; ++i) {
        size_t list_size = rand_gen() % MAX_LIST_SIZE;
        lists[i].resize(list_size);
        for (size_t j = 0; j < list_size; ++j) {
          lists[i][j] = T(rand_gen() % 100);
        }
        sort(lists[i].begin(), lists[i].end());
        merged.insert(merged.end(), lists[i].begin(), lists[i].end());
      }
      sort(merged.begin(), merged.end());

      BidirectionalMergeIterator<typename vector<T>::iterator> it;
      for (size_t i = 0; i < num_lists; ++i) {
        it.add_list(lists[i].begin(), lists[i].end());
      }

      size_t steps_forward = merged.size() / 2;
      for (size_t i = 0; i < steps_forward; ++i) {
        REQUIRE_FALSE(it.is_past_end());
        CHECK(*it == merged[i]);
        ++it;
      }

      for (size_t i = steps_forward; i > 0; --i) {
        --it;
        CHECK(*it == merged[i - 1]);
      }
      REQUIRE(it.is_at_begin());

      for (size_t i = 0; i < merged.size(); ++i) {
        REQUIRE_FALSE(it.is_past_end());
        CHECK(*it == merged[i]);
        ++it;
      }
      REQUIRE(it.is_past_end());
    }
  }
}

TEST_CASE_TEMPLATE("constructor from range", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};
  vector<T> v3{3, 6, 9};

  using Iterator = typename vector<T>::iterator;
  using MergeIt = MergeIterator<Iterator>;
  using Triple = typename MergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());
  triples.emplace_back(v3.begin(), v3.end());

  MergeIt it(triples.begin(), triples.end());

  vector<T> expected{1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("constructor from range with custom comparator", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{5, 4, 3};
  vector<T> v2{10, 9, 8};

  using Iterator = typename vector<T>::iterator;
  using MergeIt = MergeIterator<Iterator, greater<T>>;
  using Triple = typename MergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());

  MergeIt it(triples.begin(), triples.end(), greater<T>());

  vector<T> expected{10, 9, 8, 5, 4, 3};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("constructor from range with allocator", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  using Iterator = typename vector<T>::iterator;
  using MergeIt = MergeIterator<Iterator>;
  using Triple = typename MergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());

  std::allocator<Iterator> alloc;
  MergeIt it(triples.begin(), triples.end(), alloc);

  vector<T> expected{1, 2, 4, 5};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("assign from range", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};
  vector<T> v3{3, 6, 9};

  using Iterator = typename vector<T>::iterator;
  using MergeIt = MergeIterator<Iterator>;
  using Triple = typename MergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());
  triples.emplace_back(v3.begin(), v3.end());

  MergeIt it;
  it.assign(triples.begin(), triples.end());

  vector<T> expected{1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("assign replaces existing content", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};
  vector<T> v3{3, 6, 7, 8};

  using Iterator = typename vector<T>::iterator;
  using MergeIt = MergeIterator<Iterator>;
  using Triple = typename MergeIt::IteratorTriple;

  vector<Triple> triples1;
  triples1.emplace_back(v1.begin(), v1.end());
  triples1.emplace_back(v2.begin(), v2.end());

  MergeIt it(triples1.begin(), triples1.end());
  ++it;
  ++it;
  REQUIRE(*it == T(4));

  vector<Triple> triples2;
  triples2.emplace_back(v3.begin(), v3.end());
  it.assign(triples2.begin(), triples2.end());

  vector<T> expected{3, 6, 7, 8};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());
}

TEST_CASE_TEMPLATE("bidirectional constructor from range", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};
  vector<T> v3{3, 6, 9};

  using Iterator = typename vector<T>::iterator;
  using BMergeIt = BidirectionalMergeIterator<Iterator>;
  using Triple = typename BMergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());
  triples.emplace_back(v3.begin(), v3.end());

  BMergeIt it(triples.begin(), triples.end());

  vector<T> expected{1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE(
    "bidirectional constructor from range with custom comparator", T,
    INT_TYPES_TO_TEST) {
  vector<T> v1{5, 4, 3};
  vector<T> v2{10, 9, 8};

  using Iterator = typename vector<T>::iterator;
  using BMergeIt = BidirectionalMergeIterator<Iterator, greater<T>>;
  using Triple = typename BMergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());

  BMergeIt it(triples.begin(), triples.end(), greater<T>());

  vector<T> expected{10, 9, 8, 5, 4, 3};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional constructor from range with allocator", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};

  using Iterator = typename vector<T>::iterator;
  using BMergeIt = BidirectionalMergeIterator<Iterator>;
  using Triple = typename BMergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());

  std::allocator<Iterator> alloc;
  BMergeIt it(triples.begin(), triples.end(), alloc);

  vector<T> expected{1, 2, 4, 5};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional assign from range", T, INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4, 7};
  vector<T> v2{2, 5, 8};
  vector<T> v3{3, 6, 9};

  using Iterator = typename vector<T>::iterator;
  using BMergeIt = BidirectionalMergeIterator<Iterator>;
  using Triple = typename BMergeIt::IteratorTriple;

  vector<Triple> triples;
  triples.emplace_back(v1.begin(), v1.end());
  triples.emplace_back(v2.begin(), v2.end());
  triples.emplace_back(v3.begin(), v3.end());

  BMergeIt it;
  it.assign(triples.begin(), triples.end());

  vector<T> expected{1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_CASE_TEMPLATE("bidirectional assign replaces existing content", T,
                   INT_TYPES_TO_TEST) {
  vector<T> v1{1, 4};
  vector<T> v2{2, 5};
  vector<T> v3{3, 6, 7, 8};

  using Iterator = typename vector<T>::iterator;
  using BMergeIt = BidirectionalMergeIterator<Iterator>;
  using Triple = typename BMergeIt::IteratorTriple;

  vector<Triple> triples1;
  triples1.emplace_back(v1.begin(), v1.end());
  triples1.emplace_back(v2.begin(), v2.end());

  BMergeIt it(triples1.begin(), triples1.end());
  ++it;
  ++it;
  REQUIRE(*it == T(4));

  vector<Triple> triples2;
  triples2.emplace_back(v3.begin(), v3.end());
  it.assign(triples2.begin(), triples2.end());

  vector<T> expected{3, 6, 7, 8};
  for (size_t i = 0; i < expected.size(); ++i) {
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i]);
    ++it;
  }
  REQUIRE(it.is_past_end());

  for (size_t i = expected.size(); i > 0; --i) {
    --it;
    REQUIRE_FALSE(it.is_past_end());
    CHECK(*it == expected[i - 1]);
  }
  REQUIRE(it.is_at_begin());
}

TEST_SUITE_END();
