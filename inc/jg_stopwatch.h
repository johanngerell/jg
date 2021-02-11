#pragma once

#include <chrono>

namespace jg {

/// The simplest possible .Net StopWatch rip-off. Continues to measure time from instantiation, regardless
/// of how many times `ms()`, `us()` or `ns()` are called. Only restarts when assigned a new instance.
///
/// @example
///     stopwatch sw; // starts timing
///     op1();
///     std::cout << "op1() took " << sw.ns() << " ns\n";
///     sw = {}; // "restarts" timing
///     op2();
///     std::cout << "op2() took " << sw.ms() << " ms\n";
class stopwatch final
{
public:
    std::chrono::milliseconds::rep ms() const { return rep<std::chrono::milliseconds>(); }
    std::chrono::microseconds::rep us() const { return rep<std::chrono::microseconds>(); }
    std::chrono::nanoseconds::rep  ns() const { return rep<std::chrono::nanoseconds>(); }

private:
    using clock = std::chrono::high_resolution_clock;

    template <typename T>
    typename T::rep rep() const { return std::chrono::duration_cast<T>(clock::now() - m_start).count(); }
    
    clock::time_point m_start{clock::now()};
};

} // namespace jg
