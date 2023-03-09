project(renderer)

add_subdirectory("Shaders")

add_subdirectory("ext/glfw")

add_subdirectory("ext/glm")
include_directories("ext/glm")

find_package(Vulkan REQUIRED FATAL_ERROR)

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
	"Vulkan/src/VulkanCommands.h")

add_library(renderer ${SOURCES})

target_compile_features(renderer PRIVATE cxx_std_17)

target_link_libraries(renderer glfw ${Vulkan_LIBRARIES})

target_include_directories(renderer PRIVATE ${Vulkan_INCLUDE_DIRS})

add_dependencies(renderer Shaders)