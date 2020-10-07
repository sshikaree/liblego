cmake_minimum_required(VERSION 3.5)

project(css_selector_parser LANGUAGES C)
#SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(
    SRC
    src/tests/parser_test.c
    ../dynamic_string/dynamic_string.h
    ../dynamic_string/dynamic_string.c
    src/selector.c
    src/selector.h
    src/parser.c
    src/parser.h
)

add_executable(css_selector_parser ${SRC})
set_property(TARGET css_selector_parser PROPERTY CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
#target_include_directories(dynamic_string PUBLIC "/usr/lib/llvm-10/lib/clang/10.0.0/include/")
#target_include_directories(dynamic_string PUBLIC "/usr/lib/llvm-9/lib/clang/9.0.1/include/")
#target_include_directories(dynamic_string PUBLIC "/usr/include/linux/")
#target_include_directories(dynamic_string PUBLIC "/usr/lib/gcc/x86_64-linux-gnu/9/include/")
