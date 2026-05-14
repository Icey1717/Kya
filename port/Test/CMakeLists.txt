project(KyaPortTest)

add_subdirectory("ext/googletest")

set(GTEST_ROOT "${CMAKE_CURRENT_LIST_DIR}/ext/googletest")
message(STATUS "Using google test root: ${GTEST_ROOT}")

include(GoogleTest)

add_executable(KyaPortTest "src/tests.cpp")

target_compile_definitions(KyaPortTest PRIVATE TEST_DATA_DIRECTORY="${CMAKE_CURRENT_LIST_DIR}/data/")

if (ENABLE_ADDRESS_SANITIZER)
	add_custom_command(TARGET KyaPortTest POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${KYA_ASAN_RUNTIME_DLL}"
			"$<TARGET_FILE_DIR:KyaPortTest>/clang_rt.asan_dynamic-x86_64.dll")
endif()

target_compile_definitions(Renderer PRIVATE HEADLESS)

set_target_properties(KyaPortTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${KYA_EXECUTABLE_DIRECTORY}")
target_link_libraries(KyaPortTest PRIVATE gtest gtest_main Kya Renderer DebugMenu glfw)

add_test(KyaPortTest KyaPortTest)
set_tests_properties(KyaPortTest PROPERTIES WORKING_DIRECTORY ${KYA_EXECUTABLE_DIRECTORY})
