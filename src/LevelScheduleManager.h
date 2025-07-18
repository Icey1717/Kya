#ifndef _LEVELSCHEDULEMANAGER_H
#define _LEVELSCHEDULEMANAGER_H

#include "Types.h"

#include "edC/edCBank.h"

#include <stdlib.h>
#include "LargeObject.h"
#include "ScenaricCondition.h"
#include "SectorManager.h"

#define IMPLEMENTATION_GUARD_OBJECTIVE(x)

#define SCENE_VAR_FREED_WOLFEN 0x0
#define SCENE_VAR_BOUNCE_JUMP 0x7
#define SCENE_VAR_CLIMB 0x8
#define SCENE_VAR_BOOMY 0x9
#define SCENE_VAR_FIGHT_RING 0xa
#define SCENE_VAR_MAX_HEALTH 0x14

class CActorNativShop;

struct SaveDataSection_44484c42 {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	int field_0xc;
	int field_0x10;
};

struct LoadLoopObject_50 {
	int field_0x0;
	undefined4 field_0x4;
	ulong messageKey;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	undefined4 field_0x20;
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

struct SaveDataSection_56454c42 {
	int field_0x0;
	int field_0x4;
	uint field_0x8;
	int field_0xc;
	struct SaveDataSection_44484c42 sec;
};

struct CChunk {
	uint field_0x0;
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
	struct SaveDataSection_56454c42 field_0x34;
	undefined field_0x58[65448];
};

struct S_COMPANION_INFO
{
	uint conditionIdentifier;
	ScenaricCondition cond;
};

struct S_LVLNFO_SECTOR_V7_V9
{
	uint sectorIndex;
	uint field_0x4;
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

struct S_SUBSECTOR_INFO
{
	ulong field_0x0;
	uint field_0x8;
	int field_0xc;
	uint field_0x10;
	int field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	uint field_0x1c;
	edDList_material* field_0x20;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
};

struct S_LEVEL_INFO {
	ulong titleMsgHash;
	int bankSizeLevel;
	int bankSizeSect;
	int bankSizeIOP;
	int sectorCount_0x14;
	int maxElevatorId;
	int sectorStartIndex;
	int field_0x20;
	char levelName[12];
	int field_0x30;
	char levelPath[20];
	S_COMPANION_INFO* aCompanionInfo;
	char* pSimpleConditionData;
	undefined4 field_0x50;
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
	int field_0x4;
	undefined4 field_0x8;

	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;

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

	// End Manager

	virtual void Level_PreTerm();

	void Level_FillRunInfo(int levelID, int elevatorID, int param_4);
	void Level_LoadObjectives(ByteCode* pMemoryStream);

	void MoreLoadLoopObjectSetup(bool param_2);
	void LevelsInfo_ReadHeader_V7_V9(char* fileData, S_LEVEL_INFO* pLevelInfo);
	int* LevelsInfo_ReadSectors_V7_V9(S_LVLNFO_SECTOR_V7_V9* pFileBuffer, int count, S_LEVEL_INFO* pLevelInfo);
	void LevelsInfo_ReadTeleporters_V7_V9(S_LVLNFO_TELEPORTERS_V7_V9* pFileData, int count, S_LEVEL_INFO* pLevelInfo);
	void LevelsInfo_ReadLanguageFileNames_V7_V9(S_LVLNFO_LANGUAGE_V7_V9* param_2, int nbObj, undefined4 param_4);
	void Levels_LoadInfoBank();
	void Episode_ComputeCurrent();
	void Episode_LoadFromIniFile();

	void SaveGame_SaveCurLevelState(int param_2);

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
	struct SaveBigAlloc* pSaveData_0x48;
	int pSaveDataEnd_0x4c;
	CChunk* aSaveDataArray[8];
	int currentSaveIndex;
	undefined4 field_0x74;
	int field_0x78;
	undefined4 field_0x7c;
	undefined4 field_0x80;
	float field_0x84;
	float field_0x88;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	S_LEVEL_INFO aLevelInfo[16];
	int field_0x4210;
	float field_0x4214;
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
