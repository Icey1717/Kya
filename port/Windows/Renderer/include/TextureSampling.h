#pragma once

#include <stdint.h>
#include "GIFReg.h"

namespace Renderer
{
	// TEX1 keeps signed K in units of 1/16. Keep decoding independent of Vulkan.
	struct TextureSampling
	{
		// Native and GS projections have proportional W rows. Q_GS is the
		// native reciprocal clip W multiplied by this ratio (also for orthographic draws).
		static float GetGsQScale(const float* nativeProjection, const float* gsProjection)
		{
			if (!gsProjection) return 1.0f;
			int component = 3;
			for (int i = 7; i < 16; i += 4) {
				if (gsProjection[i] * gsProjection[i] > gsProjection[component] * gsProjection[component]) component = i;
			}
			return gsProjection[component] != 0.0f ? nativeProjection[component] / gsProjection[component] : 1.0f;
		}

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
