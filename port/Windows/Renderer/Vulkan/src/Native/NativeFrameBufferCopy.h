#pragma once

#include "VulkanIncludes.h"

namespace Renderer::Native::FrameBufferCopy
{
	void Setup();
	void Cleanup();
	void Record(VkCommandBuffer cmd);
	const VkDescriptorSet& GetDescriptorSet(int frame);
}
