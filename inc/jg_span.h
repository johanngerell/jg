#pragma once

namespace jg {

// Trivial drop-in pre-C++20 replacement for std::span.
template <typename T>
class span final
{
public:
    using iterator = T*;

    constexpr span() noexcept = default;
    constexpr span(iterator first, iterator last) : m_first{first} , m_last{last} {}
    constexpr span(iterator first, size_t size) : span{first, first + size} {}
    template <size_t N> constexpr span(T (&array)[N]) noexcept : span{array, N} {}
    constexpr span(const span& other) noexcept = default;
    constexpr span& operator=(const span& other) noexcept = default;

    constexpr T* data() const noexcept { return m_first; }
    constexpr size_t size() const noexcept { return static_cast<size_t>(m_last - m_first); }
    constexpr size_t size_bytes() const noexcept { return size() * sizeof(T); }

    constexpr T& operator[](size_t index) const { return m_first[index]; }
    constexpr T& front() const  { return *m_first; }
    constexpr T& back() const  { return *(m_last - 1); }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_last == m_first; }

    constexpr iterator begin() const noexcept { return m_first; }
    constexpr iterator end() const noexcept { return m_last; }

    constexpr span first(size_t size) const { return {m_first, m_first + size}; }
    constexpr span last(size_t size) const { return {m_last - size, m_last}; }
    constexpr span subspan(size_t offset, size_t size) const { return {m_first + offset, m_first + offset + size}; }

private:
    iterator m_first{};
    iterator m_last{};
};

} // namespace jg
