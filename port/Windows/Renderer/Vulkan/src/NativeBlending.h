#pragma once

#include <cstdint>
#include <string>

#include "Pipeline.h"
#include "VulkanIncludes.h"

namespace Renderer
{
	struct SimpleTexture;
	struct SimpleMesh;

	namespace Native
	{
		void SetBlendingDynamicState(SimpleTexture* pTexture, SimpleMesh* pMesh, const VkCommandBuffer& cmd);
	}
}