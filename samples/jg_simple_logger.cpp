#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <jg_simple_logger.h>
#include <jg_stopwatch.h>
#include <jg_benchmark.h>

int main()
{
    std::cout << "jg_simple_logger sample...\n\n";

    const std::vector<const char*> logs
    {
        "log 1.a\n",
        "log 1.b\n",
        "log 1.c\n",
        "log 1.d\n",
        "log 1.e\n",
        "log 1.f\n",
        "log 1.g\n",
        "log 1.h\n",
        "log 1.i\n",
        "log 1.j\n"
    };

    {
        std::cout << "10 back-to-back logs...\n\n";
        const auto t1 = jg::benchmark(10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                jg::log() << logs[i];
        });
        std::cout << "\n... " << t1.count() / 10 << " ns\n";
    }
    
    std::cout << "\n10 'cooked' logs, v1...\n\n";
    std::vector<jg::timestamp> timestamps;
    std::generate_n(std::back_inserter(timestamps), 10, []
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return jg::timestamp::now();
    });

    {
        const auto t2 = jg::benchmark(10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                std::clog << timestamps[i] << logs[i];
        });
        std::cout << "\n... " << t2.count() / 10 << " ns\n";
    }

    {
        std::cout << "\n10 'cooked' logs, v2...\n\n";
        const auto t3 = jg::benchmark(10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                std::clog << to_string(timestamps[i]) << logs[i];
        });
        std::cout << "\n... " << t3.count() / 10 << " ns\n";
    }

    {
        std::cout << "\n10 timestamps...\n\n";
        const auto t4 = jg::benchmark(10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                timestamps[i] = jg::timestamp::now();
        });
        std::cout << "... " << t4.count() / 10 << " ns\n";
        std::cout << "... last: " << timestamps.back() << "\n... first: " << timestamps.front() << "\n";
    }

    {
        std::cout << "\n10 to_string(timestamp)...\n\n";
        std::vector<std::string> strings(10);
        const auto t5 = jg::benchmark(10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                strings[i] = to_string(timestamps[i]);
        });
        std::cout << "... " << t5.count() / 10 << " ns\n";
        std::cout << "... last: " << strings.back() << "\n... first: " << strings.front() << "\n";
    }

    {
        std::cout << "\n10 operator<<(ostream, timestamp)...\n\n";
        const auto t6 = jg::benchmark(10, [&]
        {
            for (size_t i = 0; i < 10; ++i)
                std::clog << timestamps[i] << "\n";
        });
        std::cout << "... " << t6.count() / 10 << " ns\n";
    }

    std::cout << "\n...done\n";
}