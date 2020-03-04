#include <iostream>
#include <jg_stacktrace.h>

int main()
{
    std::cout << "jg_stacktrace sample...\n\n";

    const auto trace = jg::stack_trace()
                       .include_frame_count(10) // can be less
                       .skip_frame_count(1) // skips main()
                       .capture();

    for (const auto& frame : trace) 
        std::cout << frame << "\n";
    
    std::cout << "\n...done";
}