project(DebugMenu)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES 
	"src/DebugMaterialPreviewer.h"
	"src/DebugMaterialPreviewer.cpp"
	"src/DebugCallstackPreviewer.h"
	"src/DebugCallstackPreviewer.cpp"
	"src/DebugMenu.cpp"
	"src/DebugHelpers.cpp"
	"src/DebugRendererVulkan.cpp"
	"include/DebugRenderer.h"
	"include/DebugHelpers.h"
	"include/DebugMenu.h"
	"include/Callstack.h"
)

add_library(DebugMenu ${SOURCES})

target_link_libraries(DebugMenu Renderer Log Kya Dbghelp.lib)

target_include_directories(DebugMenu PUBLIC "include")