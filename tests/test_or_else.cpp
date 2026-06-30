#include <doctest/doctest.h>

#if TYGHBN_USE_MODULES
import tyghbn;
#else
#include <tyghbn/tyghbn.hpp>
#endif

using namespace tyghbn;

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
