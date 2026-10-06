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

	struct PSSamplerSelector
	{
		union
		{
			struct
			{
				uint32_t tau : 1;
				uint32_t tav : 1;
				uint32_t ltf : 1;
				uint32_t magPoint : 1;
				uint32_t mipLinear : 1;
				uint32_t maxLevel : 3;
				uint32_t palette : 1;
			};

			uint32_t key;
		};

		operator uint32_t() { return key; }

		PSSamplerSelector() : key(0) {}
	};

	PSSamplerSelector EmulateTextureSampler(int width, int height, const GIFReg::GSClamp& CLAMP, const GIFReg::GSTex& TEX, const GIFReg::GSPrim& PRIM);

	inline PSSamplerSelector GetTextureSamplerSelector(const GIFReg::GSClamp& clamp, const GIFReg::GSTex1& tex1, uint32_t mipLevels)
	{
		const auto settings = Renderer::TextureSampling::Decode(tex1);
		PSSamplerSelector selector;
		selector.tau = clamp.WMS != 1;
		selector.tav = clamp.WMT != 1;
		selector.ltf = !settings.minLinear;
		selector.magPoint = !settings.magLinear;
		selector.mipLinear = settings.mipLinear;
		selector.maxLevel = std::min(settings.maxLevel, mipLevels - 1);
		return selector;
	}
}
