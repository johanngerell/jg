#ifdef JG_OS_IMPL
#undef JG_OS_INCLUDED
#endif

#ifndef JG_OS_INCLUDED
#define JG_OS_INCLUDED

#include <ctime>

namespace jg::os {

tm* localtime_safe(time_t time, tm& result);
tm localtime_safe(time_t time);

} // namespace jg::os

#ifdef JG_OS_IMPL
#undef JG_OS_IMPL

namespace jg::os {

tm* localtime_safe(time_t time, tm& result)
{
#ifdef _WIN32
    return localtime_s(&result, &time) == 0 ? &result : nullptr;
#else
    return localtime_r(&time, &result);
#endif
}

tm localtime_safe(time_t time)
{
    tm result;
#ifdef _WIN32
    return localtime_s(&result, &time) == 0 ? result : tm();
#else
    return localtime_r(&time, &result), result;
#endif
}

} // namespace jg::os

#endif // ifdef JG_OS_IMPL
#endif // ifndef JG_OS_INCLUDED
