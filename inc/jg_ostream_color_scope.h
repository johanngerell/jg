#pragma once

#include <iostream>
#include <string_view>

namespace jg
{

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

class ostream_color_scope final
{
public:
    ostream_color_scope(std::ostream& stream, fg_color fg)
        : m_stream{stream}
    {
        m_stream << "\033[" << fg.code << 'm';
    }

    ostream_color_scope(std::ostream& stream, fg_color fg, bg_color bg)
        : m_stream{stream}
    {
        m_stream << "\033[" << fg.code << ';' << bg.code << 'm';
    }

    template <typename T>
    friend std::ostream& operator<<(const ostream_color_scope& self, const T& t)
    {
        self.m_stream << t;
        return self.m_stream;
    }

    ~ostream_color_scope()
    {
        m_stream << "\033[0m";
    }

private:
    std::ostream& m_stream;
};

} // namespace jg
