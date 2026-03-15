#pragma once

#include "PathFollow.h"
#include "Native/NativeDebugShapes.h"

namespace Debug {
	namespace Path {
		inline void DrawPathFollow(const CPathFollow* pPath, bool bIsActive)
		{
			if (!pPath || !pPath->aSplinePoints || pPath->splinePointCount < 2) {
				return;
			}

			const float r = bIsActive ? 1.0f : 0.3f;
			const float g = bIsActive ? 1.0f : 0.5f;
			const float b = bIsActive ? 0.0f : 1.0f;
			constexpr float a = 1.0f;
			constexpr float kNodeRadius = 1.0f;

			for (int i = 0; i < pPath->splinePointCount - 1; ++i) {
				const edF32VECTOR4& cur = pPath->aSplinePoints[i];
				const edF32VECTOR4& next = pPath->aSplinePoints[i + 1];
				Renderer::Native::DebugShapes::AddLine(cur.x, cur.y, cur.z, next.x, next.y, next.z, r, g, b, a);
				Renderer::Native::DebugShapes::AddSphere(cur.x, cur.y, cur.z, kNodeRadius, r, g, b, a);
			}

			// Draw the last node
			const edF32VECTOR4& last = pPath->aSplinePoints[pPath->splinePointCount - 1];
			Renderer::Native::DebugShapes::AddSphere(last.x, last.y, last.z, kNodeRadius, r, g, b, a);
		}

		inline void DrawPathFollowReaderAbsolute(const CPathFollowReaderAbsolute& reader, bool bIsActive)
		{
			DrawPathFollow(reader.pActor3C_0x0, bIsActive);
		}
	}
}
