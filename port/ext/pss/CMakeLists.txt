cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "clang")

project(pss_test)

set(SOURCES "main.c")

add_executable(pss_test ${SOURCES})

add_subdirectory(pss)

target_link_libraries(pss_test PRIVATE pss)