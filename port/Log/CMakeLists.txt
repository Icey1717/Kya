set(TargetName "Log")

add_subdirectory("spdlog")

set(SOURCES 
	"log.h" 
)

add_library(${TargetName} ${SOURCES})

set_target_properties(${TargetName} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${TargetName} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")