#include <gtest/gtest.h>

#ifdef PLATFORM_WIN
#include "../../../src/EdenLib/edFile/sources/ps2/WinSaveFile.h"
#include "../../DebugMenu/src/DebugSaveLoadPaths.h"
#include "../../DebugMenu/src/DebugSaveCheckpoint.h"
#include "edFile/edFilePath.h"
#include "SaveManagement.h"
#include "edFile/edFileCRC32.h"
#include <iterator>
#include <algorithm>
#include <cstring>
#include <vector>

namespace
{
	void AppendSaveWord(std::string& bytes, uint32_t value)
	{
		bytes.append(reinterpret_cast<const char*>(&value), sizeof(value));
	}

	std::string SaveChunk(uint32_t hash, const std::string& payload)
	{
		std::string bytes;
		for (auto word : {0x06667666u, hash, 0u, static_cast<uint32_t>(payload.size())}) AppendSaveWord(bytes, word);
		return bytes + payload;
	}

	std::string CheckpointSave(bool truncatedActor = false)
	{
		std::string header, levelHeader, classes;
		for (auto word : {3u, 4u, 100u, 0u, 0u}) AppendSaveWord(header, word);
		for (auto word : {3u, 0u, 0u}) AppendSaveWord(levelHeader, word);
		// Include another class before the hero to exercise actor record skipping.
		for (auto word : {2u, 1u, 8u, 2u, 3u, 0x138u, 1u}) AppendSaveWord(classes, word);
		std::string actors(24 + 4 + 0x138, '\0');
		const float position[] = {10.5f, -2.0f, 30.0f};
		std::memcpy(actors.data() + 28, position, sizeof(position));
		const uint32_t sector = 4;
		std::memcpy(actors.data() + 28 + 24, &sector, sizeof(sector));
		if (truncatedActor) actors.pop_back();
		return SaveChunk(0x56415342, SaveChunk(0x44485342, header) +
			SaveChunk(0x56454c42, SaveChunk(0x44484c42, levelHeader) +
				SaveChunk(0x4c434c42, classes) + SaveChunk(0x43414c42, actors)));
	}
}

TEST(WindowsSaveCheckpoint, ReadsHeroCheckpointWithoutLoadingGame)
{
	Debug::SaveLoad::SavedCheckpoint checkpoint;
	ASSERT_TRUE(Debug::SaveLoad::ReadSavedCheckpoint(CheckpointSave(), checkpoint));
	EXPECT_EQ(checkpoint.level, 3);
	EXPECT_EQ(checkpoint.sector, 4);
	ASSERT_TRUE(checkpoint.hasPosition);
	EXPECT_FLOAT_EQ(checkpoint.position[0], 10.5f);
	EXPECT_FLOAT_EQ(checkpoint.position[1], -2.0f);
	EXPECT_FLOAT_EQ(checkpoint.position[2], 30.0f);
}

TEST(WindowsSaveCheckpoint, ArchiveIdentitySurvivesReloadAndDistinguishesCheckpoints)
{
	Debug::SaveLoad::SavedCheckpoint checkpoint, reloaded;
	ASSERT_TRUE(Debug::SaveLoad::ReadSavedCheckpoint(CheckpointSave(), checkpoint));
	ASSERT_TRUE(Debug::SaveLoad::ReadSavedCheckpoint(CheckpointSave(), reloaded));
	const auto name = Debug::SaveLoad::GetCheckpointArchiveName(checkpoint);
	ASSERT_FALSE(name.empty());
	EXPECT_EQ(name, Debug::SaveLoad::GetCheckpointArchiveName(reloaded));
	EXPECT_EQ(std::filesystem::path(name).filename(), name);
	reloaded.level++;
	EXPECT_NE(name, Debug::SaveLoad::GetCheckpointArchiveName(reloaded));
	reloaded = checkpoint;
	reloaded.sector++;
	EXPECT_NE(name, Debug::SaveLoad::GetCheckpointArchiveName(reloaded));
	reloaded = checkpoint;
	reloaded.position[0] += 1.0f;
	EXPECT_NE(name, Debug::SaveLoad::GetCheckpointArchiveName(reloaded));
	reloaded = checkpoint;
	reloaded.rotation[1] = 1.0f;
	EXPECT_NE(name, Debug::SaveLoad::GetCheckpointArchiveName(reloaded));
	reloaded = checkpoint;
	reloaded.rotation[0] = -0.0f;
	EXPECT_EQ(name, Debug::SaveLoad::GetCheckpointArchiveName(reloaded));
	reloaded.hasPosition = false;
	EXPECT_TRUE(Debug::SaveLoad::GetCheckpointArchiveName(reloaded).empty());
}

TEST(WindowsSaveCheckpoint, RejectsTruncatedChunksAndActorRecords)
{
	Debug::SaveLoad::SavedCheckpoint checkpoint;
	const auto save = CheckpointSave();
	for (size_t size = 0; size < save.size(); ++size) {
		EXPECT_FALSE(Debug::SaveLoad::ReadSavedCheckpoint(std::string_view(save.data(), size), checkpoint));
	}
	EXPECT_FALSE(Debug::SaveLoad::ReadSavedCheckpoint(CheckpointSave(true), checkpoint));
}

TEST(WindowsSaveCheckpoint, ReportsSectorWhenHeroStateIsAbsent)
{
	std::string header;
	for (auto word : {1u, 2u, 0u, 0u, 0u}) AppendSaveWord(header, word);
	Debug::SaveLoad::SavedCheckpoint checkpoint;
	ASSERT_TRUE(Debug::SaveLoad::ReadSavedCheckpoint(SaveChunk(0x56415342, SaveChunk(0x44485342, header)), checkpoint));
	EXPECT_EQ(checkpoint.sector, 2);
	EXPECT_FALSE(checkpoint.hasPosition);
}

TEST(WindowsSavePaths, ResolvesFullWidthSaveManagerSerials)
{
	char pathBuffer[4] = {};
	char drive[8] = {};
	char cardRoot[] = "<mc>0:\\";
	edFilePathSplit(drive, pathBuffer, nullptr, nullptr, cardRoot);
	ASSERT_STREQ(pathBuffer, "0:");
	char serialBuffer[16];
	std::copy_n("BESLES-51473-KYA", sizeof(serialBuffer), serialBuffer);
	std::filesystem::path directory;
	ASSERT_TRUE(Debug::SaveLoad::TryGetBackupDirectory(
		{pathBuffer, sizeof(pathBuffer)}, {serialBuffer, sizeof(serialBuffer)}, directory));
	EXPECT_EQ(directory, std::filesystem::absolute("BESLES-51473-KYA"));
	char otherSerial[16];
	std::copy_n("BASLUS-20440-KYA", sizeof(otherSerial), otherSerial);
	ASSERT_TRUE(Debug::SaveLoad::TryGetBackupDirectory(
		{pathBuffer, sizeof(pathBuffer)}, {otherSerial, sizeof(otherSerial)}, directory));
	EXPECT_EQ(directory, std::filesystem::absolute("BASLUS-20440-KYA"));
}

TEST(WindowsSavePaths, RejectsUnterminatedOrUninitializedBuffers)
{
	std::filesystem::path directory;
	const std::string_view validPath("0:\0", 3);
	const std::string_view validSerial("BESLES-51473-KYA\0", 16);
	EXPECT_FALSE(Debug::SaveLoad::TryGetBackupDirectory("xxxx", validSerial, directory));
	EXPECT_FALSE(Debug::SaveLoad::TryGetBackupDirectory(std::string_view("\0", 1), validSerial, directory));
	EXPECT_FALSE(Debug::SaveLoad::TryGetBackupDirectory(validPath, std::string_view("\0", 1), directory));
	EXPECT_FALSE(Debug::SaveLoad::TryGetBackupDirectory(validPath, std::string_view("..\0", 3), directory));
}

TEST(WindowsSavePaths, IgnoresBytesAfterSerialTerminator)
{
	const char serialBuffer[16] = {'K', 'Y', 'A', '\0', 'x'};
	std::filesystem::path directory;
	ASSERT_TRUE(Debug::SaveLoad::TryGetBackupDirectory(std::string_view("0:\0", 3),
		{serialBuffer, sizeof(serialBuffer)}, directory));
	EXPECT_EQ(directory, std::filesystem::absolute("KYA"));
}

class WindowsSave : public testing::Test
{
protected:
	std::filesystem::path directory;
	std::filesystem::path destination;

	void SetUp() override
	{
		wchar_t name[MAX_PATH];
		ASSERT_NE(GetTempFileNameW(std::filesystem::temp_directory_path().c_str(), L"kyt", 0, name), 0u);
		ASSERT_TRUE(DeleteFileW(name));
		directory = name;
		ASSERT_TRUE(std::filesystem::create_directory(directory));
		destination = directory / "slot_0.dat";
		std::ofstream(destination, std::ios::binary) << "original save";
	}

	void TearDown() override
	{
		std::error_code error;
		std::filesystem::remove_all(directory, error);
	}

	std::string Read()
	{
		std::ifstream input(destination, std::ios::binary);
		return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
	}
};

TEST_F(WindowsSave, PublishesOnlyOnSuccessfulClose)
{
	WinSaveFile save(destination);
	ASSERT_TRUE(save.IsOpen());
	ASSERT_TRUE(save.Write("new", 3));
	EXPECT_EQ(Read(), "original save");
	ASSERT_TRUE(save.Write(" save", 5));
	ASSERT_TRUE(save.Commit());
	EXPECT_EQ(Read(), "new save");
}

TEST_F(WindowsSave, AbandonedWritePreservesOriginal)
{
	{
		WinSaveFile save(destination);
		ASSERT_TRUE(save.IsOpen());
		ASSERT_TRUE(save.Write("partial", 7));
	}
	EXPECT_EQ(Read(), "original save");
	EXPECT_EQ(std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator()), 1);
}

TEST_F(WindowsSave, FailedReplacementPreservesOriginal)
{
	HANDLE lock = CreateFileW(destination.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	ASSERT_NE(lock, INVALID_HANDLE_VALUE);
	{
		WinSaveFile save(destination);
		EXPECT_TRUE(save.IsOpen());
		EXPECT_TRUE(save.Write("replacement", 11));
		EXPECT_FALSE(save.Commit());
		EXPECT_EQ(Read(), "original save");
	}
	CloseHandle(lock);
	EXPECT_EQ(Read(), "original save");
}

TEST_F(WindowsSave, CreatesNewSaveAndRejectsMissingDirectory)
{
	WinSaveFile missing(directory / "missing" / "slot.dat");
	EXPECT_FALSE(missing.IsOpen());
	EXPECT_FALSE(missing.Commit());
	EXPECT_EQ(Read(), "original save");
	const auto newPath = directory / "slot_1.dat";
	WinSaveFile save(newPath);
	ASSERT_TRUE(save.IsOpen());
	ASSERT_TRUE(save.Write("new", 3));
	EXPECT_FALSE(std::filesystem::exists(newPath));
	ASSERT_TRUE(save.Commit());
	EXPECT_EQ(std::filesystem::file_size(newPath), 3u);
}
TEST_F(WindowsSave, AutosaveRetainsTenPreviousVersionsPerSlot)
{
	for (int slot = 0; slot < 2; ++slot) {
		const auto path = directory / ("slot_" + std::to_string(slot) + ".dat");
		std::ofstream(path, std::ios::binary) << "version 0";
		for (int version = 1; version <= 12; ++version) {
			const std::string contents = "version " + std::to_string(version);
			WinSaveFile save(path, 0x2e);
			ASSERT_TRUE(save.IsOpen());
			ASSERT_TRUE(save.Write(contents.data(), static_cast<unsigned int>(contents.size())));
			ASSERT_TRUE(save.Commit());
		}
		for (int index = 1; index <= 10; ++index) {
			auto backup = path;
			backup += ".bak." + std::to_string(index);
			std::ifstream input(backup, std::ios::binary);
			const std::string contents{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
			EXPECT_EQ(contents, "version " + std::to_string(12 - index));
		}
	}
	EXPECT_EQ(std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator()), 22);
}

TEST_F(WindowsSave, ManualSaveAndOtherFilesDoNotCreateBackups)
{
	{
		WinSaveFile save(destination, 6);
		ASSERT_TRUE(save.Write("manual", 6));
		ASSERT_TRUE(save.Commit());
	}
	const auto settings = directory / "settings.dat";
	std::ofstream(settings) << "old settings";
	{
		WinSaveFile save(settings, 0x2e);
		ASSERT_TRUE(save.Write("settings", 8));
		ASSERT_TRUE(save.Commit());
	}
	EXPECT_EQ(std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator()), 2);
}

TEST_F(WindowsSave, FailedBackupRotationPreventsAutosaveReplacement)
{
	auto backup = destination;
	backup += ".bak.1";
	std::ofstream(backup) << "older save";
	HANDLE lock = CreateFileW(backup.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	ASSERT_NE(lock, INVALID_HANDLE_VALUE);
	{
		WinSaveFile save(destination, 0x2e);
		EXPECT_TRUE(save.Write("replacement", 11));
		EXPECT_FALSE(save.Commit());
	}
	CloseHandle(lock);
	EXPECT_EQ(Read(), "original save");
	EXPECT_EQ(std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator()), 2);
}

TEST_F(WindowsSave, FirstAutosaveNeedsNoBackup)
{
	const auto path = directory / "slot_3.dat";
	WinSaveFile save(path, 0x2e);
	ASSERT_TRUE(save.Write("first", 5));
	ASSERT_TRUE(save.Commit());
	EXPECT_EQ(std::filesystem::file_size(path), 5u);
	EXPECT_EQ(std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator()), 2);
}

TEST_F(WindowsSave, FailedBackupCopyPreservesOriginal)
{
	HANDLE lock = CreateFileW(destination.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	ASSERT_NE(lock, INVALID_HANDLE_VALUE);
	{
		WinSaveFile save(destination, 0x2e);
		EXPECT_TRUE(save.Write("replacement", 11));
		EXPECT_FALSE(save.Commit());
	}
	CloseHandle(lock);
	EXPECT_EQ(Read(), "original save");
	EXPECT_EQ(std::distance(std::filesystem::directory_iterator(directory), std::filesystem::directory_iterator()), 1);
}

namespace
{
	std::string BuildBackupSaveBytes(const std::string& mainBlock)
	{
		SaveDataDesc desc;
		SaveDataHeader header{};
		header.hash = 0x4544454e;
		header.headerSize = sizeof(SaveDataHeader);
		header.initialBlockSize = sizeof(SaveDataDesc);
		header.mainBlockSize = static_cast<int>(mainBlock.size());
		header.initialBlockCrc = edFileComputeCRC32(&desc, sizeof(desc));
		header.mainBlockCrc = edFileComputeCRC32(
			const_cast<char*>(mainBlock.data()), static_cast<uint>(mainBlock.size()));
		header.headerCrc = edFileComputeCRC32(&header.headerSize, 0x14);

		std::string bytes;
		bytes.append(reinterpret_cast<const char*>(&header), sizeof(header));
		bytes.append(reinterpret_cast<const char*>(&desc), sizeof(desc));
		bytes.append(mainBlock);
		return bytes;
	}

	constexpr size_t kBackupPrefixSize = sizeof(SaveDataHeader) + sizeof(SaveDataDesc);
}

class SaveManagementStage : public testing::Test
{
protected:
	std::vector<char> stagingBuffer;
	void* previousAlloc = nullptr;
	int previousMaxBufferSize = 0;
	uint previousSaveSize = 0;
	int previousSlotID = 0;
	uint previousFileExistsFlags = 0;
	SaveDataHeader previousHeader{};
	SaveDataDesc previousDescs[4];

	void SetUp() override
	{
		previousAlloc = gSaveManagement.pBigAlloc_0x34;
		previousMaxBufferSize = gSaveManagement.gameSaveMaxBufferSize;
		previousSaveSize = gSaveManagement.saveSize_0x44;
		previousSlotID = gSaveManagement.slotID_0x28;
		previousFileExistsFlags = gSaveManagement.fileExistsFlags;
		previousHeader = gSaveManagement.saveDataHeader;
		std::memcpy(previousDescs, gSaveManagement.aSaveDataDescriptions, sizeof(previousDescs));

		// Sentinel-fill the staging buffer and saveSize so tests can prove
		// rejected input leaves both completely untouched.
		stagingBuffer.assign(0x10000, static_cast<char>(0xCC));
		gSaveManagement.pBigAlloc_0x34 = reinterpret_cast<SaveBigAlloc*>(stagingBuffer.data());
		gSaveManagement.gameSaveMaxBufferSize = 0x10000;
		gSaveManagement.saveSize_0x44 = 0xdeadbeefu;
		gSaveManagement.slotID_0x28 = 7;
		gSaveManagement.fileExistsFlags = 0xa5a5a5a5u;
		gSaveManagement.saveDataHeader = SaveDataHeader{};
		gSaveManagement.saveDataHeader.hash = 0x11223344u;
		for (auto& desc : gSaveManagement.aSaveDataDescriptions) desc.levelId = 0x99;
	}

	void TearDown() override
	{
		gSaveManagement.pBigAlloc_0x34 = reinterpret_cast<SaveBigAlloc*>(previousAlloc);
		gSaveManagement.gameSaveMaxBufferSize = previousMaxBufferSize;
		gSaveManagement.saveSize_0x44 = previousSaveSize;
		gSaveManagement.slotID_0x28 = previousSlotID;
		gSaveManagement.fileExistsFlags = previousFileExistsFlags;
		gSaveManagement.saveDataHeader = previousHeader;
		std::memcpy(gSaveManagement.aSaveDataDescriptions, previousDescs, sizeof(previousDescs));
	}

	void ExpectUntouched(int expectedSlotID, uint expectedSaveSize, uint expectedFileExistsFlags,
		const SaveDataHeader& expectedHeader)
	{
		EXPECT_EQ(gSaveManagement.slotID_0x28, expectedSlotID);
		EXPECT_EQ(gSaveManagement.saveSize_0x44, expectedSaveSize);
		EXPECT_EQ(gSaveManagement.fileExistsFlags, expectedFileExistsFlags);
		EXPECT_EQ(std::memcmp(&gSaveManagement.saveDataHeader, &expectedHeader, sizeof(SaveDataHeader)), 0);
		for (auto& desc : gSaveManagement.aSaveDataDescriptions) EXPECT_EQ(desc.levelId, 0x99u);
		EXPECT_TRUE(std::all_of(stagingBuffer.begin(), stagingBuffer.end(),
			[](char c) { return c == static_cast<char>(0xCC); }));
	}
};

TEST_F(SaveManagementStage, StagesValidPayloadAndUpdatesOnlySaveSize)
{
	const std::string payload = "direct-load-backup-payload";
	const auto bytes = BuildBackupSaveBytes(payload);
	const int slotBefore = gSaveManagement.slotID_0x28;
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;

	EXPECT_TRUE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));

	EXPECT_EQ(gSaveManagement.saveSize_0x44, payload.size());
	EXPECT_EQ(std::memcmp(stagingBuffer.data(), payload.data(), payload.size()), 0);
	// Bytes past the staged payload must remain untouched.
	EXPECT_TRUE(std::all_of(stagingBuffer.begin() + payload.size(), stagingBuffer.end(),
		[](char c) { return c == static_cast<char>(0xCC); }));
	EXPECT_EQ(gSaveManagement.slotID_0x28, slotBefore);
	EXPECT_EQ(std::memcmp(&gSaveManagement.saveDataHeader, &headerBefore, sizeof(SaveDataHeader)), 0);
	for (auto& desc : gSaveManagement.aSaveDataDescriptions) EXPECT_EQ(desc.levelId, 0x99u);
}

TEST_F(SaveManagementStage, RejectsNullData)
{
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(nullptr, 64));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsMissingDestinationAllocation)
{
	gSaveManagement.pBigAlloc_0x34 = nullptr;
	const auto bytes = BuildBackupSaveBytes("payload");
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsBufferShorterThanOuterHeader)
{
	const auto bytes = BuildBackupSaveBytes("payload");
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), sizeof(SaveDataHeader) - 1));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsWrongHeaderHash)
{
	auto bytes = BuildBackupSaveBytes("payload");
	uint32_t badHash = 0;
	std::memcpy(bytes.data(), &badHash, sizeof(badHash));
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsWrongHeaderSize)
{
	auto bytes = BuildBackupSaveBytes("payload");
	SaveDataHeader header;
	std::memcpy(&header, bytes.data(), sizeof(header));
	header.headerSize = sizeof(SaveDataHeader) - 1;
	header.headerCrc = edFileComputeCRC32(&header.headerSize, 0x14);
	std::memcpy(bytes.data(), &header, sizeof(header));
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsCorruptHeaderCrc)
{
	auto bytes = BuildBackupSaveBytes("payload");
	// Flip a header-covered byte without recomputing headerCrc.
	bytes[static_cast<size_t>(offsetof(SaveDataHeader, initialBlockSize))] ^= 0xFF;
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsNegativeInitialBlockSize)
{
	auto bytes = BuildBackupSaveBytes("payload");
	SaveDataHeader header;
	std::memcpy(&header, bytes.data(), sizeof(header));
	header.initialBlockSize = -1;
	header.headerCrc = edFileComputeCRC32(&header.headerSize, 0x14);
	std::memcpy(bytes.data(), &header, sizeof(header));
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsNegativeMainBlockSize)
{
	auto bytes = BuildBackupSaveBytes("payload");
	SaveDataHeader header;
	std::memcpy(&header, bytes.data(), sizeof(header));
	header.mainBlockSize = -1;
	header.headerCrc = edFileComputeCRC32(&header.headerSize, 0x14);
	std::memcpy(bytes.data(), &header, sizeof(header));
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsZeroMainBlockSize)
{
	auto bytes = BuildBackupSaveBytes("");
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsWrongInitialBlockSize)
{
	auto bytes = BuildBackupSaveBytes("payload");
	SaveDataHeader header;
	std::memcpy(&header, bytes.data(), sizeof(header));
	header.initialBlockSize = sizeof(SaveDataDesc) - 1;
	header.headerCrc = edFileComputeCRC32(&header.headerSize, 0x14);
	std::memcpy(bytes.data(), &header, sizeof(header));
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsCorruptDescriptorCrc)
{
	auto bytes = BuildBackupSaveBytes("payload");
	bytes[sizeof(SaveDataHeader)] ^= 0xFF;
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsCorruptMainBlockCrc)
{
	auto bytes = BuildBackupSaveBytes("payload");
	bytes[kBackupPrefixSize] ^= 0xFF;
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsMainBlockLargerThanSuppliedBytes)
{
	auto bytes = BuildBackupSaveBytes("payload");
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size() - 1));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsMainBlockLargerThanFixedAllocation)
{
	const std::string payload(0x10001, 'x');
	const auto bytes = BuildBackupSaveBytes(payload);
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}

TEST_F(SaveManagementStage, RejectsMainBlockLargerThanGameSaveMaxBufferSize)
{
	const std::string payload(64, 'x');
	const auto bytes = BuildBackupSaveBytes(payload);
	gSaveManagement.gameSaveMaxBufferSize = static_cast<int>(payload.size()) - 1;
	const SaveDataHeader headerBefore = gSaveManagement.saveDataHeader;
	const uint fileExistsFlagsBefore = gSaveManagement.fileExistsFlags;
	EXPECT_FALSE(gSaveManagement.stage_backup_save(bytes.data(), bytes.size()));
	ExpectUntouched(7, 0xdeadbeefu, fileExistsFlagsBefore, headerBefore);
}
#endif
