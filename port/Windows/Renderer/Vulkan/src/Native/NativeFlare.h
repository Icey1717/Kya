#pragma once

#include "FlareDraw.h"
#include "VulkanIncludes.h"

namespace Renderer::Native::Flare
{
	void Setup();
	void Cleanup();
	void CreateFramebuffer();
	void DestroyFramebuffer();
	void BeginFrame();
	void Record(VkCommandBuffer cmd, const FlareDraw& flare);
}
