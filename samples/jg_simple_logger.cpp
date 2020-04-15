#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <jg_simple_logger.h>

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

    std::cout << "10 back-to-back logs\n\n";
    for (const auto& log : logs)
        jg::log() << log;

    std::cout << "\n10 'cooked' logs, v1\n\n";
    std::vector<jg::timestamp> timestamps;
    std::generate_n(std::back_inserter(timestamps), 10, []
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return jg::timestamp::now();
    });

    for (size_t i = 0; i < 10; ++i)
        std::clog << timestamps[i] << logs[i];

    std::cout << "\n10 'cooked' logs, v2\n\n";
    for (size_t i = 0; i < 10; ++i)
        std::clog << to_string(timestamps[i]) << logs[i];

    std::cout << "\n...done";
}