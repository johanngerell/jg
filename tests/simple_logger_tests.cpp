#include <iomanip>
#include <jg_mock.h>
#include <jg_test.h>

#define JG_SIMPLE_LOGGER_IMPL
#include <jg_simple_logger.h>

#define JG_OS_IMPL
#include <jg_os.h>

namespace
{

struct simple_logger_tests final : jg::test_suites_base<simple_logger_tests>
{
    auto operator()()
    {
        return jg::test_suites { "simple_logger", {
            jg::test_suite { "timestamp_format", {
                jg::test_case { "", [] {
                    // "Www Mmm dd hh:mm:ss yyyy\n"
                    std::istringstream ss("16:35:12");
                    std::tm tm;
                    jg::os::localtime_safe(time(nullptr), tm);
                    ss >> std::get_time(&tm, "%H:%M:%S");
                    std::time_t time = mktime(&tm);
                    jg::timestamp t{time, 123};
                    std::cout << "---->" << t << "<----\n";
                }}
            }}
        }};
    }
} _;

}
