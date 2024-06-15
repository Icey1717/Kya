#pragma once

#include "VulkanIncludes.h"
#include "GIFReg.h"

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
			};

			uint32_t key;
		};

		operator uint32_t() { return key & 0x7; }

		PSSamplerSelector() : key(0) {}
	};

	PSSamplerSelector EmulateTextureSampler(int width, int height, const GIFReg::GSClamp& CLAMP, const GIFReg::GSTex& TEX, const GIFReg::GSPrim& PRIM);
}
