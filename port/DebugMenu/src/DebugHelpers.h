#pragma once

#include "Types.h"
#include "imgui.h"

namespace DebugHelpers {
	namespace ImGui {
		static inline void TextVector4(const char* name, const edF32VECTOR4& v) {
			::ImGui::Text("%s: x: %.3f, y: %.3f, z: %.3f, w: %.3f", name, v.x, v.y, v.z, v.w);
		}

		static inline void TextVector3(const char* name, const edF32VECTOR3& v) {
			::ImGui::Text("%s: x: %.3f, y: %.3f, z: %.3f", name, v.x, v.y, v.z);
		}

		static inline void TextHash4(const char* name, const uint& hash) {
			Hash_4 hash4 = hash;
			::ImGui::Text("Hash: %c%c%c%c", hash4.name[0], hash4.name[1], hash4.name[2], hash4.name[3]);
		}
	}
}