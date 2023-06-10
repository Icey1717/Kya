#pragma once

#include "VulkanIncludes.h"

#include <vector>

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

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

GLFWwindow* GetGLFWWindow();
VkInstance GetInstance();
uint32_t GetGraphicsQueueFamily();