#define CATCH_CONFIG_MAIN
#include <windows.h>
#include "catch2/catch.hpp"
#include "../inc/jg_mock.h"

// There should be NO tests in this file. We only want to compile this file ONCE. It'll reduce the total
// test compilation time, since it'll put the entire Catch2 implementation in one translation unit, which
// won't change when individual tests in other files change.

// Link-time function dependencies that needs mocking in unit tests can only be defined once, and that is
// preferably done here. Mocking such functions for usage in unit tests in different translation units is
// made easier by using the JG_MOCK_PROXY macro family from jg::mock.

namespace jg
{
namespace win32
{
namespace wrappers
{
    // This is the namespace where jg::win32 wraps equally named Win32 API functions and just forwards
    // the call to said Win32 API functions. The purpose of doing this is to get rid of the Win32 API
    // *implementation* dependency in unit tests. This is a good place to define the wrapper functions
    // so that the unit tests are abl to mock them.
    //
    // NOTE: These wrappers can often preferably be placed in small and specific wrapper interfaces
    // instead, but some functions, like the 3 first ones below, must be callable from a "static context"
    // (like a window procedure) where there are no good way to apply an interface instance in a "best
    // practices" way.

    JG_MOCK(,,, LONG_PTR, GetWindowLongPtrA, HWND, int);
    JG_MOCK(,,, LONG_PTR, SetWindowLongPtrA, HWND, int, LONG_PTR);
    JG_MOCK(,,, LRESULT, DefWindowProcA, HWND, UINT, WPARAM, LPARAM);

} // namespace wrappers
} // namespace win32
} // namespace jg