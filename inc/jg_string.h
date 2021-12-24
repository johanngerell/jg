#pragma once

#include <array>
#include <cstring>
#include <charconv>
#include <optional>
#include <string>
#include "jg_span.h"
#include "jg_verify.h"

namespace jg {

inline std::string& trim_left(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return string.erase(0, string.find_first_not_of(chars));
}
 
inline std::string& trim_right(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return string.erase(string.find_last_not_of(chars) + 1);
}
 
inline std::string& trim(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return trim_left(trim_right(string, chars), chars);
}

constexpr bool starts_with(std::string_view string, std::string_view start)
{
    return string.rfind(start, 0) == 0;
}

/// Splits the `delimiter`-separated `string` into exactly `NumTokens` tokens.
/// @returns A fix-sized array with `NumToken` tokens if `string` has exactly `NumTokens` tokens separated
///          by a `delimiter` character; otherwise `std::nullopt`.
template <size_t NumTokens>
constexpr std::optional<std::array<std::string_view, NumTokens>> split(std::string_view string, char delimiter)
{
    static_assert(NumTokens > 0);

    std::array<std::string_view, NumTokens> tokens;

    for (size_t i = 0; i < NumTokens; ++i)
    {
        const size_t offset = string.find(delimiter);

        if (offset != std::string_view::npos && i == NumTokens - 1)
            return std::nullopt; // Shouldn't find a delimiter after the last token.

        if (offset == std::string_view::npos && i != NumTokens - 1)
            return std::nullopt; // Should find a delimiter before the last token.

        tokens[i] = string.substr(0, offset);

        if (offset != std::string_view::npos)
            string.remove_prefix(offset + 1);
    }

    return tokens;
}

template <typename FwdIt>
std::string join(FwdIt first, FwdIt last, std::string_view delimiter)
{
    if (first == last)
        return {};

    std::string joined{*first};

    while (++first != last)
        joined.append(delimiter).append(*first);

    return joined;
}

template <typename FwdIt>
struct ostream_joiner final
{
    FwdIt first{};
    FwdIt last{};
    std::string_view delimiter;

    friend std::ostream& operator<<(std::ostream& stream, const ostream_joiner& self)
    {
        bool separate = false;
        for(auto it = self.first; it != self.last; ++it)
            stream << (separate ? self.delimiter : (separate = true, "")) << *it;
        
        return stream;
    }
};

template <typename FwdIt>
ostream_joiner<FwdIt> ostream_join(FwdIt first, FwdIt last, std::string_view delimiter)
{
    return {first, last, delimiter};
}

template <typename TConv, typename TRet = TConv>
constexpr std::optional<TRet> from_chars(std::string_view string)
{
    TConv value{};
    auto [_, result] = std::from_chars(string.data(), string.data() + string.size(), value);

    if (result == std::errc())
        return static_cast<TRet>(value);

    return std::nullopt;
}

} // namespace jg
