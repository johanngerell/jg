cmake_minimum_required(VERSION 3.12)

project(jg)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../inc)

if (MSVC)
    add_compile_options(/Zc:__cplusplus /W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(jg_stacktrace jg_stacktrace.cpp)
add_executable(jg_span jg_span.cpp)
add_executable(jg_simple_logger jg_simple_logger.cpp)
add_executable(jg_flags jg_flags.cpp)
