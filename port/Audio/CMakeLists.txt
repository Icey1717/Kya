project(Audio)

add_library(Audio
	edMusicData.cpp
	edMusicData.h
	edMusicSynth.cpp
	edMusicSynth.h
	edMusicService.cpp
	edMusicService.h
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

add_executable(KyaMusicRender music_render.cpp)
target_link_libraries(KyaMusicRender PRIVATE Audio)
