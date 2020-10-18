cmake_minimum_required(VERSION 3.5)

project(liblego LANGUAGES C)
#SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig)

pkg_check_modules(LibTidy REQUIRED tidy)
include_directories(${LibTidy_INCLUDE_DIRS})
link_directories(${LibTidy_LIBRARY_DIRS})


set(
	LIBLEGO_SOURCES
	
#	src/lego.c
	src/selector.c
    src/parser.c
    
 	src/util/dynamic_string.c
 	src/util/slist.c

)

set (
    LIBLEGO_HEADERS

    src/selector.h   	
    src/parser.h
   	src/util/dynamic_string.h
	include/util/slist.h
   	include/lego.h
   	include/parser_error.h

)

set(
	PARSER_TEST_SOURCES
	src/tests/parser_test.c
)

set(
	SELECTOR_TEST_SOURCES
	src/tests/selector_test.c
)

add_library(
    lego
    
    ${LIBLEGO_SOURCES}
    ${LIBLEGO_HEADERS}
)

include_directories(include/ src/)
target_include_directories(lego PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(parser_test ${LIBLEGO_SOURCES} ${LIBLEGO_HEADERS} ${PARSER_TEST_SOURCES})
set_property(TARGET parser_test PROPERTY CMAKE_C_STANDARD 11)
target_link_libraries(parser_test ${LibTidy_LIBRARIES} lego)

add_executable(selector_test ${LIBLEGO_SOURCES} ${LIBLEGO_HEADERS} ${SELECTOR_TEST_SOURCES})
set_property(TARGET selector_test PROPERTY CMAKE_C_STANDARD 11)
target_link_libraries(selector_test ${LibTidy_LIBRARIES} lego)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


