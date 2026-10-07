#pragma once

#include <cstdint>

namespace Renderer::Native
{
	// Captured at the scene's fog boundary, after its children and display list.
	struct FogDraw
	{
		uint32_t flags = 0;
		int32_t depthOffset = 0;
		uint32_t color[4]{ 0, 0, 0, 128 }; // GS bytes, including framebuffer alpha.
		float gsNear = 65534.0f;
		float gsFar = 1.0f;
		float depthProjection[4]{}; // Rational native-to-GS depth conversion; zero uses synthetic near/far mapping.
		float viewport[4]{ 0.0f, 0.0f, 1.0f, 1.0f }; // Normalized scissor XYWH.
	};
}
