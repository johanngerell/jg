#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include "jg_state_scope.h"
#include "jg_ostream_color_scope.h"

namespace jg
{

struct test_case final
{
    const char* description{};
    std::function<void()> func;
    size_t assertion_fail_count{};
};

struct test_suite final
{
    const char* description{};
    std::vector<test_case> tests;
};

using test_suites = std::vector<test_suite>;

namespace detail
{

struct test_statistics final
{
    size_t suite_count{};
    size_t case_count{};
    size_t assertion_count{};
    size_t case_fail_count{};
    size_t assertion_fail_count{};
};

test_statistics*  g_test_statistics{};
test_case*        g_test_case{};
test_suite*       g_test_suite{};

} // namespace detail

int test_run(test_suites&& suites)
{
    detail::test_statistics statistics{};
    statistics.suite_count = suites.size();

    {
        state_scope_value test_statistics_scope(detail::g_test_statistics, &statistics, nullptr);

        for (auto& suite : suites)
        {
            std::cout << "Running test suite ";
            jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << suite.description << "'\n";
            statistics.case_count += suite.tests.size();

            state_scope_value test_suite_scope(detail::g_test_suite, &suite, nullptr);

            for (auto& test : suite.tests)
            {
                state_scope_value test_case_scope(detail::g_test_case, &test, nullptr);
                test.func();

                if (test.assertion_fail_count > 0)
                    statistics.case_fail_count++;
            }
        }
    }

    if (statistics.case_count == 0)
        jg::ostream_color_scope(std::cout, jg::fg_yellow_bright()) << "No test cases\n";
    else
    {
        if (statistics.assertion_fail_count == 0)
            jg::ostream_color_scope(std::cout, jg::fg_green_bright()) << "All tests succeeded\n";

        std::cout << statistics.assertion_count  << (statistics.assertion_count == 1 ? " test assertion" : " test assertions") << '\n'
                  << statistics.case_count       << (statistics.case_count == 1 ? " test case" : " test cases") << '\n'
                  << statistics.suite_count      << (statistics.suite_count == 1 ? " test suite" : " test suites") << '\n';
    }

    return static_cast<int>(statistics.assertion_fail_count);
}

namespace detail
{

inline void test_assert_impl(bool expr_value, const char* expr_string, const char* file, int line)
{
    // Allowing g_statistics to be unset makes it possible to use the jg_test_assert
    // macro outside of test suites, which is useful for quick main()-only tests
    // that might grow to more complete suites.

    if (g_test_statistics)
        g_test_statistics->assertion_count++;
    
    if (expr_value)
        return;

    if (g_test_case && g_test_case->assertion_fail_count++ == 0)
    {
        jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "  Failed test case ";
        jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << g_test_case->description << "'\n";
    }

    if (g_test_statistics)
        g_test_statistics->assertion_fail_count++;

    jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "    Failed test assertion ";
    jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << expr_string << '\'';
    std::cout << " at ";
    jg::ostream_color_scope(std::cout, jg::fg_magenta_bright()) << file << ':' << line << '\n';
}

} // namespace detail
} // namespace jg

#define jg_test_assert(expr) jg::detail::test_assert_impl((expr), #expr, __FILE__,  __LINE__) 

#ifdef JG_TEST_ENABLE_SHORT_NAME
    #define test_assert jg_test_assert
#endif
