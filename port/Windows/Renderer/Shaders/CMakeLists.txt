message("Beginning shader compile")

include_directories("config")

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
	set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
	set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif()

set(DXC_COMPILER "$ENV{VULKAN_SDK}/Bin/dxc.exe")
set(SHADER_OUTPUT_FOLDER "${CMAKE_SOURCE_DIR}/bin/WIN/shaders")

message("Cleaning output folder: ${CMAKE_CURRENT_SOURCE_DIR}")
#file(REMOVE_RECURSE ${SHADER_OUTPUT_FOLDER})

message("Finding shader source files in: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE GLSL_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.frag.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.vert.glsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.geom.glsl"
	)

message(Found GLSL: ${GLSL_SOURCE_FILES})

file(GLOB_RECURSE HLSL_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.frag.hlsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.vert.hlsl"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.geom.hlsl"
	)

message(Found HLSL: ${HLSL_SOURCE_FILES})

message("Project directory: ${CMAKE_SOURCE_DIR}")

foreach(GLSL ${GLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${GLSL} NAME)
	string(REPLACE ".glsl" "" FILE_NAME ${FILE_NAME})
	set(SPIRV "${SHADER_OUTPUT_FOLDER}/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_FOLDER}
		COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} -gVS
		DEPENDS ${GLSL})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

foreach(HLSL ${HLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${HLSL} NAME)
	string(REPLACE ".hlsl" "" FILE_NAME ${FILE_NAME})
	set(SPIRV "${SHADER_OUTPUT_FOLDER}/${FILE_NAME}.vs.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_FOLDER}
		COMMAND ${DXC_COMPILER} -spirv ${HLSL} -DSHADER_MODEL=1 -T vs_6_0 -E vs_main -Fo ${SPIRV} -fspv-debug=vulkan-with-source
		DEPENDS ${HLSL})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(HLSL)

foreach(HLSL ${HLSL_SOURCE_FILES})
	get_filename_component(FILE_NAME ${HLSL} NAME)
	string(REPLACE ".hlsl" "" FILE_NAME ${FILE_NAME})
	set(SPIRV "${SHADER_OUTPUT_FOLDER}/${FILE_NAME}.gs.spv")
	add_custom_command(
		OUTPUT ${SPIRV}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_FOLDER}
		COMMAND ${DXC_COMPILER} -spirv ${HLSL} -DSHADER_MODEL=1 -T gs_6_0 -E gs_main -Fo ${SPIRV} -fspv-debug=vulkan-with-source
		DEPENDS ${HLSL})
	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(HLSL)

#PS2 Shaders
set( PS2_SHADER_SRC "src/ps2/ps2.hlsl" )
set( PS2_SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PS2_SHADER_SRC}")

add_subdirectory("compiler")

add_custom_target(
	Shaders
	DEPENDS ${SPIRV_BINARY_FILES}
	)

add_custom_command(
	TARGET Shaders
	COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_FOLDER}
)

target_sources(Shaders PRIVATE 
	${GLSL_SOURCE_FILES}
	${HLSL_SOURCE_FILES}
)