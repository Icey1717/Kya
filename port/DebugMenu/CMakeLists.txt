set(TargetName "DebugMenu")
project(${TargetName})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(SOURCES 
	"src/DebugMaterialPreviewer.h"
	"src/DebugMaterialPreviewer.cpp"
	"src/DebugCallstackPreviewer.h"
	"src/DebugCallstackPreviewer.cpp"
	"src/DebugCamera.h"
	"src/DebugCamera.cpp"
	"src/DebugMeshViewer.h"
	"src/DebugMeshViewer.cpp"
	"src/DebugMeshViewerVulkan.h"
	"src/DebugMeshViewerVulkan.cpp"
	"src/DebugMenu.cpp"
	"src/DebugMaterial.h"
	"src/DebugMaterial.cpp"
	"src/DebugRendererVulkan.cpp"
	"include/DebugRenderer.h"
	"include/DebugMenu.h"
	"include/Callstack.h"
)

add_library(${TargetName} ${SOURCES})

add_subdirectory("ext/gizmo")
add_subdirectory("ext/magic_enum")

target_link_libraries(${TargetName} PRIVATE Renderer Log Kya Dbghelp.lib imgui Gizmo magic_enum)

target_include_directories(${TargetName} PUBLIC "include")