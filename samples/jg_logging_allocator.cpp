#define JG_SIMPLE_LOGGER_IMPL
#include <jg_simple_logger.h>
#define JG_OS_IMPL
#include <jg_os.h>
#include <jg_logging_allocator.h>

int main()
{
    std::cout << "jg_logging_allocator sample...\n\n";

    {
        std::basic_string<char, std::char_traits<char>, jg::logging_allocator<char>> string;
        jg_log_info_line() << "Default constructed: '" << string << "', Capacity: " << string.capacity() << ", Length: " << string.length() << ", Sizeof: " << sizeof(string);
        string.assign(string.capacity(), '*');
        jg_log_info_line() << "Assigned capacity: '" << string << "', Capacity: " << string.capacity() << ", Length: " << string.length() << ", Sizeof: " << sizeof(string);
        string.assign(string.capacity() + 1, '*');
        jg_log_info_line() << "Assigned capacity + 1: '" << string << "', Capacity: " << string.capacity() << ", Length: " << string.length() << ", Sizeof: " << sizeof(string);
    }

    std::cout << "\n...done\n";
}
