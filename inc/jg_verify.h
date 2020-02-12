#pragma once

#ifndef NDEBUG
#include <cassert>
#include "jg_stacktrace.h"
#endif

namespace jg
{

/// Asserts that `condition` evaluates to `true` and writes a stack trace to `stdout` if it isn't.
/// Will be a no-op in optimized builds with `NDEBUG` defined.
void verify(bool condition);

/// Calls `verify(ptr)` and returns `ptr`.
/// Will be a no-op in optimized builds with `NDEBUG` defined.
template <typename T>
T* verified(T* ptr);

#ifdef NDEBUG

inline void verify(bool) {}

template <typename T>
inline T* verified(T* ptr) { return ptr; }

#else

inline void verify(bool condition)
{
    if (!condition)
        for (const auto& stack_frame : jg::stack_trace().include_frame_count(10).skip_frame_count(1).capture())
            std::cout << stack_frame << "\n";

    assert(condition);
}

template <typename T>
inline T* verified(T* ptr)
{
    verify(ptr);
    return ptr;
}

#endif

} // namespace jg
