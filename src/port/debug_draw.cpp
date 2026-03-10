#ifdef PLATFORM_WIN
#include "debug_draw.h"

// Forward-declare raw overloads to avoid pulling in Vulkan/GLFW headers.
namespace Debug { namespace Collision {
	void AddRay(float sx, float sy, float sz, float ex, float ey, float ez, float r, float g, float b, float a);
}}

namespace DebugDraw
{
	void AddRay(const edF32VECTOR4& start, const edF32VECTOR4& end, float r, float g, float b, float a)
	{
		Debug::Collision::AddRay(
			start.x, start.y, start.z,
			end.x, end.y, end.z,
			r, g, b, a
		);
	}
}
#endif
