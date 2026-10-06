#pragma once

#include "VulkanIncludes.h"
#include "GIFReg.h"
#include "TextureSampling.h"
#include <algorithm>

struct HardwareState {
	int FBP;
	bool bActivePass;
	VkRect2D scissor;
};

HardwareState& GetHardwareState();

namespace PS2 {
	void Setup();
	void BeginFrame();
	void Cleanup();
	void CleanupSamplerCache();

	enum class TextureAddress { Clamp, Repeat };
	enum class TextureFilter { Nearest, Linear };

	struct SamplerDescription
	{
		TextureAddress addressU = TextureAddress::Clamp;
		TextureAddress addressV = TextureAddress::Clamp;
		TextureFilter minFilter = TextureFilter::Linear;
		TextureFilter magFilter = TextureFilter::Linear;
		TextureFilter mipFilter = TextureFilter::Nearest;
		uint32_t maxLod = 0;

		uint32_t GetKey() const
		{
			return uint32_t(addressU) | (uint32_t(addressV) << 1) |
				(uint32_t(minFilter) << 2) | (uint32_t(magFilter) << 3) |
				(uint32_t(mipFilter) << 4) | (maxLod << 5);
		}
	};

	// Preview sampling is explicit and independent of the last material draw.
	inline SamplerDescription GetPreviewSamplerDescription(bool linear = true)
	{
		SamplerDescription sampler;
		sampler.minFilter = sampler.magFilter = linear ? TextureFilter::Linear : TextureFilter::Nearest;
		return sampler;
	}

	SamplerDescription EmulateTextureSampler(int width, int height, const GIFReg::GSClamp& CLAMP, const GIFReg::GSTex& TEX, const GIFReg::GSPrim& PRIM);

	enum class MipOverride { None, Highest, Lowest };

	struct ResolvedTextureSampling
	{
		SamplerDescription sampler;
		Renderer::TextureSampling lod;
	};

	inline ResolvedTextureSampling ResolveTextureSampling(const GIFReg::GSClamp& clamp, const GIFReg::GSTex1& tex1,
		uint32_t mipLevels, MipOverride mipOverride = MipOverride::None)
	{
		auto settings = Renderer::TextureSampling::Decode(tex1);
		const uint32_t availableMaxLevel = mipLevels ? std::min(mipLevels - 1, 6u) : 0;
		if (mipOverride != MipOverride::None) {
			settings.maxLevel = mipOverride == MipOverride::Highest ? availableMaxLevel : 0;
			settings.mipLinear = false;
			settings.fixedLod = true;
			settings.lodBias = settings.maxLevel * 16;
		}
		SamplerDescription selector;
		selector.addressU = clamp.WMS != 1 ? TextureAddress::Repeat : TextureAddress::Clamp;
		selector.addressV = clamp.WMT != 1 ? TextureAddress::Repeat : TextureAddress::Clamp;
		selector.minFilter = settings.minLinear ? TextureFilter::Linear : TextureFilter::Nearest;
		selector.magFilter = settings.magLinear ? TextureFilter::Linear : TextureFilter::Nearest;
		selector.mipFilter = settings.mipLinear ? TextureFilter::Linear : TextureFilter::Nearest;
		selector.maxLod = std::min(settings.maxLevel, availableMaxLevel);
		return { selector, settings };
	}
}
