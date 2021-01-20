#pragma once

#include <chrono>

namespace jg
{

template <typename Func>
std::chrono::nanoseconds benchmark(size_t sample_count, Func&& func)
{
    const auto t1 = std::chrono::high_resolution_clock::now();

    for (size_t sample = 0; sample < sample_count; ++sample)
        func();

    const auto t2 = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1) / sample_count;
}

} // namespace jg