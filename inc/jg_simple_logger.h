#ifdef JG_SIMPLE_LOGGER_IMPL
#undef JG_SIMPLE_LOGGER_INCLUDED
#endif

#ifndef JG_SIMPLE_LOGGER_INCLUDED
#define JG_SIMPLE_LOGGER_INCLUDED

#include <chrono>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <sys/timeb.h>
#include "jg_source_location.h"
#include "jg_verify.h"

namespace jg {

using timestamp = std::chrono::system_clock::time_point;
/// Holds the seconds and milliseconds parts of a timestamp returned from `ftime()`.
// struct timestamp final
// {
//     //decltype(timeb::time) seconds{};
//     //decltype(timeb::millitm) milliseconds{};
//     std::chrono::system_clock::time_point t;

//     static timestamp now() noexcept
//     {
//         //timeb timebuffer;
//         //ftime(&timebuffer);
//         //return {timebuffer.time, timebuffer.millitm};
//         return {std::chrono::system_clock::now()};
//     }
// };

/// Formats a `timestamp` into a 24-hour "hh:mm:ss.mmm " string. The formatted string should fit in the
/// SSO buffer in all real implementations (clang, gcc, msvc, etc.), so no allocations should occur.
std::string to_string(const timestamp& timestamp);

inline std::ostream& operator<<(std::ostream& stream, const timestamp& timestamp)
{
    return stream << to_string(timestamp);
}

enum class log_level
{
    info,
    warning,
    error,
    fatal
};

constexpr std::string_view to_string(log_level severity) noexcept
{
    switch (severity)
    {
        case log_level::info:    return "[info] ";
        case log_level::warning: return "[warning] ";
        case log_level::error:   return "[error] ";
        case log_level::fatal:   return "[fatal] ";
        default:                 return "[<unknown>] ";
    }
}

inline std::ostream& operator<<(std::ostream& stream, log_level severity)
{
    return stream << to_string(severity);
}

struct log_event final
{
    timestamp timestamp{};
    log_level level{};
    source_location location{};
};

/// Checks if logging is enabled.
bool log_enabled() noexcept;

/// Checks if logging is enabled, and if `level` is equal to or higher than the minimum level.
bool log_enabled(log_level level) noexcept;

/// Enables or disables logging. Default is enabled.
void log_set_enabled(bool enabled) noexcept;

/// Sets the stream for logging output. Default is `std::cout`.
void log_set_ostream(std::ostream& ostream) noexcept;

/// Sets the minimum logging level to output. Default is `log_level::info`.
void log_set_level(log_level level) noexcept;

/// Logs the current timestamp.
std::ostream& log();

/// Logs the current timestamp and an identifier for the info logging level.
std::ostream& log_info();

/// Logs the current timestamp and an identifier for the warning logging level.
std::ostream& log_warning();

/// Logs the current timestamp and an identifier for the error logging level.
std::ostream& log_error();

/// Logs the current timestamp and an identifier for the fatal logging level.
std::ostream& log_fatal();

/// Outputs end-of-line to the contained stream when going out of scope.
class ostream_line final
{
public:
    ostream_line(std::ostream& stream = std::cout)
        : m_stream{&stream}
    {}

    ostream_line(ostream_line&& other)
        : m_stream{other.m_stream}
    {
        other.m_stream = nullptr;
    }

    template <typename T>
    friend std::ostream& operator<<(const ostream_line& self, const T& t)
    {
        *self.m_stream << t;
        return *self.m_stream;
    }

    ~ostream_line()
    {
        *m_stream << '\n';
    }

private:
    std::ostream* m_stream;
};

/// Logs the current timestamp, and end-of-line when the returned object goes out of scope.
ostream_line log_line();

/// Logs the current timestamp, an identifier for the info logging level,
/// and end-of-line when the returned object goes out of scope.
ostream_line log_info_line();

/// Logs the current timestamp, an identifier for the warning logging level,
/// and end-of-line when the returned object goes out of scope.
ostream_line log_warning_line();

/// Logs the current timestamp, an identifier for the error logging level,
/// and end-of-line when the returned object goes out of scope.
ostream_line log_error_line();

/// Logs the current timestamp, an identifier for the fatal logging level,
/// and end-of-line when the returned object goes out of scope.
ostream_line log_fatal_line();

} // namespace jg

#define jg_log()              if (jg::log_enabled())                       jg::log()
#define jg_log_info()         if (jg::log_enabled(jg::log_level::info))    jg::log_info()
#define jg_log_warning()      if (jg::log_enabled(jg::log_level::warning)) jg::log_warning()
#define jg_log_error()        if (jg::log_enabled(jg::log_level::error))   jg::log_error()
#define jg_log_fatal()        if (jg::log_enabled(jg::log_level::fatal))   jg::log_fatal()

#define jg_log_line()         if (jg::log_enabled())                       jg::log_line()
#define jg_log_info_line()    if (jg::log_enabled(jg::log_level::info))    jg::log_info_line()
#define jg_log_warning_line() if (jg::log_enabled(jg::log_level::warning)) jg::log_info_warning()
#define jg_log_error_line()   if (jg::log_enabled(jg::log_level::error))   jg::log_info_error()
#define jg_log_fatal_line()   if (jg::log_enabled(jg::log_level::fatal))   jg::log_info_fatal()

// TODO: Look up Arthur O’Dwyer's "How to replace __FILE__ with source_location in a logging macro"
// at https://quuxplusone.github.io/blog/2020/02/12/source-location/
#define jg_new_log_event(level) jg::log_event{std::chrono::system_clock::now(), level, jg_current_source_location()}

#ifdef JG_SIMPLE_LOGGER_IMPL
#undef JG_SIMPLE_LOGGER_IMPL

#include "jg_os.h"

namespace {

struct log_configuration final
{
    bool enabled{true};
    std::ostream* ostream{&std::cout};
    jg::log_level level{jg::log_level::info};
} configuration;

} // namespace

namespace jg {

#if 0
std::string to_string2(const timestamp& time)
{
    // The reserve() should be a no-op, since the formatted string should fit in the SSO buffer for
    // all major implementations.
    std::string result;
    constexpr size_t formatted_length{sizeof("HH:MM:SS.mmm ") - 1}; // implicit \0 in the string literal
    jg::debug_verify(formatted_length <= result.capacity());
    result.reserve(formatted_length);

    const std::tm tm = jg::os::localtime_safe(time.seconds);

    if (tm.tm_hour < 10)
        result += '0';
    
    result += std::to_string(tm.tm_hour);
    result += ':';

    if (tm.tm_min < 10)
        result += '0';
    
    result += std::to_string(tm.tm_min);
    result += ':';

    if (tm.tm_sec < 10)
        result += '0';
    
    result += std::to_string(tm.tm_sec);
    result += '.';
    
    if (time.milliseconds < 100)
    {
        result += '0';

        if (time.milliseconds < 10)
            result += '0';
    }
    
    result += std::to_string(time.milliseconds);
    result += ' ';

    return result;
}

std::string to_string3(const timestamp& time)
{
    char result[sizeof("HH:MM:SS.mmm ")]; // implicit \0 in the string literal
    constexpr size_t pos_H1{0};
    constexpr size_t pos_H2{1};
    constexpr size_t pos_D1{2};
    constexpr size_t pos_M1{3};
    constexpr size_t pos_M2{4};
    constexpr size_t pos_D2{5};
    constexpr size_t pos_S1{6};
    constexpr size_t pos_S2{7};
    constexpr size_t pos_D3{8};
    constexpr size_t pos_m1{9};
    constexpr size_t pos_m2{10};
    constexpr size_t pos_m3{11};
    constexpr size_t pos_D4{12};

    const std::tm tm = jg::os::localtime_safe(time.seconds);

    if (tm.tm_hour < 10)
    {
        result[pos_H1] = '0';
        result[pos_H2] = '0' + (char)tm.tm_hour;
    }
    else
    {
        result[pos_H1] = '0' + (char)tm.tm_hour / 10;
        result[pos_H2] = '0' + (char)tm.tm_hour % 10;
    }
    
    result[pos_D1] = ':';

    if (tm.tm_min < 10)
    {
        result[pos_M1] = '0';
        result[pos_M2] = '0' + (char)tm.tm_min;
    }
    else
    {
        result[pos_M1] = '0' + (char)tm.tm_min / 10;
        result[pos_M2] = '0' + (char)tm.tm_min % 10;
    }
    
    result[pos_D2] = ':';

    if (tm.tm_sec < 10)
    {
        result[pos_S1] = '0';
        result[pos_S2] = '0' + static_cast<char>(tm.tm_sec);
    }
    else
    {
        result[pos_S1] = '0' + static_cast<char>(tm.tm_sec / 10);
        result[pos_S2] = '0' + static_cast<char>(tm.tm_sec % 10);
    }
    
    result[pos_D3] = '.';
    
    if (time.milliseconds < 10)
    {
        result[pos_m1] = '0';
        result[pos_m2] = '0';
        result[pos_m3] = '0' + static_cast<char>(time.milliseconds);
    }
    else if (time.milliseconds < 100)
    {
        result[pos_m1] = '0';
        result[pos_m2] = '0' + static_cast<char>(time.milliseconds / 10);
        result[pos_m3] = '0' + static_cast<char>(time.milliseconds % 10);
    }
    else // if (time.milliseconds < 1000)
    {
        result[pos_m1] = '0' + static_cast<char>(time.milliseconds / 100);
        result[pos_m2] = '0' + static_cast<char>((time.milliseconds % 100) / 10);
        result[pos_m3] = '0' + static_cast<char>((time.milliseconds % 100) % 10);
    }
    
    result[pos_D4] = ' ';

    return {result, sizeof(result) - 1};
}

std::string to_string4(const timestamp& time)
{
    char result[] = "00:00:00.000 ";
    constexpr size_t pos_HH{0};
    constexpr size_t pos_MM{3};
    constexpr size_t pos_SS{6};
    constexpr size_t pos_mmm{9};

    const std::tm tm = jg::os::localtime_safe(time.seconds);

    if (tm.tm_hour < 10)
        result[pos_HH + 1] = '0' + static_cast<char>(tm.tm_hour);
    else // if (tm.tm_hour < 24)
    {
        const auto d10 = std::div(tm.tm_hour, 10);
        result[pos_HH]     = '0' + static_cast<char>(d10.quot);
        result[pos_HH + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (tm.tm_min < 10)
        result[pos_MM + 1] = '0' + static_cast<char>(tm.tm_min);
    else // if (tm.tm_min < 100)
    {
        const auto d10 = std::div(tm.tm_min, 10);
        result[pos_MM]     = '0' + static_cast<char>(d10.quot);
        result[pos_MM + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (tm.tm_sec < 10)
        result[pos_SS + 1] = '0' + static_cast<char>(tm.tm_sec);
    else // if (tm.tm_sec < 100)
    {
        const auto d10 = std::div(tm.tm_sec, 10);
        result[pos_SS]     = '0' + static_cast<char>(d10.quot);
        result[pos_SS + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (time.milliseconds < 10)
        result[pos_mmm + 2] = '0' + static_cast<char>(time.milliseconds);
    else if (time.milliseconds < 100)
    {
        const auto d10 = std::div(time.milliseconds, 10);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }
    else // if (time.milliseconds < 1000)
    {
        const auto d100 = std::div(time.milliseconds, 100);
        const auto d10  = std::div(d100.rem, 10);
        result[pos_mmm]     = '0' + static_cast<char>(d100.quot);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }

    return {result, sizeof(result) - 1};
}

std::string to_string(const timestamp& time)
{
    std::string result{"00:00:00.000 "};
    constexpr size_t pos_HH{0};
    constexpr size_t pos_MM{3};
    constexpr size_t pos_SS{6};
    constexpr size_t pos_mmm{9};

    const std::tm tm = jg::os::localtime_safe(time.seconds);

    if (tm.tm_hour < 10)
        result[pos_HH + 1] = '0' + static_cast<char>(tm.tm_hour);
    else // if (tm.tm_hour < 24)
    {
        const auto d10 = std::div(tm.tm_hour, 10);
        result[pos_HH]     = '0' + static_cast<char>(d10.quot);
        result[pos_HH + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (tm.tm_min < 10)
        result[pos_MM + 1] = '0' + static_cast<char>(tm.tm_min);
    else // if (tm.tm_min < 100)
    {
        const auto d10 = std::div(tm.tm_min, 10);
        result[pos_MM]     = '0' + static_cast<char>(d10.quot);
        result[pos_MM + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (tm.tm_sec < 10)
        result[pos_SS + 1] = '0' + static_cast<char>(tm.tm_sec);
    else // if (tm.tm_sec < 100)
    {
        const auto d10 = std::div(tm.tm_sec, 10);
        result[pos_SS]     = '0' + static_cast<char>(d10.quot);
        result[pos_SS + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (time.milliseconds < 10)
        result[pos_mmm + 2] = '0' + static_cast<char>(time.milliseconds);
    else if (time.milliseconds < 100)
    {
        const auto d10 = std::div(time.milliseconds, 10);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }
    else // if (time.milliseconds < 1000)
    {
        const auto d100 = std::div(time.milliseconds, 100);
        const auto d10  = std::div(d100.rem, 10);
        result[pos_mmm]     = '0' + static_cast<char>(d100.quot);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }

    return result;
}
#endif

#if 0
std::string to_string(const timestamp& timestamp)
{
    std::string result{"00:00:00.000 "};
    //char result[] = "00:00:00.000 ";
    constexpr size_t pos_HH{0};
    constexpr size_t pos_MM{3};
    constexpr size_t pos_SS{6};
    constexpr size_t pos_mmm{9};

    const std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    const std::tm tm = jg::os::localtime_safe(tt);

    if (tm.tm_hour < 10)
        result[pos_HH + 1] = '0' + static_cast<char>(tm.tm_hour);
    else // if (tm.tm_hour < 24)
    {
        const auto d10 = std::div(tm.tm_hour, 10);
        result[pos_HH]     = '0' + static_cast<char>(d10.quot);
        result[pos_HH + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (tm.tm_min < 10)
        result[pos_MM + 1] = '0' + static_cast<char>(tm.tm_min);
    else // if (tm.tm_min < 100)
    {
        const auto d10 = std::div(tm.tm_min, 10);
        result[pos_MM]     = '0' + static_cast<char>(d10.quot);
        result[pos_MM + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    if (tm.tm_sec < 10)
        result[pos_SS + 1] = '0' + static_cast<char>(tm.tm_sec);
    else // if (tm.tm_sec < 100)
    {
        const auto d10 = std::div(tm.tm_sec, 10);
        result[pos_SS]     = '0' + static_cast<char>(d10.quot);
        result[pos_SS + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    const auto ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - std::chrono::system_clock::from_time_t(tt)).count());    

    if (ms < 10)
        result[pos_mmm + 2] = '0' + static_cast<char>(ms);
    else if (ms < 100)
    {
        const auto d10 = std::div(ms, 10);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }
    else // if (ms < 1000)
    {
        const auto d100 = std::div(ms, 100);
        const auto d10  = std::div(d100.rem, 10);
        result[pos_mmm]     = '0' + static_cast<char>(d100.quot);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }

    return result;
    //return {result, sizeof(result) - 1};
}
#endif

void write2(char* cs, int value)
{
    cs[0] = '0' + static_cast<char>(value / 10);
    cs[1] = '0' + static_cast<char>(value % 10);
}

void write3(char* cs, int value)
{
    cs[0] = '0' + static_cast<char>(value / 100);
    cs[1] = '0' + static_cast<char>((value % 100) / 10);
    cs[2] = '0' + static_cast<char>((value % 100) % 10);
}

std::string to_string(const timestamp& timestamp)
{
    const std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    const std::tm tm = jg::os::localtime_safe(tt);
    const auto ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - std::chrono::system_clock::from_time_t(tt)).count());

    std::stringstream result;
    result << std::put_time(&tm, "%T") << '.';
    result << std::setw(3) << std::setfill('0') << ms << ' ';

    return result.str();
}

#if 0
std::string to_string(const timestamp& timestamp)
{
    const std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    const std::tm tm = jg::os::localtime_safe(tt);
    const auto ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - std::chrono::system_clock::from_time_t(tt)).count());

    std::stringstream result;
    result.fill('0');
    result << std::setw(2) << tm.tm_hour << ':';
    result << std::setw(2) << tm.tm_min << ':';
    result << std::setw(2) << tm.tm_sec << '.';
    result << std::setw(3) << ms << ' ';

    return result.str();
}
#endif

#if 0
std::string to_string(const timestamp& timestamp)
{
    constexpr auto result_length{sizeof("00:00:00.000 ") - 1};
    char result[result_length + 1];
    const std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    const std::tm tm = jg::os::localtime_safe(tt);
    const auto ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - std::chrono::system_clock::from_time_t(tt)).count());
    write2(&result[0], tm.tm_hour);
    result[2] = ':';
    write2(&result[3], tm.tm_min);
    result[5] = ':';
    write2(&result[6], tm.tm_sec);
    result[8] = '.';
    write3(&result[9], ms);
    result[12] = ' ';

    return {result, result_length};
}
#endif

#if 0
void write2(char* cs, int value)
{
    const auto div10 = std::div(value, 10);
    cs[0] = '0' + static_cast<char>(div10.quot);
    cs[1] = '0' + static_cast<char>(div10.rem);
}

void write3(char* cs, int value)
{
    const auto div100 = std::div(value, 100);
    cs[0] = '0' + static_cast<char>(div100.quot);
    write2(cs + 1, div100.rem);
}

std::string to_string(const timestamp& timestamp)
{
    std::string result{"00:00:00.000 "};
    //char result[] = "00:00:00.000 ";
    const std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    const std::tm tm = jg::os::localtime_safe(tt);
    const auto ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - std::chrono::system_clock::from_time_t(tt)).count());
    write2(&result[0], tm.tm_hour);
    write2(&result[3], tm.tm_min);
    write2(&result[6], tm.tm_sec);
    write3(&result[9], ms);

    return result;
    //return {result, sizeof(result) - 1};
}
#endif

#if 0
std::string to_string(const timestamp& timestamp)
{
    std::string result{"00:00:00.000 "};
    //char result[] = "00:00:00.000 ";
    constexpr size_t pos_HH{0};
    constexpr size_t pos_MM{3};
    constexpr size_t pos_SS{6};
    constexpr size_t pos_mmm{9};

    const std::time_t tt = std::chrono::system_clock::to_time_t(timestamp);
    const std::tm tm = jg::os::localtime_safe(tt);

    {
        const auto d10 = std::div(tm.tm_hour, 10);
        result[pos_HH]     = '0' + static_cast<char>(d10.quot);
        result[pos_HH + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    {
        const auto d10 = std::div(tm.tm_min, 10);
        result[pos_MM]     = '0' + static_cast<char>(d10.quot);
        result[pos_MM + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    {
        const auto d10 = std::div(tm.tm_sec, 10);
        result[pos_SS]     = '0' + static_cast<char>(d10.quot);
        result[pos_SS + 1] = '0' + static_cast<char>(d10.rem);
    }
    
    const auto ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - std::chrono::system_clock::from_time_t(tt)).count());    

    {
        const auto d100 = std::div(ms, 100);
        const auto d10  = std::div(d100.rem, 10);
        result[pos_mmm]     = '0' + static_cast<char>(d100.quot);
        result[pos_mmm + 1] = '0' + static_cast<char>(d10.quot);
        result[pos_mmm + 2] = '0' + static_cast<char>(d10.rem);
    }

    return result;
    //return {result, sizeof(result) - 1};
}
#endif

bool log_enabled() noexcept
{
    return configuration.enabled;
}

bool log_enabled(log_level level) noexcept
{
    return configuration.enabled && level >= configuration.level;
}

void log_set_enabled(bool enabled) noexcept
{
    configuration.enabled = enabled;
}

void log_set_ostream(std::ostream& ostream) noexcept
{
    configuration.ostream = &ostream;
}

void log_set_level(log_level level) noexcept
{
    configuration.level = level;
}

std::ostream& log()
{
    return (*configuration.ostream << timestamp{std::chrono::system_clock::now()});
}

std::ostream& log_info()
{
    return log() << log_level::info;
}

std::ostream& log_warning()
{
    return log() << log_level::warning;
}

std::ostream& log_error()
{
    return log() << log_level::error;
}

std::ostream& log_fatal()
{
    return log() << log_level::fatal;
}

ostream_line log_line()
{
    return log();
}

ostream_line log_info_line()
{
    return log_info();
}

ostream_line log_warning_line()
{
    return log_warning();
}

ostream_line log_error_line()
{
    return log_error();
}

ostream_line log_fatal_line()
{
    return log_fatal();
}

} // namespace jg

#endif // ifdef JG_SIMPLE_LOGGER_IMPL
#endif // ifndef JG_SIMPLE_LOGGER_INCLUDED
