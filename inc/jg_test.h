#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include "jg_state_scope.h"
#include "jg_ostream_color_scope.h"

/// @file Testing facilities, like test assertions, test cases, test suites, test runner, etc.
/// @note One translation unit *must* define JG_TEST_IMPL before including this header. This is typically
/// done by the main cpp file. The translation unit that defines JG_TEST_IMPL is the one where the test
/// implementation will be defined and compiled. If no translation unit defines JG_TEST_IMPL, then there
/// will be undefined symbol linker errors as tests refer to the undefined functions.
/// @note By defining JG_TEST_ENABLE_SHORT_NAME before including this header, the shorter macro name
/// `jg_assert` can be used instead of `jg_test_assert`.

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
    size_t case_fail_count{};
};

struct test_suites final
{
    std::string description;
    std::vector<test_suite> items;
};

void test_add(jg::test_suites&& suites);
int test_run();

template <typename TSuitesImpl>
class test_suites_base
{
public:
    test_suites_base()
    {
        test_add((*static_cast<TSuitesImpl*>(this))());
    }

    test_suites_base(const test_suites_base&) = delete;
    test_suites_base& operator=(const test_suites_base&) = delete;
};

namespace detail
{

void test_assert_prolog();
void test_assert_epilog(const char* expr_string, const char* file, int line);

inline void test_assert_impl(bool expr_value, const char* expr_string, const char* file, int line)
{
    jg::detail::test_assert_prolog();
    if (expr_value) return;
    jg::detail::test_assert_epilog(expr_string, file, line);
}

template <typename TException, typename TExprFunc>
void test_assert_exception_impl(TExprFunc&& expr_func, const char* expr_string, const char* file, int line)
{
    jg::detail::test_assert_prolog();
    try { expr_func(); }
    catch(const TException&) { return; }
    catch(...) {}
    jg::detail::test_assert_epilog(expr_string, file, line);
}

} // namespace detail
} // namespace jg

/// These macros can be used both on their own in simple test files (like `main.cpp` with a bunch of
/// assertions) and in test cases inside suites run by `test_run()`. They will not exit the test program,
/// only output error information and propagate metrics back to `test_run()`, if that's used.
#define jg_test_assert(expr) \
    jg::detail::test_assert_impl((expr), #expr, __FILE__,  __LINE__)
#define jg_test_assert_exception(expr, exception_type) \
    jg::detail::test_assert_exception_impl<exception_type>([&] { (expr); }, #expr, __FILE__,  __LINE__)

#ifdef JG_TEST_ENABLE_SHORT_NAME
    #define jg_assert jg_test_assert
#endif

#ifdef JG_TEST_IMPL

namespace jg
{

static std::vector<test_suites> super_suites;

void test_add(jg::test_suites&& suites)
{
    super_suites.push_back(std::move(suites));
}

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

struct test_state final
{
    test_metrics metrics{};
    test_case*   current_case{};
    test_suite*  current_suite{};
    test_suites* current_suites{};
};

// Allowing the current test_state to be unset makes it possible to use the jg_test_assert macro
// outside of test cases and suites, which is useful for quick main()-only tests that doesn't call
// test_run, but that might grow to more complete suites.
static test_state* current_state{};

void test_assert_prolog()
{
    if (current_state)
        current_state->metrics.assertion_count++;
}

void test_assert_epilog(const char* expr_string, const char* file, int line)
{
    if (current_state)
    {
        if (current_state->current_suite->case_fail_count == 0)
        {
            jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "  Failed test suite ";
            jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << current_state->current_suite->description << "'\n";
        }

        if (current_state->current_case->assertion_fail_count == 0)
        {
            current_state->current_suite->case_fail_count++;
            jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << "    Failed test case ";
            jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << current_state->current_case->description << "'\n";
        }

        current_state->current_case->assertion_fail_count++;
        current_state->metrics.assertion_fail_count++;
    }

    jg::ostream_color_scope(std::cout, jg::fg_red_bright()) << (current_state ? "      " : "") << "Failed test assertion ";
    jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << expr_string << '\'';
    std::cout << " at ";
    jg::ostream_color_scope(std::cout, jg::fg_magenta_bright()) << file << ':' << line << '\n';
}

} // namespace detail

int test_run()
{
    detail::test_state state{};
    detail::current_state = &state;

    for (auto& suites : super_suites)
    {
        std::cout << "Running test super suite ";
        jg::ostream_color_scope(std::cout, jg::fg_cyan_bright()) << '\'' << suites.description << "'\n";

        state.metrics.suite_count += suites.items.size();
        state_scope_value test_suites_scope(state.current_suites, &suites, nullptr);

        for (auto& suite : suites.items)
        {
            state.metrics.case_count += suite.items.size();
            state_scope_value test_suite_scope(state.current_suite, &suite, nullptr);

            for (auto& test : suite.items)
            {
                state_scope_value test_case_scope(state.current_case, &test, nullptr);
                test.func();

                if (test.assertion_fail_count > 0)
                    state.metrics.case_fail_count++;
            }
        }
    }

    if (state.metrics.case_count == 0)
        jg::ostream_color_scope(std::cout, jg::fg_yellow_bright()) << "No test cases\n";
    else
    {
        if (state.metrics.assertion_fail_count > 0)
            ; // suite and case failures are handled in test_assert_impl
        else
            jg::ostream_color_scope(std::cout, jg::fg_green_bright()) << "All tests succeeded\n";

        std::cout << state.metrics.assertion_count  << (state.metrics.assertion_count == 1 ? " test assertion" : " test assertions") << '\n'
                  << state.metrics.case_count       << (state.metrics.case_count == 1 ? " test case" : " test cases") << '\n'
                  << state.metrics.suite_count      << (state.metrics.suite_count == 1 ? " test suite" : " test suites") << '\n';
    }

    return static_cast<int>(state.metrics.assertion_fail_count);
}

} // namespace jg

#undef JG_TEST_IMPL
#endif

#ifdef JG_TEST_MAIN
int main()
{
    return jg::test_run();
}
#endif
