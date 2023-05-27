#pragma once

#include "Vulkan/src/VulkanIncludes.h"

namespace DebugMenu {
	void Setup(VkInstance instance, uint32_t queueFamily, VkQueue graphicsQueue, GLFWwindow* window, VkRenderPass renderPass);
	void Render(VkCommandBuffer commandBuffer);
}