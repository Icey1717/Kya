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
		namespace Debug 
		{
			void Reset(const VkCommandBuffer& cmd);
			void UpdateLabel(const SimpleTexture* pTexture, const VkCommandBuffer& cmd);
		};
	}
}