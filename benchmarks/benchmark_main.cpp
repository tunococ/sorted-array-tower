#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <string>

#if TYGHBN_USE_MODULES
import tyghbn;
#else
#include <tyghbn/add_one.hpp>
#include <tyghbn/or_else.hpp>
#endif

int main() {
    ankerl::nanobench::Bench bench;

    bench.run("add_one<int>", [] {
        volatile auto r = tyghbn::add_one(42);
    });

    bench.run("add_one<double>", [] {
        volatile auto r = tyghbn::add_one(3.14);
    });

    bench.run("or_else<int>", [] {
        volatile auto r = tyghbn::or_else(2, 1);
    });

    bench.run("or_else<int> with zero", [] {
        volatile auto r = tyghbn::or_else(0, 1);
    });

    return 0;
}