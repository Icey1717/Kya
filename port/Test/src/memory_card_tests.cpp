#include <gtest/gtest.h>
#include "../../DebugMenu/src/DebugMemoryCard.h"
#ifdef PLATFORM_WIN
#include "../../../src/EdenLib/edFile/sources/ps2/WinSaveFile.h"
#include "../../DebugMenu/src/DebugMemoryCardFile.h"
#endif

namespace
{
	constexpr const char* serial = "BESLES-51473-KYA";
	std::vector<char> CardFixture(bool ecc)
	{
		const size_t stride = ecc ? 528 : 512;
		std::vector<char> image(64 * 2 * stride, char(0xff));
		auto write = [&](size_t offset, const void* source, size_t size) {
			const auto* data = static_cast<const char*>(source);
			for (size_t i = 0; i < size; ++i) image[(offset + i) / 512 * stride + (offset + i) % 512] = data[i];
		};
		auto word = [&](size_t offset, uint32_t value) { write(offset, &value, 4); };
		write(0, "Sony PS2 Memory Card Format ", 28);
		word(0x28, 0x00020200); word(0x30, 64); word(0x34, 4); word(0x38, 58); word(0x3c, 0);
		word(0x150, 2); word(0x50, 1); word(1024, 2);
		word(2048, 0xffffffff); word(2052, 0xffffffff); word(2056, 0x80000003); word(2060, 0xffffffff);
		auto entry = [&](size_t offset, uint32_t mode, const char* name, uint32_t count, uint32_t cluster) {
			const char empty[512] = {};
			write(offset, empty, 512); word(offset, mode); word(offset + 4, count); word(offset + 16, cluster);
			write(offset + 64, name, std::strlen(name) + 1);
		};
		entry(4096, 0x8020, ".", 2, 0);
		entry(4608, 0x8020, serial, 2, 1);
		entry(5120, 0x8020, ".", 0, 0);
		entry(5632, 0x8010, "slot_0.dat", 1800, 2);
		return image;
	}
}

TEST(MemoryCardExport, ReplacesAcrossPagesAndClustersWithoutChangingOtherData)
{
	for (bool ecc : {false, true}) {
		const auto original = CardFixture(ecc);
		Debug::SaveLoad::MemoryCard card(original);
		std::string save(1900, '\0');
		for (size_t i = 0; i < save.size(); ++i) save[i] = char(i * 17);
		card.ReplaceSlot(serial, 0, save);
		Debug::SaveLoad::MemoryCard reopened(card.GetBytes());
		EXPECT_EQ(reopened.ReadSlot(serial, 0), save);
		const size_t stride = ecc ? 528 : 512;
		for (size_t i = 0; i < original.size(); ++i) {
			const auto logical = i / stride * 512 + i % stride;
			const bool payload = logical >= 6144 && logical < 6144 + save.size() && i % stride < 512;
			const bool length = logical >= 5636 && logical < 5640 && i % stride < 512;
			const bool changedPage = i / stride == 11 || (i / stride >= 12 && i / stride <= 15);
			const bool parity = ecc && changedPage && i % stride >= 512 && i % stride < 524;
			if (!payload && !length && !parity) ASSERT_EQ(original[i], card.GetBytes()[i]) << "Offset " << i;
		}
	}
}

TEST(MemoryCardExport, RejectsMissingSavesAndOversizedReplacement)
{
	Debug::SaveLoad::MemoryCard card(CardFixture(false));
	const auto original = card.GetBytes();
	EXPECT_THROW(card.ReplaceSlot(serial, 1, "save"), std::runtime_error);
	EXPECT_THROW(card.ReplaceSlot("BASLUS-20440-KYA", 0, "save"), std::runtime_error);
	EXPECT_THROW(card.ReplaceSlot(serial, 0, std::string(2049, 'x')), std::runtime_error);
	EXPECT_EQ(original, card.GetBytes());
}

TEST(MemoryCardExport, RejectsCyclicAndOutOfRangeAllocation)
{
	for (uint32_t next : {0x80000002u, 0x80001000u, 0u}) {
		auto bytes = CardFixture(false);
		std::memcpy(bytes.data() + 2056, &next, 4);
		Debug::SaveLoad::MemoryCard card(bytes);
		EXPECT_THROW(card.ReplaceSlot(serial, 0, "save"), std::runtime_error);
		EXPECT_EQ(bytes, card.GetBytes());
	}
}

TEST(MemoryCardExport, RegeneratesKnownEccForZeroPage)
{
	Debug::SaveLoad::MemoryCard card(CardFixture(true));
	card.ReplaceSlot(serial, 0, std::string(1024, '\0'));
	for (size_t page : {12u, 13u}) {
		for (size_t chunk = 0; chunk < 4; ++chunk) {
			const auto offset = page * 528 + 512 + chunk * 3;
			EXPECT_EQ(uint8_t(card.GetBytes()[offset]), 0x77);
			EXPECT_EQ(uint8_t(card.GetBytes()[offset + 1]), 0x7f);
			EXPECT_EQ(uint8_t(card.GetBytes()[offset + 2]), 0x7f);
		}
	}
}

TEST(MemoryCardExport, RejectsSlotSharingTheDirectoryAllocation)
{
	auto bytes = CardFixture(false);
	const uint32_t directoryCluster = 1;
	std::memcpy(bytes.data() + 5632 + 16, &directoryCluster, 4);
	const uint32_t length = 512;
	std::memcpy(bytes.data() + 5632 + 4, &length, 4);
	Debug::SaveLoad::MemoryCard card(bytes);
	EXPECT_THROW(card.ReplaceSlot(serial, 0, "save"), std::runtime_error);
	EXPECT_EQ(card.GetBytes(), bytes);
}

#ifdef PLATFORM_WIN
TEST(MemoryCardExport, PublishesAtomicallyWhileHoldingWriterExclusion)
{
	const auto directory = std::filesystem::temp_directory_path() / ("kya-card-test-" + std::to_string(GetCurrentProcessId()));
	std::filesystem::create_directories(directory);
	const auto path = directory / "card.ps2";
	{ std::ofstream stream(path, std::ios::binary); stream << "original"; }
	HANDLE lock = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, 0, nullptr);
	ASSERT_NE(lock, INVALID_HANDLE_VALUE);
	HANDLE writer = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	EXPECT_EQ(writer, INVALID_HANDLE_VALUE);
	if (writer != INVALID_HANDLE_VALUE) CloseHandle(writer);
	EXPECT_NO_THROW(Debug::SaveLoad::PublishMemoryCard(path, "replacement"));
	CloseHandle(lock);
	std::ifstream stream(path, std::ios::binary);
	std::string data((std::istreambuf_iterator<char>(stream)), {});
	EXPECT_EQ(data, "replacement");
	stream.close();
	std::filesystem::remove(path);
	std::filesystem::remove(directory);
}
#endif
