#pragma once

#include <cstdint>

namespace Renderer::Native
{
	enum class AntiAliasingMode : uint32_t { Off, PS2Approximation, FXAA, MSAA };

	struct AntiAliasingDraw
	{
		AntiAliasingMode mode = AntiAliasingMode::PS2Approximation;
		uint32_t fxaaQualityMultiplier = 1; // Edge-search budget, not a multisample count.
		bool fullResolutionPS2Capture = false;
		int32_t fogDepthOffset = 0; // Undo one submitted fog adjustment for the AA mask.
		float depthProjection[4]{ 65536.0f, 0.0f, 0.0f, 1.0f };
		float viewport[4]{ 0.0f, 0.0f, 1.0f, 1.0f };
	};
}
