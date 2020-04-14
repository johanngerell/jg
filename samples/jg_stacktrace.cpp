#include <iostream>
#include <jg_stacktrace.h>

auto get_trace()
{
    return jg::stack_trace()
               .include_frame_count(10) // actual count can be less
               .skip_frame_count(1) // skips get_trace()
               .capture();
}

int main()
{
    std::cout << "jg_stacktrace sample...\n\n";

    for (const auto& frame : get_trace()) 
        std::cout << frame << "\n";
    
    std::cout << "\n...done";
}