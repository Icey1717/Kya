project(Audio)

add_library(Audio
	edSoundSampleService.cpp
	edSoundSampleService.h
	edSoundDevice.h
	edSoundStreamService.cpp
	edSoundStreamService.h
	edSysTransferService.cpp
	edSysTransferService.h)

target_include_directories(Audio PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

target_link_libraries(Audio PUBLIC Log)

if(WIN32)
	target_link_libraries(Audio PRIVATE XAudio2 ole32)
endif()
