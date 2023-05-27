cmake_minimum_required(VERSION 3.10)

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

	add_subdirectory("port")

	set(PLATFORM_BINARY_FOLDER "WIN")
endif()

set(src_root "src")

set(main_src 
	"${src_root}/main.cpp"
	"${src_root}/IniFile.cpp"
	"${src_root}/IniFile.h"
	"${src_root}/Iop.cpp"
	"${src_root}/Iop.h"
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
	"${src_root}/PackedFont.h"
	"${src_root}/LargeObject.cpp"
	"${src_root}/LargeObject.h"
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
	"${src_root}/CinematicManager.cpp"
	"${src_root}/CinematicManager.h"
	"${src_root}/CameraViewManager.cpp"
	"${src_root}/CameraViewManager.h"
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
	"${src_root}/edC/edCFiler_BNK.cpp" 
	"${src_root}/edC/edCFiler_BNK.h"
	"${src_root}/TextIconDictionary.cpp"
	"${src_root}/TextIconDictionary.h"
	"${src_root}/edVideo/VideoA.cpp"
	"${src_root}/edVideo/VideoA.h"
	"${src_root}/edVideo/VideoB.cpp"
	"${src_root}/edVideo/VideoB.h"
	"${src_root}/edVideo/VideoC.cpp"
	"${src_root}/edVideo/VideoC.h"
	"${src_root}/edVideo/VideoD.cpp"
	"${src_root}/edVideo/VideoD.h"
	"${src_root}/Camera.cpp"
	"${src_root}/Camera.h"
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
endif()

# add the executable
add_executable(Kya 
	${main_src}
)

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
	set_target_properties(Kya PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}")
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

	target_link_libraries(Kya PRIVATE "sdr" "dma" "cdvd" "graph" "pkt" "dbc" "pad2" "shell" "scf" "mpeg" "ipu" "dev" "vu0" "mc")

	target_compile_options(Kya PRIVATE "-w")

else()
	target_compile_features(Kya PRIVATE cxx_std_17)
	target_compile_definitions(Kya PRIVATE "PLATFORM_WIN")

	target_compile_options(Kya PRIVATE "-Wno-c++11-narrowing" "-w")

	target_include_directories(Kya PRIVATE 
		"${CMAKE_CURRENT_LIST_DIR}/src"
	)

	target_link_libraries(Kya port)
endif()

set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT Kya )