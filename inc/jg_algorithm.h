#pragma once

#include <algorithm>
#include <numeric>

namespace jg {

// std::find_if isn't constexpr in C++17
template <typename FwdIt, typename Pred>
constexpr FwdIt find_if(FwdIt first, FwdIt last, Pred pred)
{
    for (; first != last; ++first)
        if (pred(*first))
            return first;
    
    return last;
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt>
auto average(FwdIt first, FwdIt last)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);

    return std::accumulate(first, last, static_cast<value_type>(0)) / (first != last ? std::distance(first, last) : 1);
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt>
auto median(FwdIt first, FwdIt last)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);

    const size_t nth = std::distance(first, last) / 2;
    std::nth_element(first, first + nth, last);
    std::advance(first, nth);

    return first != last ?
           *first :
           static_cast<value_type>(0);
}

template <typename T>
constexpr T abs_diff(T first, T second)
{
    return first >= second ? first - second : second - first;
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt, typename TAverage>
auto standard_deviation(FwdIt first, FwdIt last, TAverage average)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);

    return sqrt(std::accumulate(first, last, value_type(0),
                                [average] (auto value) {
                                    return abs_diff(value, average) * abs_diff(value, average);
                                }) / (first != last ? std::distance(first, last) : 1));
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt, typename TMedian>
auto median_absolute_deviation(FwdIt first, FwdIt last, TMedian median)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);

    std::vector<value_type> deviations;

    std::transform(first, last, std::back_inserter(deviations),
                   [median] (auto value) {
                       return abs_diff(value, median);
                   });

    return jg::median(deviations.begin(), deviations.end());
}

} // namespace jg
