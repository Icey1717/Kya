#pragma once

#include <filesystem>
#include <string_view>

namespace Debug::SaveLoad
{
	inline bool TryGetBackupDirectory(std::string_view pathBuffer, std::string_view serialBuffer,
		std::filesystem::path& directory)
	{
		// These are fixed-size fields in the decompiled save manager. Do not let
		// filesystem's array overload scan for a terminator or assert if it is missing.
		const auto pathEnd = pathBuffer.find('\0');
		const auto serialEnd = serialBuffer.find('\0');
		if (pathEnd == std::string_view::npos || serialBuffer.empty() || serialEnd == 0) {
			return false;
		}

		// edFilePathSplit puts the device unit ("0:"), not the directory,
		// in memCardPathEnd. The Windows memory-card filer discards it.
		const auto unit = pathBuffer.substr(0, pathEnd);
		if (unit != "0:" && unit != "1:") return false;
		// Both game serials occupy all 16 bytes. Their terminator is in the
		// following padding, outside the recovered array declaration.
		const std::filesystem::path serial(serialBuffer.substr(0, serialEnd));
		// A serial identifies a single directory, never a rooted path or traversal.
		if (serial.has_parent_path() || serial == "." || serial == "..") return false;
		directory = std::filesystem::absolute(serial);
		return true;
	}
}
