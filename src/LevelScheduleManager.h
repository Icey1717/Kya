#ifndef _LEVELSCHEDULEMANAGER_H
#define _LEVELSCHEDULEMANAGER_H

#include "Types.h"

#include "edC/edCBank.h"

#include <stdlib.h>
#include "LargeObject.h"
#include "ScenaricCondition.h"
#include "SectorManager.h"

#define IMPLEMENTATION_GUARD_OBJECTIVE(x)

#define SCN_GAME_NUM_FREED_WOLFENS		0x0
#define SCN_GAME_COMPANION				0x3
#define SCN_ABILITY_JUMP_BOUNCE			0x7
#define SCN_ABILITY_CLIMB				0x8
#define SCN_ABILITY_BOOMY_TYPE			0x9
#define SCN_ABILITY_FIGHT				0xa
#define SCN_ABILITY_MAGIC_EXORCISM		0xb
#define SCN_ABILITY_MAGIC_REGENERATE	0xc
#define SCN_ABILITY_JAMGUT_RIDE			0xd
#define SCN_ABILITY_BINOCULARS			0xe
#define SCN_ABILITY_RECEPTACLE			0xf
#define SCN_LEVEL_MAGIC_BOARD			0x10
#define SCN_LEVEL_MAGIC_GAUGE			0x11
#define SCN_LEVEL_MAGIC_UPDATE			0x12
#define SCN_LEVEL_MAGIC_MAX				0x13
#define SCN_LEVEL_LIFE_GAUGE			0x14
#define SCN_LEVEL_LIFE_UPDATE			0x15
#define SCN_LEVEL_LIFE_MAX				0x16

#define SCN_MAX							0x17

#define SAVEGAME_CHUNK_BSAV 0x56415342
#define SAVEGAME_CHUNK_BSHD 0x44485342
#define SAVEGAME_CHUNK_BSCN 0x4E435342
#define SAVEGAME_CHUNK_BGNF 0x464E4742
#define SAVEGAME_CHUNK_BLEV 0x56454C42
#define SAVEGAME_CHUNK_BLHD 0x44484C42
#define SAVEGAME_CHUNK_BLCL 0x4C434C42
#define SAVEGAME_CHUNK_BLAC 0x43414C42
#define SAVEGAME_CHUNK_BLCI 0x49434C42
#define SAVEGAME_CHUNK_BLMP 0x504D4C42
#define SAVEGAME_CHUNK_BOBJ 0x4A424F42

class CActorNativShop;

struct SaveDataDesc;

struct SubSectorSaveData
{
	uint flags;
	int nbExorcisedWolfen;
};

struct SaveDataChunk_BLHD
{
	int levelId;
	int nbExorcisedWolfen;
	int maxElevatorId;
	SubSectorSaveData aSubSectorData[];
};

struct SaveDataChunk_BSHD
{
	int levelId;
	int sectorId;
	int gameTime;
	int field_0xc;
	int musicId;
};

static_assert(sizeof(SaveDataChunk_BSHD) == 0x14);	

struct LoadLoopObject_50
{
	int field_0x0;
	undefined4 field_0x4;
	ulong messageKey;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	int levelId;
	undefined4 field_0x24;
	undefined4 field_0x28;
	undefined4 field_0x2c;
	edF32VECTOR4 field_0x30;
	undefined8 field_0x40;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
};

struct SaveDataChunk_BLEV {
	int field_0x0;
	int field_0x4;
	uint field_0x8;
	int field_0xc;
	struct SaveDataChunk_BLHD sec;
};

class CChunk
{
public:
	uint field_0x0;
	uint hash;
	uint size;
	int offset;

	CChunk* FindNextSubChunk(CChunk* pChunk, uint param_3);
	void* DeleteSubChunk(CChunk* pChunk);
};

static_assert(sizeof(CChunk) == 0x10);

class CChunkDesc
{
public:
	uint hash;
	uint field_0x4;
	uint size;
	int offset;

	CChunk* FindNextSubChunk(CChunk* pChunk, uint param_3);
};

struct SaveBigAlloc {
	undefined4 field_0x0;
	undefined4 field_0x4;
	undefined4 field_0x8;
	int size_0xc; /* Created by retype action */
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	struct SaveDataChunk_BLEV field_0x34;
	undefined field_0x58[65448];
};

struct S_COMPANION_INFO
{
	uint conditionIdentifier;
	ScenaricCondition cond;
};

struct S_LVLNFO_SECTOR_V7_V9
{
	uint sectorId;
	uint sectBankSize;
	int nbConditions;
};

PACK(
struct S_LVLNFO_TELEPORTERS_V7_V9
{
	uint field_0x0;
	int field_0x4;
	int field_0x8;
	uint field_0xc;
	int field_0x10;
	ulong field_0x14;
});

static_assert(sizeof(S_LVLNFO_TELEPORTERS_V7_V9) == 0x1c);

struct S_LVLNFO_LEVEL_HEADER_V7
{
	int levelId;

	char levelName[8];

	int bankSizeLevel;
	int bankSizeSect;
	int bankSizeIOP;

	// These are combined into a message key used to load the level name.
	uint hashA;
	uint hashB;

	int sectorStartIndex;
	int nbSectors;

	int field_0x28;

	int nbTeleporters;

	int field_0x30;

	int nbLanguageFileNames;
};

static_assert(sizeof(S_LVLNFO_LEVEL_HEADER_V7) == 0x38);

struct S_SUBSECTOR_INFO
{
	ulong field_0x0;
	uint teleporterActorHashCode;
	int field_0xc;
	uint field_0x10;
	int field_0x14;
	undefined nbExorcisedWolfen;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	uint flags;
	edDList_material* field_0x20;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
};

struct S_LEVEL_INFO
{
	ulong titleMsgHash;
	int bankSizeLevel;
	int bankSizeSect;
	int bankSizeIOP;
	int maxSectorId;
	int maxElevatorId;
	int sectorStartIndex;
	int field_0x20;
	char levelName[12];
	int field_0x30;
	char levelPath[20];
	S_COMPANION_INFO* aCompanionInfo;
	char* pSimpleConditionData;
	int nbExorcisedWolfen;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	S_SUBSECTOR_INFO aSubSectorInfo[12];
	SectorManagerSubObjOther aSectorSubObj[30];
};

struct S_LVLNFO_LANGUAGE_V7_V9
{
	undefined4 field_0x0;
	uint keyA;
	uint keyB;
	undefined4 field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
};

static_assert(sizeof(S_LVLNFO_LANGUAGE_V7_V9) == 0x1c);

class CActor;

struct EpStruct_8
{
	int field_0x0;
	int field_0x4;
};

struct EpStruct_80
{
	EpStruct_8 aSubObj[0x10];
};

struct Episode
{
	int minWolfen;
	int scenery;
	int monster;
	int bet;
	int field_0x10;
	int total_0x14;
};

struct GameInfo 
{
	int nbEpisodes;
	float health;
	int nbMagic;
	int nbMoney;
	int scenery;
	int monster;
	int bet;
	int bank;
	int shop;
	EpStruct_80 aEpisodes[2];
};

struct NativShopLevelSubObjSubObj
{
	undefined4 field_0x0;
	undefined4 field_0x4;
	undefined8 field_0x8;
};

struct NativShopLevelSubObj
{
	int field_0x0;
	int currentLevelId;
	undefined4 field_0x8;

	edF32VECTOR4 currentLocation;
	edF32VECTOR4 rotationQuat;

	NativShopLevelSubObjSubObj aSubObjs[5];
	
	void FUN_002d8d10(CActorNativShop* pNativ);
};

class CLevelScheduler : public CObjectManager
{
public:
	CLevelScheduler();

	// Begin Manager
	virtual void Game_Init();
	virtual void Game_Term();

	virtual void LevelLoading_Begin();
	virtual void LevelLoading_End();
	virtual bool LevelLoading_Manage();

	virtual void Level_Install();
	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual char* ProfileGetName();

	// End Manager

	virtual void Level_PostInit();
	virtual void Level_PreTerm();

	void Level_FillRunInfo(int levelID, int elevatorID, int param_4);
	void Level_LoadObjectives(ByteCode* pMemoryStream);

	void MoreLoadLoopObjectSetup(bool param_2);
	void LevelsInfo_ReadHeader_V7_V9(S_LVLNFO_LEVEL_HEADER_V7* pLevelInfoHeader, S_LEVEL_INFO* pLevelInfo);
	int* LevelsInfo_ReadSectors_V7_V9(S_LVLNFO_SECTOR_V7_V9* pFileBuffer, int count, S_LEVEL_INFO* pLevelInfo);
	void LevelsInfo_ReadTeleporters_V7_V9(S_LVLNFO_TELEPORTERS_V7_V9* pFileData, int count, S_LEVEL_INFO* pLevelInfo);
	void LevelsInfo_ReadLanguageFileNames_V7_V9(S_LVLNFO_LANGUAGE_V7_V9* pLevelInfoLanguage, int nbLanguageFileNames, int levelId);
	void Levels_LoadInfoBank();
	void Levels_UpdateDataFromSavedGame();
	void Levels_SaveDataToSavedGame();
	void Episode_ComputeCurrent();
	void Episode_LoadFromIniFile();

	void SaveGame_SaveCurLevelState(int param_2);
	CChunk* SaveGame_GetLevelChunk(int levelId);
	void SaveGame_LoadLevelState(int levelId);

	static int SaveGame_GetMaxBufferSize();

	void UpdateGameInfo(float health, int magic, int money);

	static uint GetMedallionLevel();
	static int GetBoomyLevel();
	static int GetFightLevel();

	static int ScenVar_Get(SCENARIC_VARIABLE scenVarId);
	static void ScenVar_Set(SCENARIC_VARIABLE scenVarId, int newValue);

	static void OnSceneVarSet();

	void Level_Run(undefined8 param_2, int levelID, int elevatorID, int param_5, int param_6, bool bFadeOut);
	void Level_Teleport(CActor* pActor, int levelId, int elevatorId, int cutsceneId, int param_6);
	void Level_GetSubSectorInfo(int levelIndex, int elevatorId, S_SUBSECTOR_INFO* pSubSectorInfo);
	void Level_UpdateCurLiveLevelInfo();
	void Level_TeleporterChanged();

	void SetLevelTimerFunc_002df450(float param_1, int mode);
	int GetNbAreas(int currentLevelID);
	void ExitLevel(int param_2);
	void FUN_002dc200(int elevatorId, int levelId, int param_4);

	bool Money_GiveToShop(int amount);
	bool Money_GiveToBet(int amount);
	bool Money_TakeFromBet(int amount);
	bool Money_TakeFromBank(int amount);
	bool Money_TakeFromMonster(int amount);
	bool Money_TakeFromScenery(int amount);

	CChunkDesc* GetChunkDesc(uint hash);

	void* SaveGame_BeginChunk(uint hash);
	void SaveGame_EndChunk(void* pDataEnd);

	CChunk* SaveGame_OpenChunk(uint hash);
	void SaveGame_CloseChunk();

	void SaveGame_SaveScenVars();
	void SaveGame_SaveGameInfo();
	void SaveGame_SaveGameObj();

	void LoadGame_LoadScenVars();
	void LoadGame_LoadGameInfo();
	void LoadGame_LoadGameObj();

	void ResetAutoSaveTriggerTime();
	uint SaveGame_SaveToBuffer(SaveBigAlloc* pSaveData, SaveDataDesc* pSaveDesc);
	void SaveGame_LoadFromBuffer(SaveBigAlloc* pSaveData, uint size);

public:

	static CLevelScheduler* gThis;

	static GameInfo _gGameNfo;

	struct ManagerFunctionData* pManagerFunctionData;
	int nextLevelId;
	char levelPath[14];
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	CChunk* pSaveData_0x48;
	CChunk* pSaveDataEnd_0x4c;
	CChunk* aSaveGameChunks[8];
	int curChunkIndex;
	undefined4 field_0x74;
	int bShouldLoad;
	undefined4 bShouldSave;
	undefined4 field_0x80;
	float autoSaveTriggerTime;
	float curAutoSaveTime;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	S_LEVEL_INFO aLevelInfo[16];
	int field_0x4210;
	float gameTime;
	int objCount_0x4218;
	undefined field_0x421c;
	undefined field_0x421d;
	undefined field_0x421e;
	undefined field_0x421f;
	struct LoadLoopObject_50 field_0x4220[64];
	int nbNativShopSubObjs;
	undefined field_0x5624;
	undefined field_0x5625;
	undefined field_0x5626;
	undefined field_0x5627;
	undefined field_0x5628;
	undefined field_0x5629;
	undefined field_0x562a;
	undefined field_0x562b;
	undefined field_0x562c;
	undefined field_0x562d;
	undefined field_0x562e;
	undefined field_0x562f;
	NativShopLevelSubObj aNativShopSubObjs[10];
	uint field_0x5b30;
	int currentLevelID;
	int nextLevelID;
	int outroCutsceneId;
	int baseSectorIndex; /* Created by retype action */
	int nextElevatorID; /* Created by retype action */
	int loadStage_0x5b48;
	int currentElevatorID;
	int level_0x5b50;
	int level_0x5b54;
	int* pObjectiveStreamBegin;
	int* pObjectiveStreamEnd;
	edCBankBuffer levelBank;
	edCBankBufferEntry* pLevelBankBufferEntry;
	edCBankBuffer levelIOPBank;
	edCBankBufferEntry* pIOPBankBufferEntry;
	undefined field_0x5bd8;
	undefined field_0x5bd9;
	undefined field_0x5bda;
	undefined field_0x5bdb;
	undefined field_0x5bdc;
	undefined field_0x5bdd;
	undefined field_0x5bde;
	undefined field_0x5bdf;
};

bool edSoundAreAllSoundDataLoaded();
bool edMusicAreAllMusicDataLoaded();

extern ulong gMedallionHashCodes[9];
extern ulong gFightHashCodes[8];

extern int INT_ARRAY_0048ed60[16];

#endif //_LEVELSCHEDULEMANAGER_H
