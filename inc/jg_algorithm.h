#pragma once

#include <algorithm>
#include <numeric>
#include <type_traits>

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
    using value_type = std::remove_reference_t<decltype(*std::declval<FwdIt>())>;
    static_assert(std::is_arithmetic_v<value_type>);

    return first != last ?
           std::accumulate(first, last, static_cast<value_type>(0)) / std::distance(first, last) :
           static_cast<value_type>(0);
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt>
auto median(FwdIt first, FwdIt last)
{
    using value_type = std::remove_reference_t<decltype(*std::declval<FwdIt>())>;
    static_assert(std::is_arithmetic_v<value_type>);

    const size_t nth = std::distance(first, last) / 2;
    std::nth_element(first, first + nth, last);
    std::advance(first, nth);

    return first != last ?
           *first :
           static_cast<value_type>(0);
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt, typename TAverage>
auto standard_deviation(FwdIt first, FwdIt last, TAverage average)
{
    using value_type = std::remove_reference_t<decltype(*std::declval<FwdIt>())>;
    static_assert(std::is_arithmetic_v<value_type>);

    value_type variance{};

    for (auto it = first; it != last; ++it)
        variance += (*it - average) * (*it - average);

    return first != last ?
           sqrt(variance / std::distance(first, last)) :
           static_cast<value_type>(0);
}

// TODO: Make constexpr when jg requires C++20
template <typename FwdIt, typename TMedian>
auto median_absolute_deviation(FwdIt first, FwdIt last, TMedian median)
{
    using value_type = std::remove_reference_t<decltype(*std::declval<FwdIt>())>;
    static_assert(std::is_arithmetic_v<value_type>);

    std::vector<value_type> absolute_deviations(first, last);

    for (auto& value : absolute_deviations)
        value = value >=  median ? value - median : median - value;

    return jg::median(absolute_deviations.begin(), absolute_deviations.end());
}

} // namespace jg
