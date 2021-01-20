#pragma once

#include <iostream>
#include <functional>
#include "jg_ostream_color_scope.h"

namespace jg
{

struct test_statistics final
{
    size_t suite_count{};
    size_t case_count{};
    size_t assertion_count{};
    size_t case_fail_count{};
    size_t assertion_fail_count{};
};

test_statistics* g_statistics;

using test_case = std::function<void()>;
using test_suite = std::vector<test_case>;
using test_suites = std::vector<test_suite>;

int test_run(const test_suites& suites)
{
    test_statistics statistics{};
    g_statistics = &statistics;

    statistics.suite_count = suites.size();

    for (auto& suite : suites)
    {
        statistics.case_count += suite.size();

        for (auto& test : suite)
        {
            const size_t assertion_fail_count_before = statistics.assertion_fail_count;
            test();

            if (statistics.assertion_fail_count > assertion_fail_count_before)
                statistics.case_fail_count++;
        }
    }

    g_statistics = nullptr;

    if (statistics.case_count == 0)
        jg::ostream_color_scope(std::cout, jg::fg_yellow()) << "No test cases\n";
    else
    {
        if (statistics.assertion_fail_count == 0)
            jg::ostream_color_scope(std::cout, jg::fg_green()) << "All tests succeeded\n";

        std::cout << "  " << statistics.assertion_count  << (statistics.assertion_count == 1 ? " test assertion" : " test assertions") << "\n"
                  << "  " << statistics.case_count       << (statistics.case_count == 1 ? " test case" : " test cases") << "\n"
                  << "  " << statistics.suite_count      << (statistics.suite_count == 1 ? " test suite" : " test suites") << "\n";
    }

    return static_cast<int>(statistics.assertion_fail_count);
}

namespace detail
{

inline void test_assert_impl(bool expr_value, const char* expr_string, const char* file, int line)
{
    // Allowing g_statistics to be unset makes it possible to use the test_assert
    // macro outside of test suites, which is useful for quick main()-only tests
    // that often grow to more complete suites.

    if (g_statistics)
        g_statistics->assertion_count++;
    
    if (expr_value)
        return;

    if (g_statistics)
        g_statistics->assertion_fail_count++;
    
    jg::ostream_color_scope(std::cout, jg::fg_red()) << "Test assertion '" << expr_string << "' failed at "
                                                     << file << ":" << line << "\n";
}

} // namespace detail
} // namespace jg

#define test_assert(expr) jg::detail::test_assert_impl((expr), #expr, __FILE__,  __LINE__) 
