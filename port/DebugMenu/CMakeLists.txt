set(TargetName "DebugMenu")
project(${TargetName})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES 
	"src/DebugMaterialPreviewer.h"
	"src/DebugMaterialPreviewer.cpp"
	"src/DebugMaterialPreviewer.inl"
	"src/DebugCallstackPreviewer.h"
	"src/DebugCallstackPreviewer.cpp"
	"src/DebugCamera.h"
	"src/DebugCamera.cpp"
	"src/DebugMenu.cpp"
	"src/DebugHelpers.cpp"
	"src/DebugRendererVulkan.cpp"
	"include/DebugRenderer.h"
	"include/DebugHelpers.h"
	"include/DebugMenu.h"
	"include/Callstack.h"
)

add_library(${TargetName} ${SOURCES})

target_link_libraries(${TargetName} PRIVATE Renderer Log Kya Dbghelp.lib imgui)

target_include_directories(${TargetName} PUBLIC "include")