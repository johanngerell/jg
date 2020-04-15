#include <iostream>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstdint>
#include <sys/timeb.h>

namespace jg
{

class timestamp final
{
public:
    timestamp() = default;

    static timestamp now()
    {
        __timeb64 timebuffer;
        return _ftime64_s(&timebuffer) == 0 ?
               timestamp{timebuffer.time, timebuffer.millitm} :
               timestamp{};
    }

    using format_buffer = char[26];

    /// Formats a 24-hour "hh:mm:ss.mmm " timestamp string a the supplied buffer, without allocations.
    /// @return Pointer to the formatted timestamp in `buffer`, or `format_error` if formatting failed.
    const char* format(format_buffer& buffer, const char* format_error = "") const
    {
        // _ctime64_s formats a string exactly in the format "Www Mmm dd hh:mm:ss yyyy\n",
        // with individual fixed-length fields. This string has length 25, which means that the target
        // buffer must be at least size 26 to fit the null-terminator. We repurpose the " yyyy" part for
        // the equally long ".mmm " milliseconds part, and replace the \n with a \0.
        static_assert(sizeof(buffer) == sizeof("Www Mmm dd hh:mm:ss yyyy\n"), "");
        constexpr size_t ms_offset = sizeof("Www Mmm dd hh:mm:ss") - 1;
        constexpr size_t ms_size = sizeof(buffer) - ms_offset;
        constexpr size_t ms_length = sizeof(".mmm ") - 1;
        static_assert(ms_size > ms_length, "");

        char* ms_start = buffer + ms_offset;
        const char* hour_start = buffer + sizeof("Www Mmm dd ") - 1;

        return _ctime64_s(buffer, sizeof(buffer), &m_time) == 0 && 
               snprintf(ms_start,  ms_size, ".%03hu ", m_ms) == ms_length ?
               hour_start : format_error;
    }

private:
    timestamp(int64_t time, uint16_t ms)
        : m_time{time}
        , m_ms{ms}
    {}

    int64_t  m_time{};
    uint16_t m_ms{};
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
