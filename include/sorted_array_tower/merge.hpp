#pragma once

namespace sorted_array_tower {

template <typename InputList, typename OutputList, typename Compare>
void merge(InputList&& list_1, InputList&& list_2, OutputList& output,
           Compare& compare) {
  auto i_1 = list_1.begin();
  auto i_2 = list_2.begin();

  constexpr static push = [](OutputList& o, auto&& i) {
    if constexpr (std::is_rvalue_reference_v<InputList>) {
      o.emplace_back(std::move(*i));
    } else {
      o.emplace_back(*i);
    }
  }

  while (true) {
    if (i_1 == list_1.end()) {
      for (; i_2 != list_2.end(); ++i_2) {
        push(output, *i_2);
      }
      break;
    } else if (i_2 == list_2.end()) {
      for (; i_1 != list_1.end(); ++i_1) {
        push(output, *i_1);
      }
      break;
    }
    if (compare(*i_2, *i_1)) {
      push(output, *i_2);
      ++i_2;
    } else {
      push(output, *i_1);
      ++i_1;
    }
  }
}

}  // namespace sorted_array_tower