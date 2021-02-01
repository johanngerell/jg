#include <jg_string.h>
#include <jg_test.h>

jg::test_suite test_string_split()
{
    return { "jg_string - jg::split", {
        // positive
        { "One expected token - empty string is an empty but valid token", [] {
            const auto tokens = jg::split<1>("", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "");
        }},
        { "One expected token - string without delimiter is a token", [] {
            const auto tokens = jg::split<1>("1", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "1");
        }},
        { "Two expected tokens - string with only a delimiter gives two empty but valid tokens", [] {
            const auto tokens = jg::split<2>(",", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "");
            jg_test_assert((*tokens)[1] == "");
        }},
        { "Two expected tokens - string with one ending delimiter gives two tokens", [] {
            const auto tokens = jg::split<2>("1,", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "1");
            jg_test_assert((*tokens)[1] == "");
        }},
        { "Two expected tokens - string with one beginning delimiter gives two tokens", [] {
            const auto tokens = jg::split<2>(",2", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "");
            jg_test_assert((*tokens)[1] == "2");
        }},
        { "Two expected tokens - string with one embedded delimiter gives two tokens", [] {
            const auto tokens = jg::split<2>("1,2", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "1");
            jg_test_assert((*tokens)[1] == "2");
        }},
        { "Three expected tokens - string with ending delimiter gives three tokens", [] {
            const auto tokens = jg::split<3>("1,2,", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "1");
            jg_test_assert((*tokens)[1] == "2");
            jg_test_assert((*tokens)[2] == "");
        }},
        { "Three expected tokens - string with beginning delimiter gives three tokens", [] {
            const auto tokens = jg::split<3>(",2,3", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "");
            jg_test_assert((*tokens)[1] == "2");
            jg_test_assert((*tokens)[2] == "3");
        }},
        { "Three expected tokens - string with only two delimiters gives three tokens", [] {
            const auto tokens = jg::split<3>(",,", ',');
            jg_test_assert(tokens.has_value());
            jg_test_assert((*tokens)[0] == "");
            jg_test_assert((*tokens)[1] == "");
            jg_test_assert((*tokens)[2] == "");
        }},
        // negative
        { "One expected token - string with only a delimiter can't be split", [] {
            const auto tokens = jg::split<1>(",", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "One expected token - string with beginning delimiter can't be split", [] {
            const auto tokens = jg::split<1>(",2", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "One expected token - string with ending delimiter can't be split", [] {
            const auto tokens = jg::split<1>("1,", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "One expected token - string with embedded delimiter can't be split", [] {
            const auto tokens = jg::split<1>("1,2", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "Two expected tokens - string with only two delimiter can't be split", [] {
            const auto tokens = jg::split<2>(",,", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "Two expected tokens - string with embedded and beginning delimiter can't be split", [] {
            const auto tokens = jg::split<2>(",2,3", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "Two expected tokens - string with embedded and ending delimiter can't be split", [] {
            const auto tokens = jg::split<2>("1,2,", ',');
            jg_test_assert(!tokens.has_value());
        }},
        { "Two expected tokens - string with embedded delimiters can't be split", [] {
            const auto tokens = jg::split<2>("1,2,3", ',');
            jg_test_assert(!tokens.has_value());
        }}
    }};
}
