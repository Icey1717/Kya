#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <algorithm>
#include <filesystem>
#include <string_view>
#include <stdexcept>

namespace Debug::SaveLoad
{
	// Windows 10 FileRenameInfoEx can replace a destination whose read handle
	// remains open with FILE_SHARE_DELETE. Keep writer exclusion until publication.
	inline void PublishMemoryCard(const std::filesystem::path& path, std::string_view bytes)
	{
		struct TemporaryCard
		{
			wchar_t name[MAX_PATH] = {};
			HANDLE file = INVALID_HANDLE_VALUE;
			~TemporaryCard() {
				if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
				if (name[0]) DeleteFileW(name);
			}
		} temporary;
		const auto destination = std::filesystem::absolute(path).wstring();
		const auto directory = std::filesystem::absolute(path).parent_path();
		if (!GetTempFileNameW(directory.c_str(), L"kyc", 0, temporary.name))
			throw std::runtime_error("Could not create temporary card image");
		temporary.file = CreateFileW(temporary.name, GENERIC_WRITE | DELETE, FILE_SHARE_READ | FILE_SHARE_DELETE,
			nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		DWORD written = 0;
		if (temporary.file == INVALID_HANDLE_VALUE || bytes.size() > MAXDWORD ||
			!WriteFile(temporary.file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) ||
			written != bytes.size() || !FlushFileBuffers(temporary.file))
			throw std::runtime_error("Could not flush temporary card image; original card retained");
		struct RenameInfo
		{
			DWORD flags;
			HANDLE rootDirectory;
			DWORD nameLength;
			wchar_t name[MAX_PATH];
		} rename = {};
		if (destination.size() >= MAX_PATH) throw std::runtime_error("Card path is too long");
		// FILE_RENAME_FLAG_REPLACE_IF_EXISTS | FILE_RENAME_FLAG_POSIX_SEMANTICS.
		rename.flags = 0x3;
		rename.nameLength = static_cast<DWORD>(destination.size() * sizeof(wchar_t));
		std::copy(destination.begin(), destination.end(), rename.name);
		if (!SetFileInformationByHandle(temporary.file, FileRenameInfoEx, &rename, sizeof(rename)))
			throw std::runtime_error("Could not atomically replace card image; original card and backup retained");
		temporary.name[0] = 0;
	}
}
