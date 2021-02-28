#pragma once

#include "jg_simple_logger.h"

namespace jg {

/// Utility allocator for examination of the allocation behavior of standard library types, containers, etc.
/// Originates from cppreference.com.
template <typename T>
struct logging_allocator
{
    using value_type = T;

    logging_allocator() = default;

    template <typename U>
    constexpr logging_allocator(const logging_allocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n)
    {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_alloc();

        auto p = static_cast<T*>(std::malloc(n * sizeof(T)));

        if (!p)
            throw std::bad_alloc();

        log(p, n, true);
        return p;
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        log(p, n, false);
        std::free(p);
    }

private:
    void log(T* p, std::size_t n, bool alloc) const
    {
        jg_log_info_line() << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n
                           << " bytes at " << std::hex << std::showbase
                           << reinterpret_cast<void*>(p) << std::dec;
    }
};
 
template <typename T, typename U>
bool operator==(const logging_allocator <T>&, const logging_allocator <U>&) { return true; }

template <typename T, typename U>
bool operator!=(const logging_allocator <T>&, const logging_allocator <U>&) { return false; }

} // namespace jg
