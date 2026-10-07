#pragma once
#include "VulkanIncludes.h"

namespace Renderer::Native { struct RenderPassKey; }
namespace Renderer::Native::MSAA
{
	void Setup();
	void Cleanup();
	void CreateFramebuffer();
	void DestroyFramebuffer();
	VkSampleCountFlagBits GetSamples();
	VkRenderPass CreateRenderPass(const RenderPassKey& key);
	VkFramebuffer GetFramebuffer();
	void Seed(VkCommandBuffer cmd, const RenderPassKey& key);
}
