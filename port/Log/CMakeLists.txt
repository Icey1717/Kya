project(Log)

set(SOURCES 
	"log.h" 
)

add_library(Log ${SOURCES})

set_target_properties(Log PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(Log PUBLIC "${CMAKE_CURRENT_LIST_DIR}")