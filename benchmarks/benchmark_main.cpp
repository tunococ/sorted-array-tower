#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <string>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/sorted_array_tower.hpp>
#endif

int main() {
  ankerl::nanobench::Bench bench;

  bench.run("skip_array",
            [] {
              sorted_array_tower::SkipArray<int> a;
              ankerl::nanobench::doNotOptimizeAway(a.size());
            });

  bench.run("bounded_array",
            [] {
              sorted_array_tower::BoundedArray<int> a;
              ankerl::nanobench::doNotOptimizeAway(a.size());
            });

  return 0;
}