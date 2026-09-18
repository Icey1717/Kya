#include <gtest/gtest.h>

#ifdef PLATFORM_WIN
#include "../../../src/EdenLib/edFile/sources/ps2/WinSaveFile.h"
#include <iterator>

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
#endif
