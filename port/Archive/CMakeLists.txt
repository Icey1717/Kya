set(TargetName "Archive")

set(SOURCES 
	"archive.h" 
	"archive.cpp" 
)

add_library(${TargetName} ${SOURCES})

set_target_properties(${TargetName} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${TargetName} PUBLIC "${CMAKE_CURRENT_LIST_DIR}")