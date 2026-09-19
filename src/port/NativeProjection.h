#pragma once

#include "Types.h"

// The camera stores projection-plane extents and distance, not FOV angles.
// Camera-space forward and the scene's clip distances are negative Z.
inline edF32MATRIX4 BuildNativeProjection(float halfWidth, float halfHeight, float projectionDistance,
	float nearClip, float farClip)
{
	const float depthRange = farClip - nearClip;
	return {
		projectionDistance / halfWidth, 0.0f, 0.0f, 0.0f,
		0.0f, -projectionDistance / halfHeight, 0.0f, 0.0f,
		0.0f, 0.0f, nearClip / depthRange, -1.0f,
		0.0f, 0.0f, -farClip * nearClip / depthRange, 0.0f
	};
}
