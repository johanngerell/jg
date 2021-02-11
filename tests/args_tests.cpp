#include <jg_test.h>
#include <jg_string.h>
#include "test_helpers.h"

namespace {

struct args_tests final : jg::test_suites_base<args_tests>
{
    auto operator()()
    {
        using namespace test_helpers;

        return jg::test_suites { "args", {
            jg::test_suite { "test_helpers::cmdline", {
                jg::test_case { "default constructed => expected empty traits", [] {
                    cmdline empty{};
                    jg_test_assert(empty.argc() == 0);
                    jg_test_assert(empty.argv()[0] == nullptr);
                    jg_test_assert(empty.args().begin() == empty.args().end());
                    jg_test_assert(empty.joined() == "");
                }},
                jg::test_case { "1 arg => expected 1 arg traits", [] {
                    cmdline onearg{"1"};
                    jg_test_assert(onearg.argc() == 1);
                    jg_test_assert(onearg.argv()[1] == nullptr);
                    jg_test_assert(std::next(onearg.args().begin()) == onearg.args().end());
                    jg_test_assert(onearg.joined() == "1");
                }},
                jg::test_case { "3 args => expected 3 arg traits", [] {
                    cmdline threeargs{"1", "2", "3"};
                    jg_test_assert(threeargs.argc() == 3);
                    jg_test_assert(threeargs.argv()[3] == nullptr);
                    jg_test_assert(std::next(threeargs.args().begin(), 3) == threeargs.args().end());
                    jg_test_assert(threeargs.joined() == "1 2 3");
                }}
            }},
            jg::test_suite { "iteration", {
                jg::test_case { "default constructed => expected empty traits", [] {
                    cmdline empty{};
                    jg_test_assert(empty.argc() == 0);
                    jg_test_assert(empty.argv()[0] == nullptr);
                    jg_test_assert(empty.args().begin() == empty.args().end());
                    jg_test_assert(empty.joined() == "");
                }},
                jg::test_case { "1 arg => expected 1 arg traits", [] {
                    cmdline onearg{"1"};
                    jg_test_assert(onearg.argc() == 1);
                    jg_test_assert(onearg.argv()[1] == nullptr);
                    jg_test_assert(std::next(onearg.args().begin()) == onearg.args().end());
                    jg_test_assert(onearg.joined() == "1");
                }},
                jg::test_case { "3 args => expected 3 arg traits", [] {
                    cmdline threeargs{"1", "2", "3"};
                    jg_test_assert(threeargs.argc() == 3);
                    jg_test_assert(threeargs.argv()[3] == nullptr);
                    jg_test_assert(std::next(threeargs.args().begin(), 3) == threeargs.args().end());
                    jg_test_assert(threeargs.joined() == "1 2 3");
                }},
                jg::test_case { "default construction => empty iterator range", [] {
                    const jg::args args;
                    jg_test_assert(args.begin() == args.end());
                }},
                jg::test_case { "argc == 0 => empty iterator range", [] {
                    test_helpers::cmdline cmdline{};
                    jg_test_assert(cmdline.args().begin() == cmdline.args().end());
                }},
                jg::test_case { "argc == 1 => iterator range length is 1", [] {
                    const test_helpers::cmdline cmdline{"1"};
                    jg_test_assert(std::distance(cmdline.args().begin(), cmdline.args().end()) == 1);
                }},
                jg::test_case { "argc == 5 => iterator range length is 5", [] {
                    const test_helpers::cmdline cmdline{"1", "2", "3", "4", "5"};
                    jg_test_assert(std::distance(cmdline.args().begin(), cmdline.args().end()) == 5);
                }}
            }},
            jg::test_suite { "args_has_key", {
                jg::test_case { "argc == 0 => key not found", [] {
                    jg_test_assert(!jg::args_has_key(cmdline{}.args(), ""));
                    jg_test_assert(!jg::args_has_key(cmdline{}.args(), "foo"));
                }},
                jg::test_case { "argc == 1 => existing key is found", [] {
                    jg_test_assert(jg::args_has_key(cmdline{"--foo"}.args(), "--foo"));
                }},
                jg::test_case { "argc == 1 => non existing key is not found", [] {
                    jg_test_assert(!jg::args_has_key(cmdline{"--foo"}.args(), "--bar"));
                }},
                jg::test_case { "argc == 1 => partial key is not found", [] {
                    jg_test_assert(!jg::args_has_key(cmdline{"--foo=bar"}.args(), "--foo"));
                }},
                jg::test_case { "argc == 3 => existing key is found", [] {
                    jg_test_assert(jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--foo"));
                    jg_test_assert(jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--bar"));
                    jg_test_assert(jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--baz"));
                }},
                jg::test_case { "argc == 3 => non existing key is not found", [] {
                    jg_test_assert(!jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--acme"));
                }},
                jg::test_case { "argc == 3 => partial key is not found", [] {
                    jg_test_assert(!jg::args_has_key(cmdline{"--foo=1", "--bar=2", "--baz=3"}.args(), "--foo"));
                    jg_test_assert(!jg::args_has_key(cmdline{"--foo=1", "--bar=2", "--baz=3"}.args(), "--bar"));
                    jg_test_assert(!jg::args_has_key(cmdline{"--foo=1", "--bar=2", "--baz=3"}.args(), "--baz"));
                }}
            }},
            jg::test_suite { "args_key_value", {
                jg::test_case { "argc == 0 => value not found", [] {
                    jg_test_assert(!jg::args_key_value(cmdline{}.args(), ""));
                    jg_test_assert(!jg::args_key_value(cmdline{}.args(), "foo"));
                }},
                jg::test_case { "argc == 1 => existing key value is found", [] {
                    jg_test_assert(jg::args_key_value(cmdline{"--foo=bar"}.args(), "--foo=").value() == "bar");
                }},
                jg::test_case { "argc == 1 => non existing key value is not found", [] {
                    jg_test_assert(!jg::args_key_value(cmdline{"--foo=bar"}.args(), "--bar"));
                }},
                jg::test_case { "argc == 3 => existing key value is found", [] {
                    jg_test_assert(jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--foo=").value() == "abc");
                    jg_test_assert(jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--bar=").value() == "def");
                    jg_test_assert(jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--baz=").value() == "ghi");
                }},
                jg::test_case { "argc == 3 => non existing key value is not found", [] {
                    jg_test_assert(!jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--acme="));
                }}
            }}
        }};
    }
} _;

}
