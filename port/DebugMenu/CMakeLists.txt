set(TargetName "DebugMenu")

project(${TargetName})

option(ENABLE_GOOGLE_FONT_LOADER "Download and load fonts from Google Fonts API at startup" OFF)

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
	"src/DebugMemory.h"
	"src/DebugMemory.cpp"
	"src/DebugHero.h"
	"src/DebugHero.cpp"
	"src/DebugMeshViewerVulkan.h"
	"src/DebugMeshViewerVulkan.cpp"
	"src/DebugMenu.cpp"
	"src/DebugMenuLog.h"
	"src/DebugMenuLog.cpp"
	"src/DebugMenuInput.cpp"
	"src/DebugMenuWorld.h"
	"src/DebugMenuWorld.cpp"
	"src/DebugMenuDebugPanel.h"
	"src/DebugMenuDebugPanel.cpp"
	"src/DebugMenuLayout.h"
	"src/DebugMenuLayout.cpp"
	"src/DebugMenuToolbar.h"
	"src/DebugMenuToolbar.cpp"	
	"src/DebugHelpers.h"
	"src/DebugMaterial.h"
	"src/DebugEvent.cpp"
	"src/DebugEvent.h"
	"src/DebugShop.cpp"
	"src/DebugShop.h"
    "src/DebugScene.cpp"
	"src/DebugScene.h"
    "src/DebugRendering.cpp"
	"src/DebugRendering.h"
	"src/DebugScenario.cpp"
	"src/DebugScenario.h"
	"src/DebugProjection.h"
	"src/DebugCinematic.cpp"
	"src/DebugCinematic.h"
	"src/DebugProjection.cpp"
	"src/DebugMaterial.cpp"
	"src/DebugFrameBuffer.h"
	"src/DebugFrameBuffer.cpp"

	"src/DebugTexture.h"
	"src/DebugTexture.cpp"
	"src/DebugMesh.h"
	"src/DebugMesh.cpp"
	"src/DebugSaveLoad.h"
	"src/DebugSaveLoad.cpp"
	"src/DebugTutorial.h"
	"src/DebugTutorial.cpp"
	"src/DebugInput.h"
	"src/DebugInput.cpp"
	"src/DebugHeroReplay.h"
	"src/DebugHeroReplay.cpp"
	"src/Actor/DebugActorWolfen.h"
	"src/Actor/DebugActorWolfen.cpp"
	"src/DebugSetting.cpp"
	"src/DebugSetting.h"
	"src/Actor/DebugActor.h"
	"src/Actor/DebugActor.cpp"
	"src/Actor/DebugActorCollision.h"
	"src/Actor/DebugActorCollision.cpp"
	"src/DebugSceneryCollision.h"
	"src/DebugSceneryCollision.cpp"
	"src/DebugCollision.h"
	"src/DebugCollision.cpp"
	"src/Actor/DebugActorBehaviour.h"
	"src/Actor/DebugActorBehaviour.cpp"
	"src/Actor/DebugActorWind.h"
	"src/Actor/DebugActorWind.cpp"
	"src/Actor/DebugActorShip.h"
	"src/Actor/DebugActorShip.cpp"
	"src/Actor/DebugActorJamGut.h"
	"src/Actor/DebugActorJamGut.cpp"

	"src/Components/DebugVision.h"
	"src/Components/DebugVision.cpp"

	"src/DebugWorldNames.h"
	"src/DebugWorldNames.cpp"
	"src/DebugRendererVulkan.cpp"
	"include/DebugRenderer.h"
	"include/DebugMenu.h"
	"include/Callstack.h"
	"include/DebugPathDrawing.h"
)

add_library(${TargetName} OBJECT ${SOURCES})

add_subdirectory("ext/gizmo")
add_subdirectory("ext/magic_enum")

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory("ext/json")

target_link_libraries(${TargetName} PRIVATE Renderer Port Texture Log Kya Dbghelp.lib imgui Gizmo magic_enum nlohmann_json::nlohmann_json glm)

if(ENABLE_GOOGLE_FONT_LOADER)
	target_sources(${TargetName} PRIVATE "src/GoogleFontLoader.h" "src/GoogleFontLoader.cpp")
	target_link_libraries(${TargetName} PRIVATE winhttp.lib)
	target_compile_definitions(${TargetName} PRIVATE ENABLE_GOOGLE_FONT_LOADER)
endif()

target_include_directories(${TargetName} PUBLIC "include")
target_include_directories(${TargetName} PRIVATE "src")