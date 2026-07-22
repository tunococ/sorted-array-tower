#pragma once

#include <nanobench.h>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/sorted_array_tower.hpp>
#endif

using namespace ankerl::nanobench;
using namespace sorted_array_tower;

void bench_bounded_array();
