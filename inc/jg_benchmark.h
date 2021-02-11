#pragma once

#include "jg_algorithm.h"
#include "jg_stopwatch.h"

namespace jg {

struct benchmark_result final
{
    std::vector<std::chrono::nanoseconds::rep> samples;
    std::chrono::nanoseconds::rep average{};
    std::chrono::nanoseconds::rep median{};
};

template <typename Func>
benchmark_result benchmark(size_t sample_count, Func&& func)
{
    benchmark_result result;
    result.samples.reserve(sample_count);

    for (size_t sample = 0; sample < sample_count; ++sample)
    {
        jg::stopwatch sw;
        func();
        result.samples.push_back(sw.ns());
    }

    result.average = jg::average(result.samples.begin(), result.samples.end());
    result.median = jg::median(result.samples.begin(), result.samples.end());

    return result;
}

} // namespace jg
