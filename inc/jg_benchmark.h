#pragma once

#include <algorithm>
#include <numeric>
#include <type_traits>
#include "jg_stopwatch.h"

namespace jg
{

struct benchmark_result final
{
    std::vector<std::chrono::nanoseconds::rep> samples;
    std::chrono::nanoseconds::rep average{};
    std::chrono::nanoseconds::rep median{};
};

// TODO: Move to jg_algorithm.h
template <typename FwdIt>
auto average(FwdIt first, FwdIt last)
{
    using value_type = std::remove_reference_t<decltype(*std::declval<FwdIt>())>;
    static_assert(std::is_arithmetic_v<value_type>);

    return first != last ?
           std::accumulate(first, last, static_cast<value_type>(0)) / std::distance(first, last) :
           static_cast<value_type>(0);
}

// TODO: Move to jg_algorithm.h
template <typename FwdIt>
auto median(FwdIt first, FwdIt last)
{
    using value_type = std::remove_reference_t<decltype(*std::declval<FwdIt>())>;
    static_assert(std::is_arithmetic_v<value_type>);

    const size_t nth = std::distance(first, last) / 2;
    std::nth_element(first, first + nth, last);
    std::advance(first, nth);

    return *first;
}

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

    result.average = average(result.samples.begin(), result.samples.end());
    result.median = median(result.samples.begin(), result.samples.end());

    return result;
}

} // namespace jg