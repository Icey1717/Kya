#pragma once

#include <array>

namespace Renderer::Native
{
	struct GsProjection
	{
		// GS depth = (a*n+b)/(c*n+d), where n is native framebuffer depth.
		std::array<float, 4> depth{ 1.0f, 0.0f, 0.0f, 1.0f };
		// GS Q = native reciprocal clip W / (u*n+v).
		std::array<float, 2> reciprocalW{ 0.0f, 1.0f };

		float ToGsDepth(float nativeDepth) const
		{
			return (depth[0] * nativeDepth + depth[1]) / (depth[2] * nativeDepth + depth[3]);
		}

		float ToNativeDepth(float gsDepth) const
		{
			return (depth[1] - gsDepth * depth[3]) / (gsDepth * depth[2] - depth[0]);
		}

		float ToGsQ(float nativeDepth, float nativeReciprocalW) const
		{
			return nativeReciprocalW / (reciprocalW[0] * nativeDepth + reciprocalW[1]);
		}
	};

	// Column-major camera projections whose Z/W depend on camera Z and W only.
	// Express GS Z and W in the basis of native clip Z/W; do not assume their
	// W rows are proportional. This also handles orthographic native cameras.
	inline GsProjection BuildGsProjection(const float* native, const float* gs)
	{
		GsProjection result;
		if (!gs) return result;
		const double a = native[10], b = native[14], c = native[11], d = native[15];
		const double determinant = a * d - b * c;
		// A singular camera has no invertible depth mapping. Retain the native
		// identity fallback, also used when a caller supplies no GS projection.
		if (determinant == 0.0) return result;
		result.depth = {
			float(gs[14] * c - gs[10] * d), float(gs[10] * b - gs[14] * a),
			float(gs[15] * c - gs[11] * d), float(gs[11] * b - gs[15] * a),
		};
		result.reciprocalW = {
			float((gs[11] * d - gs[15] * c) / determinant),
			float((gs[15] * a - gs[11] * b) / determinant),
		};
		return result;
	}

	inline void BuildGsDepthProjection(float* result, const float* native, const float* gs)
	{
		const auto projection = BuildGsProjection(native, gs);
		for (int i = 0; i < 4; ++i) result[i] = projection.depth[i];
	}
}
