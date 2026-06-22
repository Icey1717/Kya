#pragma once

#include "VulkanIncludes.h"

#include <vector>

#include "logging.h"

#define VULKAN_LOG(level, format, ...) MY_LOG_CATEGORY("Vulkan", level, format, ##__VA_ARGS__)

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

namespace Renderer
{
	extern bool gHeadless;
	constexpr int VertexIndexBufferSize = 0x100000;
	constexpr int VertexIndexBufferSizeGPU = 0x100000;

	using CommandBufferVector = std::vector<VkCommandBuffer>;

	struct VulkanContext
	{
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphicsQueue = VK_NULL_HANDLE;
		VkQueue presentQueue = VK_NULL_HANDLE;
		uint32_t graphicsQueueFamily = 0;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkAllocationCallbacks* allocator = nullptr;
	};

	struct SwapchainContext
	{
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkFormat imageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D extent = {};
		std::vector<VkImage>* images = nullptr;
	};

	void CreateCommandBuffers(CommandBufferVector& commandBuffers, const char* name = nullptr);
	void CreateCommandBuffers(const VkCommandPool& pool, CommandBufferVector& commandBuffers, const char* name = nullptr);

	VkCommandPool CreateCommandPool(const char* name = nullptr);

	const VulkanContext& GetVulkanContext();
	const SwapchainContext& GetSwapchainContext();
}

VkDevice GetDevice();
VkFormat GetSwapchainImageFormat();
std::vector<VkImage>& GetSwapchainImages();
VkPhysicalDevice GetPhysicalDevice();
const VkCommandBuffer& GetCurrentCommandBuffer();
uint32_t GetCurrentFrame();
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
const VkBuffer& GetUniformBuffer(int index);
const VkQueue& GetGraphicsQueue();
const VkCommandPool& GetCommandPool();
const VkExtent2D& GetSwapchainExtent();

void SetObjectName(const uint64_t objHandle, const VkObjectType objType, const char* format, ...);

GLFWwindow* GetGLFWWindow();
VkInstance GetInstance();
uint32_t GetGraphicsQueueFamily();

VkAllocationCallbacks* GetAllocator();
void SetAllocationTrackingEnabled(bool enabled);
uint32_t GetAllocationCount(VkSystemAllocationScope scope);
