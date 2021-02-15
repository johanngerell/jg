#ifdef JG_SIMPLE_LOGGER_IMPL
#undef JG_SIMPLE_LOGGER_INCLUDED
#endif

#ifndef JG_SIMPLE_LOGGER_INCLUDED
#define JG_SIMPLE_LOGGER_INCLUDED

#include <iostream>
#include <sstream>
#include <string>
#include <sys/timeb.h>
#include <cstdio>
#include <cstdint>

namespace jg {

/// Holds the seconds and milliseconds parts of a timestamp returned from `ftime()`.
struct timestamp final
{
    decltype(timeb::time) seconds{};
    decltype(timeb::millitm) milliseconds{};

    static timestamp now() noexcept
    {
        timeb timebuffer;
        ftime(&timebuffer);
        return {timebuffer.time, timebuffer.millitm};
    }
};

/// A utility buffer type of the minimum size needed to format a `timestamp`.
using timestamp_buffer = char[26];

/// Formats a `timestamp` into a 24-hour "hh:mm:ss.mmm " string in a supplied buffer, without allocations.
/// @return Pointer to the formatted timestamp in `buffer`, or `error_return` if formatting failed.
const char* timestamp_format(const timestamp& time, timestamp_buffer& buffer, const char* error_return = nullptr);

/// Formats a `timestamp` into a 24-hour "hh:mm:ss.mmm " string in a supplied buffer, without allocations.
/// @note Fails if `size` is smaller than `sizeof(timestamp_buffer)`.
/// @return Pointer to the formatted timestamp in `buffer`, or `error_return` if formatting failed.
const char* timestamp_format(const timestamp& time, char* buffer, size_t size, const char* error_return = nullptr);

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
    }
}

inline std::ostream& operator<<(std::ostream& stream, log_level severity)
{
    return stream << to_string(severity);
}

inline std::string to_string(const timestamp& timestamp)
{
    timestamp_buffer buffer;
    return timestamp_format(timestamp, buffer);
}

inline std::ostream& operator<<(std::ostream& stream, const timestamp& timestamp)
{
    timestamp_buffer buffer;
    return stream << timestamp_format(timestamp, buffer);
}

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

const char* timestamp_format(const timestamp& time, timestamp_buffer& buffer, const char* error_return)
{
    return timestamp_format(time, buffer, sizeof(buffer), error_return);
}

const char* timestamp_format(const timestamp& time, char* buffer, size_t size, const char* error_return)
{
    // jg::os::ctime_safe formats a string exactly in the format "Www Mmm dd hh:mm:ss yyyy\n", with
    // individual fixed-length fields. This string has length 25, which means that the target buffer
    // must be at least size 26 to fit the null-terminator. We repurpose the " yyyy" part for the
    // equally long ".mmm " milliseconds part, and replace the \n with a \0 to end the string there.
    if (size < sizeof("Www Mmm dd hh:mm:ss yyyy\n"))
        return error_return;

    auto format_ms = [&]
    {
        constexpr size_t ms_offset = sizeof("Www Mmm dd hh:mm:ss") - 1;
        const     size_t ms_size   = size - ms_offset;
        constexpr size_t ms_length = sizeof(".mmm ") - 1;
        return snprintf(buffer + ms_offset, ms_size, ".%03hu ", time.milliseconds) == ms_length;
    };

    constexpr size_t time_offset = sizeof("Www Mmm dd ") - 1;

    return jg::os::ctime_safe(buffer, size, time.seconds) && format_ms() ?
           buffer + time_offset :
           error_return;
}

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
    return (*configuration.ostream << timestamp::now());
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
