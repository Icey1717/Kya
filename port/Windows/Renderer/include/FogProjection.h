#pragma once

namespace Renderer::Native
{
	// Column-major projection matrices, restricted to their camera Z/W rows.
	// Eliminating camera Z gives gsZ = (a * nativeDepth + b) / (c * nativeDepth + d).
	// Unlike a near/far lerp, this retains the original GS projection's W offset.
	inline void BuildFogDepthProjection(float* result, const float* native, const float* gs)
	{
		result[0] = gs[14] * native[11] - gs[10] * native[15];
		result[1] = gs[10] * native[14] - gs[14] * native[10];
		result[2] = gs[15] * native[11] - gs[11] * native[15];
		result[3] = gs[11] * native[14] - gs[15] * native[10];
	}
}
