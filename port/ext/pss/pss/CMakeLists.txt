project(pss)

set(SOURCES 
	"src/pss.c" 
	"src/decode.c"
)

add_library(pss ${SOURCES})

add_subdirectory("ext/libmpeg2")
add_subdirectory("ext/libfmemopen")

target_link_libraries(pss libmpeg2 libfmemopen)

target_include_directories(pss PRIVATE "${CMAKE_CURRENT_LIST_DIR}/ext")
target_include_directories(pss PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")