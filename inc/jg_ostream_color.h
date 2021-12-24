#pragma once

#include <iostream>
#include <optional>
#include <string_view>

namespace jg {

template <typename Tag>
struct ansi_color
{
    std::string_view code;
};

struct fg_tag{};
using fg_color = ansi_color<fg_tag>;

struct bg_tag{};
using bg_color = ansi_color<bg_tag>;

constexpr fg_color fg_normal()         { return {"39"}; }
constexpr fg_color fg_black()          { return {"30"}; }
constexpr fg_color fg_red()            { return {"31"}; }
constexpr fg_color fg_green()          { return {"32"}; }
constexpr fg_color fg_yellow()         { return {"33"}; }
constexpr fg_color fg_blue()           { return {"34"}; }
constexpr fg_color fg_magenta()        { return {"35"}; }
constexpr fg_color fg_cyan()           { return {"36"}; }
constexpr fg_color fg_white()          { return {"37"}; }
constexpr fg_color fg_black_bright()   { return {"90"}; }
constexpr fg_color fg_red_bright()     { return {"91"}; }
constexpr fg_color fg_green_bright()   { return {"92"}; }
constexpr fg_color fg_yellow_bright()  { return {"93"}; }
constexpr fg_color fg_blue_bright()    { return {"94"}; }
constexpr fg_color fg_magenta_bright() { return {"95"}; }
constexpr fg_color fg_cyan_bright()    { return {"96"}; }
constexpr fg_color fg_white_bright()   { return {"97"}; }

constexpr bg_color bg_normal()         { return {"49"}; }
constexpr bg_color bg_black()          { return {"40"}; }
constexpr bg_color bg_red()            { return {"41"}; }
constexpr bg_color bg_green()          { return {"42"}; }
constexpr bg_color bg_yellow()         { return {"43"}; }
constexpr bg_color bg_blue()           { return {"44"}; }
constexpr bg_color bg_magenta()        { return {"45"}; }
constexpr bg_color bg_cyan()           { return {"46"}; }
constexpr bg_color bg_white()          { return {"47"}; }
constexpr bg_color bg_black_bright()   { return {"100"}; }
constexpr bg_color bg_red_bright()     { return {"101"}; }
constexpr bg_color bg_green_bright()   { return {"102"}; }
constexpr bg_color bg_yellow_bright()  { return {"103"}; }
constexpr bg_color bg_blue_bright()    { return {"104"}; }
constexpr bg_color bg_magenta_bright() { return {"105"}; }
constexpr bg_color bg_cyan_bright()    { return {"106"}; }
constexpr bg_color bg_white_bright()   { return {"107"}; }

/// Outputs an ANSI foreground color code, and optionally an ANSI background color code, to the current
/// output stream. The beginning color code is inserted into the stream at instantiation, and the ending
/// color code is inserted when the instance goes out of scope.
/// @example
///     std::cout << "This is default colored... "
///               << jg::ostream_color(jg::fg_green())
///               << "but this is green... "
///               << "and this too\n";           // <-- The jg::ostream_color instance goes out of scope
///     std::cout << "Back to default color\n";
class ostream_color final
{
public:
    ostream_color(fg_color fg)
        : m_fg{fg}
    {}

    ostream_color(fg_color fg, bg_color bg)
        : m_fg{fg}
        , m_bg{bg}
    {}

    friend std::ostream& operator<<(std::ostream& stream, const ostream_color& self)
    {
        self.m_stream = &stream;
        stream << "\033[" << self.m_fg.code;

        if (self.m_bg)
            stream << ';' << self.m_bg->code;

        return stream << 'm';
    }

    ~ostream_color()
    {
        if (m_stream)
            *m_stream << "\033[0m";
    }

private:
    mutable std::ostream* m_stream{};
    fg_color m_fg;
    std::optional<bg_color> m_bg; // TODO: Use a "no color" value instead of optional
};

} // namespace jg
