#pragma once

#include "Types.h"
#include "imgui.h"
#include <fstream>
#include "ed3D.h"

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

	const ImVec4 sValidColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	const ImVec4 sInvalidColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	template<typename T>
	inline void TextValidValue(char* fmt, T p)
	{
		::ImGui::TextColored(p ? sValidColor : sInvalidColor, fmt, p);
	};

	inline void ListChunckDetails(ed_Chunck* pChunck)
	{
		::ImGui::Text("Chunk Details");
		::ImGui::Text("Header: %s", pChunck->GetHeaderString().c_str());
		::ImGui::Text("Size: %d (0x%x)", pChunck->size, pChunck->size);
		::ImGui::Text("Next Chunk Offset: %d (0x%x)", pChunck->nextChunckOffset, pChunck->nextChunckOffset);
	}

	// Function to write the matrix to a binary file
	template<typename T>
	void SaveTypeToFile(const char* filename, const T& data) {
		std::ofstream file(filename, std::ios::binary);
		if (file) {
			file.write(reinterpret_cast<const char*>(&data), sizeof(T));
			file.close();
		}
	}

	// Function to read the matrix from a binary file
	template<typename T>
	void LoadTypeFromFile(const char* filename, T& data) {
		std::ifstream file(filename, std::ios::binary);
		if (file) {
			file.read(reinterpret_cast<char*>(&data), sizeof(T));
			file.close();
		}
	}
}