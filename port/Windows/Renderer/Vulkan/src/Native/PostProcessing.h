#pragma once

#include <cstdint>
#include <string>

#include "VulkanIncludes.h"
#include "Pipeline.h"

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
				Fade,
			};

			void Setup();
			void AddPostProcessEffect(const VkCommandBuffer& cmd, Effect effect);
			const VkImageView& GetColorImageView();
			void UpdateDescriptorSets(const Effect effect, const int frameIndex, const DescriptorWriteList& writeList);

			std::string GetEffectName(Effect effect);
		}
	}
}