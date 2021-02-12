#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iterator>
#include <thread>
#include <vector>
#define JG_OS_IMPL
#include <jg_os.h>
#define JG_SIMPLE_LOGGER_IMPL
#include <jg_simple_logger.h>
#include <jg_stopwatch.h>
#include <jg_benchmark.h>
#include <jg_string.h>

template <typename FwdIt>
struct joiner final
{
    FwdIt first{};
    FwdIt last{};
    const char* separator{", "};

    friend std::ostream& operator<<(std::ostream& stream, const joiner& self)
    {
        bool separate = false;
        for(auto it = self.first; it != self.last; ++it)
            stream << (separate ? self.separator : (separate = true, "")) << *it;
        
        return stream;
    }
};

template <typename FwdIt>
joiner<FwdIt> join(FwdIt first, FwdIt last, const char* separator = ", ")
{
    return {first, last, separator};
}

int main()
{
    std::cout << "jg_simple_logger sample...\n\n";

    const std::vector<const char*> logs_with_newline
    {
        "abcdefghij\n", "bcdefghija\n", "cdefghijab\n", "defghijabc\n", "efghijabcd\n",
        "fghijabcde\n", "ghijabcdef\n", "hijabcdefg\n", "ijabcdefgh\n", "jabcdefghi\n"
    };

    const std::vector<const char*> logs_without_newline
    {
        "abcdefghij", "bcdefghija", "cdefghijab", "defghijabc", "efghijabcd",
        "fghijabcde", "ghijabcdef", "hijabcdefg", "ijabcdefgh", "jabcdefghi"
    };

    std::vector<jg::benchmark_result> benchmarks;

    {
        auto r = jg::benchmark("jg::log_info with nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg::log_info() << logs_with_newline[i];
        });
        benchmarks.push_back(std::move(r));
    }

    {
        auto r = jg::benchmark("jg::log_info_line no nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg::log_info_line() << logs_without_newline[i];
        });
        benchmarks.push_back(std::move(r));
    }

    {
        auto r = jg::benchmark("jg_log_info with nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg_log_info() << logs_with_newline[i];
        });
        benchmarks.push_back(std::move(r));
    }

    {
        auto r = jg::benchmark("jg_log_info_line no nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg_log_info_line() << logs_without_newline[i];
        });
        benchmarks.push_back(std::move(r));
    }

    std::vector<jg::timestamp> timestamps(100);

    {
        auto r = jg::benchmark("jg::timestamp::now", 10, 100, [&]
        {
            for (size_t i = 0; i < 100; ++i)
                timestamps[i] = jg::timestamp::now();
        });
        benchmarks.push_back(std::move(r));
    }

    {
        auto r = jg::benchmark("jg::timestamp_format", 10, 100, [&]
        {
            jg::timestamp_buffer buffer;
            for (size_t i = 0; i < 100; ++i)
                jg::timestamp_format(timestamps[i], buffer);
        });
        benchmarks.push_back(std::move(r));
    }

    {
        std::vector<std::string> strings(100);
        auto r = jg::benchmark("jg::to_string(timestamp)", 10, 100, [&]
        {
            for (size_t i = 0; i < 100; ++i)
                strings[i] = to_string(timestamps[i]);
        });
        benchmarks.push_back(std::move(r));
    }

    const auto column1_width = std::max_element(
        benchmarks.begin(),
        benchmarks.end(),
        [](const auto& first, const auto& second) {
            return first.description.length() < second.description.length();
        })->description.length() + 3;

    const std::string_view header1{"average (ns)"};
    const std::string_view header2{"median (ns)"};
    const std::string_view header3{"stddev (ns)"};
    const std::string_view header4{"samples (ns)"};
    const std::string_view header_{"------------"};
    const auto columnN_width{header_.length() + 2};

    std::cout << std::setw(column1_width + columnN_width) << header1 << std::setw(columnN_width) << header2 << std::setw(columnN_width) << header3 << std::setw(columnN_width) << header4 << '\n'
              << std::setw(column1_width + columnN_width) << header_ << std::setw(columnN_width) << header_ << std::setw(columnN_width) << header_ << std::setw(columnN_width) << header_ << '\n';

    for (const auto& b : benchmarks)
    {
        std::cout << std::setw(column1_width) << std::left << b.description << std::right
                  << std::setw(columnN_width) << b.average
                  << std::setw(columnN_width) << b.median
                  << std::setw(columnN_width) << b.std_deviation
                  << "  [" << join(b.samples.begin(), b.samples.end()) << "]\n";
    }

    std::cout << "\n...done\n";
}
