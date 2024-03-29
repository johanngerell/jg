#include <iomanip>
#include <sstream>
#include <jg_mock.h>
#include <jg_test.h>

#define JG_SIMPLE_LOGGER_IMPL
#include <jg_simple_logger.h>

#define JG_OS_IMPL
#include <jg_os.h>

namespace {

// Makes a "correct" timestamp for testing purposes.
jg::timestamp make_timestamp(size_t hours, size_t minutes, size_t seconds, size_t milliseconds)
{
    if (hours > 23)         throw std::range_error("hours > 23");
    if (minutes > 59)       throw std::range_error("minutes > 59");
    if (seconds > 59)       throw std::range_error("seconds > 59");
    if (milliseconds > 999) throw std::range_error("milliseconds > 999");

#if 0
    std::stringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << hours << ':'
       << std::setw(2) << minutes << ':'
       << std::setw(2) << seconds;

    std::tm tm{};
    ss >> std::get_time(jg::os::localtime_safe(time(nullptr), tm), "%H:%M:%S");

    return {mktime(&tm), static_cast<decltype(jg::timestamp::milliseconds)>(milliseconds)};
#endif

    std::time_t tt = time(nullptr);
    std::tm tm = jg::os::localtime_safe(tt);
    tm.tm_sec = (int)seconds;
    tm.tm_min = (int)minutes;
    tm.tm_hour = (int)hours;
    //tm.tm_isdst = -1;
    tt = mktime(&tm);
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(tt);
    tp += std::chrono::milliseconds(milliseconds);
    return {tp};
}

jg::test_adder simple_logger_tests { "simple_logger", {
    jg::test_suite { "to_string", {
        jg::test_case { "make_timestamp should fail on too big values", [] {
            jg_test_assert_exception(make_timestamp(24,  1,  1,    1), std::range_error);
            jg_test_assert_exception(make_timestamp( 1, 60,  1,    1), std::range_error);
            jg_test_assert_exception(make_timestamp( 1,  1, 60,    1), std::range_error);
            jg_test_assert_exception(make_timestamp( 1,  1,  1, 1000), std::range_error);
        }},
        jg::test_case { "to_string", [] {
            jg_test_assert(jg::to_string(make_timestamp(0, 0, 0, 0)) == "00:00:00.000 ");
            jg_test_assert(jg::to_string(make_timestamp(1, 1, 1, 1)) == "01:01:01.001 ");
            jg_test_assert(jg::to_string(make_timestamp(16, 35, 12, 123)) == "16:35:12.123 ");
            jg_test_assert(jg::to_string(make_timestamp(23, 59, 59, 999)) == "23:59:59.999 ");
        }}
    }}
}};

}
