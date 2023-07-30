set(TargetName "Hash")

set(SOURCES 
	"hash.h" 
)

add_library(${TargetName} ${SOURCES})

set_target_properties(${TargetName} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${TargetName} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")