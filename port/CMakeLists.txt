project(Port)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("Log")
add_subdirectory("Hash")
add_subdirectory("Archive")

add_subdirectory("ext/pss/pss")

add_subdirectory("Windows/Renderer")

set(SOURCES 
	"Windows/WindowsFunctions.cpp" 
	"include/port.h"
)

add_library(Port ${SOURCES})

target_include_directories(Port PUBLIC "include")

target_link_libraries(Port PUBLIC Renderer pss)

add_subdirectory(DebugMenu)