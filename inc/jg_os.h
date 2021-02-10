#ifdef JG_OS_IMPL
#undef JG_OS_INCLUDED
#endif

#ifndef JG_OS_INCLUDED
#define JG_OS_INCLUDED

#include <ctime>

namespace jg {
namespace os {

/// Formats a time string in the format "Www Mmm dd hh:mm:ss yyyy\n", with individual fixed-length fields.
/// This string has length 25, which means that `size` must be at least size 26 to fit the null-terminator.
bool ctime_safe(char* buffer, size_t size, time_t time);

tm* localtime_safe(time_t time, tm& result);

} // namespace os
} // namespace jg

#ifdef JG_OS_IMPL
#undef JG_OS_IMPL

namespace jg {
namespace os {

bool ctime_safe(char* buffer, size_t size, time_t time)
{
    // ctime_s and ctime_r format a string exactly in the format "Www Mmm dd hh:mm:ss yyyy\n", with
    // individual fixed-length fields. This string has length 25, which means that the target buffer
    // must be at least size 26 to fit the null-terminator.
    if (size < sizeof("Www Mmm dd hh:mm:ss yyyy\n"))
        return false;

#ifdef _WIN32
    return ctime_s(buffer, size, &time) == 0;
#else
    return ctime_r(&time, buffer) != nullptr;
#endif
}

tm* localtime_safe(time_t time, tm& result)
{
#ifdef _WIN32
    return localtime_s(&result, &time) == 0 ? &result : nullptr;
#else
    return localtime_r(&time, &result);
#endif
}

} // namespace os
} // namespace jg

#endif // ifdef JG_OS_IMPL
#endif // ifndef JG_OS_INCLUDED