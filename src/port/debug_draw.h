#pragma once

#include "Types.h"

namespace DebugDraw
{
#ifdef PLATFORM_WIN
	void AddLine(const edF32VECTOR4& start, const edF32VECTOR4& end, float r, float g, float b, float a);
	void AddOBB(const edF32MATRIX4& transform, float hw, float hh, float hd, float r, float g, float b, float a);
#else
	inline void AddLine(const edF32VECTOR4&, const edF32VECTOR4&, float, float, float, float) {}
	inline void AddOBB(const edF32MATRIX4&, float, float, float, float, float, float, float) {}
#endif
}
