#pragma once

#include "VulkanIncludes.h"

namespace Renderer::Native::DisplayList
{
	void Setup();
	void Cleanup();
	VkCommandBuffer& FinalizeCommandBuffer(bool bEndCommandBuffer = true);
}
