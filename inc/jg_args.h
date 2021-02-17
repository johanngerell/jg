#pragma once

#include "jg_algorithm.h"
#include "jg_string.h"

namespace jg {

/// Iterates over command line arguments passed to the main() function in its argc-argv parameters.
///
/// @note Subsystems that use other entry points than main() often expose the corresponding command line
///       arguments in other ways. For instance, Windows GUI programs that use WinMain() can access
///       them in the C runtime variables __argc and __argv.
/// @note An argc-argv pair supplied by the system is trusted by jg::args; As per spec, the array pointed
///       at by argv is assumed to always have argc + 1 entries, where argv[argc] is nullptr.
class args final
{
public:
    constexpr args() = default;
    constexpr args(int argc, char** argv)
        : m_first{&argv[0]}
        , m_last{m_first + argc}
    {}

    using iterator = char**;

    constexpr iterator begin() const { return m_first; }
    constexpr iterator end() const { return m_last; }

private:
    iterator m_first{};
    iterator m_last{};
};

/// Checks if there is an item in `args` that starts with `key` and then returns the remainder of the item.
/// If the item is "--foo=bar", then a check for the key "--foo=" will return "bar".
/// If no item starts with `key`, then `std::nullopt` is returned.
constexpr std::optional<std::string_view> args_key_value(jg::args args, std::string_view key) noexcept
{
    // Use std::find_if in C++20
    auto it = jg::find_if(args.begin(),
                          args.end(),
                          [key] (auto& arg) { return starts_with(arg, key); });

    if (it != args.end())
        return *it + key.length();
    
    return std::nullopt;
}

/// Checks if there is an item in `args` that equals `key`.
/// If the item is "--foo=bar", then the key "--foo" isn't a match.
constexpr bool args_has_key(jg::args args, std::string_view key) noexcept
{
    // Use std::any_of in C++20
    auto it = jg::find_if(args.begin(),
                          args.end(),
                          [key] (auto& arg) { return key == arg; });

    return it != args.end();
}

} // namespace jg
