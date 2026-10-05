#pragma once

#include <cmath>

namespace Renderer::Native
{
	struct FlareScreenData
	{
		float center[2]{};
		float halfSize[2]{};
		float marker[4]{}; // UV center and half-size of the PS2 visibility marker.
		float depth = 0.0f;
	};

	// Both projections are affine in 1 / camera Z: GS near/far map to Vulkan 1/0.
	inline bool ProjectFlare(float x, float y, float markerX, float markerY, float radius,
		float width, float height, float gsDepth, float gsNear, float gsFar, FlareScreenData& result)
	{
		if (!(width > 0.0f && height > 0.0f && radius > 0.0f) ||
			!std::isfinite(gsNear) || !std::isfinite(gsFar) || gsNear == gsFar) return false;
		const float originX = (4096.0f - width) * 0.5f;
		const float originY = (4096.0f - height) * 0.5f;
		result.center[0] = (x - originX) / width;
		result.center[1] = (y - originY) / height;
		result.halfSize[0] = radius / width;
		result.halfSize[1] = radius / height;
		result.marker[0] = (markerX - originX) / width;
		result.marker[1] = (markerY - originY) / height;
		result.marker[2] = 8.0f / width;
		result.marker[3] = 8.0f / height;
		result.depth = (gsDepth - gsFar) / (gsNear - gsFar);
		return std::isfinite(result.center[0]) && std::isfinite(result.center[1]) &&
			std::isfinite(result.marker[0]) && std::isfinite(result.marker[1]) &&
			std::isfinite(result.halfSize[0]) && std::isfinite(result.halfSize[1]) &&
			std::isfinite(result.depth) && result.depth >= 0.0f && result.depth <= 1.0f;
	}
}
