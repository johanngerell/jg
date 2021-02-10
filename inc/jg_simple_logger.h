#include <iostream>
#include <sstream>
#include <string>
#include <sys/timeb.h>
#include <cstdio>
#include <cstdint>
#include "jg_os.h"

namespace jg {

/// Holds the second and millisecond parts of a timestamp returned from `ftime()`.
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

using timestamp_buffer = char[26];

/// Formats a 24-hour "hh:mm:ss.mmm " timestamp string in a supplied buffer, without allocations.
/// @return Pointer to the formatted timestamp in `buffer`, or `error_return` if formatting failed.
const char* timestamp_format(const timestamp& time, timestamp_buffer& buffer, const char* error_return = nullptr);

/// Formats a 24-hour "hh:mm:ss.mmm " timestamp string in a supplied buffer, without allocations.
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

void log_set_enabled(bool enabled) noexcept;
void log_set_ostream(std::ostream& ostream) noexcept;
void log_set_level(log_level level) noexcept;

std::ostream& log();
std::ostream& log_info();
std::ostream& log_warning();
std::ostream& log_error();
std::ostream& log_fatal();

} // namespace jg

namespace jg {
namespace detail {

bool log_allowed(log_level level) noexcept;

} // namespace detail
} // namespace jg

#define jg_log_info()    if (jg::detail::log_allowed(jg::log_level::info))    jg::log_info()
#define jg_log_warning() if (jg::detail::log_allowed(jg::log_level::warning)) jg::log_warning()
#define jg_log_error()   if (jg::detail::log_allowed(jg::log_level::error))   jg::log_error()
#define jg_log_fatal()   if (jg::detail::log_allowed(jg::log_level::fatal))   jg::log_fatal()

#ifdef JG_SIMPLE_LOGGER_IMPL
#undef JG_SIMPLE_LOGGER_IMPL

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

    auto format_ms = [&] () -> bool
    {
        constexpr size_t ms_offset = sizeof("Www Mmm dd hh:mm:ss") - 1;
        const size_t ms_size = size - ms_offset;
        constexpr size_t ms_length = sizeof(".mmm ") - 1;
        return snprintf(buffer + ms_offset, ms_size, ".%03hu ", time.milliseconds) == ms_length;
    };

    constexpr size_t time_offset = sizeof("Www Mmm dd ") - 1;

    return jg::os::ctime_safe(buffer, size, time.seconds) && format_ms() ? buffer + time_offset : error_return;
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

} // namespace jg

namespace jg {
namespace detail {

bool log_allowed(log_level level) noexcept
{
    return configuration.enabled && level >= configuration.level;
}

} // namespace detail
} // namespace jg

#endif