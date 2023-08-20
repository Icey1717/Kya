project(Renderer)

find_package(Vulkan REQUIRED FATAL_ERROR)

add_subdirectory("Shaders")

add_subdirectory("ext/glfw")

add_subdirectory("ext/glm")
include_directories("ext/glm")

add_subdirectory("ext/SPIRV-Reflect")

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
	"Vulkan/src/VulkanShader.h"
	"Vulkan/src/VulkanShader.cpp"
	"Vulkan/src/FrameBuffer.h"
	"Vulkan/src/FrameBuffer.cpp"
	"Vulkan/src/Pipeline.h"
	"Vulkan/src/Pipeline.cpp"
	"Vulkan/src/TextureCache.h"
	"Vulkan/src/TextureCache.cpp"
	"Vulkan/src/UniformBuffer.h"
	"Vulkan/src/UniformBuffer.cpp"
	"Vulkan/src/GSState.h"
	"Vulkan/src/GSVector.h"
	"include/renderer.h"
	"include/delegate.h"
)

add_library(Renderer ${SOURCES})

target_compile_features(Renderer PRIVATE cxx_std_17)

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

add_library(reflect-print ${ReflectPrintSrc})
target_link_libraries(reflect-print PRIVATE ${Vulkan_LIBRARIES} spirv-reflect-static)
target_include_directories(reflect-print PUBLIC "ext/SPIRV-Reflect/examples/")

target_link_libraries(Renderer PRIVATE glfw ${Vulkan_LIBRARIES} spirv-reflect-static reflect-print imgui Log Hash Archive)

target_include_directories(Renderer PUBLIC ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR} "${CMAKE_CURRENT_LIST_DIR}/Vulkan/src")

target_include_directories(Renderer PUBLIC "include")

target_include_directories(Renderer PRIVATE "Shaders/config")

add_dependencies(Renderer Shaders)