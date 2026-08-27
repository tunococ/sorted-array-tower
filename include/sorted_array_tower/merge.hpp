#pragma once

#include <concepts>
#include <iterator>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace sorted_array_tower {

/**
 * @brief Generic merge function.
 * 
 * This function is meant to merge two sorted lists defined by iterator
 * intervals `[begin_1, end_1)` and `[begin_2, end_2)`, using `compare` as a
 * *less_then* method for two iterators. The output is reported by calling
 * `output(i)` multiple times with `i` being an iterator from one of the two
 * lists, and the sequence of `i`'s in calls to `output(i)` will be
 * non-decreasing according to `compare`.
 */
template <typename InputIterator, typename OutputFunction, typename Compare>
void merge(InputIterator begin_1, InputIterator end_1, InputIterator begin_2,
           InputIterator end_2, OutputFunction&& output, Compare&& compare) {
  auto i_1 = begin_1;
  auto i_2 = begin_2;

  while (true) {
    if (i_1 == end_1) {
      for (; i_2 != end_2; ++i_2) {
        output(i_2);
      }
      break;
    }
    if (i_2 == end_2) {
      for (; i_1 != end_1; ++i_1) {
        output(i_1);
      }
      break;
    }
    if (compare(i_2, i_1)) {
      output(i_2);
      ++i_2;
    } else {
      output(i_1);
      ++i_1;
    }
  }
}

}  // namespace sorted_array_tower