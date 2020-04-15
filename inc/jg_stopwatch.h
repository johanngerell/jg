#pragma once

#include <chrono>

namespace jg
{

class stopwatch final
{
public:
    std::chrono::microseconds::rep us() const
    {
        return duration_rep<std::chrono::microseconds>();
    }

    std::chrono::milliseconds::rep ms() const
    {
        return duration_rep<std::chrono::milliseconds>();
    }

    void restart()
    {
        m_start = clock::now();
    }

private:
    using clock = std::chrono::high_resolution_clock;

    template <typename T>
    typename T::rep duration_rep() const
    {
        return std::chrono::duration_cast<T>(clock::now() - m_start).count();
    }
    
    clock::time_point m_start = clock::now();
};

} // namespace jg