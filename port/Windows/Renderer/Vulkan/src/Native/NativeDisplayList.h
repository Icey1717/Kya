#pragma once

#include "VulkanIncludes.h"

namespace Renderer::Native::DisplayList
{
	void Setup();
	VkCommandBuffer& FinalizeCommandBuffer(bool bEndCommandBuffer = true);
}