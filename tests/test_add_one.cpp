#include <string>

#include <doctest/doctest.h>

#if TYGHBN_USE_MODULES
import tyghbn;
#else
#include <tyghbn/tyghbn.hpp>
#endif

using namespace tyghbn;

TEST_SUITE("add_one") {

    TEST_CASE("char") {
        CHECK(add_one('a') == 'b');
    }

    TEST_CASE("string") {
        CHECK(add_one(std::string("a")) == std::string("a1"));
    }
}
