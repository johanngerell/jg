#pragma once

#include "jg_verify.h"

namespace jg
{

template <typename T>
class span final
{
public:
    constexpr span() = default;

    constexpr span(T* data, size_t size)
        : m_data{verified(data)}
        , m_size{size}
    {}

    template <size_t N>
    constexpr span(T (&array)[N])
        : span{array, N}
    {}

    template <size_t N>
    constexpr static span from_array(T (&array)[N])
    {
        return {array, N};
    }

    constexpr T* data() { return m_data; }
    constexpr const T* data() const { return m_data; }
    constexpr size_t size() const { return m_size; }

    constexpr explicit operator bool() const { return size() > 0; }

    using iterator = T*;
    using const_iterator = const T*;

    constexpr iterator begin() { return m_data; }
    constexpr iterator end() { return m_data + m_size; }

    constexpr const_iterator begin() const { return m_data; }
    constexpr const_iterator end() const { return m_data + m_size; }

    constexpr const_iterator cbegin() const { return m_data; }
    constexpr const_iterator cend() const { return m_data + m_size; }

private:
    T* m_data{};
    size_t m_size{};
};

template <typename T, size_t N>
constexpr static span<T> make_span(T (&array)[N])
{
    return span<T>::from_array(array);
}

} // namespace jg
