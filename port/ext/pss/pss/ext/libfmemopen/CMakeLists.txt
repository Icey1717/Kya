project(libfmemopen)

set(SOURCES
    fmemopen_windows/libfmemopen.c
)

add_library(libfmemopen ${SOURCES})

target_include_directories(libfmemopen PUBLIC ${CMAKE_CURRENT_LIST_DIR}/fmemopen_windows)
