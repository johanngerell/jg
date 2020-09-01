#include <iostream>
#include <jg_string.h>

struct foo final
{
    std::string s;
    bool b;
};

int main()
{
    std::cout << "jg_span sample...\n\n";

    const int ints[] { 1, 2, 3, 4, 5 };

    for (const auto i : jg::make_span(ints)) 
        std::cout << i << "\n";

    const foo foos[] { { "true=", true }, { "false=", false } };

    for (const auto& f : jg::make_span(foos)) 
        std::cout << std::boolalpha << f.s << f.b << "\n";

    std::cout << "\n...done";
}