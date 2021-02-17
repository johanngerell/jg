#pragma once

#include "jg_verify.h"

namespace jg {

// Trivial drop-in pre-C++20 replacement for std::span.
template <typename T>
class span final
{
public:
    constexpr span() = default;

    using iterator = T*;
    using const_iterator = const T*;

    constexpr span(iterator first, iterator last) : m_first{first} , m_last{last} {}
    constexpr span(T* data, size_t size) : span{verified(data), data + size} {}
    template <size_t N> constexpr span(T (&array)[N]) : span{array, N} {}

    template <size_t N> constexpr static span from_array(T (&array)[N]) { return {array, N}; }

    constexpr T* data() { return m_first; }
    constexpr const T* data() const { return m_first; }
    constexpr size_t size() const { return static_cast<size_t>(m_last - m_first); }

    constexpr explicit operator bool() const { return m_last != m_first; }

    constexpr iterator begin() { return m_first; }
    constexpr iterator end() { return m_last; }

    constexpr const_iterator begin() const { return m_first; }
    constexpr const_iterator end() const { return m_last; }

    constexpr const_iterator cbegin() const { return m_first; }
    constexpr const_iterator cend() const { return m_last; }

private:
    iterator m_first{};
    iterator m_last{};
};

template <typename T, size_t N>
constexpr inline span<T> make_span(T (&array)[N])
{
    return span<T>::from_array(array);
}

} // namespace jg
