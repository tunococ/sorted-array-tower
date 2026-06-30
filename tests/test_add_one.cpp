#include <string>

#include <doctest/doctest.h>

#if SORTED_ARRAY_TOWER_USE_MODULES
import sorted_array_tower;
#else
#include <sorted_array_tower/sorted_array_tower.hpp>
#endif

using namespace sorted_array_tower;

TEST_SUITE("add_one") {

    TEST_CASE("char") {
        CHECK(add_one('a') == 'b');
    }

    TEST_CASE("string") {
        CHECK(add_one(std::string("a")) == std::string("a1"));
    }
}
