#include <initializer_list>
#include <vector>
#include <jg_args.h>
#include <jg_test.h>
#include <jg_string.h>

namespace test_helpers
{

// Makes it easy to fake command line argc-argv pairs, jg::args, etc. in tests.
class cmdline final
{
public:
    cmdline(std::initializer_list<const char*> strings)
        : m_strings{strings.begin(), strings.end()}
        , m_argv{[this]
          {               
              std::vector<char*> ptrs{m_strings.size() + 1}; // As per spec, argv[argc] is nullptr
              std::transform(m_strings.begin(), m_strings.end(), ptrs.begin(),
                             [] (auto& string) { return string.data(); });
              return ptrs;
          }()}
        , m_args{static_cast<int>(m_argv.size() - 1), m_argv.data()}
    {}

    int             argc() const   { return m_argv.size() - 1; } // don't count last nullptr
    char**          argv() const   { return m_argv.data(); }
    const jg::args& args() const   { return m_args; }
    std::string     joined() const { return jg::join(m_strings.begin(), m_strings.end(), " "); }

private:
    std::vector<std::string> m_strings;
    mutable std::vector<char*> m_argv;
    jg::args m_args;
};

} // namespace test_helpers

jg::test_suite test_args()
{
    using namespace test_helpers;

    return { "jg_args", {
        // test_helpers::cmdline
        { "default constructed => expected empty traits", [] {
            cmdline empty{};
            jg_test_assert(empty.argc() == 0);
            jg_test_assert(empty.argv()[0] == nullptr);
            jg_test_assert(empty.args().begin() == empty.args().end());
            jg_test_assert(empty.joined() == "");
        }},
        { "1 arg => expected 1 arg traits", [] {
            cmdline onearg{"1"};
            jg_test_assert(onearg.argc() == 1);
            jg_test_assert(onearg.argv()[1] == nullptr);
            jg_test_assert(std::next(onearg.args().begin()) == onearg.args().end());
            jg_test_assert(onearg.joined() == "1");
        }},
        { "3 args => expected 3 arg traits", [] {
            cmdline threeargs{"1", "2", "3"};
            jg_test_assert(threeargs.argc() == 3);
            jg_test_assert(threeargs.argv()[3] == nullptr);
            jg_test_assert(std::next(threeargs.args().begin(), 3) == threeargs.args().end());
            jg_test_assert(threeargs.joined() == "1 2 3");
        }},
        // iterator range
        { "default construction => empty iterator range", [] {
            const jg::args args;
            jg_test_assert(args.begin() == args.end());
        }},
        { "argc == 0 => empty iterator range", [] {
            test_helpers::cmdline cmdline{};
            jg_test_assert(cmdline.args().begin() == cmdline.args().end());
        }},
        { "argc == 1 => iterator range length is 1", [] {
            const test_helpers::cmdline cmdline{"1"};
            jg_test_assert(std::distance(cmdline.args().begin(), cmdline.args().end()) == 1);
        }},
        { "argc == 5 => iterator range length is 5", [] {
            const test_helpers::cmdline cmdline{"1", "2", "3", "4", "5"};
            jg_test_assert(std::distance(cmdline.args().begin(), cmdline.args().end()) == 5);
        }},
        // args_has_key
        { "argc == 0 => key not found", [] {
            jg_test_assert(!jg::args_has_key(cmdline{}.args(), ""));
            jg_test_assert(!jg::args_has_key(cmdline{}.args(), "foo"));
        }},
        { "argc == 1 => existing key is found", [] {
            jg_test_assert(jg::args_has_key(cmdline{"--foo"}.args(), "--foo"));
        }},
        { "argc == 1 => non existing key is not found", [] {
            jg_test_assert(!jg::args_has_key(cmdline{"--foo"}.args(), "--bar"));
        }},
        { "argc == 1 => partial key is not found", [] {
            jg_test_assert(!jg::args_has_key(cmdline{"--foo=bar"}.args(), "--foo"));
        }},
        { "argc == 3 => existing key is found", [] {
            jg_test_assert(jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--foo"));
            jg_test_assert(jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--bar"));
            jg_test_assert(jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--baz"));
        }},
        { "argc == 3 => non existing key is not found", [] {
            jg_test_assert(!jg::args_has_key(cmdline{"--foo", "--bar", "--baz"}.args(), "--acme"));
        }},
        { "argc == 3 => partial key is not found", [] {
            jg_test_assert(!jg::args_has_key(cmdline{"--foo=1", "--bar=2", "--baz=3"}.args(), "--foo"));
            jg_test_assert(!jg::args_has_key(cmdline{"--foo=1", "--bar=2", "--baz=3"}.args(), "--bar"));
            jg_test_assert(!jg::args_has_key(cmdline{"--foo=1", "--bar=2", "--baz=3"}.args(), "--baz"));
        }},
        // args_key_value
        { "argc == 0 => value not found", [] {
            jg_test_assert(!jg::args_key_value(cmdline{}.args(), ""));
            jg_test_assert(!jg::args_key_value(cmdline{}.args(), "foo"));
        }},
        { "argc == 1 => existing key value is found", [] {
            jg_test_assert(jg::args_key_value(cmdline{"--foo=bar"}.args(), "--foo=").value() == "bar");
        }},
        { "argc == 1 => non existing key value is not found", [] {
            jg_test_assert(!jg::args_key_value(cmdline{"--foo=bar"}.args(), "--bar"));
        }},
        { "argc == 3 => existing key value is found", [] {
            jg_test_assert(jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--foo=").value() == "abc");
            jg_test_assert(jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--bar=").value() == "def");
            jg_test_assert(jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--baz=").value() == "ghi");
        }},
        { "argc == 3 => non existing key value is not found", [] {
            jg_test_assert(!jg::args_key_value(cmdline{"--foo=abc", "--bar=def", "--baz=ghi"}.args(), "--acme="));
        }}
    }};
}
