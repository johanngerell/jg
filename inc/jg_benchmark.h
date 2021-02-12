#pragma once

#include "jg_algorithm.h"
#include "jg_stopwatch.h"

namespace jg {

struct benchmark_result final
{
    using sample_type = std::chrono::nanoseconds::rep;

    std::string description;
    std::vector<sample_type> samples;
    sample_type average{};
    sample_type median{};
    sample_type std_deviation{};
};

template <typename Func>
benchmark_result benchmark(std::string_view description, size_t sample_count, size_t func_internal_count, Func&& func)
{
    benchmark_result result;
    result.description = description;
    result.samples.reserve(sample_count);

    for (size_t sample = 0; sample < sample_count; ++sample)
    {
        jg::stopwatch sw;
        func();
        result.samples.push_back(sw.ns() / func_internal_count);
    }

    result.average = jg::average(result.samples.begin(), result.samples.end());
    result.median = jg::median(result.samples.begin(), result.samples.end());

    benchmark_result::sample_type variance{};

    for (auto& value : result.samples)
        variance += (value - result.average) * (value - result.average);

    variance /= result.samples.size();
    result.std_deviation = sqrt(variance);

    return result;
}

} // namespace jg
