#pragma once

#include <type_traits>
#include <tuple>

namespace jg {

template <typename Callable>
struct callable_traits : callable_traits<decltype(&std::remove_reference_t<Callable>::operator())>
{
};

template <typename Class, typename Return, typename... Args>
struct callable_traits<Return(Class::*)(Args...) const> : callable_traits<Return(*)(Args...)>
{
};

template <typename Class, typename Return, typename... Args>
struct callable_traits<Return(Class::*)(Args...)> : callable_traits<Return(*)(Args...)>
{
};

template <typename Return, typename... Args>
struct callable_traits<Return(*)(Args...)>
{
    using return_t = Return;

    template <size_t Index>
    using arg_t = std::tuple_element_t<Index, std::tuple<Args...>>;
};

template <typename Callable, size_t Index = 0>
using param_t = typename callable_traits<Callable>::template arg_t<Index>;

template <typename Callable>
using return_t = typename callable_traits<Callable>::return_t;

} // namespace jg
