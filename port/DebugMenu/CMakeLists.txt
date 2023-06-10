project(DebugMenu)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES 
	"src/DebugMenu.h"
	"src/DebugMenu.cpp"
	"src/DebugRendererVulkan.cpp"
	"include/DebugRenderer.h"
)

add_library(DebugMenu ${SOURCES})

target_link_libraries(DebugMenu Renderer Log Kya)

target_include_directories(DebugMenu PUBLIC "include")