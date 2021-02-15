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

std::vector<jg::benchmark_result> benchmark()
{
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

    std::vector<jg::timestamp> timestamps(100);
    std::vector<std::string> strings(100);

    return
    {
        jg::benchmark("jg::log_info with nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg::log_info() << logs_with_newline[i];
        }),
        jg::benchmark("jg::log_info_line no nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg::log_info_line() << logs_without_newline[i];
        }),
        jg::benchmark("jg_log_info with nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg_log_info() << logs_with_newline[i];
        }),
        jg::benchmark("jg_log_info_line no nl", 10, 10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg_log_info_line() << logs_without_newline[i];
        }),
        jg::benchmark("jg::timestamp", 10, 100, [&]
        {
            for (size_t i = 0; i < 100; ++i)
                timestamps[i] = jg::timestamp();
        }),
        jg::benchmark("jg::timestamp::now", 10, 100, [&]
        {
            for (size_t i = 0; i < 100; ++i)
                timestamps[i] = jg::timestamp::now();
        }),
        jg::benchmark("jg::timestamp_format", 10, 100, [&]
        {
            jg::timestamp_buffer buffer;
            for (size_t i = 0; i < 100; ++i)
                jg::timestamp_format(timestamps[i], buffer);
        }),
        jg::benchmark("jg::to_string(timestamp)", 10, 100, [&]
        {
            for (size_t i = 0; i < 100; ++i)
                strings[i] = to_string(timestamps[i]);
        })
    };   
}

void output_result(const std::vector<jg::benchmark_result>& benchmarks)
{
    using namespace std::string_literals;

    const std::vector<std::string> column_labels
    {
        "average (ns)"s,
        "median (ns)"s,
        "std (ns)"s,
        "mad (ns)"s,
        "samples (ns)"s
    };

    const size_t column1_width = std::max_element(
        benchmarks.begin(),
        benchmarks.end(),
        [](const auto& b1, const auto& b2)
        { return b1.description.length() < b2.description.length(); })->description.length() + 3;

    const size_t columnN_width = std::max_element(
        column_labels.begin(),
        column_labels.end(),
        [](const auto& l1, const auto& l2)
        { return l1.length() < l2.length(); })->length() + 2;

    std::cout << '\n';

    auto width = column1_width + columnN_width;
    for (auto& label : column_labels)
    {
        std::cout << std::setw(width) << label;
        width = columnN_width;
    }

    std::cout << '\n';

    const std::string header_(columnN_width - 2, '-');
    width = column1_width + columnN_width;
    for (size_t i = 0; i < column_labels.size(); ++i)
    {
        std::cout << std::setw(width) << header_;
        width = columnN_width;
    }

    std::cout << '\n';

    for (const auto& b : benchmarks)
    {
        std::cout << std::setw(column1_width) << std::left << b.description << std::right
                  << std::setw(columnN_width) << b.average
                  << std::setw(columnN_width) << b.median
                  << std::setw(columnN_width) << b.std_deviation
                  << std::setw(columnN_width) << b.median_abs_deviation
                  << "  [" << join(b.samples.begin(), b.samples.end()) << "]\n";
    }
}

int main()
{
    std::cout << "jg_simple_logger sample...\n\n";

    output_result(benchmark());

    std::cout << "\n...done\n";
}
