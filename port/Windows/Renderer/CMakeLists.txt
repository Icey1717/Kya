project(renderer)

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
	"Vulkan/src/MD5.h"
	"DebugMenu/DebugMenu.h"
	"DebugMenu/DebugMenu.cpp"
	"include/renderer.h"
)

add_library(renderer ${SOURCES})

target_compile_features(renderer PRIVATE cxx_std_17)

set (ReflectPrintSrc
	"ext/SPIRV-Reflect/common/output_stream.h"
	"ext/SPIRV-Reflect/common/output_stream.cpp"
	"ext/SPIRV-Reflect/examples/common.h"
	"ext/SPIRV-Reflect/examples/common.cpp"
)

add_library(reflect-print ${ReflectPrintSrc})
target_link_libraries(reflect-print ${Vulkan_LIBRARIES} spirv-reflect-static)
target_include_directories(reflect-print PUBLIC "ext/SPIRV-Reflect/examples/")

target_link_libraries(renderer glfw ${Vulkan_LIBRARIES} spirv-reflect-static reflect-print imgui)

target_include_directories(renderer PRIVATE ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR})

target_include_directories(renderer PUBLIC "include")

add_dependencies(renderer Shaders)