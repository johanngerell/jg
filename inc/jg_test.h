#pragma once

#include <iostream>
#include <functional>
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
        detail::g_test_statistics = &statistics; // TODO: use state_scope_value

        for (auto& suite : suites)
        {
            detail::g_test_suite = &suite;
            std::cout << "Running test suite '" << suite.description << "'\n";
            statistics.case_count += suite.tests.size();

            for (auto& test : suite.tests)
            {
                detail::g_test_case = &test;
                test.func();

                if(test.assertion_fail_count > 0)
                    statistics.case_fail_count++;

                detail::g_test_case = nullptr;
            }

            detail::g_test_suite = nullptr;
        }

        detail::g_test_statistics = nullptr;
    }

    if (statistics.case_count == 0)
        jg::ostream_color_scope(std::cout, jg::fg_yellow()) << "No test cases\n";
    else
    {
        if (statistics.assertion_fail_count == 0)
            jg::ostream_color_scope(std::cout, jg::fg_green()) << "All tests succeeded\n";

        std::cout << statistics.assertion_count  << (statistics.assertion_count == 1 ? " test assertion" : " test assertions") << "\n"
                  << statistics.case_count       << (statistics.case_count == 1 ? " test case" : " test cases") << "\n"
                  << statistics.suite_count      << (statistics.suite_count == 1 ? " test suite" : " test suites") << "\n";
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
        jg::ostream_color_scope(std::cout, jg::fg_red()) << "  Failed test case ";
        std::cout << "'" << g_test_case->description << "'\n";
    }

    if (g_test_statistics)
        g_test_statistics->assertion_fail_count++;

    jg::ostream_color_scope(std::cout, jg::fg_red()) << "    Failed test assertion ";
    std::cout << "'" << expr_string << "' at " << file << ":" << line << "\n";
}

} // namespace detail
} // namespace jg

#define jg_test_assert(expr) jg::detail::test_assert_impl((expr), #expr, __FILE__,  __LINE__) 

#ifdef JG_TEST_ENABLE_SHORT_NAME
    #define test_assert jg_test_assert
#endif
