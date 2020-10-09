cmake_minimum_required(VERSION 3.5)

project(css_selector_parser LANGUAGES C)
#SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig)

pkg_check_modules(LibTidy REQUIRED tidy)
include_directories(${LibTidy_INCLUDE_DIRS})
link_directories(${LibTidy_LIBRARY_DIRS})
add_definitions(${LibTdy_CFLAGS_OTHER})

set(
	PROJECT_SRC
#    src/tests/parser_test.c
#	src/tests/selector_test.c
    src/selector.c
    src/selector.h
    src/parser.c
    src/parser.h

	../dynamic_string/dynamic_string.h
	../dynamic_string/dynamic_string.c
)

set(
	PARSER_TEST_SRC
	src/tests/parser_test.c
)

set(
	SELECTOR_TEST_SRC
	src/tests/selector_test.c
#	src/selector.c
#	src/selector.h
#	src/parser.c
#	src/parser.h

#	../dynamic_string/dynamic_string.h
#	../dynamic_string/dynamic_string.c
)



add_executable(parser_test ${PROJECT_SRC} ${PARSER_TEST_SRC})
set_property(TARGET parser_test PROPERTY CMAKE_C_STANDARD 11)
target_link_libraries(parser_test ${LibTidy_LIBRARIES})

add_executable(selector_test ${PROJECT_SRC} ${SELECTOR_TEST_SRC})
set_property(TARGET selector_test PROPERTY CMAKE_C_STANDARD 11)
target_link_libraries(selector_test ${LibTidy_LIBRARIES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


