#pragma once

#include "Types.h"
#include "imgui.h"
#include "DebugUi.h"
#include <cstdio>
#include <fstream>
#include "ed3D.h"
#include <iostream>
#include <filesystem>

namespace DebugHelpers {
	namespace ImGui {
		static inline void TextVector4(const char* name, const edF32VECTOR4& v) {
			char value[192];
			std::snprintf(value, sizeof(value), "X %.3f  Y %.3f  Z %.3f  W %.3f", v.x, v.y, v.z, v.w);
			Debug::Ui::Readout(name, value);
		}

		static inline void TextVector3(const char* name, const edF32VECTOR3& v) {
			char value[144];
			std::snprintf(value, sizeof(value), "X %.3f  Y %.3f  Z %.3f", v.x, v.y, v.z);
			Debug::Ui::Readout(name, value);
		}

		static inline void TextHash4(const char* name, const uint& hash) {
			Hash_4 hash4 = hash;
			::ImGui::Text("Hash: %c%c%c%c", hash4.name[0], hash4.name[1], hash4.name[2], hash4.name[3]);
		}
	}

	const ImVec4 sValidColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	const ImVec4 sInvalidColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

	inline ImVec4 GetValidatedColor(bool valid) {
		return valid ? sValidColor : sInvalidColor;
	}

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
			// Check for errors
			if (!file) {
				// Handle error if needed
				std::cerr << "Error writing to file: " << filename << std::endl;
			}
			file.close();
		}
	}

	template<typename T>
	void SaveTypeToFile(std::filesystem::path filename, const T& data) {
		SaveTypeToFile(filename.string().c_str(), data);
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

	template<typename T>
	void LoadTypeFromFile(std::filesystem::path filename, T& data) {
		LoadTypeFromFile(filename.string().c_str(), data);
	}
}
