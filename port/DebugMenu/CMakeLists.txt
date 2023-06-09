project(debug_menu)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES 
	"src/DebugMenu.h"
	"src/DebugMenu.cpp"
	"src/DebugRendererVulkan.h"
	"src/DebugRendererVulkan.cpp"
)

add_library(debug_menu ${SOURCES})

target_link_libraries(debug_menu renderer)