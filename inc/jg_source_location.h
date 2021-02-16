#pragma once

namespace jg {

/// Trivial pre-C++20 replacement for std::source_location.
class source_location final
{
public:
    constexpr source_location() noexcept = default;
    constexpr source_location(const char* file_name, size_t line) noexcept
        : m_file_name{file_name}
        , m_line{line}
    {}

    constexpr source_location(const source_location&) noexcept = default;
    constexpr source_location(source_location&&) noexcept = default;

    constexpr source_location& operator=(const source_location&) noexcept = default;
    constexpr source_location& operator=(source_location&&) noexcept = default;

    constexpr const char* file_name() const noexcept { return m_file_name; }
    constexpr size_t line() const noexcept { return m_line; }

private:
    const char* m_file_name{};
    size_t m_line{};
};

} // namespace jg

#define jg_current_source_location() jg::source_location(__FILE__, __LINE__)
