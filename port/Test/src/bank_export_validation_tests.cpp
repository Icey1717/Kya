#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "Actor.h"
#include "ActorFactory.h"
#include "MemoryStream.h"
#include "edBank/edBankFile.h"

namespace
{
std::string EnvString(const char* Name)
{
	const char* Value = std::getenv(Name);
	return Value ? std::string(Value) : std::string();
}

double EnvNumber(const char* Name)
{
	const std::string Value = EnvString(Name);
	return Value.empty() ? 0.0 : std::stod(Value);
}
}

// End-to-end check for a bank exported by the Unreal editor. The test reads the
// file through the port's own bank container structures and parses the edited
// actor with CActor::Create, so the layout/behaviour is checked against the same
// code the running game uses. The actor chunk offset is supplied by the exporter
// test (KyaReadActorOffsets) and the expected values come from the edit.
TEST(RunningPortExport, ValidatesEditedBank)
{
	const std::string Filename = EnvString("KYA_VALIDATION_BANK");
	if (Filename.empty())
	{
		GTEST_SKIP() << "Set KYA_VALIDATION_BANK to the Unreal-exported bank.";
	}
	std::ifstream Input(Filename, std::ios::binary);
	ASSERT_TRUE(Input.good()) << "Cannot open " << Filename;
	const std::vector<std::uint8_t> Bytes((std::istreambuf_iterator<char>(Input)), {});
	ASSERT_GE(Bytes.size(), sizeof(edCFiler_Bnk_static_header));

	// Native container layout (see src/EdenLib/Include/edBank/edBankFile.h).
	const auto* StaticHeader = reinterpret_cast<const edCFiler_Bnk_static_header*>(Bytes.data());
	auto* FileHeader = const_cast<edCBankFileHeader*>(&StaticHeader->fileHeader);
	EXPECT_EQ(std::memcmp(FileHeader->header, "KNAB", 4), 0);
	ASSERT_GT(FileHeader->fileCount, 0u);

	const unsigned char* SceneData = nullptr;
	std::int64_t SceneSize = 0;
	for (unsigned Index = 0; Index < FileHeader->fileCount; ++Index)
	{
		const FileHeaderFileData* Entry = FileHeader->get_entry(Index);
		const FileTypeData* Type = FileHeader->get_entry_typepair(Index);
		if (!Entry || !Type) continue;
		if (Type->type == 6 && Type->stype == 1)
		{
			SceneData = reinterpret_cast<const unsigned char*>(FileHeader->get_entry_data(Index));
			SceneSize = Entry->size;
			break;
		}
	}
	ASSERT_NE(SceneData, nullptr) << "The exported bank has no scene entry.";
	EXPECT_GT(SceneSize, 0);

	const auto ActorOffset = static_cast<std::int64_t>(EnvNumber("KYA_VALIDATION_ACTOR_OFFSET"));
	const auto ActorType = static_cast<unsigned>(static_cast<int>(EnvNumber("KYA_VALIDATION_ACTOR_TYPE")));
	ASSERT_GT(ActorOffset, 0);
	ASSERT_LT(ActorOffset, static_cast<std::int64_t>(Bytes.size()));
	ASSERT_EQ(std::memcmp(StaticHeader->header, "\x40\x00\x00\x00", 4), 0);

	// Parse the record in the same order CActorManager::Level_AddAll and
	// CActor::Create consume it (src/b-witch/ActorManager.cpp, Actor.cpp).
	ByteCode Code;
	Code.data = reinterpret_cast<char*>(const_cast<std::uint8_t*>(Bytes.data()));
	Code.currentSeekPos = reinterpret_cast<char*>(const_cast<std::uint8_t*>(Bytes.data()) + ActorOffset);
	Code.size = static_cast<int>(Bytes.size());
	Code.streamEnd = reinterpret_cast<char*>(const_cast<std::uint8_t*>(Bytes.data()) + Bytes.size());
	Code.GetChunk();
	EXPECT_EQ(Code.GetS32(), 0);
	EXPECT_EQ(Code.GetU32(), ActorType);

	const char* ActorName = Code.GetString();
	ASSERT_NE(ActorName, nullptr);
	EXPECT_STREQ(ActorName, EnvString("KYA_VALIDATION_ACTOR_NAME").c_str());
	Code.Align(4);
	const auto* CinData = reinterpret_cast<const CinNamedObject30*>(Code.currentSeekPos);
	Code.currentSeekPos += sizeof(CinNamedObject30);

	const int MacroCount = Code.GetS32();
	if (MacroCount > 0) Code.currentSeekPos += MacroCount * 8;

	// Actor resource relocation table (count followed by relative jump pairs).
	Code.Align(4);
	const int RelocCount = Code.GetS32();
	char* const RelocTable = Code.currentSeekPos;
	if (RelocCount > 0) Code.currentSeekPos += RelocCount * 8;
	std::int64_t RelocDelta = 0;
	if (RelocCount > 0)
	{
		for (int Index = 0; Index < RelocCount; ++Index)
		{
			std::int32_t Relative = 0;
			std::memcpy(&Relative, RelocTable + Index * 8 + 4, sizeof(Relative));
			RelocDelta += Relative;
		}
	}
	Code.currentSeekPos += RelocDelta;

	const int ModelCount = Code.GetS32();
	for (int Model = 0; Model < ModelCount; ++Model)
	{
		Code.GetString();
		Code.GetString();
	}

	const auto* SubObj = reinterpret_cast<const KyaUpdateObjA*>(Code.currentSeekPos);
	Code.currentSeekPos += sizeof(KyaUpdateObjA);

	// CActor::Create copies these native values into pCinData/subObjA.
	EXPECT_NEAR(CinData->position.x, EnvNumber("KYA_VALIDATION_POSITION_X"), 1e-3);
	EXPECT_NEAR(CinData->position.y, EnvNumber("KYA_VALIDATION_POSITION_Y"), 1e-3);
	EXPECT_NEAR(CinData->position.z, EnvNumber("KYA_VALIDATION_POSITION_Z"), 1e-3);
	EXPECT_NEAR(SubObj->visibilityDistance, EnvNumber("KYA_VALIDATION_VISIBILITY"), 1e-3);
	EXPECT_NEAR(SubObj->cullingDistance, EnvNumber("KYA_VALIDATION_CULLING"), 1e-3);

	if (EnvString("KYA_VALIDATION_NATIVE_CREATE") != "1")
	{
		return;
	}

	// Integration check: run the port's own CActor creation on the exported record
	// and inspect the resulting object, not just the parsed bytes. The exporter
	// test selects a meshless, collisionless, behaviourless actor, so the native
	// Create path needs no scene or file-manager resources.
	ByteCode CreateCode;
	CreateCode.data = reinterpret_cast<char*>(const_cast<std::uint8_t*>(Bytes.data()));
	CreateCode.currentSeekPos = reinterpret_cast<char*>(const_cast<std::uint8_t*>(Bytes.data()) + ActorOffset);
	CreateCode.size = static_cast<int>(Bytes.size());
	CreateCode.streamEnd = reinterpret_cast<char*>(const_cast<std::uint8_t*>(Bytes.data()) + Bytes.size());
	CreateCode.GetChunk();
	EXPECT_EQ(CreateCode.GetS32(), 0);
	EXPECT_EQ(CreateCode.GetU32(), ActorType);

	int ActorSize = 0;
	CActor* const Actor = CActorFactory::Factory(static_cast<ACTOR_CLASS>(ActorType), 1, &ActorSize, nullptr);
	ASSERT_NE(Actor, nullptr);
	ASSERT_GT(ActorSize, 0);
	Actor->typeID = static_cast<ACTOR_CLASS>(ActorType);
	Actor->actorManagerIndex = 0;
	Actor->Create(&CreateCode);

	ASSERT_NE(Actor->pCinData, nullptr);
	ASSERT_NE(Actor->subObjA, nullptr);
	EXPECT_STREQ(Actor->name, EnvString("KYA_VALIDATION_ACTOR_NAME").c_str());
	EXPECT_NEAR(Actor->pCinData->position.x, EnvNumber("KYA_VALIDATION_POSITION_X"), 1e-3);
	EXPECT_NEAR(Actor->pCinData->position.y, EnvNumber("KYA_VALIDATION_POSITION_Y"), 1e-3);
	EXPECT_NEAR(Actor->pCinData->position.z, EnvNumber("KYA_VALIDATION_POSITION_Z"), 1e-3);
	EXPECT_NEAR(Actor->subObjA->visibilityDistance, EnvNumber("KYA_VALIDATION_VISIBILITY"), 1e-3);
	EXPECT_NEAR(Actor->subObjA->cullingDistance, EnvNumber("KYA_VALIDATION_CULLING"), 1e-3);
}
