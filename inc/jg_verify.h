#pragma once

#include <cassert>
#include "jg_stacktrace.h"
#endif

namespace jg
{

/// Verifies that `condition` evaluates to `true`.
///
/// If `condition` is `false` and...
///
///   * JG_VERIFY_ENABLE_STACK_TRACE is defined, or NDEBUG isn't defined, then a stack trace is written to `stdout`,
///   * JG_VERIFY_ENABLE_TERMINATE is defined, then `std::terminate()` is called,
///   * JG_VERIFY_ENABLE_TERMINATE isn't defined, then `assert(condition)` fails (which is a no-op if NDEBUG is defined).
///
/// If NDEBUG is defined and neither JG_VERIFY_ENABLE_STACK_TRACE nor JG_VERIFY_ENABLE_TERMINATE is defined, then
/// this function will be a no-op in an optimized build.
///
/// The compilation flags JG_VERIFY_ENABLE_STACK_TRACE and JG_VERIFY_ENABLE_TERMINATE enables a "checked release" build
/// configuration which is optimized, but still fails fast and hard in tests.
inline void verify(bool condition)
{
#if defined(JG_VERIFY_ENABLE_STACK_TRACE) || !defined(NDEBUG)
    if (!condition)
        for (const auto& stack_frame : jg::stack_trace().include_frame_count(10).skip_frame_count(1).capture())
            std::cout << stack_frame << "\n";
#endif

#ifdef JG_VERIFY_ENABLE_TERMINATE
    if (!condition)
        std::terminate();
#else
    assert(condition);
#endif
}

/// Calls `verify(ptr)` and returns `ptr`. Will be a no-op when `verify(ptr)` is a no-op.
template <typename T>
inline T* verified(T* ptr)
{
    verify(ptr);
    return ptr;
}

} // namespace jg
