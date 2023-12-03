#pragma once

#include "VulkanRenderer.h"

namespace DebugMeshViewer {
	namespace Vulkan {
		void Setup();
		void Render(const VkFramebuffer& framebuffer, const VkExtent2D& extent);
	}
}