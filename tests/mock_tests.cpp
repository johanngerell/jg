#include <jg_mock.h>
#include <jg_test.h>

bool test_free_function(char, bool, int, const char*);
JG_MOCK_REF_EX(,,, bool, test_free_function, char, bool, int, const char*);

// For checking if the auxiliary result is properly verified when used without being set.
JG_MOCK_REF_EX(,,, void, mock_assert, bool);

namespace {

bool using_mock_test_free_function(char c, bool b, int i, const char* s)
{
    return test_free_function(c, b, i, s);
}

struct test_abstract_class
{
    virtual bool function1(char, bool, int, const char*) = 0;

    virtual ~test_abstract_class() = default;
};

struct mock_test_abstract_class final : test_abstract_class
{
    JG_MOCK_EX(,,, bool, function1, char, bool, int, const char*);
};

bool using_test_abstract_class(test_abstract_class& t, char c, bool b, int i, const char* s)
{
    return t.function1(c, b, i, s);
}

using namespace std::string_literals;

jg::test_adder mock_tests { "mock", {
    jg::test_suite { "free functions", {
        jg::test_case { "non-void called without setting result causes assertion", [] {
            mock_assert_.reset(); // only needed for free function mocks
            test_free_function_.reset(); // only needed for free function mocks

            using_mock_test_free_function('a', true, 4711, "foobar1");

            jg_test_assert(test_free_function_.called());
            jg_test_assert(mock_assert_.called());
            jg_test_assert(mock_assert_.param<1>() == false);
        }},
        jg::test_case { "state is empty before call", [] {
            test_free_function_.reset(); // only needed for free function mocks

            jg_test_assert(!test_free_function_.called());
            jg_test_assert(test_free_function_.count() == 0);
            jg_test_assert(test_free_function_.param<1>() == 0);
            jg_test_assert(test_free_function_.param<2>() == false);
            jg_test_assert(test_free_function_.param<3>() == 0);
            jg_test_assert(test_free_function_.param<4>() == nullptr);
            jg_test_assert(test_free_function_.result == false);
            jg_test_assert(test_free_function_.func == nullptr);
        }},
        jg::test_case { "reset clears state after call, except persistent state", [] {
            test_free_function_.reset(); // only needed for free function mocks
            test_free_function_.result = true;
            test_free_function_.func = [] (auto, auto, auto, auto) { return true; };

            jg_test_assert(using_mock_test_free_function('a', true, 4711, "foobar1"));
            test_free_function_.reset();
            jg_test_assert(!test_free_function_.called());
            jg_test_assert(test_free_function_.count() == 0);
            jg_test_assert(test_free_function_.param<1>() == 0);
            jg_test_assert(test_free_function_.param<2>() == false);
            jg_test_assert(test_free_function_.param<3>() == 0);
            jg_test_assert(test_free_function_.param<4>() == nullptr);
            // Not reset
            jg_test_assert(test_free_function_.result == true);
            jg_test_assert(test_free_function_.func != nullptr);
        }},
        jg::test_case { "with result", [] {
            test_free_function_.reset(); // only needed for free function mocks
            test_free_function_.result = true;

            jg_test_assert(using_mock_test_free_function('a', true, 4711, "foobar1"));
            jg_test_assert(using_mock_test_free_function('b', true, 4712, "foobar2"));
            jg_test_assert(using_mock_test_free_function('c', true, 4713, "foobar3"));
            jg_test_assert(test_free_function_.param<1>() == 'c');
            jg_test_assert(test_free_function_.param<2>() == true);
            jg_test_assert(test_free_function_.param<3>() == 4713);
            jg_test_assert(test_free_function_.param<4>() == "foobar3"s);
            jg_test_assert(test_free_function_.called());
            jg_test_assert(test_free_function_.count() == 3);
            jg_test_assert(test_free_function_.prototype() == "bool test_free_function(char, bool, int, const char*)");
        }},
        jg::test_case { "with func", [] {
            test_free_function_.reset(); // only needed for free function mocks
            test_free_function_.func = [] (char, bool, int, const char*) { return true; };

            jg_test_assert(using_mock_test_free_function('a', true, 4711, "foobar1"));
            jg_test_assert(using_mock_test_free_function('b', true, 4712, "foobar2"));
            jg_test_assert(using_mock_test_free_function('c', true, 4713, "foobar3"));
            jg_test_assert(test_free_function_.param<1>() == 'c');
            jg_test_assert(test_free_function_.param<2>() == true);
            jg_test_assert(test_free_function_.param<3>() == 4713);
            jg_test_assert(test_free_function_.param<4>() == "foobar3"s);
            jg_test_assert(test_free_function_.called());
            jg_test_assert(test_free_function_.count() == 3);
            jg_test_assert(test_free_function_.prototype() == "bool test_free_function(char, bool, int, const char*)");
        }}
    }},
    jg::test_suite { "virtual functions", {
        jg::test_case { "non-void called without setting result causes assertion", [] {
            mock_assert_.reset(); // only needed for free function mocks
            mock_test_abstract_class mock;

            using_test_abstract_class(mock, 'a', true, 4711, "foobar1");

            jg_test_assert(mock.function1_.called());
            jg_test_assert(mock_assert_.called());
            jg_test_assert(mock_assert_.param<1>() == false);
        }},
        jg::test_case { "state is empty before call", [] {
            mock_test_abstract_class mock;

            jg_test_assert(!mock.function1_.called());
            jg_test_assert(mock.function1_.count() == 0);
            jg_test_assert(mock.function1_.param<1>() == 0);
            jg_test_assert(mock.function1_.param<2>() == false);
            jg_test_assert(mock.function1_.param<3>() == 0);
            jg_test_assert(mock.function1_.param<4>() == nullptr);
            jg_test_assert(mock.function1_.result == false);
            jg_test_assert(mock.function1_.func == nullptr);
        }},
        jg::test_case { "reset clears state after call, except persistent state", [] {
            mock_test_abstract_class mock;
            mock.function1_.result = true;
            mock.function1_.func = [] (auto, auto, auto, auto) { return true; };

            jg_test_assert(using_test_abstract_class(mock, 'a', true, 4711, "foobar1"));
            mock.function1_.reset();
            jg_test_assert(!mock.function1_.called());
            jg_test_assert(mock.function1_.count() == 0);
            jg_test_assert(mock.function1_.param<1>() == 0);
            jg_test_assert(mock.function1_.param<2>() == false);
            jg_test_assert(mock.function1_.param<3>() == 0);
            jg_test_assert(mock.function1_.param<4>() == nullptr);
            // Not reset
            jg_test_assert(mock.function1_.result == true);
            jg_test_assert(mock.function1_.func != nullptr);
        }},
        jg::test_case { "with result", [] {
            mock_test_abstract_class mock;
            mock.function1_.result = true;

            jg_test_assert(using_test_abstract_class(mock, 'a', true, 4711, "foobar1"));
            jg_test_assert(using_test_abstract_class(mock, 'b', true, 4712, "foobar2"));
            jg_test_assert(using_test_abstract_class(mock, 'c', true, 4713, "foobar3"));
            jg_test_assert(mock.function1_.param<1>() == 'c');
            jg_test_assert(mock.function1_.param<2>() == true);
            jg_test_assert(mock.function1_.param<3>() == 4713);
            jg_test_assert(mock.function1_.param<4>() == "foobar3"s);
            jg_test_assert(mock.function1_.called());
            jg_test_assert(mock.function1_.count() == 3);
            jg_test_assert(mock.function1_.prototype() == "bool function1(char, bool, int, const char*)");
        }},
        jg::test_case { "with func", [] {
            mock_test_abstract_class mock;
            mock.function1_.func = [] (char, bool, int, const char*) { return true; };

            jg_test_assert(using_test_abstract_class(mock, 'a', true, 4711, "foobar1"));
            jg_test_assert(using_test_abstract_class(mock, 'b', true, 4712, "foobar2"));
            jg_test_assert(using_test_abstract_class(mock, 'c', true, 4713, "foobar3"));
            jg_test_assert(mock.function1_.param<1>() == 'c');
            jg_test_assert(mock.function1_.param<2>() == true);
            jg_test_assert(mock.function1_.param<3>() == 4713);
            jg_test_assert(mock.function1_.param<4>() == "foobar3"s);
            jg_test_assert(mock.function1_.called());
            jg_test_assert(mock.function1_.count() == 3);
            jg_test_assert(mock.function1_.prototype() == "bool function1(char, bool, int, const char*)");
        }}
    }}
}};

}
