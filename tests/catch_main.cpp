#define CATCH_CONFIG_MAIN
#include <windows.h>
#include "catch2/catch.hpp"
#include "../inc/jg_mock.h"

// There should be NO tests in this file. We only want to compile this file ONCE. It'll reduce the total
// test compilation time, since it'll put the entire Catch2 implementation in one translation unit, which
// won't change when individual tests in other files change.

// Link-time function dependencies that needs mocking in unit tests can only be defined once, and that is
// preferably done here. Mocking such functions for usage in unit tests in different translation units is
// made easier by using the JG_MOCK and JG_MOCK_REF macros from jg::mock.

namespace jg
{
namespace win32
{
namespace wrappers
{

JG_MOCK(,,, LONG_PTR, GetWindowLongPtrA, HWND, int);
JG_MOCK(,,, LONG_PTR, SetWindowLongPtrA, HWND, int, LONG_PTR);
JG_MOCK(,,, LRESULT, DefWindowProcA, HWND, UINT, WPARAM, LPARAM);

} // namespace wrappers
} // namespace win32
} // namespace jg
