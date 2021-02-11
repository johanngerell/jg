#include <iomanip>
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

    std::stringstream ss;
    ss << hours << ':' << minutes << ':' << seconds;
    std::tm tm{};
    ss >> std::get_time(jg::os::localtime_safe(time(nullptr), tm), "%T");

    return {mktime(&tm), static_cast<decltype(jg::timestamp::milliseconds)>(milliseconds)};
}

struct simple_logger_tests final : jg::test_suites_base<simple_logger_tests>
{
    auto operator()()
    {
        using namespace std::string_literals;

        return jg::test_suites { "simple_logger", {
            jg::test_suite { "timestamp_format", {
                jg::test_case { "make_timestamp should fail on too big values", [] {
                    jg_test_assert_exception(make_timestamp(24,  1,  1,    1), std::range_error);
                    jg_test_assert_exception(make_timestamp( 1, 60,  1,    1), std::range_error);
                    jg_test_assert_exception(make_timestamp( 1,  1, 60,    1), std::range_error);
                    jg_test_assert_exception(make_timestamp( 1,  1,  1, 1000), std::range_error);
                }},
                // positive
                jg::test_case { "timestamp_buffer", [] {
                    jg::timestamp_buffer buffer;
                    jg_test_assert(jg::timestamp_format(make_timestamp(0, 0, 0, 0), buffer) == "00:00:00.000 "s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(1, 1, 1, 1), buffer) == "01:01:01.001 "s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(16, 35, 12, 123), buffer) == "16:35:12.123 "s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(23, 59, 59, 999), buffer) == "23:59:59.999 "s);
                }},
                jg::test_case { "big enough buffer", [] {
                    char buffer[sizeof(jg::timestamp_buffer)];
                    jg_test_assert(jg::timestamp_format(make_timestamp(0, 0, 0, 0), buffer, sizeof(buffer)) == "00:00:00.000 "s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(1, 1, 1, 1), buffer, sizeof(buffer)) == "01:01:01.001 "s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(16, 35, 12, 123), buffer, sizeof(buffer)) == "16:35:12.123 "s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(23, 59, 59, 999), buffer, sizeof(buffer)) == "23:59:59.999 "s);
                }},
                // negative
                jg::test_case { "too small buffer - without error_return", [] {
                    char buffer[sizeof(jg::timestamp_buffer) - 1];
                    jg_test_assert(jg::timestamp_format(make_timestamp(0, 0, 0, 0), buffer, sizeof(buffer)) == nullptr);
                    jg_test_assert(jg::timestamp_format(make_timestamp(1, 1, 1, 1), buffer, sizeof(buffer)) == nullptr);
                    jg_test_assert(jg::timestamp_format(make_timestamp(16, 35, 12, 123), buffer, sizeof(buffer)) == nullptr);
                    jg_test_assert(jg::timestamp_format(make_timestamp(23, 59, 59, 999), buffer, sizeof(buffer)) == nullptr);
                }},
                jg::test_case { "too small buffer - with error_return", [] {
                    char buffer[sizeof(jg::timestamp_buffer) - 1];
                    jg_test_assert(jg::timestamp_format(make_timestamp(0, 0, 0, 0), buffer, sizeof(buffer), "error text") == "error text"s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(1, 1, 1, 1), buffer, sizeof(buffer), "error text") == "error text"s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(16, 35, 12, 123), buffer, sizeof(buffer), "error text") == "error text"s);
                    jg_test_assert(jg::timestamp_format(make_timestamp(23, 59, 59, 999), buffer, sizeof(buffer), "error text") == "error text"s);
                }}
            }}
        }};
    }
} _;

}
