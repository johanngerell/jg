#pragma once

#include "jg_string.h"

namespace jg
{

class args final
{
public:
    args() = default;
    args(int argc, char** argv) : m_args{&argv[0], &argv[0] + argc} {}

    using iterator = char**;
    using const_iterator = const iterator;

    constexpr const_iterator begin() const { return m_args.begin(); }
    constexpr const_iterator end() const { return m_args.end(); }

private:
    jg::span<char*> m_args;
};

constexpr std::optional<std::string_view> arg_key_value(std::string_view arg, std::string_view key)
{
    if (starts_with(arg, key))
        return arg.substr(key.length());
    
    return std::nullopt;
}

constexpr std::optional<std::string_view> args_key_value(jg::args args, std::string_view key)
{
    for (auto arg : args)
        if (auto value = arg_key_value(arg, key))
            return value;

    return std::nullopt;
}

constexpr bool args_has_key(jg::args args, std::string_view key)
{
    // algorithms aren't constexpr in C++17
    // return std::any_of(args.begin(),
    //                    args.end(),
    //                    [key] (auto& arg) { return arg == key; });

    for (const auto arg : args)
        if (arg == key)
            return true;
    
    return false;
}

} // namespace jg