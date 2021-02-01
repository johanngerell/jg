cmake_minimum_required(VERSION 3.10)

project(jg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")

include_directories(inc)

if(MSVC)
    string(REGEX REPLACE "/W[1-3]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if (MSVC)
    add_compile_options(/Zc:__cplusplus /EHsc /W4 /WX)
else()
    # jg::mock can't be used with -pedantic due to "the ##__VA_ARGS__ trick"
    add_compile_options(-Wall -Wextra -Werror)
endif()


add_executable(jg_stacktrace samples/jg_stacktrace.cpp)
add_executable(jg_span samples/jg_span.cpp)
add_executable(jg_simple_logger samples/jg_simple_logger.cpp)
add_executable(jg_flags samples/jg_flags.cpp)
add_executable(jg_args samples/jg_args.cpp)
add_executable(jg_string samples/jg_string.cpp)
add_executable(jg_unit_tests tests/main.cpp tests/jg_optional_tests.cpp)

target_compile_definitions(jg_unit_tests PRIVATE JG_VERIFY_ASSERTION=mock_assert)
