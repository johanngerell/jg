#ifdef JG_TEST_IMPL
#undef JG_TEST_INCLUDED
#endif

#ifndef JG_TEST_INCLUDED
#define JG_TEST_INCLUDED

#include <iostream>
#include <functional>
#include <vector>
#include "jg_state_scope.h"
#include "jg_ostream_color.h"
#include "jg_source_location.h"

/// @file Testing facilities, like assertions, test cases, test suites, test suite sets, test runner, etc.
/// @note One translation unit *must* define JG_TEST_IMPL before including this header. This is typically
/// done by the main cpp file. The translation unit that defines JG_TEST_IMPL is the one where the test
/// implementation will be defined and compiled. If no translation unit defines JG_TEST_IMPL, then there
/// will be undefined symbol linker errors as tests refer to the undefined functions.
/// @note By defining JG_TEST_ENABLE_SHORT_NAME before including this header, the shorter macro name
/// `jg_assert` can be used instead of `jg_test_assert`.

namespace jg {

struct test_assertion final
{
    std::string expression;
    source_location location;
    bool succeeded{};
};

struct test_case final
{
    std::string description;
    std::function<void()> func;
    std::vector<test_assertion> assertions; // The implementation will add assertion information here.
    size_t assertion_fail_count{};

    test_case(std::string description, std::function<void()> func)
        : description{std::move(description)}
        , func{std::move(func)}
    {}
};

struct test_suite final
{
    std::string description;
    std::vector<test_case> cases;
    size_t case_fail_count{};
};

struct test_suite_set final
{
    std::string description;
    std::vector<test_suite> suites;
};

void test_add(jg::test_suite_set&& set);
int test_run();

/// A `test_adder` instance can be used for "auto discovery" of test suites spread over cpp files when
/// one cpp file defines JG_TEST_MAIN. Instantiate the `test_adder` at global scope and it will add the
/// test suites in its constructor, before `main()` is called. The `main()` that is generated when
/// JG_TEST_MAIN is defined will execute all added test suites.
/// @example
///     // flubber_tests.cpp
///     #include <jg_test.h>
///     #include "flubber.h"
/// 
///     jg::test_adder flubber_tests { "flubber test suites", {
///         jg::test_suite { "some flubber functionality test suite", {
///             jg::test_case { "flubber me this", [] {
///                 jg_test_assert(...);
///                 ...
///             }},
///             ...
///         }},
///         ...
///     }};
///
///     // main.cpp
///     #define JG_TEST_MAIN
///     #define JG_TEST_IMPL
///     #include <jg_test.h>
struct test_adder final
{
    test_adder(std::string description, std::vector<test_suite>&& suites)
    {
        test_add({std::move(description), std::move(suites)});
    }
};

} // namespace jg

namespace jg::detail {

void test_assert_prolog(const char* expr_string, const source_location& location);
void test_assert_epilog(const char* expr_string, const source_location& location);

inline void test_assert_impl(bool expr_value, const char* expr_string, const source_location& location)
{
    // TODO: Handle exceptions as failures.
    jg::detail::test_assert_prolog(expr_string, location);
    if (expr_value) return;
    jg::detail::test_assert_epilog(expr_string, location);
}

template <typename TException, typename TExprFunc>
void test_assert_exception_impl(TExprFunc&& expr_func, const char* expr_string, const source_location& location)
{
    jg::detail::test_assert_prolog(expr_string, location);
    try { expr_func(); }
    catch(const TException&) { return; }
    catch(...) {}
    jg::detail::test_assert_epilog(expr_string, location);
}

} // namespace jg::detail

/// The `jg_test_assert` macro can be used both on its own in simple test files (like `main.cpp` with a
/// bunch of assertions) and in test cases inside suites. It will not exit the test program, only output
/// error information and propagate metrics back to `test_run()`, if that's used.
#define jg_test_assert(expr) \
    jg::detail::test_assert_impl((expr), #expr, jg_current_source_location())

/// The `jg_test_assert_exception` macro can be used both on its own in simple test files (like `main.cpp`
/// with a bunch of assertions) and in test cases inside suites. It will not exit the test program, only
/// output error information and propagate metrics back to `test_run()`, if that's used.
#define jg_test_assert_exception(expr, exception_type) \
    jg::detail::test_assert_exception_impl<exception_type>([&] { (expr); }, #expr, jg_current_source_location())

#ifdef JG_TEST_ENABLE_SHORT_NAME
    #define jg_assert jg_test_assert
#endif

#ifdef JG_TEST_IMPL
#undef JG_TEST_IMPL

#include "jg_stopwatch.h"

namespace {

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
    test_metrics        metrics;
    jg::test_case*      current_case{};
    jg::test_suite*     current_suite{};
    jg::test_suite_set* current_set{};
};

// Using the jg_test_assert macros outside of test cases and suites, which get executed by test_run(),
// is not allowed. If that is done, the unset current state will be detected.
test_state* current_state{};

std::vector<jg::test_suite_set> suite_sets;

}

namespace jg {

void test_add(jg::test_suite_set&& set)
{
    suite_sets.push_back(std::move(set));
}

int test_run()
{
    test_state state{};
    state_scope_value test_set_scope(current_state, &state, nullptr);
    stopwatch sw;

    for (auto& set : suite_sets)
    {
        std::cout << "Running test suite set "
                  << jg::ostream_color(jg::fg_cyan_bright()) << '\'' << set.description << "'\n";

        state.metrics.suite_count += set.suites.size();
        state_scope_value test_set_scope(state.current_set, &set, nullptr);

        for (auto& suite : set.suites)
        {
            state.metrics.case_count += suite.cases.size();
            state_scope_value test_suite_scope(state.current_suite, &suite, nullptr);

            for (auto& test : suite.cases)
            {
                state_scope_value test_case_scope(state.current_case, &test, nullptr);
                test.func();

                if (test.assertion_fail_count > 0)
                    state.metrics.case_fail_count++;
            }
        }
    }

    if (state.metrics.case_count == 0)
        std::cout << jg::ostream_color(jg::fg_yellow_bright()) << "No test cases\n";
    else
    {
        if (state.metrics.assertion_fail_count > 0)
            ; // suite and case failures are handled in test_assert_impl
        else
            std::cout << jg::ostream_color(jg::fg_green_bright()) << "All tests succeeded\n";

        std::cout << state.metrics.assertion_count  << (state.metrics.assertion_count == 1 ? " test assertion" : " test assertions") << '\n'
                  << state.metrics.case_count       << (state.metrics.case_count == 1 ? " test case" : " test cases") << '\n'
                  << state.metrics.suite_count      << (state.metrics.suite_count == 1 ? " test suite" : " test suites") << '\n'
                  << sw.us()                        << " microseconds" << '\n';
    }

    return static_cast<int>(state.metrics.assertion_fail_count);
}

} // namespace jg

namespace jg::detail {

void test_assert_prolog(const char* expr_string, const source_location& location)
{
    if (!current_state)
    {
        std::cout << jg::ostream_color(jg::fg_red_bright()) << "Runaway test assertion";
        std::cout << " at ";
        std::cout << jg::ostream_color(jg::fg_magenta_bright()) << location.file_name() << ':' << location.line() << '\n';
        return;
    }

    current_state->metrics.assertion_count++;
    current_state->current_case->assertions.push_back({expr_string, location, true});
}

void test_assert_epilog(const char* expr_string, const source_location& location)
{
    if (!current_state)
        return; // test_assert_prolog outputs an error

    if (current_state->current_suite->case_fail_count == 0)
    {
        std::cout << jg::ostream_color(jg::fg_red_bright()) << "  Failed test suite ";
        std::cout << jg::ostream_color(jg::fg_cyan_bright()) << '\'' << current_state->current_suite->description << "'\n";
    }

    if (current_state->current_case->assertion_fail_count == 0)
    {
        current_state->current_suite->case_fail_count++;
        std::cout << jg::ostream_color(jg::fg_red_bright()) << "    Failed test case ";
        std::cout << jg::ostream_color(jg::fg_cyan_bright()) << '\'' << current_state->current_case->description << "'\n";
    }

    current_state->current_case->assertions.back().succeeded = false;
    current_state->current_case->assertion_fail_count++;
    current_state->metrics.assertion_fail_count++;

    std::cout << jg::ostream_color(jg::fg_red_bright()) << (current_state ? "      " : "") << "Failed test assertion ";
    std::cout << jg::ostream_color(jg::fg_cyan_bright()) << '\'' << expr_string << '\'';
    std::cout << " at ";
    std::cout << jg::ostream_color(jg::fg_magenta_bright()) << location.file_name() << ':' << location.line() << '\n';
}

} // namespace jg::detail

#endif // #ifdef JG_TEST_IMPL
#endif // #ifndef JG_TEST_INCLUDED

// TODO: Is a flag similar to JG_TEST_INCLUDED needed?
#ifdef JG_TEST_MAIN
int main()
{
    return jg::test_run();
}
#endif
