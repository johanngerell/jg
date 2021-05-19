#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include "jg_verify.h"

namespace jg {

/// Calls a unary function for each item in a range.
template <typename Range, typename Callable>
constexpr void for_each(Range& range, Callable callable)
{
    for (auto&& item : range)
        callable(item);
}

/// Calls a unary function for each item in a range, and stops
/// if the function returns false.
template <typename Range, typename Callable>
constexpr bool for_each_true(Range& range, Callable callable)
{
    for (auto&& item : range)
        if (!callable(item))
            return false;
    
    return true;
}

// std::find_if isn't constexpr in C++17
template <typename FwdIt, typename Pred>
constexpr FwdIt find_if(FwdIt first, FwdIt last, Pred pred)
{
    for (; first != last; ++first)
        if (pred(*first))
            return first;
    
    return last;
}

// TODO: Maybe make constexpr when jg requires C++20
template <typename FwdIt>
auto average(FwdIt first, FwdIt last)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);
    jg::debug_verify(first != last);

    return std::accumulate(first, last, value_type(0)) / std::distance(first, last);
}

// TODO: Maybe make constexpr when jg requires C++20
template <typename FwdIt>
auto median(FwdIt first, FwdIt last)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);
    jg::debug_verify(first != last);

    const size_t nth = std::distance(first, last) / 2;
    std::nth_element(first, first + nth, last);
    std::advance(first, nth);

    return *first;
}

template <typename T>
constexpr T abs_diff(T first, T second)
{
    return first >= second ? first - second : second - first;
}

template <typename T>
constexpr T abs_diff_squared(T first, T second)
{
    return abs_diff(first, second) * abs_diff(first, second);
}

// TODO: Maybe make constexpr when jg requires C++20
template <typename FwdIt, typename TAverage>
auto standard_deviation(FwdIt first, FwdIt last, TAverage average)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);
    jg::debug_verify(first != last);

    auto diff_squared = [average] (auto ack, auto value) {
        return ack + abs_diff_squared(average, value);
    };

    return sqrt(std::accumulate(first, last, value_type(0), diff_squared) / std::distance(first, last));
}

// TODO: Maybe make constexpr when jg requires C++20
template <typename FwdIt, typename TMedian>
auto median_absolute_deviation(FwdIt first, FwdIt last, TMedian median)
{
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    static_assert(std::is_arithmetic_v<value_type>);
    jg::debug_verify(first != last);

    auto diff = [median] (auto value) {
        return abs_diff(median, value);
    };

    std::vector<value_type> deviations;
    std::transform(first, last, std::back_inserter(deviations), diff);

    return jg::median(deviations.begin(), deviations.end());
}

} // namespace jg
