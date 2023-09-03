project(Kya_Win)

add_executable(Kya_Win "src/win_main.cpp")
target_link_libraries(Kya_Win PRIVATE Kya DebugMenu)

message("Exe directory: ${KYA_EXECUTABLE_DIRECTORY}")
set_target_properties(Kya_Win PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${KYA_EXECUTABLE_DIRECTORY}")
set_target_properties(Kya_Win PROPERTIES OUTPUT_NAME "Kya_$<CONFIG>.exe")
