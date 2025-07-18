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
	"src/DebugHero.h"
	"src/DebugHero.cpp"
	"src/DebugMeshViewerVulkan.h"
	"src/DebugMeshViewerVulkan.cpp"
	"src/DebugMenu.cpp"
	"src/DebugHelpers.h"
	"src/DebugMaterial.h"
	"src/DebugEvent.cpp"
	"src/DebugEvent.h"
	"src/DebugShop.cpp"
	"src/DebugShop.h"
	"src/DebugProjection.h"
	"src/DebugCinematic.cpp"
	"src/DebugCinematic.h"
	"src/DebugProjection.cpp"
	"src/DebugMaterial.cpp"
	"src/DebugFrameBuffer.h"
	"src/DebugFrameBuffer.cpp"
	"src/DebugCheckpoint.h"
	"src/DebugCheckpoint.cpp"
	"src/DebugTexture.h"
	"src/DebugTexture.cpp"
	"src/DebugMesh.h"
	"src/DebugMesh.cpp"
	"src/DebugWolfen.h"
	"src/DebugWolfen.cpp"
	"src/DebugRendererVulkan.cpp"
	"src/DebugSetting.cpp"
	"src/DebugSetting.h"
	"src/Actor/DebugActor.h"
	"src/Actor/DebugActor.cpp"
	"src/Actor/DebugActorBehaviour.h"
	"src/Actor/DebugActorBehaviour.cpp"
	"include/DebugRenderer.h"
	"include/DebugMenu.h"
	"include/Callstack.h"
)

add_library(${TargetName} ${SOURCES})

add_subdirectory("ext/gizmo")
add_subdirectory("ext/magic_enum")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("ext/json")

target_link_libraries(${TargetName} PRIVATE Renderer Texture Log Kya Dbghelp.lib imgui Gizmo magic_enum nlohmann_json::nlohmann_json)

target_include_directories(${TargetName} PUBLIC "include")
target_include_directories(${TargetName} PRIVATE "src")