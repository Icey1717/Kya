#ifdef PLATFORM_WIN
#include "debug_draw.h"

// Forward-declare raw overloads to avoid pulling in Vulkan/GLFW headers.
namespace Renderer { namespace Native { namespace DebugShapes {
	void AddLine(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a);
	void AddOBB(const float* mat4x4, float hw, float hh, float hd, float r, float g, float b, float a);
}}}

namespace DebugDraw
{
	void AddLine(const edF32VECTOR4& start, const edF32VECTOR4& end, float r, float g, float b, float a)
	{
		Renderer::Native::DebugShapes::AddLine(
			start.x, start.y, start.z,
			end.x, end.y, end.z,
			r, g, b, a
		);
	}

	void AddOBB(const edF32MATRIX4& transform, float hw, float hh, float hd, float r, float g, float b, float a)
	{
		Renderer::Native::DebugShapes::AddOBB(transform.raw, hw, hh, hd, r, g, b, a);
	}
}
#endif
