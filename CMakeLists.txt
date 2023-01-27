cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang")

# set the project name
project(Kya)

set(proj_root "src")

if (PS2)
	message("PS2 Build")
	set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

	set(PLATFORM_BINARY_FOLDER "PS2")
else()
	message("Win Build")

	add_subdirectory("port")

	set(PLATFORM_BINARY_FOLDER "WIN")
endif()

set(main_src 
	"${proj_root}/main.cpp"
	"${proj_root}/IniFile.cpp"
	"${proj_root}/IniFile.h"
	"${proj_root}/Iop.cpp"
	"${proj_root}/Iop.h"
	"${proj_root}/edPacket.cpp"
	"${proj_root}/edPacket.h"
	"${proj_root}/edMem.cpp"
	"${proj_root}/edMem.h"
	"${proj_root}/edSystem.cpp"
	"${proj_root}/edSystem.h"
	"${proj_root}/edText.cpp"
	"${proj_root}/edText.h"
	"${proj_root}/ed3D.cpp"
	"${proj_root}/ed3D.h"
	"${proj_root}/edDlist.cpp"
	"${proj_root}/edDlist.h"
	"${proj_root}/PackedFont.h"
	"${proj_root}/LargeObject.cpp"
	"${proj_root}/LargeObject.h"
	"${proj_root}/LevelScheduleManager.cpp"
	"${proj_root}/LevelScheduleManager.h"
	"${proj_root}/PauseManager.cpp"
	"${proj_root}/PauseManager.h"
	"${proj_root}/CinematicManager.cpp"
	"${proj_root}/CinematicManager.h"
	"${proj_root}/Rendering/DisplayList.cpp"
	"${proj_root}/Rendering/DisplayList.h"
	"${proj_root}/Rendering/Font.cpp"
	"${proj_root}/Rendering/Font.h"
	"${proj_root}/edC/edCBank.cpp"
	"${proj_root}/edC/edCBank.h"
	"${proj_root}/edC/edCBankBuffer.cpp"
	"${proj_root}/edC/edCBankBuffer.h"
	"${proj_root}/edC/edCFiler.cpp"
	"${proj_root}/edC/edCFiler.h"
	"${proj_root}/edC/edCFiler_Static.cpp"
	"${proj_root}/edC/edCFiler_Static.h"
	"${proj_root}/edC/edCFiler_CDVD.cpp"
	"${proj_root}/edC/edCFiler_CDVD.h"
	"${proj_root}/TimeController.cpp"
	"${proj_root}/TimeController.h"
)

if (PS2)
	list(APPEND main_src	
		"${proj_root}/ezmpegstr/ldimage.c"
		"${proj_root}/ezmpegstr/playpss.c"
		"${proj_root}/ezmpegstr/playpss.h"
		"${proj_root}/ezmpegstr/audiodec.c"
		"${proj_root}/ezmpegstr/audiodec.h"
		"${proj_root}/ezmpegstr/strfile.c"
		"${proj_root}/ezmpegstr/strfile.h"
		"${proj_root}/ezmpegstr/ezmpegstr.h"

		"/usr/local/sce/ee/src/lib/vu0/libvu0.c"
	)
endif()

# add the executable
add_executable(Kya 
	${main_src}
)

if (PS2)
	message("Using src directory: ${CMAKE_CURRENT_SOURCE_DIR}")

	string(APPEND CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/app.cmd ${CMAKE_CURRENT_SOURCE_DIR}/crt0.o -mno-crt0")
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
	add_custom_command(TARGET Kya POST_BUILD
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/DoRobo" "${CMAKE_CURRENT_SOURCE_DIR}/assets/" "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PLATFORM_BINARY_FOLDER}/"
)
endif()

target_include_directories(Kya PRIVATE ${proj_root})

if (PS2)
	set(EXECUTABLE_OUTPUT_PATH ${proj_root})

	target_compile_definitions(Kya PRIVATE PLATFORM_PS2)

	target_include_directories(Kya PRIVATE	"/usr/local/sce/ee/include"
											"/usr/local/sce/common/include"
											"/usr/local/sce/Shell/Include")

	target_link_directories(Kya PRIVATE		"/usr/local/sce/ee/lib"
											"/usr/local/sce/Shell/Lib")

	target_link_libraries(Kya PRIVATE "sdr" "dma" "cdvd" "graph" "pkt" "dbc" "pad2" "shell" "scf" "mpeg" "ipu" "dev" "vu0" "mc")

else()
	target_compile_features(Kya PRIVATE cxx_std_17)
	target_compile_definitions(Kya PRIVATE "PLATFORM_WIN")

	target_compile_options(Kya PRIVATE "-Wno-c++11-narrowing")

	target_include_directories(Kya PRIVATE 
		"${CMAKE_CURRENT_LIST_DIR}/src"
	)

	target_link_libraries(Kya port)
endif()