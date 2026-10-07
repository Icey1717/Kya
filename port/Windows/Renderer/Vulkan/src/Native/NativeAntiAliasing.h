#pragma once

#include "AntiAliasingDraw.h"
#include "VulkanIncludes.h"

namespace Renderer::Native::AntiAliasing
{
	void Setup();
	void Cleanup();
	void CreateFramebuffer();
	void DestroyFramebuffer();
	void BeginFrame();
	void Record(VkCommandBuffer cmd, const AntiAliasingDraw& aa);
}
