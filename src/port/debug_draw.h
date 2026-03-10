#pragma once

#include "Types.h"

namespace DebugDraw
{
#ifdef PLATFORM_WIN
	void AddRay(const edF32VECTOR4& start, const edF32VECTOR4& end, float r, float g, float b, float a);
#endif
}
