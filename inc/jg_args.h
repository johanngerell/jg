#pragma once

#include "jg_string.h"

namespace jg
{

/// Provides iteration over the command line arguments given to the main function in C and C++ programs.
///
/// @note Subsystems that use other entry points than `main()` often expose the corresponding command
///       line parameters in other ways. For instance, Windows GUI programs that use `WinMain()` can
///       access them in the C runtime variables __argc and __argv.
/// @note An argc-argv pair that is supplied by the system is trusted by jg::args; the array pointed at
///       by argv is assumed to always have argc entries.
class args final
{
public:
    args() = default;
    args(int argc, char** argv)
        : m_first{&argv[0]}
        , m_last{m_first + argc}
    {}

    using iterator = char**;
    using const_iterator = const iterator;

    constexpr const_iterator begin() const { return m_first; }
    constexpr const_iterator end() const { return m_last; }

private:
    iterator m_first{};
    iterator m_last{};
};

namespace detail
{

// algorithms aren't constexpr in C++17
template <typename FwdIt, typename Pred>
constexpr FwdIt find_if(FwdIt first, FwdIt last, Pred pred)
{
    for (; first != last; ++first)
        if (pred(*first))
            return first;
    
    return last;
}

} // namespace detail

constexpr std::optional<std::string_view> args_key_value(jg::args args, std::string_view key) noexcept
{
    // std::find_if in C++20
    auto it = detail::find_if(args.begin(),
                              args.end(),
                              [key] (auto& arg) { return starts_with(arg, key); });

    if (it != args.end())
        return std::string_view(*it).substr(key.length());
    
    return std::nullopt;
}

constexpr bool args_has_key(jg::args args, std::string_view key) noexcept
{
    // std::any_of in C++20
    auto it = detail::find_if(args.begin(),
                              args.end(),
                              [key] (auto& arg) { return key == arg; });

    return it != args.end();
}

} // namespace jg
