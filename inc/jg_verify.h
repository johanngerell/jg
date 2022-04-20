#pragma once

#include <cassert>
#if defined(JG_VERIFY_ENABLE_STACK_TRACE)
#include "jg_stacktrace.h"
#endif

#if defined(JG_VERIFY_ASSERTION)
    void JG_VERIFY_ASSERTION(bool);
#endif

namespace jg {

/// Verifies that `condition` evaluates to `true`.
///
/// If `condition` is `false` and...
///
///   * JG_VERIFY_ENABLE_STACK_TRACE is defined, then a stack trace is written to `stdout`,
///   * JG_VERIFY_ENABLE_TERMINATE is defined, then `std::terminate()` is called,
///   * JG_VERIFY_ENABLE_TERMINATE isn't defined, then `assert(condition)` fails (which is a no-op if NDEBUG is defined).
///
/// If NDEBUG is defined and neither JG_VERIFY_ENABLE_STACK_TRACE, JG_VERIFY_ENABLE_TERMINATE, or
/// JG_VERIFY_ASSERTION is defined, then this function will be a no-op in an optimized build.
///
/// The compilation flags JG_VERIFY_ENABLE_STACK_TRACE and JG_VERIFY_ENABLE_TERMINATE enables a "checked release" build
/// configuration which is optimized, but still fails fast and hard in tests.
#if (__cplusplus > 201402L)
inline void verify([[maybe_unused]] bool condition)
#else
inline void verify(bool condition)
#endif // >= C++17
{
#if defined(JG_VERIFY_ENABLE_STACK_TRACE)
    if (!condition)
        for (const auto& frame : stack_trace().take(10).skip(1).capture())
            std::cout << frame << "\n";
#endif

#if defined(JG_VERIFY_ENABLE_TERMINATE)
    if (!condition)
        std::terminate();
#endif

#if defined(JG_VERIFY_ASSERTION)
    JG_VERIFY_ASSERTION(condition);
#else
    assert(condition);
#endif
}

/// Verifies that `condition` evaluates to `true` when NDEBUG is not defined (a.k.a. a debug build)
///
/// If `condition` is `false` and...
///
///   * JG_VERIFY_ENABLE_STACK_TRACE is defined, then a stack trace is written to `stdout`,
///   * JG_VERIFY_ENABLE_TERMINATE is defined, then `std::terminate()` is called,
///   * JG_VERIFY_ENABLE_TERMINATE isn't defined, then `assert(condition)` fails.
///
/// If NDEBUG is defined then this function will be a no-op (a.k.a. a release build).
inline void debug_verify(bool condition)
{
    (void)condition;

#if !defined(NDEBUG)
    verify(condition);
#endif
}

template <typename Func>
inline void debug_verify(bool condition, Func on_failure)
{
    (void)condition;
    (void)on_failure;

#if !defined(NDEBUG)
    if (!condition)
        on_failure();
    verify(condition);
#endif
}

/// Calls `verify(ptr)` and returns `ptr`. Will be a no-op when `verify(ptr)` is a no-op.
template <typename T>
inline T* verified(T* ptr)
{
    verify(ptr);
    return ptr;
}

inline bool verified(bool condition)
{
    verify(condition);
    return condition;
}

} // namespace jg
