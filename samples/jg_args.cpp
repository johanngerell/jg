#include <iostream>
#include <jg_args.h>
#include <jg_test.h>

jg::test_suite test_args()
{
    return
    {
        "jg_args",
        {
            // iterator range
            { 
                "default construction => empty iterator range", []
                {
                    jg::args args;
                    jg_test_assert(args.begin() == args.end());
                }
            },
            {
                "argc == 0 => empty iterator range", []
                {
                    int argc = 0;
                    jg::args args{argc, nullptr};
                    jg_test_assert(args.begin() == args.end());
                }
            },
            {
                "argc == 1 => iterator range length is 1", []
                {
                    int argc = 1;
                    char arg1[]{"1"};
                    char* argv[1]{arg1};
                    jg::args args{argc, argv};
                    jg_test_assert(std::distance(args.begin(), args.end()) == 1);
                }
            },
            {
                "argc == 5 => iterator range length is 5", []
                {
                    int argc = 5;
                    char arg1[]{"1"}, arg2[]{"2"}, arg3[]{"3"}, arg4[]{"4"}, arg5[]{"5"};
                    char* argv[5]{arg1, arg2, arg3, arg4, arg5};
                    jg::args args{argc, argv};
                    jg_test_assert(std::distance(args.begin(), args.end()) == 5);
                }
            },
            // args_has_key
            {
                "", []
                {
                }
            }
        }
    };
}

int main()
{
    std::cout << "jg_args sample...\n\n";

    return jg::test_run(
    {
        test_args()
    });
}