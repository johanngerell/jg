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
    std::string description;
    std::function<void()> func;
    size_t assertion_fail_count{};
};

struct test_suite final
{
    std::string description;
    std::vector<test_case> items;
    size_t test_fail_count{};
};

struct test_suites final
{
    std::string description;
    std::vector<test_suite> items;
};

using test_super_suites = std::vector<test_suites>;

namespace detail
{

struct test_metrics final
{
    size_t suite_count{};
    size_t case_count{};
    size_t assertion_count{};
    size_t case_fail_count{};
    size_t assertion_fail_count{};
};

class test_state final
{
public:
    static test_state& instance()
    {
        static test_state state;
        return state;
    }

    test_metrics* current_metrics{};
    test_case*    current_case{};
    test_suite*   current_suite{};
    test_suites*  current_suites{};

private:
    test_state() = default;
};

inline void test_assert_impl(bool expr_value, const char* expr_string, const char* file, int line)
{
    // Allowing the current test_state to be unset makes it possible to use the jg_test_assert
    // macro outside of test suites, which is useful for quick main()-only tests
    // that might grow to more complete suites.

    auto& state = detail::test_state::instance();

    if (state.current_metrics)
        state.current_metrics->assertion_count++;
    
    if (expr_value)
        return;

    if (state.current_suite &&
        state.current_suite->test_fail_count == 0)
    {
        jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "  Failed test suite ";
        jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << state.current_suite->description << "'\n";
    }

    if (state.current_case &&
        state.current_case->assertion_fail_count++ == 0)
    {
        if (state.current_suite)
            state.current_suite->test_fail_count++;

        jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "    Failed test case ";
        jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << state.current_case->description << "'\n";
    }

    if (state.current_metrics)
        state.current_metrics->assertion_fail_count++;

    jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "      Failed test assertion ";
    jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << expr_string << '\'';
    std::cout << " at ";
    jg::ostream_color_scope(std::cout, jg::fg_magenta_bright()) << file << ':' << line << '\n';
}

} // namespace detail

inline int test_run(test_super_suites&& super_suites)
{
    detail::test_metrics metrics{};

    for (auto& suites : super_suites)
    {
        std::cout << "Running test super suite ";
        jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << suites.description << "'\n";
        metrics.suite_count += suites.items.size();

        auto& state = detail::test_state::instance();
        state_scope_value test_metrics_scope(state.current_metrics, &metrics, nullptr);
        state_scope_value test_suites_scope(state.current_suites, &suites, nullptr);

        for (auto& suite : suites.items)
        {
            metrics.case_count += suite.items.size();

            state_scope_value test_suite_scope(state.current_suite, &suite, nullptr);

            for (auto& test : suite.items)
            {
                state_scope_value test_case_scope(state.current_case, &test, nullptr);
                test.func();

                if (test.assertion_fail_count > 0)
                    metrics.case_fail_count++;
            }
        }
    }

    if (metrics.case_count == 0)
        jg::ostream_color_scope(std::cout, jg::fg_yellow_bright()) << "No test cases\n";
    else
    {
        if (metrics.assertion_fail_count > 0)
            ; // suite and case failures are handled in test_assert_impl
        else
            jg::ostream_color_scope(std::cout, jg::fg_green_bright()) << "All tests succeeded\n";

        std::cout << metrics.assertion_count  << (metrics.assertion_count == 1 ? " test assertion" : " test assertions") << '\n'
                  << metrics.case_count       << (metrics.case_count == 1 ? " test case" : " test cases") << '\n'
                  << metrics.suite_count      << (metrics.suite_count == 1 ? " test suite" : " test suites") << '\n';
    }

    return static_cast<int>(metrics.assertion_fail_count);
}

} // namespace jg

#define jg_test_assert(expr) jg::detail::test_assert_impl((expr), #expr, __FILE__,  __LINE__) 

#ifdef JG_TEST_ENABLE_SHORT_NAME
    #define test_assert jg_test_assert
#endif
