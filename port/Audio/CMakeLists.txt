project(Audio)

add_library(Audio edSysTransferService.cpp edSysTransferService.h)

target_include_directories(Audio PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

target_link_libraries(Audio PUBLIC Log)
