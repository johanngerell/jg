#pragma once

#include <type_traits>
#include "jg_verify.h"

namespace jg
{

template <typename TImpl, typename TFlag = unsigned char>
class flags_base
{
public:
    using flag_t = TFlag;

    static_assert(std::is_unsigned<flag_t>::value, "");

    flags_base() = default;

    constexpr flags_base(flag_t flags) noexcept
        : m_flags(flags)
    {
        verify_without_zero_check(flags);
    }

    constexpr bool has(flag_t flags) const noexcept
    {
        verify(flags);
        return (m_flags & flags) == flags;
    }

    constexpr bool is(flag_t flags) const noexcept
    {
        verify(flags);
        return m_flags == flags;
    }

    constexpr flags_base& add(flag_t flags) noexcept
    {
        verify(flags);
        m_flags |= flags;
        return *this;
    }

    constexpr flags_base& remove(flag_t flags) noexcept
    {
        verify(flags);
        m_flags &= ~flags;
        return *this;
    }

    constexpr flag_t value() const noexcept
    {
        return m_flags;
    }

private:
    constexpr static void verify_without_zero_check(flag_t flags) noexcept
    {
        jg::verify((TImpl::all & flags) == flags);
    }

    constexpr static void verify(flag_t flags) noexcept
    {
        jg::verify(flags);
        verify_without_zero_check(flags);
    }

    flag_t m_flags{};
};

} // namespace jg