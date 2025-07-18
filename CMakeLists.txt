cmake_minimum_required(VERSION 3.19.4)

# Override SPIRV-Reflect settings
OPTION(SPIRV_REFLECT_EXECUTABLE     "Build spirv-reflect executable" OFF)
OPTION(SPIRV_REFLECT_EXAMPLES       "Build stripper examples" OFF)
OPTION(SPIRV_REFLECT_STRIPPER       "Build stripper utility" OFF)
OPTION(SPIRV_REFLECT_STATIC_LIB     "Build a SPIRV-Reflect static library" ON)
OPTION(SPIRV_REFLECT_BUILD_TESTS    "Build the SPIRV-Reflect test suite" OFF)
OPTION(SPIRV_REFLECT_ENABLE_ASSERTS "Enable asserts for debugging" OFF)

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
	"${src_root}/ed3DScratchPadGlobalVar.cpp"
	"${src_root}/ed3DScratchPadGlobalVar.h"
	"${src_root}/kya.cpp"
	"${src_root}/kya.h"
	"${src_root}/Actor.cpp"
	"${src_root}/Actor.h"
	"${src_root}/ActorHero.cpp"
	"${src_root}/ActorHero.h"
	"${src_root}/IniFile.cpp"
	"${src_root}/IniFile.h"
	"${src_root}/CompatibilityHandlingPS2.cpp"
	"${src_root}/CompatibilityHandlingPS2.h"
	"${src_root}/edStr.cpp"
	"${src_root}/edStr.h"
	"${src_root}/edPacket.cpp"
	"${src_root}/edPacket.h"
	"${src_root}/edMem.cpp"
	"${src_root}/edMem.h"
	"${src_root}/edText.cpp"
	"${src_root}/edText.h"
	"${src_root}/CollisionRay.cpp"
	"${src_root}/CollisionRay.h"
	"${src_root}/EdFileBase.cpp"
	"${src_root}/EdFileBase.h"
	"${src_root}/Cheat.cpp"
	"${src_root}/Cheat.h"
	"${src_root}/ed3D.cpp"
	"${src_root}/ed3D.h"
	"${src_root}/Squad.cpp"
	"${src_root}/Squad.h"
	"${src_root}/ChessBoard.cpp"
	"${src_root}/ChessBoard.h"
	"${src_root}/CameraFightData.cpp"
	"${src_root}/CameraFightData.h"
	"${src_root}/edDlist.cpp"
	"${src_root}/edDlist.h"
	"${src_root}/edList.cpp"
	"${src_root}/edList.h"
	"${src_root}/profile.cpp"
	"${src_root}/profile.h"
	"${src_root}/Debug.cpp"
	"${src_root}/Debug.h"
	"${src_root}/PackedFont.h"
	"${src_root}/LargeObject.cpp"
	"${src_root}/LargeObject.h"
	"${src_root}/BootData.cpp"
	"${src_root}/BootData.h"
	"${src_root}/SaveManagement.cpp"
	"${src_root}/SaveManagement.h"
	"${src_root}/MemoryStream.cpp"
	"${src_root}/MemoryStream.h"
	"${src_root}/SpriteWidget.cpp"
	"${src_root}/SpriteWidget.h"
	"${src_root}/LevelScheduleManager.cpp"
	"${src_root}/LevelScheduleManager.h"
	"${src_root}/EventManager.cpp"
	"${src_root}/EventManager.h"
	"${src_root}/Frontend.cpp"
	"${src_root}/Frontend.h"
	"${src_root}/FrontEndDisp.cpp"
	"${src_root}/FrontEndDisp.h"
	"${src_root}/FrontEndLife.cpp"
	"${src_root}/FrontEndLife.h"
	"${src_root}/FrontEndEnemy.cpp"
	"${src_root}/FrontEndEnemy.h"
	"${src_root}/FrontEndMoney.cpp"
	"${src_root}/FrontEndMoney.h"
	"${src_root}/FrontEndBank.cpp"
	"${src_root}/FrontEndBank.h"
	"${src_root}/FrontEndWidget.cpp"
	"${src_root}/FrontEndWidget.h"
	"${src_root}/EventTrack.cpp"
	"${src_root}/EventTrack.h"
	"${src_root}/FileManager3D.cpp"
	"${src_root}/FileManager3D.h"
	"${src_root}/Pause.cpp"
	"${src_root}/Pause.h"
	"${src_root}/SectorManager.cpp"
	"${src_root}/SectorManager.h"
	"${src_root}/DlistManager.cpp"
	"${src_root}/DlistManager.h"
	"${src_root}/ActorManager.cpp"
	"${src_root}/ActorManager.h"
	"${src_root}/AnmManager.cpp"
	"${src_root}/AnmManager.h"
	"${src_root}/SwitchBehaviour.cpp"
	"${src_root}/SwitchBehaviour.h"
	"${src_root}/edMathTCBSpline.cpp"
	"${src_root}/edMathTCBSpline.h"
	"${src_root}/CinematicManager.cpp"
	"${src_root}/CinematicManager.h"
	"${src_root}/InventoryInfo.cpp"
	"${src_root}/InventoryInfo.h"
	"${src_root}/Vision.cpp"
	"${src_root}/Vision.h"
	"${src_root}/CameraGame.cpp"
	"${src_root}/CameraGame.h"
	"${src_root}/CameraCinematic.cpp"
	"${src_root}/CameraCinematic.h"
	"${src_root}/CameraMouseQuake.cpp"
	"${src_root}/CameraMouseQuake.h"
	"${src_root}/CameraFixePerso.cpp"
	"${src_root}/CameraFixePerso.h"
	"${src_root}/CameraFixe.cpp"
	"${src_root}/CameraFixe.h"
	"${src_root}/CameraRail.cpp"
	"${src_root}/CameraRail.h"
	"${src_root}/CameraBoomy.cpp"
	"${src_root}/CameraBoomy.h"
	"${src_root}/camera.cpp"
	"${src_root}/camera.h"
	"${src_root}/CameraViewManager.cpp"
	"${src_root}/CameraViewManager.h"
	"${src_root}/ScenaricCondition.cpp"
	"${src_root}/ScenaricCondition.h"
	"${src_root}/PoolAllocators.cpp"
	"${src_root}/PoolAllocators.h"
	"${src_root}/InputManager.cpp"
	"${src_root}/InputManager.h"
	"${src_root}/ActorFactory.cpp"
	"${src_root}/ActorFactory.h"
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
	"${src_root}/MathOps.cpp"
	"${src_root}/MathOps.h"
	"${src_root}/TimeController.cpp"
	"${src_root}/TimeController.h"
	"${src_root}/LocalizationManager.cpp"
	"${src_root}/LocalizationManager.h"
	"${src_root}/LightManager.cpp"
	"${src_root}/LightManager.h"
	"${src_root}/TranslatedTextData.cpp"
	"${src_root}/TranslatedTextData.h"
	"${src_root}/MenuMessageBox.cpp"
	"${src_root}/MenuMessageBox.h"
	"${src_root}/PathManager.cpp"
	"${src_root}/PathManager.h"
	"${src_root}/Path.cpp"
	"${src_root}/Path.h"
	"${src_root}/PathFollow.cpp"
	"${src_root}/PathFollow.h"
	"${src_root}/PathDynamic.cpp"
	"${src_root}/PathDynamic.h"
	"${src_root}/PathFinder.cpp"
	"${src_root}/PathFinder.h"
	"${src_root}/PathFinderClient.cpp"
	"${src_root}/PathFinderClient.h"
	"${src_root}/NaviMesh.cpp"
	"${src_root}/NaviMesh.h"
	"${src_root}/NaviCell.cpp"
	"${src_root}/NaviCell.h"
	"${src_root}/Settings.cpp"
	"${src_root}/Settings.h"
	"${src_root}/WayPoint.cpp"
	"${src_root}/WayPoint.h"
	"${src_root}/MapManager.cpp"
	"${src_root}/MapManager.h"
	"${src_root}/Animation.cpp"
	"${src_root}/Animation.h"
	"${src_root}/CollisionManager.cpp"
	"${src_root}/CollisionManager.h"
	"${src_root}/Dynamic.cpp"
	"${src_root}/Dynamic.h"

	"${src_root}/Fx.cpp"
	"${src_root}/Fx.h"
	"${src_root}/Fx_Tail.cpp"
	"${src_root}/Fx_Tail.h"
	"${src_root}/Fx_Spark.cpp"
	"${src_root}/Fx_Spark.h"
	"${src_root}/FxLightEmitter.cpp"
	"${src_root}/FxLightEmitter.h"
	"${src_root}/FxParticle.cpp"
	"${src_root}/FxParticle.h"
	"${src_root}/FxLod.cpp"
	"${src_root}/FxLod.h"
	"${src_root}/FxSound.cpp"
	"${src_root}/FxSound.h"
	"${src_root}/FxPath.cpp"
	"${src_root}/FxPath.h"
	"${src_root}/FxGroup.cpp"
	"${src_root}/FxGroup.h"
	"${src_root}/FxComposite.cpp"
	"${src_root}/FxComposite.h"
	"${src_root}/FxRandom.cpp"
	"${src_root}/FxRandom.h"

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
	"${src_root}/EdenLib/edFile/include/ps2/_edFileFilerCDVD.h"

	"${src_root}/EdenLib/edFile/sources/edFileFiler.cpp"
	"${src_root}/EdenLib/edFile/sources/edFile.cpp"
	"${src_root}/EdenLib/edFile/sources/edFilePath.cpp"
	"${src_root}/EdenLib/edFile/sources/edFileCRC32.cpp"
	"${src_root}/EdenLib/edFile/sources/ps2/_edFileFilerCDVD.cpp"

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
	"${src_root}/Actor_Cinematic.cpp"
	"${src_root}/Actor_Cinematic.h"
	"${src_root}/ActorAutonomous.cpp"
	"${src_root}/ActorAutonomous.h"
	"${src_root}/ActorBoomy.cpp"
	"${src_root}/ActorBoomy.h"
	"${src_root}/ActorHeroPrivate.cpp"
	"${src_root}/ActorHeroPrivate.h"
	"${src_root}/ActorHero_Inventory.cpp"
	"${src_root}/ActorHero_Inventory.h"
	"${src_root}/ActorMicken.cpp"
	"${src_root}/ActorMicken.h"
	"${src_root}/ActorWolfen.cpp"
	"${src_root}/ActorWolfen.h"
	"${src_root}/ActorNativ.cpp"
	"${src_root}/ActorNativ.h"
	"${src_root}/ActorMovingPlatform.cpp"
	"${src_root}/ActorMovingPlatform.h"
	"${src_root}/ActorMovable.cpp"
	"${src_root}/ActorMovable.h"
	"${src_root}/ActorBonus.cpp"
	"${src_root}/ActorBonus.h"
	"${src_root}/ActorRope.cpp"
	"${src_root}/ActorRope.h"
	"${src_root}/ActorFighter.cpp"
	"${src_root}/ActorFighter.h"
	"${src_root}/ActorGravityAware.cpp"
	"${src_root}/ActorGravityAware.h"
	"${src_root}/ActorSwitch.cpp"
	"${src_root}/ActorSwitch.h"
	"${src_root}/ActorWind.cpp"
	"${src_root}/ActorWind.h"
	"${src_root}/ActorCredits.cpp"
	"${src_root}/ActorCredits.h"
	"${src_root}/ActorCommander.cpp"
	"${src_root}/ActorCommander.h"
	"${src_root}/ActorAmbre.cpp"
	"${src_root}/ActorAmbre.h"
	"${src_root}/ActorFruit.cpp"
	"${src_root}/ActorFruit.h"
	"${src_root}/ActorBridge.cpp"
	"${src_root}/ActorBridge.h"
	"${src_root}/ActorClusteriser.cpp"
	"${src_root}/ActorClusteriser.h"
	"${src_root}/ActorCompanion.cpp"
	"${src_root}/ActorCompanion.h"
	"${src_root}/ActorDCA.cpp"
	"${src_root}/ActorDCA.h"
	"${src_root}/ActorProjectile.cpp"
	"${src_root}/ActorProjectile.h"
	"${src_root}/ActorCheckpointManager.cpp"
	"${src_root}/ActorCheckpointManager.h"
	"${src_root}/ActorWeapon.cpp"
	"${src_root}/ActorWeapon.h"
	"${src_root}/ActorAraignos.cpp"
	"${src_root}/ActorAraignos.h"
	"${src_root}/ActorTeleporter.cpp"
	"${src_root}/ActorTeleporter.h"
	"${src_root}/ActorAmortos.cpp"
	"${src_root}/ActorAmortos.h"
	"${src_root}/ActorTrap.cpp"
	"${src_root}/ActorTrap.h"
	"${src_root}/ActorFx.cpp"
	"${src_root}/ActorFx.h"
	"${src_root}/ActorMoney.cpp"
	"${src_root}/ActorMoney.h"
	"${src_root}/ActorBox.cpp"
	"${src_root}/ActorBox.h"
	"${src_root}/ActorAton.cpp"
	"${src_root}/ActorAton.h"
	"${src_root}/ActorBasicBox.cpp"
	"${src_root}/ActorBasicBox.h"
	"${src_root}/ActorNativCmd.cpp"
	"${src_root}/ActorNativCmd.h"
	"${src_root}/ActorFogManager.cpp"
	"${src_root}/ActorFogManager.h"
	"${src_root}/ActorShoot.cpp"
	"${src_root}/ActorShoot.h"
	"${src_root}/ActorShocker.cpp"
	"${src_root}/ActorShocker.h"
	"${src_root}/ActorBlazer.cpp"
	"${src_root}/ActorBlazer.h"
	"${src_root}/ActorBunch.cpp"
	"${src_root}/ActorBunch.h"
	"${src_root}/ActorBrazul.cpp"
	"${src_root}/ActorBrazul.h"
	"${src_root}/ActorStiller.cpp"
	"${src_root}/ActorStiller.h"
	"${src_root}/ActorShockerCmd.cpp"
	"${src_root}/ActorShockerCmd.h"
	"${src_root}/ActorRune.cpp"
	"${src_root}/ActorRune.h"
	"${src_root}/ActorBonusFountain.cpp"
	"${src_root}/ActorBonusFountain.h"
	"${src_root}/ActorEventGenerator.cpp"
	"${src_root}/ActorEventGenerator.h"
	"${src_root}/ActorWoof.cpp"
	"${src_root}/ActorWoof.h"
	"${src_root}/ActorExplosiveDistributor.cpp"
	"${src_root}/ActorExplosiveDistributor.h"
	"${src_root}/ActorElectrolla.cpp"
	"${src_root}/ActorElectrolla.h"
	"${src_root}/ActorHedgehog.cpp"
	"${src_root}/ActorHedgehog.h"
	"${src_root}/ActorNoseMonster.cpp"
	"${src_root}/ActorNoseMonster.h"
	"${src_root}/ActorWoodMonster.cpp"
	"${src_root}/ActorWoodMonster.h"
	"${src_root}/ActorHelperSign.cpp"
	"${src_root}/ActorHelperSign.h"
	"${src_root}/ActorEgg.cpp"
	"${src_root}/ActorEgg.h"
	"${src_root}/ActorAmortosCmd.cpp"
	"${src_root}/ActorAmortosCmd.h"
	"${src_root}/ActorPunchingBall.cpp"
	"${src_root}/ActorPunchingBall.h"
	"${src_root}/ActorNativShop.cpp"
	"${src_root}/ActorNativShop.h"
	"${src_root}/ActorAddOnPurchase.cpp"
	"${src_root}/ActorAddOnPurchase.h"
	"${src_root}/ActorJamgut.cpp"
	"${src_root}/ActorJamgut.h"
	"${src_root}/ActorShip.cpp"
	"${src_root}/ActorShip.h"
	"${src_root}/ActorCareBoy.cpp"
	"${src_root}/ActorCareBoy.h"
	"${src_root}/ActorHunter.cpp"
	"${src_root}/ActorHunter.h"
	"${src_root}/ActorAcceleratos.cpp"
	"${src_root}/ActorAcceleratos.h"
	"${src_root}/ActorPattern.cpp"
	"${src_root}/ActorPattern.h"
	"${src_root}/ActorPatternCmd.cpp"
	"${src_root}/ActorPatternCmd.h"
	"${src_root}/ActorMiniGame.cpp"
	"${src_root}/ActorMiniGame.h"
	"${src_root}/ActorMiniGamesOrganizer.cpp"
	"${src_root}/ActorMiniGamesOrganizer.h"
	"${src_root}/ActorMiniGamesManager.cpp"
	"${src_root}/ActorMiniGamesManager.h"
	"${src_root}/ActorMiniGameBoxCounter.cpp"
	"${src_root}/ActorMiniGameBoxCounter.h"
	"${src_root}/ActorMiniGameTimeAttack.cpp"
	"${src_root}/ActorMiniGameTimeAttack.h"
	"${src_root}/ActorMiniGameDistance.cpp"
	"${src_root}/ActorMiniGameDistance.h"
	"${src_root}/ActorMiniGameBoomy.cpp"
	"${src_root}/ActorMiniGameBoomy.h"
	"${src_root}/ActorBombLauncher.cpp"
	"${src_root}/ActorBombLauncher.h"
	"${src_root}/ActorWantedZoo.cpp"
	"${src_root}/ActorWantedZoo.h"
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

target_include_directories(Kya PRIVATE ${src_root} "${src_root}/EdenLib/edBank/include" "${src_root}/EdenLib/edFile/include" "${src_root}/EdenLib/edParticles/include")

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
		"src"
	)

	target_link_libraries(Kya PUBLIC Port Log VU Job steinwurf::recycle)

	if (ENABLE_ISPC)
		target_sources(Kya PRIVATE ${ISPC_OBJECTS})
		target_include_directories(Kya PRIVATE "${ISPC_INCLUDE_DIR}")
		target_compile_definitions(Kya PRIVATE "ENABLE_ISPC")
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
