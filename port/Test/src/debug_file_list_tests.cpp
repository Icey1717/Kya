#include <gtest/gtest.h>
#include "../../DebugMenu/src/DebugFileList.h"
#include <chrono>
#include <fstream>

class DebugFileListTest : public ::testing::Test
{
protected:
	std::filesystem::path root;
	void SetUp() override
	{
		root = std::filesystem::temp_directory_path() /
			("kya-file-list-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
		std::filesystem::create_directory(root);
	}
	void TearDown() override
	{
		std::error_code error;
		std::filesystem::remove_all(root, error);
	}
	void Write(const std::filesystem::path& path) { std::ofstream file(path); file << "test"; }
};

TEST_F(DebugFileListTest, KeepsSnapshotUntilExplicitRefresh)
{
	Write(root / "b.chkp");
	Write(root / "ignored.rec");
	Debug::FileList list(root, ".chkp");
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"b.chkp"}));
	Write(root / "a.chkp");
	std::filesystem::remove(root / "b.chkp");
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"b.chkp"}));
	list.Invalidate();
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"a.chkp"}));
}

TEST_F(DebugFileListTest, DirectorySwitchImmediatelyReplacesSnapshot)
{
	const auto other = root / "other-level";
	std::filesystem::create_directory(other);
	Write(root / "first.chkp");
	Write(other / "second.chkp");
	Debug::FileList list(root, ".chkp");
	ASSERT_EQ(list.GetFiles(), (std::vector<std::string>{"first.chkp"}));
	list.SetDirectory(other);
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"second.chkp"}));
	list.SetDirectory(root);
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"first.chkp"}));
}

TEST_F(DebugFileListTest, MissingDirectoryAndBackupChangesRefreshOnInvalidation)
{
	const auto recordings = root / "recordings";
	Debug::FileList list(recordings, ".rec");
	ASSERT_TRUE(list.GetFiles().empty());
	std::filesystem::create_directory(recordings);
	Write(recordings / "last_recording_backup.rec");
	Write(recordings / "a.rec");
	EXPECT_TRUE(list.GetFiles().empty());
	list.Invalidate();
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"a.rec", "last_recording_backup.rec"}));
	std::filesystem::remove(recordings / "last_recording_backup.rec");
	list.Invalidate();
	EXPECT_EQ(list.GetFiles(), (std::vector<std::string>{"a.rec"}));
}
