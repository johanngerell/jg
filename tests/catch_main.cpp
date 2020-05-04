#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "../inc/jg_mock.h"

// There should be NO tests in this file. We only want to compile this file ONCE. It'll reduce the total
// test compilation time, since it'll put the entire Catch2 implementation in one translation unit, which
// won't change when individual tests in other files change.

// Link-time function dependencies that needs mocking in unit tests can only be defined once, and that is
// preferably done here. Mocking such functions for usage in unit tests in different translation units is
// made easier by using the JG_MOCK and JG_MOCK_REF macros from jg::mock.

// Used to verify that noexcept functions fail (as per jg::verify) when they should.
JG_MOCK(,,, void, test_assert, bool);
