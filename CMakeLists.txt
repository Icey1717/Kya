cmake_minimum_required(VERSION 3.19.4)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS YES)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project(Kya)

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
	"${src_root}/IniFile.cpp"
	"${src_root}/IniFile.h"
	"${src_root}/Iop.cpp"
	"${src_root}/Iop.h"
	"${src_root}/edStr.cpp"
	"${src_root}/edStr.h"
	"${src_root}/edPacket.cpp"
	"${src_root}/edPacket.h"
	"${src_root}/edMem.cpp"
	"${src_root}/edMem.h"
	"${src_root}/edSystem.cpp"
	"${src_root}/edSystem.h"
	"${src_root}/edText.cpp"
	"${src_root}/edText.h"
	"${src_root}/ed3D.cpp"
	"${src_root}/ed3D.h"
	"${src_root}/edDlist.cpp"
	"${src_root}/edDlist.h"
	"${src_root}/edList.cpp"
	"${src_root}/edList.h"
	"${src_root}/PackedFont.h"
	"${src_root}/LargeObject.cpp"
	"${src_root}/LargeObject.h"
	"${src_root}/BootData.cpp"
	"${src_root}/BootData.h"
	"${src_root}/SaveManagement.cpp"
	"${src_root}/SaveManagement.h"
	"${src_root}/MemoryStream.cpp"
	"${src_root}/MemoryStream.h"
	"${src_root}/LevelScheduleManager.cpp"
	"${src_root}/LevelScheduleManager.h"
	"${src_root}/FrontendManager.cpp"
	"${src_root}/FrontendManager.h"
	"${src_root}/FileManager3D.cpp"
	"${src_root}/FileManager3D.h"
	"${src_root}/PauseManager.cpp"
	"${src_root}/PauseManager.h"
	"${src_root}/SectorManager.cpp"
	"${src_root}/SectorManager.h"
	"${src_root}/ActorManager.cpp"
	"${src_root}/ActorManager.h"
	"${src_root}/CinematicManager.cpp"
	"${src_root}/CinematicManager.h"
	"${src_root}/CameraViewManager.cpp"
	"${src_root}/CameraViewManager.h"
	"${src_root}/ScenaricCondition.cpp"
	"${src_root}/ScenaricCondition.h"
	"${src_root}/InputManager.cpp"
	"${src_root}/InputManager.h"
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
	"${src_root}/edC/edCFiler_CDVD.cpp"
	"${src_root}/edC/edCFiler_CDVD.h"
	"${src_root}/MathOps.cpp"
	"${src_root}/MathOps.h"
	"${src_root}/TimeController.cpp"
	"${src_root}/TimeController.h"
	"${src_root}/LocalizationManager.cpp"
	"${src_root}/LocalizationManager.h"
	"${src_root}/TranslatedTextData.cpp"
	"${src_root}/TranslatedTextData.h"
	"${src_root}/MenuMessageBox.cpp"
	"${src_root}/MenuMessageBox.h"
	"${src_root}/PathManager.cpp"
	"${src_root}/PathManager.h"
	"${src_root}/Settings.cpp"
	"${src_root}/Settings.h"
	"${src_root}/WayPoint.cpp"
	"${src_root}/WayPoint.h"
	"${src_root}/MapManager.cpp"
	"${src_root}/MapManager.h"
	"${src_root}/Fx.cpp"
	"${src_root}/Fx.h"
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
	"${src_root}/edVideo/VideoA.cpp"
	"${src_root}/edVideo/VideoA.h"
	"${src_root}/edVideo/VideoB.cpp"
	"${src_root}/edVideo/VideoB.h"
	"${src_root}/edVideo/VideoC.cpp"
	"${src_root}/edVideo/VideoC.h"
	"${src_root}/edVideo/VideoD.cpp"
	"${src_root}/edVideo/VideoD.h"
	"${src_root}/edVideo/Camera.cpp"
	"${src_root}/edVideo/Camera.h"
	"${src_root}/edVideo/CameraStack.cpp"
	"${src_root}/edVideo/CameraStack.h"
	"${src_root}/port/pointer_conv.cpp"
	"${src_root}/port/pointer_conv.h"
)

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
	)
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

target_include_directories(Kya PRIVATE ${src_root})

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

	target_link_libraries(Kya PUBLIC Port Log)
endif()

set_property( GLOBAL PROPERTY VS_STARTUP_PROJECT Kya )

option(SKIP_MOVIES "Skip into cinematics" OFF)
option(LOAD_FOREVER "Holds on the loading screen forever" OFF)

if (SKIP_MOVIES OR PLATFORM_PS2)
	#target_compile_definitions(Kya PUBLIC SKIP_MOVIES)
endif()

target_compile_definitions(Kya PUBLIC SKIP_MOVIES)

if (LOAD_FOREVER)
	target_compile_definitions(Kya PUBLIC LOAD_FOREVER)
endif()

if (NOT PS2)
	add_subdirectory("port/Windows/Host")
	add_subdirectory("port/Test")
endif()