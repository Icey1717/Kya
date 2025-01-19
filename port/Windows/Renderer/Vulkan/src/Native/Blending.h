#pragma once

#include <cstdint>
#include <string>

#include "VulkanIncludes.h"

namespace Renderer
{
	struct SimpleTexture;
	struct SimpleMesh;

	namespace Native
	{
		void SetBlendingDynamicState(const SimpleTexture* pTexture, bool bAlphaBlendEnabled, const VkCommandBuffer& cmd);
		void SetBlendingDynamicState(const SimpleTexture* pTexture, const SimpleMesh* pMesh, const VkCommandBuffer& cmd);
	}
}