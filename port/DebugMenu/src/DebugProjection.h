#pragma once

#include <imgui.h>

union edF32VECTOR4;

namespace Debug {
	namespace Projection {
		bool WorldToScreen(const edF32VECTOR4& worldPos, ImVec2& screenPos);
	}
}