#pragma once

#include "VulkanRenderer.h"
#include "renderer.h"

namespace DebugMeshViewer {
	VkImage GetScreenshotImage();

	namespace Vulkan {
		void Setup();
		void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent, Renderer::CommandBufferList& commandBufferList);
	}
}
