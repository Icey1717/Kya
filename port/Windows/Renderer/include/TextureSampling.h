#pragma once

#include <stdint.h>
#include "GIFReg.h"

namespace Renderer
{
	// TEX1 keeps signed K in units of 1/16. Keep decoding independent of Vulkan.
	struct TextureSampling
	{
		bool fixedLod;
		uint32_t maxLevel;
		bool magLinear;
		bool minLinear;
		bool mipLinear;
		uint32_t lodScale;
		int32_t lodBias;

		static TextureSampling Decode(const GIFReg::GSTex1& tex1)
		{
			const uint32_t minFilter = tex1.MMIN;
			return { tex1.LCM != 0, minFilter >= 2 && minFilter <= 5 ? uint32_t(tex1.MXL) : 0,
				tex1.MMAG != 0, minFilter == 1 || minFilter == 4 || minFilter == 5,
				minFilter == 3 || minFilter == 5, uint32_t(tex1.L), tex1.K };
		}
	};
}
