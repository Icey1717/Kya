#pragma once

#include "FogDraw.h"
#include "VulkanIncludes.h"

namespace Renderer::Native::Fog
{
	void Setup();
	void Cleanup();
	void CreateFramebuffer();
	void DestroyFramebuffer();
	void BeginFrame();
	void Record(VkCommandBuffer cmd, const FogDraw& fog);
}
