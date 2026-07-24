#include <deque>
#include <functional>
#include <vector>

#include "benchmarks.hpp"

using namespace std;

using Value = int;

void bench_copy() {
  Bench bench;
  bench.title("BoundedVector and BoundedArray - copy");

  constexpr size_t capacity = 1000;
  bench.minEpochIterations(50000);

  Value model[capacity];
  fill(model, model + capacity, 1);

  BoundedArray<Value> model_bounded_a(capacity, capacity, 1);
  BoundedVector<Value> model_bounded_v(capacity, capacity, 1);
  vector<Value> model_v(capacity, 1);
  deque<Value> model_d(capacity, 1);

  BoundedArray<Value> bounded_a(capacity, capacity, 0);
  BoundedVector<Value> bounded_v(capacity, capacity, 0);
  vector<Value> v(capacity, 0);
  deque<Value> d(capacity, 0);

  auto task = [&](auto& dst, auto const& src) {
    dst.clear();
    dst = src;
    doNotOptimizeAway(dst);
  };

  bench.run("vector", [&]() { task(v, model_v); });
  bench.run("deque", [&]() { task(d, model_d); });
  bench.run("BoundedVector",
            [&]() { task(bounded_v, model_bounded_v); });
  bench.run("BoundedArray",
            [&]() { task(bounded_a, model_bounded_a); });
}

void bench_back_operations() {
  Bench bench;
  bench.title("BoundedVector and BoundedArray - back operations");

  constexpr size_t capacity = 1000;
  bench.minEpochIterations(50000);

  BoundedArray<Value> bounded_a(capacity, capacity, 0);
  BoundedVector<Value> bounded_v(capacity, capacity, 0);
  vector<Value> v(capacity, 0);
  deque<Value> d(capacity, 0);

  auto task = [&](auto& x) {
    x.clear();
    while (x.size() < capacity) {
      x.emplace_back(1);
    }
    doNotOptimizeAway(x);
    while (!x.empty()) {
      x.pop_back();
    }
    doNotOptimizeAway(x);
  };

  bench.run("vector", [&]() { task(v); });
  bench.run("deque", [&]() { task(d); });
  bench.run("BoundedVector", [&]() { task(bounded_v); });
  bench.run("BoundedArray", [&]() { task(bounded_a); });
}

void bench_front_and_back_operations() {
  Bench bench;
  bench.title("BoundedArray - front and back operations");

  constexpr size_t capacity = 1000;
  bench.minEpochIterations(10000);

  BoundedArray<Value> bounded_a(capacity, capacity, 0);
  deque<Value> d(capacity, 0);

  auto task = [&](auto& x) {
    x.clear();
    while (x.size() < capacity / 2) {
      x.emplace_back(1);
    }
    doNotOptimizeAway(x);
    while (x.size() > capacity / 4) {
      x.pop_front();
    }
    doNotOptimizeAway(x);
    while (x.size() < capacity) {
      x.emplace_back(3);
    }
    doNotOptimizeAway(x);
    while (x.size() > 2) {
      x.pop_front();
      x.pop_back();
    }
    doNotOptimizeAway(x);
  };

  bench.run("deque", [&]() { task(d); });
  bench.run("BoundedArray", [&]() { task(bounded_a); });
}

void bench_bounded_array() {
  bench_copy();
  bench_back_operations();
  bench_front_and_back_operations();
}
