project(Renderer)

find_package(Vulkan REQUIRED FATAL_ERROR)

add_subdirectory("Shaders")

add_subdirectory("ext/glfw")

add_subdirectory("ext/glm")
include_directories("ext/glm")

add_subdirectory("ext/SPIRV-Reflect")

add_subdirectory("Vulkan/src/pcsx2/TextureUpload")

add_subdirectory("ext/imgui")

set (SOURCES
	"Vulkan/src/VulkanRenderer.cpp"
	"Vulkan/src/VulkanRenderer.h"
	"Vulkan/src/VulkanReflect.h"
	"Vulkan/src/VulkanReflect.cpp"
	"Vulkan/src/VulkanPS2.h"
	"Vulkan/src/VulkanPS2.cpp"
	"Vulkan/src/VulkanHardwarePS2.h"
	"Vulkan/src/VulkanHardwarePS2.cpp"

	"Vulkan/src/Objects/VulkanImage.cpp"
	"Vulkan/src/Objects/VulkanImage.h"
	"Vulkan/src/Objects/VulkanBuffer.cpp"
	"Vulkan/src/Objects/VulkanBuffer.h"
	"Vulkan/src/Objects/VulkanColorImage.cpp"
	"Vulkan/src/Objects/VulkanColorImage.h"
	"Vulkan/src/Objects/VulkanCommands.cpp"
	"Vulkan/src/Objects/VulkanCommands.h"
	"Vulkan/src/Objects/VulkanShader.h"
	"Vulkan/src/Objects/VulkanShader.cpp"
	"Vulkan/src/Objects/FrameBuffer.h"
	"Vulkan/src/Objects/FrameBuffer.cpp"
	"Vulkan/src/Objects/Pipeline.h"
	"Vulkan/src/Objects/Pipeline.cpp"
	"Vulkan/src/Objects/UniformBuffer.h"
	"Vulkan/src/Objects/UniformBuffer.cpp"

	"Vulkan/src/Native/NativeRenderer.h"
	"Vulkan/src/Native/NativeRenderer.cpp"
	"Vulkan/src/Native/Blending.h"
	"Vulkan/src/Native/Blending.cpp"
	"Vulkan/src/Native/PostProcessing.h"
	"Vulkan/src/Native/PostProcessing.cpp"
	"Vulkan/src/Native/NativeDebug.h"
	"Vulkan/src/Native/NativeDebug.cpp"
	"Vulkan/src/Native/NativeDebugShapes.h"
	"Vulkan/src/Native/NativeDebugShapes.cpp"
	"Vulkan/src/Native/NativeDisplayList.h"
	"Vulkan/src/Native/NativeDisplayList.cpp"

	"Vulkan/src/pcsx2/VKBuilders.h"
	"Vulkan/src/pcsx2/VKBuilders.cpp"
	"Vulkan/src/pcsx2/Selectors.cpp"

	"Vulkan/src/Texture/TextureCacheRowOffset.h"
	"Vulkan/src/Texture/TextureCache.h"
	"Vulkan/src/Texture/TextureCache.cpp"
	"Vulkan/src/Texture/TextureUpdate.h"
	"Vulkan/src/Texture/TextureUpdate.cpp"
	"Vulkan/src/Texture/TextureUpscale.h"
	"Vulkan/src/Texture/TextureUpscale.cpp"

	"include/renderer.h"
	"include/displaylist.h"
	"include/GIFReg.h"
	"include/delegate.h"
	"include/GSState.h"
	"include/ScopedTimer.h"
)

add_library(Renderer ${SOURCES})

target_compile_definitions(Renderer PUBLIC "GLFW_INCLUDE_VULKAN")
target_compile_definitions(Renderer PUBLIC "GLM_FORCE_RADIANS")
target_compile_definitions(Renderer PRIVATE "GLM_FORCE_INTRINSICS")


#if either performance or memory profiling is enabled
if(ENABLE_PERFORMANCE_PROFILING OR ENABLE_MEMORY_PROFILING)
	target_compile_definitions(Renderer PRIVATE "ENABLE_PERFORMANCE_PROFILING")
endif()

#target_compile_features(Renderer PRIVATE cxx_std_20)

set (ReflectPrintSrc
	"ext/SPIRV-Reflect/common/output_stream.h"
	"ext/SPIRV-Reflect/common/output_stream.cpp"
	"ext/SPIRV-Reflect/examples/common.h"
	"ext/SPIRV-Reflect/examples/common.cpp"
)

option(RENDERER_HEADLESS "Skip rendering commands" OFF)

option(ENABLE_UPSCALING "Enable AI texture upscaling via ncnn and Real-ESRGAN" OFF)

if (ENABLE_UPSCALING)
	include(FetchContent)

	set(NCNN_BUILD_BENCHMARK    OFF CACHE BOOL "" FORCE)
	set(NCNN_BUILD_TOOLS        OFF CACHE BOOL "" FORCE)
	set(NCNN_BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
	set(NCNN_BUILD_TESTS        OFF CACHE BOOL "" FORCE)
	set(NCNN_SHARED_LIB         OFF CACHE BOOL "" FORCE)
	set(NCNN_OPENMP             OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(
		ncnn
		GIT_REPOSITORY https://github.com/Tencent/ncnn.git
		GIT_TAG        20240410
		GIT_SHALLOW    TRUE
	)
	# cmake_minimum_required in ncnn's CMakeLists.txt pre-dates the 3.5
	# minimum that CMake 4.0+ requires. Setting this cache variable before
	# MakeAvailable propagates the workaround into the subproject.
	set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
	FetchContent_MakeAvailable(ncnn)

	target_compile_definitions(Renderer PRIVATE UPSCALING_ENABLED)
	target_link_libraries(Renderer PRIVATE ncnn)
endif()

option(ENABLE_UPSCALING_ONNX "Enable AI texture upscaling via ONNX Runtime (.onnx model)" OFF)

if (ENABLE_UPSCALING_ONNX)
	include(FetchContent)

	set(ORT_VERSION "1.20.1")
	FetchContent_Declare(
		onnxruntime
		URL "https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/onnxruntime-win-x64-${ORT_VERSION}.zip"
		DOWNLOAD_EXTRACT_TIMESTAMP TRUE
	)
	FetchContent_MakeAvailable(onnxruntime)

	# The zip may or may not have an outer versioned subdirectory; search for
	# the main C++ API header to locate the include root robustly.
	file(GLOB_RECURSE _ORT_CXX_API_H "${onnxruntime_SOURCE_DIR}/onnxruntime_cxx_api.h")
	if(NOT _ORT_CXX_API_H)
		message(FATAL_ERROR "ONNX Runtime headers not found in downloaded package (${onnxruntime_SOURCE_DIR})")
	endif()
	list(GET _ORT_CXX_API_H 0 _ORT_CXX_API_H)
	get_filename_component(_ORT_INCLUDE_DIR "${_ORT_CXX_API_H}" DIRECTORY)
	get_filename_component(_ORT_ROOT       "${_ORT_INCLUDE_DIR}" DIRECTORY)

	target_compile_definitions(Renderer PRIVATE UPSCALING_ONNX_ENABLED)
	target_include_directories(Renderer PRIVATE "${_ORT_INCLUDE_DIR}")
	target_link_libraries(Renderer PRIVATE "${_ORT_ROOT}/lib/onnxruntime.lib")

	# Copy the runtime DLL next to the executable so Windows can find it.
	add_custom_command(TARGET Renderer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${_ORT_ROOT}/lib/onnxruntime.dll"
			"${CMAKE_SOURCE_DIR}/bin/WIN/onnxruntime.dll"
		COMMENT "Copying onnxruntime.dll to bin/WIN"
	)
endif()

if (RENDERER_HEADLESS OR FORCE_RENDERER_HEADLESS) 
	target_compile_definitions(Renderer PRIVATE HEADLESS)
endif()

	target_compile_definitions(Renderer PRIVATE "PLATFORM_WIN")

add_library(reflect-print ${ReflectPrintSrc})
target_link_libraries(reflect-print PRIVATE ${Vulkan_LIBRARIES} spirv-reflect-static)
target_include_directories(reflect-print PUBLIC "ext/SPIRV-Reflect/examples/")

target_link_libraries(Renderer PRIVATE glfw ${Vulkan_LIBRARIES} spirv-reflect-static reflect-print imgui Log Hash Archive readerwriterqueue Tracy::TracyClient glm)
target_link_libraries(Renderer PUBLIC TextureUpload)

target_include_directories(Renderer PUBLIC ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR} "${CMAKE_CURRENT_LIST_DIR}/Vulkan/src" "${CMAKE_SOURCE_DIR}/port/include")

target_include_directories(Renderer PUBLIC "include")

target_include_directories(Renderer PRIVATE "Shaders/config")

add_dependencies(Renderer Shaders)

set(DXC_COMPILER "$ENV{VULKAN_SDK}/Bin/dxc.exe")
set(SHADER_OUTPUT_FOLDER "${CMAKE_SOURCE_DIR}/bin/WIN/shaders")
set( SHADER_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/Shaders/src/")


cmake_path(NATIVE_PATH SHADER_SRC_DIR SHADER_SRC_DIR)
string(REPLACE "\\" "\\\\" SHADER_SRC_DIR ${SHADER_SRC_DIR})

cmake_path(NATIVE_PATH DXC_COMPILER DXC_COMPILER)
string(REPLACE "\\" "\\\\" DXC_COMPILER ${DXC_COMPILER})

cmake_path(NATIVE_PATH SHADER_OUTPUT_FOLDER SHADER_OUTPUT_FOLDER)
string(REPLACE "\\" "\\\\" SHADER_OUTPUT_FOLDER ${SHADER_OUTPUT_FOLDER})

target_compile_definitions(Renderer PRIVATE
	SHADER_OUTPUT_FOLDER="${SHADER_OUTPUT_FOLDER}"
	DXC_COMPILER="${DXC_COMPILER}"
	SHADER_SRC_DIR="${SHADER_SRC_DIR}"
)
