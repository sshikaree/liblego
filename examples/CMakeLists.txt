cmake_minimum_required(VERSION 3.5)

project(lego_example LANGUAGES C)

find_package(PkgConfig)

pkg_check_modules(LibTidy REQUIRED tidy)
include_directories(${LibTidy_INCLUDE_DIRS})
link_directories(${LibTidy_LIBRARY_DIRS})

set(
	SOURCES
	main.c
)

add_executable(lego_example ${SOURCES})
add_subdirectory("../" build/)
include_directories(${liblego_INCLUDE_DIRS})
include_directories("../src/" "../include/")
set_property(TARGET lego_example PROPERTY CMAKE_C_STANDARD 11)
target_link_libraries(lego_example ${LibTidy_LIBRARIES})
target_link_libraries(lego_example lego)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")


