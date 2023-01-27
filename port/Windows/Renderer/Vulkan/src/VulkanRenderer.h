#pragma once

#include "VulkanIncludes.h"

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

VkDevice GetDevice();
VkPhysicalDevice GetPhysicalDevice();
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
const VkBuffer& GetUniformBuffer(int index);
const VkQueue& GetGraphicsQueue();
const VkCommandPool& GetCommandPool();