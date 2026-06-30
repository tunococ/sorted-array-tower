#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <string>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/add_one.hpp>
#include <sorted_array_tower/or_else.hpp>
#endif

int main() {
    ankerl::nanobench::Bench bench;

    bench.run("add_one<int>", [] {
        volatile auto r = sorted_array_tower::add_one(42);
    });

    bench.run("add_one<double>", [] {
        volatile auto r = sorted_array_tower::add_one(3.14);
    });

    bench.run("or_else<int>", [] {
        volatile auto r = sorted_array_tower::or_else(2, 1);
    });

    bench.run("or_else<int> with zero", [] {
        volatile auto r = sorted_array_tower::or_else(0, 1);
    });

    return 0;
}