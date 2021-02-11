#pragma once

#include "jg_verify.h"

namespace jg {

// Trivial drop-in pre-C++14 replacement for std::optional. std::optional is in std::experimental
// for C++14 and in std for C++17. It supports only the most basic scenarios and will always invoke
// the contained type's default, copy, or move constructor. 
template <typename T>
class optional final
{
public:
    optional() = default;

    optional(const optional& other)
        : m_value{other.m_value}
        , m_set{other.m_set}
    {}

    optional(optional&& other)
        : m_value{std::move(other.m_value)}
        , m_set{other.m_set}
    {}

    optional(const T& value)
        : m_value{value}
        , m_set{true}
    {}

    optional(T&& value)
        : m_value{std::move(value)}
        , m_set{true}
    {}

    optional& operator=(const optional& other)
    {
        m_value = other.m_value;
        m_set = other.m_set;
        return *this;
    }

    optional& operator=(optional&& other)
    {
        m_value = std::move(other.m_value);
        m_set = other.m_set;
        return *this;
    }

    optional& operator=(const T& value)
    {
        m_value = value;
        m_set = true;
        return *this;
    }

    optional& operator=(T&& value)
    {
        m_value = std::move(value);
        m_set = true;
        return *this;
    }

    constexpr bool has_value() const noexcept { return m_set; }
    constexpr explicit operator bool() const noexcept { return has_value(); }

    // Note that std::optional::value() isn't noexcept and throws when there's no value.
    constexpr T& value() noexcept { verify(has_value()); return m_value; }
    constexpr const T& value() const noexcept { verify(has_value()); return m_value; }

    // Note that std::optional::operator*() isn't noexcept and doesn't check if there's a value.
    constexpr T& operator*() noexcept { verify(has_value()); return m_value; }
    constexpr const T& operator*() const noexcept { verify(has_value()); return m_value; }

    // Note that std::optional::operator->() isn't noexcept and doesn't check if there's a value.
    constexpr T* operator->() noexcept { verify(has_value()); return &m_value; }
    constexpr const T* operator->() const noexcept { verify(has_value()); return &m_value; }    

private:
    T m_value{};
    bool m_set{};
};

} // namespace jg
