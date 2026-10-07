#pragma once

#include <cmath>
#include <algorithm>
#include "GsProjection.h"

namespace Renderer::Native
{
	struct FlareScreenData
	{
		float center[2]{};
		float halfSize[2]{};
		float marker[4]{}; // UV center and half-size of the PS2 visibility marker.
		float depth = 0.0f;
	};

	// Screen sizing stays in GS units; depth uses the inverse shared camera conversion.
	inline bool ProjectFlare(float x, float y, float markerX, float markerY, float radius,
		float width, float height, float gsDepth, const GsProjection& projection, FlareScreenData& result)
	{
		if (!(width > 0.0f && height > 0.0f && radius > 0.0f)) return false;
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
		result.depth = projection.ToNativeDepth(gsDepth);
		// Float matrix coefficients can put an exact clip-plane point a few
		// ULPs outside [0,1]. Reject real clipping before removing that roundoff.
		if (!std::isfinite(result.depth) || result.depth < -0.000001f || result.depth > 1.000001f) return false;
		result.depth = std::clamp(result.depth, 0.0f, 1.0f);
		return std::isfinite(result.center[0]) && std::isfinite(result.center[1]) &&
			std::isfinite(result.marker[0]) && std::isfinite(result.marker[1]) &&
			std::isfinite(result.halfSize[0]) && std::isfinite(result.halfSize[1]) &&
			std::isfinite(result.depth) && result.depth >= 0.0f && result.depth <= 1.0f;
	}
}
