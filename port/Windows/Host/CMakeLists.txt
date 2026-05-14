project(Kya_Win)

add_executable(Kya_Win "src/win_main.cpp")
target_link_libraries(Kya_Win PRIVATE Kya DebugMenu)

if (ENABLE_ADDRESS_SANITIZER)
	add_custom_command(TARGET Kya_Win POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${KYA_ASAN_RUNTIME_DLL}"
			"$<TARGET_FILE_DIR:Kya_Win>/clang_rt.asan_dynamic-x86_64.dll")
endif()

message("Exe directory: ${KYA_EXECUTABLE_DIRECTORY}")
set_target_properties(Kya_Win PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${KYA_EXECUTABLE_DIRECTORY}")
set_target_properties(Kya_Win PROPERTIES OUTPUT_NAME "Kya_$<CONFIG>")
