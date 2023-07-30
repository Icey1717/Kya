#include "archive.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>

bool Archive::PackFiles(const std::vector<std::string>& filenames, const std::string& archiveFilename) {
	std::ofstream archive(archiveFilename, std::ios::binary | std::ios::out);
	if (!archive) {
		std::cerr << "Failed to create archive file: " << archiveFilename << std::endl;
		return false;
	}

	std::vector<FileEntry> fileEntries;
	std::size_t offset = 0;

	for (const auto& filename : filenames) {
		std::ifstream file(filename, std::ios::binary | std::ios::in);
		if (!file) {
			std::cerr << "Failed to open file: " << filename << std::endl;
			continue;
		}

		file.seekg(0, std::ios::end);
		std::size_t size = static_cast<std::size_t>(file.tellg());
		file.seekg(0, std::ios::beg);

		std::vector<char> buffer(size);
		file.read(buffer.data(), size);

		FileEntry entry;
		strcpy_s(entry.filename, 256, filename.c_str());
		entry.offset = offset;
		entry.size = size;
		fileEntries.push_back(entry);

		archive.write(buffer.data(), size);
		offset += size;
	}

	// Write the file entry table at the end of the archive
	for (const auto& entry : fileEntries) {
		archive.write(reinterpret_cast<const char*>(&entry), sizeof(FileEntry));
	}

	int len = fileEntries.size();
	archive.write(reinterpret_cast<const char*>(&len), sizeof(len));

	return true;
}

static std::streampos GetFileSize(std::ifstream& file) {
	std::streampos currentPosition = file.tellg(); // Get the current position
	file.seekg(0, std::ios::end); // Move to the end of the file
	std::streampos fileSize = file.tellg(); // Get the position at the end (file size)
	file.seekg(currentPosition); // Move back to the original position
	return fileSize;
}

bool Archive::ExtractFile(const std::string& archiveFilename, const std::string& filename, std::vector<char>& bytes) {
	std::ifstream archive(archiveFilename, std::ios::binary | std::ios::in);
	if (!archive) {
		std::cerr << "Failed to open archive file: " << archiveFilename << std::endl;
		return false;
	}

	// Read the file entry table from the end of the archive
	std::vector<FileEntry> fileEntries;
	std::streampos fileSize = GetFileSize(archive);
	std::streampos lenOffset = fileSize - static_cast<std::streampos>(sizeof(int));

	archive.seekg(lenOffset, std::ios::beg);
	int len;
	archive.read(reinterpret_cast<char*>(&len), sizeof(int));

	std::streampos entryTableOffset = fileSize - (static_cast<std::streampos>(sizeof(FileEntry) * len)) - static_cast<std::streampos>(sizeof(int));

	archive.seekg(entryTableOffset, std::ios::beg);
	for (int i = 0; i < len; i++) {
		FileEntry entry;
		archive.read(reinterpret_cast<char*>(&entry), sizeof(FileEntry));
		fileEntries.push_back(entry);
	}

	// Find the entry corresponding to the specified filename
	auto it = std::find_if(fileEntries.begin(), fileEntries.end(), [&filename](const FileEntry& entry) {
		return entry.filename == filename;
		});

	if (it == fileEntries.end()) {
		std::cerr << "File not found in the archive: " << filename << std::endl;
		return false;
	}

	// Extract the file bytes
	bytes.resize(it->size);
	archive.seekg(it->offset, std::ios::beg);
	archive.read(bytes.data(), it->size);

	return true;
}