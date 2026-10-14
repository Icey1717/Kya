#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Debug::SaveLoad
{
	// PS2 filesystem layout: https://github.com/PCSX2/myMCpp/blob/master/docs/memcard.md
	// Only replace existing Kya slots: keep allocation, icons and other games intact.
	class MemoryCard
	{
	public:
		explicit MemoryCard(std::vector<char> image) : bytes(std::move(image))
		{
			if (bytes.size() < 512 || std::memcmp(bytes.data(), "Sony PS2 Memory Card Format ", 28) != 0 ||
				Word(0x28, 2) != 512 || Word(0x2a, 2) != 2 || Word(0x150, 1) != 2)
				throw std::runtime_error("Not a supported formatted PS2 card");
			clusters = Word(0x30);
			if (clusters < 64 || clusters > 65536) throw std::runtime_error("Invalid card geometry");
			if (bytes.size() == size_t(clusters) * 1024) stride = 512;
			else if (bytes.size() == size_t(clusters) * 1056) stride = 528;
			else throw std::runtime_error("Unsupported card image size");
			allocOffset = Word(0x34);
			allocEnd = Word(0x38);
			if (allocOffset >= clusters || allocEnd > clusters - allocOffset || !allocEnd)
				throw std::runtime_error("Invalid card allocation range");
		}

		struct Entry { size_t offset; uint32_t size; uint32_t cluster; };

		Entry FindSlot(std::string_view serial, int slot) const
		{
			if ((serial != "BESLES-51473-KYA" && serial != "BASLUS-20440-KYA") || slot < 0 || slot > 3)
				throw std::runtime_error("Invalid Kya directory or slot");
			const auto root = Word(0x3c);
			const auto directory = Find(root, Word(size_t(root + allocOffset) * 1024 + 4), serial, 0x20);
			return Find(directory.cluster, directory.size, "slot_" + std::to_string(slot) + ".dat", 0x10);
		}

		void ReplaceSlot(std::string_view serial, int slot, std::string_view save)
		{
			// Refuse damaged cards whose files share clusters. Otherwise replacing a
			// Kya slot could overwrite another game even though its own chain is valid.
			std::set<uint32_t> allocated;
			const auto root = Word(0x3c);
			ValidateDirectory(root, Word(size_t(root + allocOffset) * 1024 + 4), allocated, 0);
			const auto entry = FindSlot(serial, slot);
			const auto chain = Chain(entry.cluster);
			if (save.empty() || save.size() > chain.size() * 1024)
				throw std::runtime_error("Save does not fit the existing card slot");
			for (size_t i = 0; i < save.size(); ++i)
				SetByte(size_t(chain[i / 1024] + allocOffset) * 1024 + i % 1024, save[i]);
			for (unsigned i = 0; i < 4; ++i) SetByte(entry.offset + 4 + i, char(save.size() >> (i * 8)));
			UpdateEcc();
		}

		std::string ReadSlot(std::string_view serial, int slot) const
		{
			const auto entry = FindSlot(serial, slot);
			const auto chain = Chain(entry.cluster);
			if (entry.size > chain.size() * 1024) throw std::runtime_error("Truncated card slot");
			std::string result(entry.size, '\0');
			for (size_t i = 0; i < result.size(); ++i)
				result[i] = bytes.at(Physical(size_t(chain[i / 1024] + allocOffset) * 1024 + i % 1024));
			return result;
		}

		const std::vector<char>& GetBytes() const { return bytes; }

	private:
		size_t Physical(size_t offset) const
		{
			const auto physical = offset / 512 * stride + offset % 512;
			if (physical >= bytes.size()) throw std::runtime_error("Card reference outside image");
			return physical;
		}
		uint32_t Word(size_t offset, unsigned count = 4) const
		{
			uint32_t value = 0;
			for (unsigned i = 0; i < count; ++i) value |= uint32_t(uint8_t(bytes.at(Physical(offset + i)))) << (8 * i);
			return value;
		}
		void SetByte(size_t offset, char value)
		{
			bytes.at(Physical(offset)) = value;
			modifiedPages.insert(offset / 512);
		}
		std::vector<uint32_t> Chain(uint32_t cluster) const
		{
			std::vector<uint32_t> result;
			std::set<uint32_t> seen;
			while (cluster != 0xffffffff) {
				if (cluster >= allocEnd || !seen.insert(cluster).second) throw std::runtime_error("Invalid or cyclic card allocation chain");
				result.push_back(cluster);
				const auto indirect = Word(0x50 + (cluster / 65536) * 4);
				const auto fat = Word(size_t(indirect) * 1024 + (cluster / 256 % 256) * 4);
				if (indirect >= allocOffset || fat >= allocOffset) throw std::runtime_error("Invalid card FAT reference");
				const auto next = Word(size_t(fat) * 1024 + (cluster % 256) * 4);
				if (!(next & 0x80000000)) throw std::runtime_error("Card chain references a free cluster");
				cluster = next == 0xffffffff ? next : next & 0x7fffffff;
			}
			return result;
		}
		Entry Find(uint32_t cluster, uint32_t count, std::string_view name, uint32_t mode) const
		{
			const auto chain = Chain(cluster);
			if (count > chain.size() * 2) throw std::runtime_error("Invalid card directory length");
			for (uint32_t i = 0; i < count; ++i) {
				const size_t offset = size_t(chain[i / 2] + allocOffset) * 1024 + (i % 2) * 512;
				if ((Word(offset, 2) & (0x8000 | mode)) != (0x8000 | mode)) continue;
				std::string entryName;
				for (unsigned j = 0; j < 32 && Word(offset + 64 + j, 1); ++j) entryName += char(Word(offset + 64 + j, 1));
				if (entryName == name) return {offset, Word(offset + 4), Word(offset + 16)};
			}
			throw std::runtime_error("No existing " + std::string(name) + " on this card");
		}
		void ValidateDirectory(uint32_t cluster, uint32_t count, std::set<uint32_t>& allocated, unsigned depth) const
		{
			if (depth > 32) throw std::runtime_error("Card directory nesting is invalid");
			const auto chain = Chain(cluster);
			for (auto item : chain) if (!allocated.insert(item).second) throw std::runtime_error("Card files share allocated clusters");
			if (count > chain.size() * 2) throw std::runtime_error("Invalid card directory length");
			for (uint32_t i = 0; i < count; ++i) {
				const size_t offset = size_t(chain[i / 2] + allocOffset) * 1024 + i % 2 * 512;
				const auto mode = Word(offset, 2);
				if (!(mode & 0x8000)) continue;
				// The first two entries are directory links and do not own clusters.
				const auto first = Word(offset + 64, 1);
				if (first == '.' && (Word(offset + 65, 1) == 0 ||
					(Word(offset + 65, 1) == '.' && Word(offset + 66, 1) == 0))) continue;
				const auto size = Word(offset + 4);
				const auto start = Word(offset + 16);
				if (mode & 0x20) ValidateDirectory(start, size, allocated, depth + 1);
				else if ((mode & 0x10) && size) {
					const auto fileChain = Chain(start);
					if (size > fileChain.size() * 1024) throw std::runtime_error("Truncated card file");
					for (auto item : fileChain) if (!allocated.insert(item).second)
						throw std::runtime_error("Card files share allocated clusters");
				}
			}
		}
		void UpdateEcc()
		{
			if (stride != 528) return;
			// Public-domain mymc ECC algorithm, also used by PCSX2 MemoryCardFile.cpp.
			constexpr uint8_t masks[] = {0x07, 0x16, 0x25, 0x34, 0x43, 0x52, 0x61, 0x70};
			for (auto page : modifiedPages) {
				for (unsigned chunk = 0; chunk < 4; ++chunk) {
					uint8_t column = 0x77, line0 = 0x7f, line1 = 0x7f;
					for (unsigned i = 0; i < 128; ++i) {
						const auto value = uint8_t(bytes[page * stride + chunk * 128 + i]);
						for (unsigned bit = 0; bit < 8; ++bit) if (value & (1 << bit)) column ^= masks[bit];
						if (std::popcount(value) & 1) { line0 ^= uint8_t(~i); line1 ^= uint8_t(i); }
					}
					const auto spare = page * stride + 512 + chunk * 3;
					bytes[spare] = char(column); bytes[spare + 1] = char(line0 & 0x7f); bytes[spare + 2] = char(line1);
				}
			}
			modifiedPages.clear();
		}

		std::vector<char> bytes;
		size_t stride = 512;
		uint32_t clusters = 0, allocOffset = 0, allocEnd = 0;
		std::set<size_t> modifiedPages;
	};
}
