#pragma once

#include "jg_string.h"

namespace jg
{

class args final
{
public:
    args() = default;
    args(int argc, char** argv) : m_argc{argc}, m_argv{argv} {}

    using iterator = char**;
    using const_iterator = const iterator;

    constexpr const_iterator begin() const { return &m_argv[0]; }
    constexpr const_iterator end() const { return begin() + m_argc; }

private:
    int m_argc{};
    char** m_argv{};
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