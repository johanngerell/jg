#pragma once

#include <string>
#include "jg_span.h"
#include "jg_verify.h"

namespace jg
{

inline std::string& trim_left(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return string.erase(0, string.find_first_not_of(chars));
}
 
inline std::string& trim_right(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return string.erase(string.find_last_not_of(chars) + 1);
}
 
inline std::string& trim(std::string& string, const std::string& chars = "\t\n\v\f\r ")
{
    return trim_left(trim_right(string, chars), chars);
}

class string_ref
{
public:
    constexpr string_ref() = default;

    constexpr string_ref(const char* string)
        : m_string{string, strlen(string)}
    {}

    constexpr string_ref(const char* string, size_t length)
        : m_string{string, length}
    {}

    template <size_t N>
    constexpr string_ref(const char (&string)[N])
    {
        // Don't want char arrays with embedded nulls.
        debug_verify(strlen(string) == N - 1);
        return {string, N - 1};
    }

    template <size_t N>
    constexpr static string_ref literal(const char (&string)[N])
    {
        // Don't want char arrays with embedded nulls.
        debug_verify(strlen(string) == N - 1);
        return {string, N - 1};
    }

    string_ref find(const char* string) const
    {
        if (const char* match = strstr(c_str(), verified(string)))
            return {match, length() - (match - c_str())};
        else
            return {};
    }

    constexpr explicit operator bool() const
    {
        return m_string.operator bool();
    }

    constexpr const char* c_str() const { return m_string.data(); }
    constexpr size_t length() const { return m_string.size() - 1; }

private:
    span<const char> m_string;
};

inline bool operator==(const string_ref& first, const string_ref& second)
{
    return strcmp(first.c_str(), second.c_str()) == 0;
}

inline bool operator!=(const string_ref& first, const string_ref& second)
{
    return !(first == second);
}

template <size_t MAX_LENGTH>
class fixed_string final
{
public:
    constexpr fixed_string(const char* string, size_t length)
    {
        verify(length <= MAX_LENGTH);
        strncpy(m_string, verified(string), length + 1);
        m_string[length] = '\0';
    }

    constexpr fixed_string(const char* string)
        : fixed_string(string, strlen(verified(string)))
    {
    }

    constexpr const char* c_str() const { return m_string; }
    constexpr size_t length() const { return strlen(m_string); }

    constexpr explicit operator string_ref() const { return string_ref{c_str(), length()}; }

private:
    char m_string[MAX_LENGTH + 1];
};

template <size_t M, size_t N>
inline bool operator==(const fixed_string<M>& first, const fixed_string<N>& second)
{
    return strcmp(first.c_str(), second.c_str()) == 0;
}

template <size_t M, size_t N>
inline bool operator!=(const fixed_string<M>& first, const fixed_string<N>& second)
{
    return !(first == second);
}

template <size_t N>
inline bool operator==(const string_ref& first, const fixed_string<N>& second)
{
    return strcmp(first.c_str(), second.c_str()) == 0;
}

template <size_t N>
inline bool operator!=(const string_ref& first, const fixed_string<N>& second)
{
    return !(first == second);
}

template <size_t N>
inline bool operator==(const fixed_string<N>& first, const string_ref& second)
{
    return strcmp(first.c_str(), second.c_str()) == 0;
}

template <size_t N>
inline bool operator!=(const fixed_string<N>& first, const string_ref& second)
{
    return !(first == second);
}

}
