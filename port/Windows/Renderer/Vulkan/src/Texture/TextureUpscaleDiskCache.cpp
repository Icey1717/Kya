#include "TextureUpscaleDiskCache.h"
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <atomic>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace
{
	constexpr std::array<char, 8> magic{'K', 'Y', 'A', 'T', 'E', 'X', '0', '1'};
	constexpr uint64_t headerSize = 8 + 64 + 4 + 4 + 32;
	// Bump the key version if normalization, quantization, or alpha handling changes.
	constexpr char recipe[] = "Kya RGBA8 RGB-NCHW01 nearest-alpha v1";
	class Sha256
	{
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
	public:
		Sha256() {
			if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
				throw std::runtime_error("Cannot initialize SHA256");
			if (BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) < 0) {
				BCryptCloseAlgorithmProvider(algorithm, 0); algorithm = nullptr;
				throw std::runtime_error("Cannot create SHA256 hash");
			}
		}
		~Sha256() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
		Sha256(const Sha256&) = delete;
		Sha256& operator=(const Sha256&) = delete;
		void Add(const void* bytes, size_t size) {
			if (size > std::numeric_limits<ULONG>::max() || BCryptHashData(hash, reinterpret_cast<PUCHAR>(const_cast<void*>(bytes)), static_cast<ULONG>(size), 0) < 0)
				throw std::runtime_error("SHA256 hashing failed");
		}
		std::array<uint8_t, 32> Finish() {
			std::array<uint8_t, 32> digest{};
			if (BCryptFinishHash(hash, digest.data(), static_cast<ULONG>(digest.size()), 0) < 0)
				throw std::runtime_error("SHA256 finalization failed");
			return digest;
		}
	};
	std::string Hex(const std::array<uint8_t, 32>& digest) {
		constexpr char digits[] = "0123456789abcdef";
		std::string result;
		for (auto byte : digest) { result += digits[byte >> 4]; result += digits[byte & 15]; }
		return result;
	}
	bool ValidKey(const std::string& key) {
		return key.size() == 64 && key.find_first_not_of("0123456789abcdef") == std::string::npos;
	}
	uint64_t PixelBytes(uint32_t width, uint32_t height) {
		if (!width || !height || width > 32768 || height > 32768) throw std::runtime_error("Invalid cached texture dimensions");
		const uint64_t size = uint64_t(width) * height * 4;
		if (!width || !height || size > 512 * 1024 * 1024) throw std::runtime_error("Invalid cached texture dimensions");
		return size;
	}
}

std::filesystem::path Renderer::Native::UpscaleDiskCache::Directory()
{
	wchar_t executable[32768];
	const auto length = GetModuleFileNameW(nullptr, executable, 32768);
	if (!length || length == 32768) throw std::runtime_error("Cannot locate executable directory");
	return std::filesystem::path(std::wstring(executable, length)).parent_path() / "texture-upscale-cache";
}
std::string Renderer::Native::UpscaleDiskCache::HashFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file) throw std::runtime_error("Cannot hash model: " + path.string());
	Sha256 hash;
	std::array<char, 65536> buffer;
	while (file.read(buffer.data(), buffer.size()) || file.gcount()) hash.Add(buffer.data(), static_cast<size_t>(file.gcount()));
	if (!file.eof()) throw std::runtime_error("Model hash read failed");
	return Hex(hash.Finish());
}
std::string Renderer::Native::UpscaleDiskCache::Key(const std::string& modelHash, uint32_t width, uint32_t height, uint32_t scale, const std::vector<uint8_t>& original)
{
	if (!ValidKey(modelHash) || !scale || scale > 8 || original.size() != PixelBytes(width, height)) throw std::runtime_error("Invalid texture cache key input");
	Sha256 hash;
	hash.Add(recipe, sizeof(recipe)); hash.Add(modelHash.data(), modelHash.size());
	hash.Add(&width, sizeof(width)); hash.Add(&height, sizeof(height)); hash.Add(&scale, sizeof(scale));
	hash.Add(original.data(), original.size());
	return Hex(hash.Finish());
}
bool Renderer::Native::UpscaleDiskCache::Read(const std::filesystem::path& directory, const std::string& key, uint32_t width, uint32_t height, std::vector<uint8_t>& pixels)
{
	if (!ValidKey(key)) return false;
	const auto size = PixelBytes(width, height);
	const auto path = directory / (key + ".rgba");
	std::error_code error;
	if (std::filesystem::file_size(path, error) != headerSize + size || error) return false;
	std::ifstream file(path, std::ios::binary);
	std::array<char, 8> signature{};
	std::array<char, 64> storedKey{};
	uint32_t storedWidth = 0, storedHeight = 0;
	std::array<uint8_t, 32> checksum{};
	if (!file.read(signature.data(), signature.size()) || !file.read(storedKey.data(), storedKey.size()) ||
		!file.read(reinterpret_cast<char*>(&storedWidth), 4) || !file.read(reinterpret_cast<char*>(&storedHeight), 4) ||
		!file.read(reinterpret_cast<char*>(checksum.data()), checksum.size()) || signature != magic ||
		std::string(storedKey.data(), storedKey.size()) != key || storedWidth != width || storedHeight != height) return false;
	std::vector<uint8_t> result(static_cast<size_t>(size));
	if (!file.read(reinterpret_cast<char*>(result.data()), result.size())) return false;
	Sha256 hash; hash.Add(result.data(), result.size());
	if (hash.Finish() != checksum) return false;
	pixels = std::move(result);
	return true;
}
void Renderer::Native::UpscaleDiskCache::Write(const std::filesystem::path& directory, const std::string& key, uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels)
{
	if (!ValidKey(key) || pixels.size() != PixelBytes(width, height)) throw std::runtime_error("Invalid cached texture output");
	std::filesystem::create_directories(directory);
	static std::atomic<uint64_t> serial{0};
	const auto path = directory / (key + ".rgba");
	const auto temporary = directory / (key + "." + std::to_string(GetCurrentProcessId()) + "." + std::to_string(++serial) + ".tmp");
	try {
		Sha256 hash; hash.Add(pixels.data(), pixels.size()); const auto checksum = hash.Finish();
		std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
		file.write(magic.data(), magic.size()); file.write(key.data(), key.size());
		file.write(reinterpret_cast<const char*>(&width), 4); file.write(reinterpret_cast<const char*>(&height), 4);
		file.write(reinterpret_cast<const char*>(checksum.data()), checksum.size());
		file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size()); file.close();
		if (!file || !MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			throw std::runtime_error("Cannot save texture cache entry");
	} catch (...) { std::error_code ignored; std::filesystem::remove(temporary, ignored); throw; }
}
