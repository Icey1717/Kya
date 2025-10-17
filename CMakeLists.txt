cmake_minimum_required(VERSION 3.19.4)

# Override SPIRV-Reflect settings
OPTION(SPIRV_REFLECT_EXECUTABLE     "Build spirv-reflect executable" OFF)
OPTION(SPIRV_REFLECT_EXAMPLES       "Build stripper examples" OFF)
OPTION(SPIRV_REFLECT_STRIPPER       "Build stripper utility" OFF)
OPTION(SPIRV_REFLECT_STATIC_LIB     "Build a SPIRV-Reflect static library" ON)
OPTION(SPIRV_REFLECT_BUILD_TESTS    "Build the SPIRV-Reflect test suite" OFF)
OPTION(SPIRV_REFLECT_ENABLE_ASSERTS "Enable asserts for debugging" OFF)

# Performance profiling option
if(NOT DEFINED ENABLE_PERFORMANCE_PROFILING)
	set(ENABLE_PERFORMANCE_PROFILING OFF CACHE BOOL "Enable performance profiling")
endif()

# Memory profiling option
if(NOT DEFINED ENABLE_MEMORY_PROFILING)
	set(ENABLE_MEMORY_PROFILING OFF CACHE BOOL "Enable memory profiling")
endif()

#if either performance or memory profiling is enabled
if(ENABLE_PERFORMANCE_PROFILING OR ENABLE_MEMORY_PROFILING)
	# add global define for TRACY_ENABLE
	add_compile_definitions(TRACY_ENABLE)
	add_compile_definitions(TRACY_CALLSTACK=10)
endif()

include("src/port/ispc/ispc.cmake")

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang")


set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS YES)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

if (PS2)
else()
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang")


set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS YES)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

project(Kya)

message("Detecting ISPC")
message("$ENV{ISPC}")

if(NOT "$ENV{ISPC}" STREQUAL "")
	set("ENABLE_ISPC" ON)
	set(CMAKE_ISPC_COMPILER "$ENV{ISPC}")
else()
	set("ENABLE_ISPC" OFF)
endif()

if (PS2)
	message("PS2 Build")
	set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

	set(PLATFORM_BINARY_FOLDER "PS2")
else()
	message("Win Build")

	if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		message(STATUS "Enabling edit and continue")
		set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
		set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO")
		set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
	endif()

	enable_testing()

	add_subdirectory("port")

	set(PLATFORM_BINARY_FOLDER "WIN")
endif()

set(src_root "src")

set(main_src 
	"${src_root}/b-witch/ed3DScratchPadGlobalVar.cpp"
	"${src_root}/b-witch/ed3DScratchPadGlobalVar.h"
	"${src_root}/b-witch/kya.cpp"
	"${src_root}/b-witch/kya.h"
	"${src_root}/b-witch/Actor.cpp"
	"${src_root}/b-witch/Actor.h"
	"${src_root}/b-witch/ActorHero.cpp"
	"${src_root}/b-witch/ActorHero.h"
	"${src_root}/b-witch/IniFile.cpp"
	"${src_root}/b-witch/IniFile.h"
	"${src_root}/b-witch/CompatibilityHandlingPS2.cpp"
	"${src_root}/b-witch/CompatibilityHandlingPS2.h"
	"${src_root}/b-witch/edStr.cpp"
	"${src_root}/b-witch/edStr.h"
	"${src_root}/b-witch/edPacket.cpp"
	"${src_root}/b-witch/edPacket.h"
	"${src_root}/b-witch/edMem.cpp"
	"${src_root}/b-witch/edMem.h"
	"${src_root}/b-witch/edText.cpp"
	"${src_root}/b-witch/edText.h"
	"${src_root}/b-witch/CollisionRay.cpp"
	"${src_root}/b-witch/CollisionRay.h"
	"${src_root}/b-witch/EdFileBase.cpp"
	"${src_root}/b-witch/EdFileBase.h"
	"${src_root}/b-witch/Cheat.cpp"
	"${src_root}/b-witch/Cheat.h"
	"${src_root}/b-witch/ed3D.cpp"
	"${src_root}/b-witch/ed3D.h"
	"${src_root}/b-witch/Squad.cpp"
	"${src_root}/b-witch/Squad.h"
	"${src_root}/b-witch/ChessBoard.cpp"
	"${src_root}/b-witch/ChessBoard.h"
	"${src_root}/b-witch/CameraFightData.cpp"
	"${src_root}/b-witch/CameraFightData.h"
	"${src_root}/b-witch/edDlist.cpp"
	"${src_root}/b-witch/edDlist.h"
	"${src_root}/b-witch/edList.cpp"
	"${src_root}/b-witch/edList.h"
	"${src_root}/b-witch/profile.cpp"
	"${src_root}/b-witch/profile.h"
	"${src_root}/b-witch/Debug.cpp"
	"${src_root}/b-witch/Debug.h"
	"${src_root}/b-witch/PackedFont.h"
	"${src_root}/b-witch/LargeObject.cpp"
	"${src_root}/b-witch/LargeObject.h"
	"${src_root}/b-witch/BootData.cpp"
	"${src_root}/b-witch/BootData.h"
	"${src_root}/b-witch/SaveManagement.cpp"
	"${src_root}/b-witch/SaveManagement.h"
	"${src_root}/b-witch/MemoryStream.cpp"
	"${src_root}/b-witch/MemoryStream.h"
	"${src_root}/b-witch/SpriteWidget.cpp"
	"${src_root}/b-witch/SpriteWidget.h"
	"${src_root}/b-witch/LevelScheduleManager.cpp"
	"${src_root}/b-witch/LevelScheduleManager.h"
	"${src_root}/b-witch/EventManager.cpp"
	"${src_root}/b-witch/EventManager.h"
	"${src_root}/b-witch/Frontend.cpp"
	"${src_root}/b-witch/Frontend.h"
	"${src_root}/b-witch/FrontEndDisp.cpp"
	"${src_root}/b-witch/FrontEndDisp.h"
	"${src_root}/b-witch/FrontEndLife.cpp"
	"${src_root}/b-witch/FrontEndLife.h"
    "${src_root}/b-witch/FrontEndMagic.cpp"
	"${src_root}/b-witch/FrontEndMagic.h"
	"${src_root}/b-witch/FrontEndInventory.cpp"
	"${src_root}/b-witch/FrontEndInventory.h"
	"${src_root}/b-witch/FrontEndEnemy.cpp"
	"${src_root}/b-witch/FrontEndEnemy.h"
	"${src_root}/b-witch/FrontEndMoney.cpp"
	"${src_root}/b-witch/FrontEndMoney.h"
	"${src_root}/b-witch/FrontEndBank.cpp"
	"${src_root}/b-witch/FrontEndBank.h"
	"${src_root}/b-witch/FrontEndWidget.cpp"
	"${src_root}/b-witch/FrontEndWidget.h"
	"${src_root}/b-witch/EventTrack.cpp"
	"${src_root}/b-witch/EventTrack.h"
	"${src_root}/b-witch/FileManager3D.cpp"
	"${src_root}/b-witch/FileManager3D.h"
	"${src_root}/b-witch/Pause.cpp"
	"${src_root}/b-witch/Pause.h"
	"${src_root}/b-witch/SectorManager.cpp"
	"${src_root}/b-witch/SectorManager.h"
	"${src_root}/b-witch/DlistManager.cpp"
	"${src_root}/b-witch/DlistManager.h"
	"${src_root}/b-witch/ActorManager.cpp"
	"${src_root}/b-witch/ActorManager.h"
	"${src_root}/b-witch/AnmManager.cpp"
	"${src_root}/b-witch/AnmManager.h"
	"${src_root}/b-witch/SwitchBehaviour.cpp"
	"${src_root}/b-witch/SwitchBehaviour.h"
	"${src_root}/b-witch/edMathTCBSpline.cpp"
	"${src_root}/b-witch/edMathTCBSpline.h"
	"${src_root}/b-witch/CinematicManager.cpp"
	"${src_root}/b-witch/CinematicManager.h"
	"${src_root}/b-witch/InventoryInfo.cpp"
	"${src_root}/b-witch/InventoryInfo.h"
	"${src_root}/b-witch/Vision.cpp"
	"${src_root}/b-witch/Vision.h"
	"${src_root}/b-witch/CameraGame.cpp"
	"${src_root}/b-witch/CameraGame.h"
	"${src_root}/b-witch/CameraCinematic.cpp"
	"${src_root}/b-witch/CameraCinematic.h"
	"${src_root}/b-witch/CameraMouseQuake.cpp"
	"${src_root}/b-witch/CameraMouseQuake.h"
	"${src_root}/b-witch/CameraFixePerso.cpp"
	"${src_root}/b-witch/CameraFixePerso.h"
	"${src_root}/b-witch/CameraFixe.cpp"
	"${src_root}/b-witch/CameraFixe.h"
	"${src_root}/b-witch/CameraRail.cpp"
	"${src_root}/b-witch/CameraRail.h"
	"${src_root}/b-witch/CameraBoomy.cpp"
	"${src_root}/b-witch/CameraBoomy.h"
	"${src_root}/b-witch/camera.cpp"
	"${src_root}/b-witch/camera.h"
	"${src_root}/b-witch/CameraViewManager.cpp"
	"${src_root}/b-witch/CameraViewManager.h"
	"${src_root}/b-witch/ScenaricCondition.cpp"
	"${src_root}/b-witch/ScenaricCondition.h"
	"${src_root}/b-witch/PoolAllocators.cpp"
	"${src_root}/b-witch/PoolAllocators.h"
	"${src_root}/b-witch/InputManager.cpp"
	"${src_root}/b-witch/InputManager.h"
	"${src_root}/b-witch/ActorFactory.cpp"
	"${src_root}/b-witch/ActorFactory.h"
	"${src_root}/Rendering/DisplayList.cpp"
	"${src_root}/Rendering/DisplayList.h"
	"${src_root}/Rendering/DisplayListData.cpp"
	"${src_root}/Rendering/DisplayListData.h"
	"${src_root}/Rendering/CameraPanMasterHeader.cpp"
	"${src_root}/Rendering/CameraPanMasterHeader.h"
	"${src_root}/Rendering/CustomShell.cpp"
	"${src_root}/Rendering/CustomShell.h"
	"${src_root}/Rendering/Font.cpp"
	"${src_root}/Rendering/Font.h"
	"${src_root}/Rendering/edCTextStyle.cpp"
	"${src_root}/Rendering/edCTextStyle.h"
	"${src_root}/Rendering/edCTextFormat.cpp"
	"${src_root}/Rendering/edCTextFormat.h"
	"${src_root}/Rendering/edCTextFont.cpp"
	"${src_root}/Rendering/edCTextFont.h"
	"${src_root}/Rendering/OneTimeCommands.h"
	"${src_root}/edC/edCBank.cpp"
	"${src_root}/edC/edCBank.h"
	"${src_root}/edC/edCBankBuffer.cpp"
	"${src_root}/edC/edCBankBuffer.h"
	"${src_root}/edC/edCFiler.cpp"
	"${src_root}/edC/edCFiler.h"
	"${src_root}/edC/edCFiler_Static.cpp"
	"${src_root}/edC/edCFiler_Static.h"
	"${src_root}/b-witch/MathOps.cpp"
	"${src_root}/b-witch/MathOps.h"
	"${src_root}/b-witch/TimeController.cpp"
	"${src_root}/b-witch/TimeController.h"
	"${src_root}/b-witch/LocalizationManager.cpp"
	"${src_root}/b-witch/LocalizationManager.h"
	"${src_root}/b-witch/LightManager.cpp"
	"${src_root}/b-witch/LightManager.h"
	"${src_root}/b-witch/TranslatedTextData.cpp"
	"${src_root}/b-witch/TranslatedTextData.h"
	"${src_root}/b-witch/MenuMessageBox.cpp"
	"${src_root}/b-witch/MenuMessageBox.h"
	"${src_root}/b-witch/PathManager.cpp"
	"${src_root}/b-witch/PathManager.h"
	"${src_root}/b-witch/Path.cpp"
	"${src_root}/b-witch/Path.h"
	"${src_root}/b-witch/PathFollow.cpp"
	"${src_root}/b-witch/PathFollow.h"
	"${src_root}/b-witch/PathDynamic.cpp"
	"${src_root}/b-witch/PathDynamic.h"
	"${src_root}/b-witch/PathFinder.cpp"
	"${src_root}/b-witch/PathFinder.h"
	"${src_root}/b-witch/PathFinderClient.cpp"
	"${src_root}/b-witch/PathFinderClient.h"
	"${src_root}/b-witch/NaviMesh.cpp"
	"${src_root}/b-witch/NaviMesh.h"
	"${src_root}/b-witch/NaviCell.cpp"
	"${src_root}/b-witch/NaviCell.h"
	"${src_root}/b-witch/Settings.cpp"
	"${src_root}/b-witch/Settings.h"
	"${src_root}/b-witch/WayPoint.cpp"
	"${src_root}/b-witch/WayPoint.h"
	"${src_root}/b-witch/MapManager.cpp"
	"${src_root}/b-witch/MapManager.h"
	"${src_root}/b-witch/Animation.cpp"
	"${src_root}/b-witch/Animation.h"
	"${src_root}/b-witch/CollisionManager.cpp"
	"${src_root}/b-witch/CollisionManager.h"
	"${src_root}/b-witch/Dynamic.cpp"
	"${src_root}/b-witch/Dynamic.h"

	"${src_root}/b-witch/Fx.cpp"
	"${src_root}/b-witch/Fx.h"
	"${src_root}/b-witch/Fx_Tail.cpp"
	"${src_root}/b-witch/Fx_Tail.h"
	"${src_root}/b-witch/Fx_Spark.cpp"
	"${src_root}/b-witch/Fx_Spark.h"
	"${src_root}/b-witch/FxLightEmitter.cpp"
	"${src_root}/b-witch/FxLightEmitter.h"
	"${src_root}/b-witch/FxParticle.cpp"
	"${src_root}/b-witch/FxParticle.h"
	"${src_root}/b-witch/FxLod.cpp"
	"${src_root}/b-witch/FxLod.h"
	"${src_root}/b-witch/FxSound.cpp"
	"${src_root}/b-witch/FxSound.h"
	"${src_root}/b-witch/FxPath.cpp"
	"${src_root}/b-witch/FxPath.h"
	"${src_root}/b-witch/FxGroup.cpp"
	"${src_root}/b-witch/FxGroup.h"
	"${src_root}/b-witch/FxComposite.cpp"
	"${src_root}/b-witch/FxComposite.h"
	"${src_root}/b-witch/FxRandom.cpp"
	"${src_root}/b-witch/FxRandom.h"

	"${src_root}/edC/edCFiler_BNK.cpp"
	"${src_root}/edC/edCFiler_BNK.h"
	"${src_root}/EdenLib/edText/sources/edTextResources.cpp"
	"${src_root}/EdenLib/edText/sources/edTextResources.h"
	"${src_root}/EdenLib/edCinematic/Sources/Cinematic.cpp"
	"${src_root}/EdenLib/edCinematic/Sources/Cinematic.h"
	"${src_root}/EdenLib/edCinematic/Sources/CinResCollection.cpp"
	"${src_root}/EdenLib/edCinematic/Sources/CinResCollection.h"
	"${src_root}/EdenLib/edCinematic/Sources/CinScene.cpp"
	"${src_root}/EdenLib/edCinematic/Sources/CinScene.h"
	"${src_root}/EdenLib/edAnim/AnmSkeleton.cpp"
	"${src_root}/EdenLib/edAnim/AnmSkeleton.h"
	"${src_root}/EdenLib/edCollision/sources/edCollisions.cpp"
	"${src_root}/EdenLib/edCollision/sources/edCollisions.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevScanPsxPorts.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevScanPsxPorts.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevScanPorts.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevScanPorts.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevInitRoot.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevInitRoot.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevInitLibs.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevInitLibs.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevFindPsxDevice.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevFindPsxDevice.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevDualShock2.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevDualShock2.h"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevAttachPsxPort.cpp"
	"${src_root}/EdenLib/edDev/Sources/ps2/_edDevAttachPsxPort.h"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRouteRates.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRouteRates.h"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRepeatRate.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRepeatRate.h"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRepeatDelay.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRepeatDelay.h"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRates.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevSetRates.h"
	"${src_root}/EdenLib/edDev/Sources/edDevReadUpdate.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevReadUpdate.h"
	"${src_root}/EdenLib/edDev/Sources/edDevInit.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevInit.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteType.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteType.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteReleased.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteReleased.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRoutePressed.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRoutePressed.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteClick.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteClick.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteAnalog.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetRouteAnalog.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanAnalog.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanAnalog.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetNumPorts.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetNumPorts.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetNumChans.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetNumChans.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetDevType.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetDevType.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetDevConected.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetDevConected.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanType.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanType.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanPressed.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanPressed.h"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanClick.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevGetChanClick.h"
	"${src_root}/EdenLib/edDev/Sources/edDevAddRouteList.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevAddRouteList.h"
	"${src_root}/EdenLib/edDev/Sources/edDevAddRoute.cpp"
	"${src_root}/EdenLib/edDev/Sources/edDevAddRoute.h"
	"${src_root}/EdenLib/edDev/Sources/edDev.h"

	"${src_root}/EdenLib/edBank/sources/edBank.cpp"
	"${src_root}/EdenLib/edBank/sources/edBankBuffer.cpp"
	"${src_root}/EdenLib/edBank/sources/edBankDebug.cpp"
	"${src_root}/EdenLib/edBank/sources/edBankFile.cpp"
	"${src_root}/EdenLib/edBank/sources/edBankFiler.cpp"
	"${src_root}/EdenLib/edBank/sources/edBankStackFile.cpp"

	"${src_root}/EdenLib/edBank/include/edBank.h"
	"${src_root}/EdenLib/edBank/include/edBankBuffer.h"
	"${src_root}/EdenLib/edBank/include/edBankDebug.h"
	"${src_root}/EdenLib/edBank/include/edBankFile.h"
	"${src_root}/EdenLib/edBank/include/edBankFiler.h"
	"${src_root}/EdenLib/edBank/include/edBankStackFile.h"

	"${src_root}/EdenLib/edFile/include/edFileFiler.h"
	"${src_root}/EdenLib/edFile/include/edFile.h"
	"${src_root}/EdenLib/edFile/include/edFilePath.h"
	"${src_root}/EdenLib/edFile/include/edFileCRC32.h"
	"${src_root}/EdenLib/edFile/include/edFileNoWaitStack.h"
	"${src_root}/EdenLib/edFile/include/ps2/_edFileFilerCDVD.h"
	"${src_root}/EdenLib/edFile/include/ps2/_edFileFilerMCard.h"

	"${src_root}/EdenLib/edFile/sources/edFileFiler.cpp"
	"${src_root}/EdenLib/edFile/sources/edFile.cpp"
	"${src_root}/EdenLib/edFile/sources/edFilePath.cpp"
	"${src_root}/EdenLib/edFile/sources/edFileCRC32.cpp"
	"${src_root}/EdenLib/edFile/sources/edFileNoWaitStack.cpp"
	"${src_root}/EdenLib/edFile/sources/ps2/_edFileFilerCDVD.cpp"
	"${src_root}/EdenLib/edFile/sources/ps2/_edFileFilerMCard.cpp"

	"${src_root}/EdenLib/edSys/sources/EdSystem.cpp"
	"${src_root}/EdenLib/edSys/sources/EdSystem.h"
	"${src_root}/EdenLib/edSys/sources/EdHandlers.cpp"
	"${src_root}/EdenLib/edSys/sources/EdHandlers.h"
	"${src_root}/EdenLib/edSys/sources/ps2/_edSystem.cpp"
	"${src_root}/EdenLib/edSys/sources/ps2/_edSystem.h"

	"${src_root}/EdenLib/edParticles/sources/edParticleShaper.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticleSelector.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticles2.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticles.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticleGroup.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticleGenerator.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticleEffector.cpp"
	"${src_root}/EdenLib/edParticles/sources/edParticle.cpp"
	"${src_root}/EdenLib/edParticles/include/edParticles.h"

	"${src_root}/edVideo/VideoA.cpp"
	"${src_root}/edVideo/VideoA.h"
	"${src_root}/edVideo/VideoB.cpp"
	"${src_root}/edVideo/VideoB.h"
	"${src_root}/edVideo/VideoC.cpp"
	"${src_root}/edVideo/VideoC.h"
	"${src_root}/edVideo/VideoD.cpp"
	"${src_root}/edVideo/VideoD.h"
	"${src_root}/edVideo/Viewport.cpp"
	"${src_root}/edVideo/Viewport.h"
	"${src_root}/edVideo/CameraStack.cpp"
	"${src_root}/edVideo/CameraStack.h"
	"${src_root}/port/pointer_conv.cpp"
	"${src_root}/port/pointer_conv.h"
)

set (actor_src
	"${src_root}/b-witch/Actor_Cinematic.cpp"
	"${src_root}/b-witch/Actor_Cinematic.h"
	"${src_root}/b-witch/ActorAutonomous.cpp"
	"${src_root}/b-witch/ActorAutonomous.h"
	"${src_root}/b-witch/ActorBoomy.cpp"
	"${src_root}/b-witch/ActorBoomy.h"
	"${src_root}/b-witch/ActorHeroPrivate.cpp"
	"${src_root}/b-witch/ActorHeroPrivate.h"
	"${src_root}/b-witch/ActorHero_Inventory.cpp"
	"${src_root}/b-witch/ActorHero_Inventory.h"
	"${src_root}/b-witch/ActorMicken.cpp"
	"${src_root}/b-witch/ActorMicken.h"
	"${src_root}/b-witch/ActorWolfen.cpp"
	"${src_root}/b-witch/ActorWolfen.h"
	"${src_root}/b-witch/ActorNativ.cpp"
	"${src_root}/b-witch/ActorNativ.h"
	"${src_root}/b-witch/ActorMovingPlatform.cpp"
	"${src_root}/b-witch/ActorMovingPlatform.h"
	"${src_root}/b-witch/ActorMovable.cpp"
	"${src_root}/b-witch/ActorMovable.h"
	"${src_root}/b-witch/ActorBonus.cpp"
	"${src_root}/b-witch/ActorBonus.h"
	"${src_root}/b-witch/ActorRope.cpp"
	"${src_root}/b-witch/ActorRope.h"
	"${src_root}/b-witch/ActorFighter.cpp"
	"${src_root}/b-witch/ActorFighter.h"
	"${src_root}/b-witch/ActorGravityAware.cpp"
	"${src_root}/b-witch/ActorGravityAware.h"
	"${src_root}/b-witch/ActorSwitch.cpp"
	"${src_root}/b-witch/ActorSwitch.h"
	"${src_root}/b-witch/ActorWind.cpp"
	"${src_root}/b-witch/ActorWind.h"
	"${src_root}/b-witch/ActorCredits.cpp"
	"${src_root}/b-witch/ActorCredits.h"
	"${src_root}/b-witch/ActorCommander.cpp"
	"${src_root}/b-witch/ActorCommander.h"
	"${src_root}/b-witch/ActorAmbre.cpp"
	"${src_root}/b-witch/ActorAmbre.h"
	"${src_root}/b-witch/ActorFruit.cpp"
	"${src_root}/b-witch/ActorFruit.h"
	"${src_root}/b-witch/ActorBridge.cpp"
	"${src_root}/b-witch/ActorBridge.h"
	"${src_root}/b-witch/ActorClusteriser.cpp"
	"${src_root}/b-witch/ActorClusteriser.h"
	"${src_root}/b-witch/ActorCompanion.cpp"
	"${src_root}/b-witch/ActorCompanion.h"
	"${src_root}/b-witch/ActorDCA.cpp"
	"${src_root}/b-witch/ActorDCA.h"
	"${src_root}/b-witch/ActorProjectile.cpp"
	"${src_root}/b-witch/ActorProjectile.h"
	"${src_root}/b-witch/ActorCheckpointManager.cpp"
	"${src_root}/b-witch/ActorCheckpointManager.h"
	"${src_root}/b-witch/ActorWeapon.cpp"
	"${src_root}/b-witch/ActorWeapon.h"
	"${src_root}/b-witch/ActorAraignos.cpp"
	"${src_root}/b-witch/ActorAraignos.h"
	"${src_root}/b-witch/ActorTeleporter.cpp"
	"${src_root}/b-witch/ActorTeleporter.h"
	"${src_root}/b-witch/ActorAmortos.cpp"
	"${src_root}/b-witch/ActorAmortos.h"
	"${src_root}/b-witch/ActorTrap.cpp"
	"${src_root}/b-witch/ActorTrap.h"
	"${src_root}/b-witch/ActorFx.cpp"
	"${src_root}/b-witch/ActorFx.h"
	"${src_root}/b-witch/ActorMoney.cpp"
	"${src_root}/b-witch/ActorMoney.h"
	"${src_root}/b-witch/ActorBox.cpp"
	"${src_root}/b-witch/ActorBox.h"
	"${src_root}/b-witch/ActorAton.cpp"
	"${src_root}/b-witch/ActorAton.h"
	"${src_root}/b-witch/ActorBasicBox.cpp"
	"${src_root}/b-witch/ActorBasicBox.h"
	"${src_root}/b-witch/ActorNativCmd.cpp"
	"${src_root}/b-witch/ActorNativCmd.h"
	"${src_root}/b-witch/ActorFogManager.cpp"
	"${src_root}/b-witch/ActorFogManager.h"
	"${src_root}/b-witch/ActorShoot.cpp"
	"${src_root}/b-witch/ActorShoot.h"
	"${src_root}/b-witch/ActorShocker.cpp"
	"${src_root}/b-witch/ActorShocker.h"
	"${src_root}/b-witch/ActorBlazer.cpp"
	"${src_root}/b-witch/ActorBlazer.h"
	"${src_root}/b-witch/ActorBunch.cpp"
	"${src_root}/b-witch/ActorBunch.h"
	"${src_root}/b-witch/ActorBrazul.cpp"
	"${src_root}/b-witch/ActorBrazul.h"
	"${src_root}/b-witch/ActorStiller.cpp"
	"${src_root}/b-witch/ActorStiller.h"
	"${src_root}/b-witch/ActorShockerCmd.cpp"
	"${src_root}/b-witch/ActorShockerCmd.h"
	"${src_root}/b-witch/ActorRune.cpp"
	"${src_root}/b-witch/ActorRune.h"
	"${src_root}/b-witch/ActorBonusFountain.cpp"
	"${src_root}/b-witch/ActorBonusFountain.h"
	"${src_root}/b-witch/ActorEventGenerator.cpp"
	"${src_root}/b-witch/ActorEventGenerator.h"
	"${src_root}/b-witch/ActorWoof.cpp"
	"${src_root}/b-witch/ActorWoof.h"
	"${src_root}/b-witch/ActorExplosiveDistributor.cpp"
	"${src_root}/b-witch/ActorExplosiveDistributor.h"
	"${src_root}/b-witch/ActorElectrolla.cpp"
	"${src_root}/b-witch/ActorElectrolla.h"
	"${src_root}/b-witch/ActorHedgehog.cpp"
	"${src_root}/b-witch/ActorHedgehog.h"
	"${src_root}/b-witch/ActorNoseMonster.cpp"
	"${src_root}/b-witch/ActorNoseMonster.h"
	"${src_root}/b-witch/ActorWoodMonster.cpp"
	"${src_root}/b-witch/ActorWoodMonster.h"
	"${src_root}/b-witch/ActorHelperSign.cpp"
	"${src_root}/b-witch/ActorHelperSign.h"
	"${src_root}/b-witch/ActorEgg.cpp"
	"${src_root}/b-witch/ActorEgg.h"
	"${src_root}/b-witch/ActorAmortosCmd.cpp"
	"${src_root}/b-witch/ActorAmortosCmd.h"
	"${src_root}/b-witch/ActorPunchingBall.cpp"
	"${src_root}/b-witch/ActorPunchingBall.h"
	"${src_root}/b-witch/ActorNativShop.cpp"
	"${src_root}/b-witch/ActorNativShop.h"
	"${src_root}/b-witch/ActorAddOnPurchase.cpp"
	"${src_root}/b-witch/ActorAddOnPurchase.h"
	"${src_root}/b-witch/ActorJamgut.cpp"
	"${src_root}/b-witch/ActorJamgut.h"
	"${src_root}/b-witch/ActorShip.cpp"
	"${src_root}/b-witch/ActorShip.h"
	"${src_root}/b-witch/ActorCareBoy.cpp"
	"${src_root}/b-witch/ActorCareBoy.h"
	"${src_root}/b-witch/ActorHunter.cpp"
	"${src_root}/b-witch/ActorHunter.h"
	"${src_root}/b-witch/ActorAcceleratos.cpp"
	"${src_root}/b-witch/ActorAcceleratos.h"
	"${src_root}/b-witch/ActorPattern.cpp"
	"${src_root}/b-witch/ActorPattern.h"
	"${src_root}/b-witch/ActorPatternCmd.cpp"
	"${src_root}/b-witch/ActorPatternCmd.h"
	"${src_root}/b-witch/ActorMiniGame.cpp"
	"${src_root}/b-witch/ActorMiniGame.h"
	"${src_root}/b-witch/ActorMiniGamesOrganizer.cpp"
	"${src_root}/b-witch/ActorMiniGamesOrganizer.h"
	"${src_root}/b-witch/ActorMiniGamesManager.cpp"
	"${src_root}/b-witch/ActorMiniGamesManager.h"
	"${src_root}/b-witch/ActorMiniGameBoxCounter.cpp"
	"${src_root}/b-witch/ActorMiniGameBoxCounter.h"
	"${src_root}/b-witch/ActorMiniGameTimeAttack.cpp"
	"${src_root}/b-witch/ActorMiniGameTimeAttack.h"
	"${src_root}/b-witch/ActorMiniGameDistance.cpp"
	"${src_root}/b-witch/ActorMiniGameDistance.h"
	"${src_root}/b-witch/ActorMiniGameBoomy.cpp"
	"${src_root}/b-witch/ActorMiniGameBoomy.h"
	"${src_root}/b-witch/ActorBombLauncher.cpp"
	"${src_root}/b-witch/ActorBombLauncher.h"
	"${src_root}/b-witch/ActorWantedZoo.cpp"
	"${src_root}/b-witch/ActorWantedZoo.h"
)

list(APPEND main_src ${actor_src})

set(ps2_src_root "ps2")

if (PS2)
	list(APPEND main_src	
		"${ps2_src_root}/ezmpegstr/ldimage.c"
		"${ps2_src_root}/ezmpegstr/playpss.c"
		"${ps2_src_root}/ezmpegstr/playpss.h"
		"${ps2_src_root}/ezmpegstr/audiodec.c"
		"${ps2_src_root}/ezmpegstr/audiodec.h"
		"${ps2_src_root}/ezmpegstr/strfile.c"
		"${ps2_src_root}/ezmpegstr/strfile.h"
		"${ps2_src_root}/ezmpegstr/ezmpegstr.h"

		"/usr/local/sce/ee/src/lib/vu0/libvu0.c"
	)
else()
	list(APPEND main_src	
		"${src_root}/port/vu1_emu.cpp"
		"${src_root}/port/vu1_emu.h"
		"${src_root}/port/hardware_draw.cpp"
		"${src_root}/port/hardware_draw.h"
		"${src_root}/port/input.cpp"
		"${src_root}/port/input.h"
	)
endif()

if (${ENABLE_ISPC})
	# Specify your ISPC source files
	set(ISPC_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/src/port/ispc/simple.ispc"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/port/ispc/vtx.ispc"
	)

	# Compile ISPC sources
	compile_ispc(ISPC_OBJECTS TARGET Kya SOURCES ${ISPC_SOURCES})
endif()

# add the executable
if (PS2)
	add_executable(Kya 
		${main_src}
	)
else()
	add_library(Kya 
		${main_src}
	)
endif()

if (PS2)
	message("Using src directory: ${CMAKE_CURRENT_SOURCE_DIR}")

	add_custom_target(compile_crt0
	COMMAND ${CMAKE_C_COMPILER} -c -xassembler-with-cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/ps2/out/crt0.o ${CMAKE_CURRENT_SOURCE_DIR}/ps2/crt0.s
	BYPRODUCTS "${CMAKE_CURRENT_SOURCE_DIR}/ps2/out/crt0.o"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ps2/crt0.s"
	COMMENT "Compiling crt0.s"
	)
	
	add_dependencies(Kya compile_crt0)

	string(APPEND CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/ps2/app.cmd ${CMAKE_CURRENT_SOURCE_DIR}/ps2/out/crt0.o -mno-crt0")
	set_target_properties(Kya PROPERTIES OUTPUT_NAME "kya.elf")
	set_target_properties(Kya PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}")
	#set(CMAKE_VERBOSE_MAKEFILE on)

	message("Making ISO")

	add_custom_command(TARGET Kya POST_BUILD
		COMMAND cp -r "${CMAKE_CURRENT_SOURCE_DIR}/assets/*" "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}/"
		COMMAND mkisofs -o "${CMAKE_CURRENT_SOURCE_DIR}/kya.iso" "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}"
	)
else()
	set(KYA_EXECUTABLE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}")
	message("Making Exe")

	add_custom_target(sync_files
		COMMAND ${CMAKE_COMMAND} -E echo "Syncing files using robocopy..."
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/DoRobo" "${CMAKE_CURRENT_SOURCE_DIR}/assets/" "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}/"
	)

	add_dependencies(Kya sync_files)
endif()

target_include_directories(Kya PRIVATE "${src_root}" "${src_root}/b-witch" "${src_root}/EdenLib/edBank/include" "${src_root}/EdenLib/edFile/include" "${src_root}/EdenLib/edParticles/include")

if (PS2)
	target_include_directories(Kya PRIVATE ${ps2_src_root})
endif()

if (PS2)
	set(EXECUTABLE_OUTPUT_PATH ${src_root})

	target_compile_definitions(Kya PRIVATE PLATFORM_PS2)

	target_include_directories(Kya PRIVATE	"/usr/local/sce/ee/include"
											"/usr/local/sce/common/include"
											"/usr/local/sce/Shell/Include")

	target_link_directories(Kya PRIVATE		"/usr/local/sce/ee/lib"
											"/usr/local/sce/Shell/Lib")

	target_link_libraries(Kya PRIVATE "sdr" "dma" "cdvd" "graph" "pkt" "dbc" "pad2" "shell" "scf" "mpeg" "ipu" "dev" "vu0" "mc" "pc")

	target_compile_options(Kya PRIVATE "-w")

else()
	target_compile_features(Kya PUBLIC cxx_std_17)
	target_compile_definitions(Kya PUBLIC "PLATFORM_WIN")

	target_compile_options(Kya PUBLIC "-Wno-c++11-narrowing" "-w")

	target_include_directories(Kya PUBLIC 
		"${src_root}" "${src_root}/b-witch"
	)

	target_link_libraries(Kya PUBLIC Port Log VU Job steinwurf::recycle)

	if (ENABLE_ISPC)
		target_sources(Kya PRIVATE ${ISPC_OBJECTS})
		target_include_directories(Kya PRIVATE "${ISPC_INCLUDE_DIR}")
		target_compile_definitions(Kya PRIVATE "ENABLE_ISPC")
	endif()

	if (ENABLE_PERFORMANCE_PROFILING)
		target_compile_definitions(Kya PUBLIC "ENABLE_PERFORMANCE_PROFILING")
	endif()

	if (ENABLE_MEMORY_PROFILING)
		target_compile_definitions(Kya PUBLIC "ENABLE_MEMORY_PROFILING")
	endif()

	target_sources(Kya PRIVATE debug/PackedType.natvis)
endif()

set_property( GLOBAL PROPERTY VS_STARTUP_PROJECT Kya )

option(SKIP_MOVIES "Skip into cinematics" OFF)
option(LOAD_FOREVER "Holds on the loading screen forever" OFF)

if (SKIP_MOVIES OR PLATFORM_PS2)
	#target_compile_definitions(Kya PUBLIC SKIP_MOVIES)
endif()

target_compile_definitions(Kya PUBLIC SKIP_MOVIES DEBUG_FEATURES)

if (LOAD_FOREVER)
	target_compile_definitions(Kya PUBLIC LOAD_FOREVER)
endif()

if (NOT PS2)
	add_subdirectory("port/Windows/Host")
	add_subdirectory("port/Test")
endif()