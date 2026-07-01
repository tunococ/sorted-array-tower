#include <doctest/doctest.h>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/sorted_array_tower.hpp>
#endif

using namespace sorted_array_tower;

TEST_SUITE("or_else") {
  TEST_CASE("int") {
    CHECK(or_else(2, 1) == 2);
    CHECK(or_else(0, 1) == 1);
  }

  // This case should give a gap in the coverage report.
  TEST_CASE("char") {
    CHECK(or_else('a', 'b') == 'a');
  }
}
