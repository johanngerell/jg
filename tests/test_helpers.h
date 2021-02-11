#pragma once

#include <initializer_list>
#include <vector>
#include <jg_args.h>

namespace test_helpers {

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
