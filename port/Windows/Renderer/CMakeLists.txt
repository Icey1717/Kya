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
	"Vulkan/src/VulkanImage.cpp"
	"Vulkan/src/VulkanImage.h"
	"Vulkan/src/VulkanBuffer.cpp"
	"Vulkan/src/VulkanBuffer.h"
	"Vulkan/src/VulkanColorImage.cpp"
	"Vulkan/src/VulkanColorImage.h"
	"Vulkan/src/VulkanCommands.cpp"
	"Vulkan/src/VulkanCommands.h"
	"Vulkan/src/VulkanReflect.h"
	"Vulkan/src/VulkanReflect.cpp"
	"Vulkan/src/VulkanPS2.h"
	"Vulkan/src/VulkanPS2.cpp"
	"Vulkan/src/VulkanHardwarePS2.h"
	"Vulkan/src/VulkanHardwarePS2.cpp"
	"Vulkan/src/VulkanShader.h"
	"Vulkan/src/VulkanShader.cpp"
	"Vulkan/src/FrameBuffer.h"
	"Vulkan/src/FrameBuffer.cpp"
	"Vulkan/src/Pipeline.h"
	"Vulkan/src/Pipeline.cpp"
	"Vulkan/src/Native/NativeRenderer.h"
	"Vulkan/src/Native/NativeRenderer.cpp"
	"Vulkan/src/Native/Blending.h"
	"Vulkan/src/Native/Blending.cpp"
	"Vulkan/src/Native/PostProcessing.h"
	"Vulkan/src/Native/PostProcessing.cpp"
	"Vulkan/src/Native/NativeDebug.h"
	"Vulkan/src/Native/NativeDebug.cpp"
	"Vulkan/src/Native/NativeDisplayList.h"
	"Vulkan/src/Native/NativeDisplayList.cpp"
	"Vulkan/src/TextureCache.h"
	"Vulkan/src/TextureCache.cpp"
	"Vulkan/src/UniformBuffer.h"
	"Vulkan/src/UniformBuffer.cpp"
	"Vulkan/src/pcsx2/VKBuilders.h"
	"Vulkan/src/pcsx2/VKBuilders.cpp"
	"Vulkan/src/pcsx2/Selectors.h"
	"Vulkan/src/pcsx2/Selectors.cpp"
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
