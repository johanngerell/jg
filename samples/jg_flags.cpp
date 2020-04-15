#include <iostream>
#include <iomanip>
#include <jg_flags.h>

struct options : jg::flags_base<options>
{
    constexpr static flag_t none   = 0;
    constexpr static flag_t first  = 1;
    constexpr static flag_t second = 2;
    constexpr static flag_t third  = 4;
    constexpr static flag_t all    = first | second | third;

    using jg::flags_base<options>::flags_base;
};

int main()
{
    std::cout << "jg_flags sample...\n\n";

    options o1;
    std::cout << "o1 == 'none' is " << std::boolalpha << (o1.value() == options::none) << '\n';

    options o2{options::none};
    std::cout << "o2 == 'none' is " << std::boolalpha << (o2.value() == options::none) << '\n';

    options o3{options::first | options::second};
    std::cout << "o3.has(first) is " << std::boolalpha << o3.has(options::first) << '\n';
    std::cout << "o3.has(second) is " << std::boolalpha << o3.has(options::second) << '\n';
    std::cout << "o3.has(third) is " << std::boolalpha << o3.has(options::third) << '\n';

    std::cout << "\n...done";
}