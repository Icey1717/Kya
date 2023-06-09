project(port)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("ext/pss/pss")

add_subdirectory("Windows/Renderer")

set(SOURCES 
	"Windows/WindowsFunctions.cpp" 
	"include/port.h"
)

add_library(port ${SOURCES})

target_include_directories(port PUBLIC "include")

target_link_libraries(port renderer)

target_link_libraries(port pss)

add_subdirectory(DebugMenu)