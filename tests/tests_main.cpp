#include <jg_mock.h>
#define JG_TEST_IMPL
#include <jg_test.h>

// There should be NO tests in this file. We only want to compile this file ONCE. It'll reduce the total
// test compilation time as only individual tests in other translation units change.

// Link-time function dependencies that needs mocking in unit tests can only be defined once, and that is
// preferably done here too. Mocking such functions for usage in unit tests in different translation units
// is made easier by using the JG_MOCK and JG_MOCK_REF macros from jg::mock.

// Used to verify that noexcept functions in jg "fail" (as per jg::verify) when they should.
// The specific name "mock_assert" is defined by JG_VERIFY_ASSERTION in CMakeLists.txt.
JG_MOCK(,,, void, mock_assert, bool);

jg::test_suites test_args();
jg::test_suites test_optional();
jg::test_suites test_string();

int main()
{
    return jg::test_run(
    {
        test_args(),
        test_optional(),
        test_string()
    });
}