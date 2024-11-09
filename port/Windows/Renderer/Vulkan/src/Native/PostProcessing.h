#pragma once

#include <cstdint>
#include <string>

#include "VulkanIncludes.h"

namespace Renderer
{
	namespace Native
	{
		namespace PostProcessing
		{
			enum class Effect
			{
				Greyscale,
				AlphaFix,
			};

			void Setup();
			void AddPostProcessEffect(const VkCommandBuffer& cmd, Effect effect);
			const VkImageView& GetColorImageView();
		}
	}
}