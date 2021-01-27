#include <iostream>
#include <string>
#include <sys/timeb.h>
#include <ctime>
#include <cstdio>
#include <cstdint>

namespace jg
{

class timestamp final
{
public:
    timestamp() = default;

    static timestamp now()
    {
        timeb timebuffer;
        ftime(&timebuffer);
        return {timebuffer.time, timebuffer.millitm};
    }

    using format_buffer = char[26];

    /// Formats a 24-hour "hh:mm:ss.mmm " timestamp string in a supplied buffer, without allocations.
    /// @return Pointer to the formatted timestamp in `buffer`, or `error_return` if formatting failed.
    const char* format(format_buffer& buffer, const char* error_return = nullptr) const
    {
        // ctime_s and ctime_r format a string exactly in the format "Www Mmm dd hh:mm:ss yyyy\n", with
        // individual fixed-length fields. This string has length 25, which means that the target buffer
        // must be at least size 26 to fit the null-terminator. We repurpose the " yyyy" part for the
        // equally long ".mmm " milliseconds part, and replace the \n with a \0 to end the string there.
        static_assert(sizeof(buffer) == sizeof("Www Mmm dd hh:mm:ss yyyy\n"), "");

        auto format_time = [&] () -> bool
        {
#ifdef _WIN32
            return ctime_s(buffer, sizeof(buffer), &m_s) == 0;
#else
            return ctime_r(&m_s, buffer) != nullptr;
#endif
        };

        auto format_ms = [&] () -> bool
        {
            constexpr size_t ms_offset = sizeof("Www Mmm dd hh:mm:ss") - 1;
            constexpr size_t ms_size = sizeof(buffer) - ms_offset;
            constexpr size_t ms_length = sizeof(".mmm ") - 1;
            return snprintf(buffer + ms_offset, ms_size, ".%03hu ", m_ms) == ms_length;
        };

        constexpr size_t time_offset = sizeof("Www Mmm dd ") - 1;
        return format_time() && format_ms() ? buffer + time_offset : error_return;
    }

private:
    timestamp(time_t s, unsigned short ms)
        : m_s{s}
        , m_ms{ms}
    {}

    time_t  m_s{};
    unsigned short m_ms{};
};

inline std::string to_string(const timestamp& timestamp)
{
    timestamp::format_buffer buffer;
    return timestamp.format(buffer);
}

inline std::ostream& operator<<(std::ostream& stream, const timestamp& timestamp)
{
    timestamp::format_buffer buffer;
    return stream << timestamp.format(buffer);
}

inline std::ostream& log()
{
    return (std::clog << timestamp::now());
}

} // namespace jg
