#pragma once

#include <cstdint>
#include <string>

#include "VulkanIncludes.h"
#include "GIFReg.h"

namespace Renderer
{
	struct SimpleTexture;
	struct SimpleMesh;

	namespace Native
	{
		struct BlendingState
		{
			uint32_t hwBlendMode;
		};

		BlendingState SetBlendingDynamicState(const GIFReg::GSAlpha& alpha, bool bAlphaBlendEnabled, const VkCommandBuffer& cmd);
		BlendingState SetBlendingDynamicState(const SimpleTexture* pTexture, bool bAlphaBlendEnabled, const VkCommandBuffer& cmd);
		BlendingState SetBlendingDynamicState(const SimpleTexture* pTexture, const SimpleMesh* pMesh, const VkCommandBuffer& cmd);
	}
}