#include <jg_mock.h>

// There should be NO tests in this file. We only want to compile this file ONCE. It'll reduce the total
// test compilation time as only individual tests in other translation units change.

// Link-time function dependencies that needs mocking in unit tests can only be defined once, and that is
// preferably done here too. Mocking such functions for usage in unit tests in different translation units
// is made easier by using the JG_MOCK and JG_MOCK_REF macros from jg::mock.

// For verifying that noexcept functions in jg "fail" (as per jg::verify) when they should.
// The specific name `mock_assert` is defined by JG_VERIFY_ASSERTION in CMakeLists.txt.
JG_MOCK_EX(,,, void, mock_assert, bool);

// For testing that mocking of free functions work. The corresponding JG_MOCK_REF is in mock_tests.cpp.
JG_MOCK_EX(,,, bool, test_free_function, char, bool, int, const char*);

#define JG_TEST_MAIN
#define JG_TEST_IMPL
#include <jg_test.h>
