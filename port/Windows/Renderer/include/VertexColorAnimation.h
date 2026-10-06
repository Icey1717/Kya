#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Renderer
{
	struct VertexColorFrames
	{
		uint32_t first = 0;
		uint32_t second = 0;
		float fraction = 0.0f;
	};

	// ed3DFlushStrip consumes the controller's frame value, not elapsed time.
	// Both forward and reverse playback select the same adjacent color bases.
	inline VertexColorFrames SelectVertexColorFrames(float frame, uint32_t frameCount)
	{
		if (frameCount <= 1 || !std::isfinite(frame) || frame <= 0.0f) return {};
		const float last = static_cast<float>(frameCount - 1);
		if (frame >= last) return { frameCount - 1, frameCount - 1, 0.0f };
		const auto first = static_cast<uint32_t>(frame);
		return { first, first + 1, frame - static_cast<float>(first) };
	}

	inline uint32_t InterpolateVertexColor(uint32_t first, uint32_t second, float fraction)
	{
		uint32_t rgba = 0;
		// Packet weights are 256*(1-fraction) and 256*fraction. Keep all
		// four channels in GS byte units, including its 0x80 alpha convention.
		for (uint32_t shift = 0; shift < 32; shift += 8) {
			const float a = static_cast<float>((first >> shift) & 0xff);
			const float b = static_cast<float>((second >> shift) & 0xff);
			rgba |= static_cast<uint32_t>(std::clamp(a + (b - a) * fraction, 0.0f, 255.0f)) << shift;
		}
		return rgba;
	}
}
