#pragma once

#include <vector>
#include <string>

namespace Archive {
	// The file entry structure to store metadata about each file in the archive
	struct FileEntry {
		char filename[256];
		std::size_t offset; // Offset in the archive where the file data starts
		std::size_t size;   // Size of the file data
	};

	bool PackFiles(const std::vector<std::string>& filenames, const std::string& archiveFilename);
	bool ExtractFile(const std::string& archiveFilename, const std::string& filename, std::vector<char>& bytes);
}