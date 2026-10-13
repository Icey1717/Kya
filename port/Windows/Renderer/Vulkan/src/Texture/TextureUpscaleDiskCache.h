#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace Renderer::Native::UpscaleDiskCache
{
	std::filesystem::path Directory();
	std::string HashFile(const std::filesystem::path& path);
	std::string Key(const std::string& modelHash, uint32_t width, uint32_t height, uint32_t scale, const std::vector<uint8_t>& original);
	bool Read(const std::filesystem::path& directory, const std::string& key, uint32_t width, uint32_t height, std::vector<uint8_t>& pixels);
	void Write(const std::filesystem::path& directory, const std::string& key, uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels);
}
