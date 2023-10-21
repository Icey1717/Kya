set(TargetName "Job")

set(SOURCES 
	"thread-pool/include/BS_thread_pool.hpp" 
)

add_library(${TargetName} ${SOURCES})

set_target_properties(${TargetName} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${TargetName} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/thread-pool/include")