#include "LevelScheduleManager.h"
#include "IniFile.h"
#include "edMem.h"
#include "edBank/edBankFile.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#include <libvu0.h>
#else
#include "renderer.h"
#include "port.h"
#endif

#include "EdenLib/edSys/sources/EdSystem.h"
#include "MemoryStream.h"
#include "ed3D.h"
#include "FileManager3D.h"
#include "edStr.h"
#include "CinematicManager.h"
#include "ScenaricCondition.h"
#include "WayPoint.h"
#include "PathManager.h"
#include "ActorManager.h"
#include "DlistManager.h"
#include "AnmManager.h"
#include "Animation.h"
#include "CollisionManager.h"
#include "EventManager.h"
#include "port/pointer_conv.h"
#include "EventTrack.h"
#include "ActorHero.h"
#include "TimeController.h"
#include "CameraViewManager.h"
#include "ActorTeleporter.h"
#include "FrontEndDisp.h"
#include "Fx.h"
#include "FxParticle.h"
#include "MathOps.h"
#include "SaveManagement.h"
#include "Pause.h"
#include "ActorBoomy.h"
#include "ActorNativ.h"
#include "ActorNativShop.h"
#include "InventoryInfo.h"
#include "ActorWolfen.h"

#define LEVEL_SCHEDULER_LOG(level, format, ...) MY_LOG_CATEGORY("levelScheduler", level, format, ##__VA_ARGS__)

struct ScenarioVariable {
	int defaultValue;
	int currentValue;
};

ScenarioVariable _gScenVarInfo[98] = {
	{0x0, 0x0},
	{0x0, 0x0},
	{0x64, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0xFFFFFFFF, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x1, 0x0},
	{0x32, 0x0},
	{0x1F4, 0x0},

	// Health
	{0x32, 0x0},
	{0x32, 0x0},
	{0xFA, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
	{0x0, 0x0},
};

CChunkDesc _gGameChunks[12] =
{
	{ SAVEGAME_CHUNK_BSAV, 0x0, 0x50000, 0x000 },
	{ SAVEGAME_CHUNK_BSHD, 0x1, 0x40001, 0x014 },
	{ SAVEGAME_CHUNK_BSCN, 0x1, 0x60000, 0x000 },
	{ SAVEGAME_CHUNK_BGNF, 0x1, 0x20003, 0x134 },
	{ SAVEGAME_CHUNK_BLEV, 0x1, 0x10000, 0x000 },
	{ SAVEGAME_CHUNK_BLHD, 0x1, 0x70000, 0x000 },
	{ SAVEGAME_CHUNK_BLCL, 0x1, 0x40000, 0x000 },
	{ SAVEGAME_CHUNK_BLAC, 0x1, 0x20000, 0x000 },
	{ SAVEGAME_CHUNK_BLCI, 0x1, 0x20000, 0x000 },
	{ SAVEGAME_CHUNK_BLMP, 0x1, 0x50000, 0x000 },
	{ SAVEGAME_CHUNK_BOBJ, 0x1, 0x20000, 0x000 },
	{ 0x0, 0x0, 0x0, 0x0 },
};

CLevelScheduler* CLevelScheduler::gThis = NULL;

Episode g_EpisodeDataArray_0048eb0[16];

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::Money_GiveToShop(int amount)
{
	_gGameNfo.nbMoney = _gGameNfo.nbMoney - amount;
	_gGameNfo.shop = _gGameNfo.shop + amount;

	return true;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::Money_GiveToBet(int amount)
{
	int iVar1;
	int iVar2;

	_gGameNfo.bet = _gGameNfo.bet + amount;

	if ((_gScenVarInfo[6].currentValue < 0) ||
		(iVar1 = _gScenVarInfo[6].currentValue, _gGameNfo.nbEpisodes <= _gScenVarInfo[6].currentValue)) {
		iVar1 = _gGameNfo.nbEpisodes + -1;
	}

	iVar1 = _gGameNfo.bet - g_EpisodeDataArray_0048eb0[iVar1].bet;
	if (iVar1 < 1) {
		if (iVar1 < 0) {
			iVar2 = _gGameNfo.bank;
			if (-iVar1 <= _gGameNfo.bank) {
				iVar2 = -iVar1;
			}
			if (0 < _gGameNfo.bank) {
				_gGameNfo.bank = _gGameNfo.bank - iVar2;
				_gGameNfo.bet = _gGameNfo.bet + iVar2;
			}
		}
	}
	else {
		_gGameNfo.bet = _gGameNfo.bet - iVar1;
		_gGameNfo.bank = _gGameNfo.bank + iVar1;
	}

	_gGameNfo.nbMoney = _gGameNfo.nbMoney - amount;

	return true;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::Money_TakeFromBet(int amount)
{
	int iVar1;
	int iVar2;

	_gGameNfo.bet = _gGameNfo.bet - amount;
	if ((_gScenVarInfo[6].currentValue < 0) ||
		(iVar1 = _gScenVarInfo[6].currentValue, _gGameNfo.nbEpisodes <= _gScenVarInfo[6].currentValue)) {
		iVar1 = _gGameNfo.nbEpisodes + -1;
	}

	iVar1 = _gGameNfo.bet - g_EpisodeDataArray_0048eb0[iVar1].bet;
	if (iVar1 < 1) {
		if (iVar1 < 0) {
			iVar2 = _gGameNfo.bank;
			if (-iVar1 <= _gGameNfo.bank) {
				iVar2 = -iVar1;
			}
			if (0 < _gGameNfo.bank) {
				_gGameNfo.bank = _gGameNfo.bank - iVar2;
				_gGameNfo.bet = _gGameNfo.bet + iVar2;
			}
		}
	}
	else {
		_gGameNfo.bet = _gGameNfo.bet - iVar1;
		_gGameNfo.bank = _gGameNfo.bank + iVar1;
	}

	_gGameNfo.nbMoney = _gGameNfo.nbMoney + amount;

	return true;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::Money_TakeFromBank(int amount)
{
	_gGameNfo.nbMoney = _gGameNfo.nbMoney + amount;
	_gGameNfo.bank = _gGameNfo.bank - amount;

	return true;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::Money_TakeFromMonster(int amount)
{
	_gGameNfo.monster = _gGameNfo.monster - amount;
	_gGameNfo.bank = _gGameNfo.bank + amount;
	return true;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::Money_TakeFromScenery(int amount)
{
	_gGameNfo.nbMoney = _gGameNfo.nbMoney + amount;
	_gGameNfo.scenery = _gGameNfo.scenery - amount;

	return true;
}

GameInfo CLevelScheduler::_gGameNfo{};

const char* g_CD_LevelPath_00433bf8 = "CDEURO/Level/";
const char* g_szRouter_00433c08 = "Router";
const char* g_szSetPath_00433c10 = "SetPath";
const char* g_szAddLevel_00433c48 = "AddLevel";
const char* g_szNoPathError_00433c20 = "No path found in .INI -> use default !\n";

CLevelScheduler::CLevelScheduler()
{
	pSaveData_0x48 = (CChunk*)0x0;
	pSaveDataEnd_0x4c = 0;
	curChunkIndex = 0;
}

uint SearchForSection_002e3bf0(undefined* param_1, uint param_2, int param_3)
{
	for (; (param_2 < (uint)param_1 + *(int*)(param_1 + 0xc) + 0x10 && (param_3 != *(int*)(param_2 + 4))); param_2 = param_2 + *(int*)(param_2 + 0xc) + 0x10) {
	}
	if (((uint)param_1 + *(int*)(param_1 + 0xc) + 0x10 <= param_2) || (param_3 != *(int*)(param_2 + 4))) {
		param_2 = 0;
	}
	return param_2;
}

void CLevelScheduler::Level_FillRunInfo(int levelID, int elevatorID, int param_4)
{
	undefined* puVar1;
	int iVar2;
	uint uVar3;
	S_LEVEL_INFO* pLevelInfo;
	CChunk* pCVar2;

	if (aLevelInfo[levelID].levelName[0] == '\0') {
		levelID = nextLevelId;
		elevatorID = -1;
	}
	this->nextLevelID = levelID;
	this->outroCutsceneId = -1;
	this->nextElevatorID = elevatorID;
	pLevelInfo = &aLevelInfo[levelID];

	this->baseSectorIndex = pLevelInfo->sectorStartIndex;
	this->currentElevatorID = -1;
	this->level_0x5b50 = 0x10;
	this->level_0x5b54 = -1;
	if (this->bShouldLoad == 0) {
		// Check maxElevatorID_0xa8 and field_0xf4
		iVar2 = this->nextElevatorID;
		if (((iVar2 != -1) && (-1 < iVar2)) && (iVar2 < pLevelInfo->maxElevatorId)) {
			iVar2 = pLevelInfo->aSubSectorInfo[iVar2].field_0xc;
			if (iVar2 != -1) {
				this->baseSectorIndex = iVar2;
			}

			this->level_0x5b50 = currentLevelID;
			this->level_0x5b54 = param_4;
		}
	}
	else {
		SaveDataChunk_BSHD* pBSHD = reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSHD) + 1);
		this->baseSectorIndex = pBSHD->sectorId;
		SaveGame_CloseChunk();
	}

	return;
}

void CLevelScheduler::Level_LoadObjectives(ByteCode* pMemoryStream)
{
	int* piVar1;
	int iVar2;
	uint uVar3;

	piVar1 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar1 + 1);
	this->pObjectiveStreamBegin = piVar1;
	uVar3 = 0;
	if (*this->pObjectiveStreamBegin != 0) {
		do {
			pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + 0x1c;
			piVar1 = (int*)pMemoryStream->currentSeekPos;
			pMemoryStream->currentSeekPos = (char*)(piVar1 + 1);
			iVar2 = *piVar1;
			if (iVar2 != 0) {
				pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + iVar2 * 0x10;
			}
			uVar3 = uVar3 + 1;
		} while (uVar3 < (uint)*this->pObjectiveStreamBegin);
	}

	piVar1 = (int*)pMemoryStream->currentSeekPos;
	pMemoryStream->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + *piVar1 * 8;
	}
	this->pObjectiveStreamEnd = piVar1;

	return;
}

char* CLevelScheduler::ProfileGetName()
{
	return "lvl schd";
}

struct LoadLoopObject_418_18 {
	SectorManagerSubObj aSubObj[6];
};

void SaveLevelInfo(S_LEVEL_INFO* pLevelInfo)
{
	CLevelScheduler* pLevelScheduleManager;
	int iVar4;
	S_SUBSECTOR_INFO* pCurSubSectorInfo;
	SubSectorSaveData* pSubSectorSaveData;

	pLevelScheduleManager = CLevelScheduler::gThis;	

	SaveDataChunk_BLHD* pBLHD = reinterpret_cast<SaveDataChunk_BLHD*>(pLevelScheduleManager->SaveGame_BeginChunk(SAVEGAME_CHUNK_BLHD));

	pSubSectorSaveData = pBLHD->aSubSectorData;
	pCurSubSectorInfo = pLevelInfo->aSubSectorInfo;
	pBLHD->levelId = pLevelScheduleManager->currentLevelID;
	pBLHD->nbExorcisedWolfen = pLevelInfo->nbExorcisedWolfen;
	pBLHD->maxElevatorId = pLevelInfo->maxElevatorId;
	iVar4 = 0;
	if (0 < pLevelInfo->maxElevatorId) {
		do {
			iVar4 = iVar4 + 1;
			pSubSectorSaveData->flags = pCurSubSectorInfo->flags;
			pSubSectorSaveData->nbExorcisedWolfen = pCurSubSectorInfo->nbExorcisedWolfen;
			pCurSubSectorInfo = pCurSubSectorInfo + 1;
			pSubSectorSaveData = pSubSectorSaveData + 1;
		} while (iVar4 < pLevelInfo->maxElevatorId);
	}

	pLevelScheduleManager->SaveGame_EndChunk(reinterpret_cast<char*>(pBLHD + 1) + (pLevelInfo->maxElevatorId * sizeof(SubSectorSaveData)));

	return;
}

void SetupLevelInfo_002d97c0(S_LEVEL_INFO* pLevelInfo, bool param_2)
{
	LoadLoopObject_418_18* pLVar1;
	int iVar2;
	S_SUBSECTOR_INFO* pLevelInfoSubObj;

	/* This will clear our current level name */
	if (param_2 != false) {
		pLevelInfo->titleMsgHash = 0;
		pLevelInfo->bankSizeLevel = 0;
		pLevelInfo->bankSizeSect = 0;
		pLevelInfo->maxSectorId = 0;
		pLevelInfo->maxElevatorId = 0;
		pLevelInfo->field_0x20 = 0;
		pLevelInfo->levelName[0] = '\0';
		pLevelInfo->aCompanionInfo = (S_COMPANION_INFO*)0x0;
		pLevelInfo->pSimpleConditionData = 0;
	}

	pLevelInfo->nbExorcisedWolfen = 0;

	pLevelInfoSubObj = pLevelInfo->aSubSectorInfo;
	iVar2 = 0;
	do {
		if (param_2 != false) {
			pLevelInfoSubObj->field_0x0 = 0;
			pLevelInfoSubObj->teleporterActorHashCode = 0xffffffff;
			pLevelInfoSubObj->field_0xc = -1;
			pLevelInfoSubObj->field_0x10 = 0;
			pLevelInfoSubObj->field_0x14 = 0;
		}

		pLevelInfoSubObj->nbExorcisedWolfen = 0;
		pLevelInfoSubObj->flags = 0;
		pLevelInfoSubObj->field_0x20 = (edDList_material*)0x0;

		if ((pLevelInfoSubObj->field_0x10 & 1) != 0) {
			pLevelInfoSubObj->flags = pLevelInfoSubObj->flags | 1;
		}

		iVar2 = iVar2 + 1;
		pLevelInfoSubObj = pLevelInfoSubObj + 1;
	} while (iVar2 < 0xc);
	
	SectorManagerSubObjOther* pSVar1 = pLevelInfo->aSectorSubObj;
	iVar2 = 0;
	do {
		if (param_2 != false) {
			pSVar1->bankSize = 0;
			pSVar1->flags = 0;
			pSVar1->nbSectorConditions = 0;
			pSVar1->aCompanionInfo = (S_COMPANION_INFO*)0x0;
		}
		iVar2 = iVar2 + 1;
		pSVar1 = pSVar1 + 1;
	} while (iVar2 < 0x1e);

	return;
}

void CLevelScheduler::MoreLoadLoopObjectSetup(bool param_2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	//MapManager* pMVar4;
	ScenarioVariable* pSVar5;
	int iVar6;
	LoadLoopObject_50* pLVar7;
	NativShopLevelSubObj* puVar8;
	int iVar9;
	S_LEVEL_INFO* pLevelInfo;

	iVar9 = 0;
	pLevelInfo = this->aLevelInfo;
	do {
		SetupLevelInfo_002d97c0(pLevelInfo, param_2);
		iVar9 = iVar9 + 1;
		pLevelInfo = pLevelInfo + 1;
	} while (iVar9 < 0x10);

	if (param_2 != false) {
		this->field_0x4210 = 0;
		this->objCount_0x4218 = 0;
		this->nbNativShopSubObjs = 0;
	}

	this->field_0x5b30 = 0;
	this->gameTime = 0.0f;


	pSVar5 = _gScenVarInfo;
	iVar9 = 0;
	do {
		iVar6 = iVar9;
		iVar9 = iVar6 + 8;
		pSVar5->currentValue = pSVar5->defaultValue;
		pSVar5[1].currentValue = pSVar5[1].defaultValue;
		pSVar5[2].currentValue = pSVar5[2].defaultValue;
		pSVar5[3].currentValue = pSVar5[3].defaultValue;
		pSVar5[4].currentValue = pSVar5[4].defaultValue;
		pSVar5[5].currentValue = pSVar5[5].defaultValue;
		pSVar5[6].currentValue = pSVar5[6].defaultValue;
		pSVar5[7].currentValue = pSVar5[7].defaultValue;
		pSVar5 = pSVar5 + 8;
	} while (iVar9 < 0x5a);

	pLVar7 = this->field_0x4220;

	_gScenVarInfo[iVar9].currentValue = _gScenVarInfo[iVar9].defaultValue;
	_gScenVarInfo[iVar9 + 1].currentValue = _gScenVarInfo[iVar9 + 1].defaultValue;

	iVar9 = 0;
	if (0 < this->objCount_0x4218) {
		do {
			pLVar7->field_0x24 = 0x20;
			pLVar7->field_0x28 = 0;
			iVar9 = iVar9 + 1;
			pLVar7->field_0x2c = 0;
			pLVar7->field_0x40 = 0;
			pLVar7->field_0x30 = gF32Vertex4Zero;
			pLVar7 = pLVar7 + 1;
		} while (iVar9 < this->objCount_0x4218);
	}

	puVar8 = this->aNativShopSubObjs;
	iVar9 = 0;
	if (0 < this->nbNativShopSubObjs) {
		do {
			if (param_2 != false) {
				puVar8->field_0x0 = -1;
				puVar8->currentLevelId = 0x10;
				puVar8->field_0x8 = 0;

				puVar8->currentLocation = gF32Vertex4Zero;
				puVar8->rotationQuat = gF32Vector4UnitZ;

				puVar8->aSubObjs[0].field_0x0 = 0x20;
				puVar8->aSubObjs[0].field_0x4 = 0;
				puVar8->aSubObjs[0].field_0x8 = 0;

				puVar8->aSubObjs[1].field_0x0 = 0x20;
				puVar8->aSubObjs[1].field_0x4 = 0;
				puVar8->aSubObjs[1].field_0x8 = 0;

				puVar8->aSubObjs[2].field_0x0 = 0x20;
				puVar8->aSubObjs[2].field_0x4 = 0;
				puVar8->aSubObjs[2].field_0x8 = 0;

				puVar8->aSubObjs[3].field_0x0 = 0x20;
				puVar8->aSubObjs[3].field_0x4 = 0;
				puVar8->aSubObjs[3].field_0x8 = 0;

				puVar8->aSubObjs[4].field_0x0 = 0x20;
				puVar8->aSubObjs[4].field_0x4 = 0;
				puVar8->aSubObjs[4].field_0x8 = 0;
			}
			iVar9 = iVar9 + 1;
			puVar8 = puVar8 + 1;
		} while (iVar9 < this->nbNativShopSubObjs);
	}
	//pMVar4 = (MapManager*)CScene::GetManager(MO_Map);
	//MapManager::SetupFunc_003f8150(pMVar4);
	return;
}

edCBankCallback _gLevelsTableBankCallback = { -1, -1, 0x0, 0, 0, 0, 0, 0 };
char* g_szLevelInfoBnkPath_00433c60 = "Info/levels.bnk";

void CLevelScheduler::LevelsInfo_ReadHeader_V7_V9(S_LVLNFO_LEVEL_HEADER_V7* pLevelInfoHeader, S_LEVEL_INFO* pLevelInfo)
{
	int iVar2;
	SectorManagerSubObjOther* pLVar3;

	pLevelInfo->titleMsgHash = ByteCode::BuildU64(pLevelInfoHeader->hashA, pLevelInfoHeader->hashB);
	pLVar3 = pLevelInfo->aSectorSubObj;
	iVar2 = 0;
	pLevelInfo->field_0x20 = pLevelInfoHeader->field_0x28;
	pLevelInfo->bankSizeLevel = pLevelInfoHeader->bankSizeLevel;
	pLevelInfo->bankSizeSect = pLevelInfoHeader->bankSizeSect;
	pLevelInfo->bankSizeIOP = pLevelInfoHeader->bankSizeIOP;
	pLevelInfo->maxSectorId = pLevelInfoHeader->nbSectors;
	pLevelInfo->field_0x30 = pLevelInfoHeader->field_0x30;
	pLevelInfo->sectorStartIndex = pLevelInfoHeader->sectorStartIndex;

	do {
		pLVar3->bankSize = 0;
		pLVar3->flags = 0;
		iVar2 = iVar2 + 6;
		pLVar3->nbSectorConditions = 0x0;
		pLVar3->aCompanionInfo = 0;
		pLVar3[1].bankSize = 0;
		pLVar3[1].flags = 0;
		pLVar3[1].nbSectorConditions = 0x0;
		pLVar3[1].aCompanionInfo = 0;
		pLVar3[2].bankSize = 0;
		pLVar3[2].flags = 0;
		pLVar3[2].nbSectorConditions = 0x0;
		pLVar3[2].aCompanionInfo = 0;
		pLVar3[3].bankSize = 0;
		pLVar3[3].flags = 0;
		pLVar3[3].nbSectorConditions = 0x0;
		pLVar3[3].aCompanionInfo = 0;
		pLVar3[4].bankSize = 0;
		pLVar3[4].flags = 0;
		pLVar3[4].nbSectorConditions = 0x0;
		pLVar3[4].aCompanionInfo = 0;
		pLVar3[5].bankSize = 0;
		pLVar3[5].flags = 0;
		pLVar3[5].nbSectorConditions = 0x0;
		pLVar3[5].aCompanionInfo = 0;
		pLVar3 = pLVar3 + 6;
	} while (iVar2 < 0x1e);

	iVar2 = 8;
	/* Set the name of the level we will load to memory */
	pLevelInfo->levelName[0] = pLevelInfoHeader->levelName[0];
	pLevelInfo->levelName[1] = pLevelInfoHeader->levelName[1];
	pLevelInfo->levelName[2] = pLevelInfoHeader->levelName[2];
	pLevelInfo->levelName[3] = pLevelInfoHeader->levelName[3];
	pLevelInfo->levelName[4] = pLevelInfoHeader->levelName[4];
	pLevelInfo->levelName[5] = pLevelInfoHeader->levelName[5];
	pLevelInfo->levelName[6] = pLevelInfoHeader->levelName[6];
	pLevelInfo->levelName[7] = pLevelInfoHeader->levelName[7];
	do {
		/* Clear the last 4 characters. */
		pLevelInfo->levelName[iVar2] = '\0';
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0xc);

	return;
}

int* CLevelScheduler::LevelsInfo_ReadSectors_V7_V9(S_LVLNFO_SECTOR_V7_V9* aLvlNfoSector, int nbSectors, S_LEVEL_INFO* pLevelInfo)
{
	int curConditionIdentifier;
	S_LEVEL_INFO* pLVar3;
	SectorManagerSubObjOther* pCurSectorSubObj;
	int curConditionIndex;
	int curSectorIndex;
	S_COMPANION_INFO* pCurCompanionInfo;
	uint nbTotalSimpleConditions;
	int simpleConditionsDataSize;
	int* pCurConditionData;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "CSectorManager::LevelsInfo_ReadSectors_V7_V9 nbSectors: 0x{:x}", nbSectors);

	simpleConditionsDataSize = 0;
	nbTotalSimpleConditions = 0;

	S_LVLNFO_SECTOR_V7_V9* pLvlNfoSector = aLvlNfoSector;

	curSectorIndex = nbSectors;
	if (nbSectors != 0) {
		do {
			if (pLevelInfo->maxSectorId < pLvlNfoSector->sectorId) {
				pLevelInfo->maxSectorId = pLvlNfoSector->sectorId;
			}

			LEVEL_SCHEDULER_LOG(LogLevel::Info, "CSectorManager::LevelsInfo_ReadSectors_V7_V9 sectorId: 0x{:x}, sectBankSize: 0x{:x}, nbConditions: 0x{:x}",
				pLvlNfoSector->sectorId, pLvlNfoSector->sectBankSize, pLvlNfoSector->nbConditions);

			pCurSectorSubObj = pLevelInfo->aSectorSubObj + pLvlNfoSector->sectorId;
			pCurSectorSubObj->bankSize = pLvlNfoSector->sectBankSize;

			curConditionIndex = 0;
			pCurConditionData = reinterpret_cast<int*>(pLvlNfoSector + 1);
			if (0 < pLvlNfoSector->nbConditions) {
				do {
					curConditionIdentifier = *pCurConditionData;

					ScenaricCondition cond;
					cond.Create(pCurConditionData + 1);

					if ((0 < curConditionIdentifier) && (curConditionIdentifier < 0x1e)) {
						pCurSectorSubObj->flags = pCurSectorSubObj->flags | 1 << (curConditionIdentifier & 0x1f);

						LEVEL_SCHEDULER_LOG(LogLevel::Info, "CSectorManager::LevelsInfo_ReadSectors_V7_V9 numSimpleConds: 0x{:x}", cond.GetNumSimpleConds());

						if (cond.GetNumSimpleConds() != 0) {
							nbTotalSimpleConditions = nbTotalSimpleConditions + 1;
							pCurSectorSubObj->nbSectorConditions = pCurSectorSubObj->nbSectorConditions + 1;
							simpleConditionsDataSize = simpleConditionsDataSize + cond.GetDataSize();

							LEVEL_SCHEDULER_LOG(LogLevel::Info, "CSectorManager::LevelsInfo_ReadSectors_V7_V9 condDataSize: 0x{:x}, simpleConditionsDataSize: 0x{:x}", cond.GetDataSize(), simpleConditionsDataSize);
						}
					}

					pCurConditionData = cond.GetEndPtr();
					curConditionIndex = curConditionIndex + 1;
				} while (curConditionIndex < pLvlNfoSector->nbConditions);
			}

			pLvlNfoSector = reinterpret_cast<S_LVLNFO_SECTOR_V7_V9*>(pCurConditionData);

			curSectorIndex = curSectorIndex + -1;
		} while (curSectorIndex != 0);
	}

	// Reset back to the first info object.
	pLvlNfoSector = aLvlNfoSector;

	if (nbTotalSimpleConditions != 0) {
		LEVEL_SCHEDULER_LOG(LogLevel::Info, "CSectorManager::LevelsInfo_ReadSectors_V7_V9 nbTotalSimpleConditions: 0x{:x}, simpleConditionsDataSize: 0x{:x}", nbTotalSimpleConditions, simpleConditionsDataSize);

		pLevelInfo->aCompanionInfo = new S_COMPANION_INFO[nbTotalSimpleConditions];
		pLevelInfo->pSimpleConditionData = reinterpret_cast<char*>(edMemAlloc(TO_HEAP(H_MAIN), simpleConditionsDataSize));
		char* pCondDataSeekPos = pLevelInfo->pSimpleConditionData;
		pCurCompanionInfo = pLevelInfo->aCompanionInfo;

		if (nbSectors != 0) {
			do {
				if (pLevelInfo->aSectorSubObj[pLvlNfoSector->sectorId].nbSectorConditions != 0) {
					pLevelInfo->aSectorSubObj[pLvlNfoSector->sectorId].aCompanionInfo = pCurCompanionInfo;
				}

				curSectorIndex = 0;
				nbTotalSimpleConditions = pLvlNfoSector->nbConditions;
				pCurConditionData = reinterpret_cast<int*>(pLvlNfoSector + 1);
				if (0 < nbTotalSimpleConditions) {
					do {
						curConditionIdentifier = *pCurConditionData;
						ScenaricCondition cond;
						cond.Create(pCurConditionData + 1);

						if ((0 < curConditionIdentifier) && (curConditionIdentifier < 0x1e)) {
							if (cond.GetNumSimpleConds() != 0) {
								simpleConditionsDataSize = cond.GetDataSize();
								memcpy(pCondDataSeekPos, cond.GetBeginPtr(), simpleConditionsDataSize);
								pCurCompanionInfo->conditionIdentifier = curConditionIdentifier;
								pCurCompanionInfo->cond.Create(reinterpret_cast<int*>(pCondDataSeekPos));

								pCondDataSeekPos = pCondDataSeekPos + simpleConditionsDataSize;
								pCurCompanionInfo = pCurCompanionInfo + 1;
							}
						}

						pCurConditionData = cond.GetEndPtr();
						curSectorIndex = curSectorIndex + 1;
					} while (curSectorIndex < nbTotalSimpleConditions);
				}

				pLvlNfoSector = reinterpret_cast<S_LVLNFO_SECTOR_V7_V9*>(pCurConditionData);

				nbSectors = nbSectors + -1;
			} while (nbSectors != 0);
		}
	}

	if ((pLevelInfo->sectorStartIndex == -1) ||
		(pLevelInfo->aSectorSubObj[pLevelInfo->sectorStartIndex].bankSize == 0)) {
		curSectorIndex = 1;
		pLVar3 = pLevelInfo;
		while ((curSectorIndex <= pLevelInfo->maxSectorId &&
			(pLVar3->aSectorSubObj[curSectorIndex].bankSize == 0))) {
			curSectorIndex = curSectorIndex + 1;
		}
		if (pLevelInfo->maxSectorId < curSectorIndex) {
			pLevelInfo->sectorStartIndex = -1;
		}
		else {
			pLevelInfo->sectorStartIndex = curSectorIndex;
		}
	}

	return pCurConditionData;
}

void CLevelScheduler::LevelsInfo_ReadLanguageFileNames_V7_V9(S_LVLNFO_LANGUAGE_V7_V9* pLevelInfoLanguage, int nbLanguageFileNames, int levelId)
{
	int iVar1;
	float fVar2;
	ulong uVar3;
	int iVar4;
	LoadLoopObject_50* pLVar6;
	float fVar5;
	float fVar6;

	iVar1 = this->objCount_0x4218;
	iVar4 = iVar1 + nbLanguageFileNames;
	if (iVar4 < 0x41) {
		this->objCount_0x4218 = iVar4;
		iVar4 = 0;
		pLVar6 = this->field_0x4220 + iVar1;
		if (0 < nbLanguageFileNames) {
			do {
				pLVar6->field_0x24 = 0x20;
				pLVar6->field_0x28 = 0;
				pLVar6->field_0x2c = 0;
				pLVar6->field_0x40 = 0;
				pLVar6->field_0x30 = gF32Vertex4Zero;

				pLVar6->field_0x0 = pLevelInfoLanguage->field_0x0;
				pLVar6->levelId = levelId;
				pLVar6->field_0x4 = pLevelInfoLanguage->field_0xc;
				pLVar6->messageKey = ByteCode::BuildU64(pLevelInfoLanguage->keyA, pLevelInfoLanguage->keyB);
				iVar4 = iVar4 + 1;
				fVar5 = pLevelInfoLanguage->field_0x14;
				fVar6 = pLevelInfoLanguage->field_0x18;
				pLVar6->field_0x10 = pLevelInfoLanguage->field_0x10;
				pLevelInfoLanguage = pLevelInfoLanguage + 1;
				pLVar6->field_0x14 = fVar5;
				pLVar6->field_0x18 = fVar6;
				pLVar6->field_0x1c = 1.0f;
				pLVar6 = pLVar6 + 1;
			} while (iVar4 < nbLanguageFileNames);
		}
	}

	return;
}

void CLevelScheduler::LevelsInfo_ReadTeleporters_V7_V9(S_LVLNFO_TELEPORTERS_V7_V9* pFileData, int count, S_LEVEL_INFO* pLevelInfo)
{
	int iVar1;
	S_SUBSECTOR_INFO* pLevelInfoSubObj;
	int iVar4;

	pLevelInfo->maxElevatorId = 0;
	if (count != 0) {
		do {
			iVar1 = pFileData->field_0x8;
			if (iVar1 < 0) {
				iVar1 = 0;
			}

			if (pLevelInfo->maxElevatorId <= iVar1) {
				pLevelInfo->maxElevatorId = iVar1 + 1;
			}

			count = count + -1;

			pLevelInfoSubObj = pLevelInfo->aSubSectorInfo + iVar1;

			pLevelInfoSubObj->field_0x0 = pFileData->field_0x14;
			pLevelInfoSubObj->teleporterActorHashCode = pFileData->field_0x0;
			pLevelInfoSubObj->field_0xc = pFileData->field_0x4;
			pLevelInfoSubObj->field_0x10 = pFileData->field_0xc;
			pLevelInfoSubObj->field_0x14 = pFileData->field_0x10;
			pFileData = pFileData + 1;
		} while (count != 0);
	}

	iVar4 = 0;
	pLevelInfoSubObj = pLevelInfo->aSubSectorInfo;
	iVar1 = 0;
	if (0 < pLevelInfo->maxElevatorId) {
		do {
			iVar4 = iVar4 + pLevelInfoSubObj->field_0x14;
			if ((pLevelInfoSubObj->field_0x10 & 1) != 0) {
				pLevelInfoSubObj->flags = pLevelInfoSubObj->flags | 1;
			}

			iVar1 = iVar1 + 1;
			pLevelInfoSubObj = pLevelInfoSubObj + 1;
		} while (iVar1 < pLevelInfo->maxElevatorId);
	}

	pLevelInfo->aSubSectorInfo[0].field_0x14 = pLevelInfo->aSubSectorInfo[0].field_0x14 + (pLevelInfo->field_0x20 - iVar4);
	pLevelInfo->field_0x20 = pLevelInfo->field_0x20 - pLevelInfo->aSubSectorInfo[0].field_0x14;

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Levels_LoadInfoBank()
{
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "LevelScheduleManager::Levels_LoadInfoBank\n");

	char cVar1;
	int curLevelId;
	undefined4* puVar3;
	edCBankBufferEntry* infoLevelsFileBuffer;
	int curFileIndex;
	int* puVar5;
	S_LEVEL_INFO* pLevelInfo;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	S_LVLNFO_LEVEL_HEADER_V7* pLevelInfoHeader;
	edBANK_ENTRY_INFO outHeader;
	char levelInfoFilePath[512];
	edCBankBuffer bank;
	edCBankInstall infoLevelsPathPtr;

	memset(&infoLevelsPathPtr, 0, sizeof(edCBankInstall));
	bank.initialize(0x10000, 1, &infoLevelsPathPtr);
	bank.bank_buffer_setcb(&_gLevelsTableBankCallback);

	/* CDEURO/LEVEL/ + Info/levels.bnk */
	levelInfoFilePath[0] = '\0';
	edStrCatMulti(levelInfoFilePath, levelPath, "Info/levels.bnk", 0);
	infoLevelsFileBuffer = bank.get_free_entry();
	infoLevelsPathPtr.filePath = levelInfoFilePath;
	bool bLoadSucess = infoLevelsFileBuffer->load(&infoLevelsPathPtr);
	if (bLoadSucess != false) {
		curFileIndex = infoLevelsFileBuffer->get_element_count();
		while (curFileIndex != 0) {
			curFileIndex = curFileIndex + -1;
			bool bGetInfoSuccess = infoLevelsFileBuffer->get_info(curFileIndex, &outHeader, (char*)0x0);
			puVar3 = (undefined4*)outHeader.fileBufferStart;
			if (bGetInfoSuccess != false) {
				pLevelInfoHeader = reinterpret_cast<S_LVLNFO_LEVEL_HEADER_V7*>(outHeader.fileBufferStart + 4);
				if (true) {
					switch (*(int*)outHeader.fileBufferStart) {
					case 9:
						curLevelId = pLevelInfoHeader->levelId;
						if ((-1 < curLevelId) && (curLevelId < 0x10)) {
							pLevelInfo = &aLevelInfo[curLevelId];

#if 0
							if (curLevelId == 0xf) {
								// Hotswap for level T

								struct LevelT
								{
									S_LVLNFO_LEVEL_HEADER_V7 header;
									S_LVLNFO_TELEPORTERS_V7_V9 teleporters[0];
									S_LVLNFO_SECTOR_V7_V9 sectors[12];
								} levelT;

								memset(&levelT, 0, sizeof(LevelT));

								pLevelInfoHeader = &levelT.header;

								pLevelInfoHeader->levelName[0] = 'L';
								pLevelInfoHeader->levelName[1] = 'E';
								pLevelInfoHeader->levelName[2] = 'V';
								pLevelInfoHeader->levelName[3] = 'E';
								pLevelInfoHeader->levelName[4] = 'L';
								pLevelInfoHeader->levelName[5] = '_';
								pLevelInfoHeader->levelName[6] = 'T';

								pLevelInfoHeader->bankSizeLevel = 0x2a3800;
								pLevelInfoHeader->bankSizeSect = 0x6f4000;
								pLevelInfoHeader->bankSizeIOP = 0x57000;
								pLevelInfoHeader->sectorStartIndex = 1;
								pLevelInfoHeader->nbSectors = 12;
								pLevelInfoHeader->nbTeleporters = 0;
								

								levelT.sectors[0].sectorId = 1;
								levelT.sectors[0].sectBankSize = 0x6f4000;
								levelT.sectors[0].nbConditions = 0;

								levelT.sectors[1].sectorId = 2;
								levelT.sectors[1].sectBankSize = 4298 * 1024;
								levelT.sectors[1].nbConditions = 0;

								levelT.sectors[2].sectorId = 3;
								levelT.sectors[2].sectBankSize = 4298 * 1024;
								levelT.sectors[2].nbConditions = 0;

								levelT.sectors[3].sectorId = 4;
								levelT.sectors[3].sectBankSize = 4298 * 1024;
								levelT.sectors[3].nbConditions = 0;

								levelT.sectors[4].sectorId = 5;
								levelT.sectors[4].sectBankSize = 1252 * 1024;
								levelT.sectors[4].nbConditions = 0;

								levelT.sectors[5].sectorId = 6;
								levelT.sectors[5].sectBankSize = 40 * 1024;
								levelT.sectors[5].nbConditions = 0;

								levelT.sectors[6].sectorId = 7;
								levelT.sectors[6].sectBankSize = 28 * 1024;
								levelT.sectors[6].nbConditions = 0;

								levelT.sectors[7].sectorId = 8;
								levelT.sectors[7].sectBankSize = 932 * 1024;
								levelT.sectors[7].nbConditions = 0;

								levelT.sectors[8].sectorId = 9;
								levelT.sectors[8].sectBankSize = 28 * 1024;
								levelT.sectors[8].nbConditions = 0;

								levelT.sectors[9].sectorId = 10;
								levelT.sectors[9].sectBankSize = 226 * 1024;
								levelT.sectors[9].nbConditions = 0;

								levelT.sectors[10].sectorId = 11;
								levelT.sectors[10].sectBankSize = 28 * 1024;
								levelT.sectors[10].nbConditions = 0;

								levelT.sectors[11].sectorId = 12;
								levelT.sectors[11].sectBankSize = 196 * 1024;
								levelT.sectors[11].nbConditions = 0;
							}
#endif

							LevelsInfo_ReadHeader_V7_V9(pLevelInfoHeader, pLevelInfo);
							S_LVLNFO_TELEPORTERS_V7_V9* pTeleporters = reinterpret_cast<S_LVLNFO_TELEPORTERS_V7_V9*>(pLevelInfoHeader + 1);
							LevelsInfo_ReadTeleporters_V7_V9(pTeleporters, pLevelInfoHeader->nbTeleporters, pLevelInfo);
							S_LVLNFO_SECTOR_V7_V9* pSectors = reinterpret_cast<S_LVLNFO_SECTOR_V7_V9*>(pTeleporters + pLevelInfoHeader->nbTeleporters);
							puVar5 = LevelsInfo_ReadSectors_V7_V9(pSectors, pLevelInfoHeader->nbSectors, pLevelInfo);
							S_LVLNFO_LANGUAGE_V7_V9* pLanguages = reinterpret_cast<S_LVLNFO_LANGUAGE_V7_V9*>(puVar5);
							LevelsInfo_ReadLanguageFileNames_V7_V9(pLanguages, pLevelInfoHeader->nbLanguageFileNames, pLevelInfoHeader->levelId);
							iVar9 = pLevelInfoHeader->field_0x30;
							char* pLevelPath = reinterpret_cast<char*>(pLanguages + pLevelInfoHeader->nbLanguageFileNames);

							if (1 < iVar9) {
								iVar9 = 1;
							}

							iVar8 = 0;
							if (0 < iVar9) {
								do {
									iVar6 = 0;
									do {
										iVar7 = iVar6;
										if (iVar6 < 0x14) {
											iVar7 = iVar6 + 1;
											pLevelInfo->levelPath[iVar6] = *pLevelPath;
										}

										cVar1 = *pLevelPath;
										pLevelPath = pLevelPath + 1;
										iVar6 = iVar7;
									} while (cVar1 != '\0');

									iVar8 = iVar8 + 1;
									pLevelInfo = pLevelInfo + 1;
								} while (iVar8 < iVar9);
							}

							this->field_0x4210 = this->field_0x4210 + this->aLevelInfo[curLevelId].field_0x20;
						}
					}
				}
			}
		}

		infoLevelsFileBuffer->close();
	}

	bank.terminate();
	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Levels_UpdateDataFromSavedGame()
{
	CChunk* pChunk;
	SaveDataChunk_BLHD* pBLHD;
	S_SUBSECTOR_INFO* pcVar2;
	ScenarioVariable* pSVar3;
	int levelId;
	CChunk* pCVar5;
	uint* puVar6;
	SubSectorSaveData* pSVar7;
	uint uVar8;

	for (pChunk = this->pSaveData_0x48->FindNextSubChunk(this->pSaveData_0x48 + 1, SAVEGAME_CHUNK_BLEV);
		pChunk != (CChunk*)0x0;
		pChunk = this->pSaveData_0x48->FindNextSubChunk(reinterpret_cast<CChunk*>(reinterpret_cast<char*>(pChunk + 1) + pChunk->offset), SAVEGAME_CHUNK_BLEV)) {
		pBLHD = reinterpret_cast<SaveDataChunk_BLHD*>(pChunk->FindNextSubChunk(pChunk + 1, SAVEGAME_CHUNK_BLHD) + 1);
		levelId = pBLHD->levelId;
		if ((-1 < levelId) && (levelId < 0x10)) {
			pSVar7 = pBLHD->aSubSectorData;

			this->aLevelInfo[levelId].nbExorcisedWolfen = pBLHD->nbExorcisedWolfen;
			pcVar2 = this->aLevelInfo[levelId].aSubSectorInfo;
			levelId = 0;
			if (0 < pBLHD->maxElevatorId) {
				do {
					levelId = levelId + 1;
					pcVar2->flags = pSVar7->flags;
					pcVar2->nbExorcisedWolfen = pSVar7->nbExorcisedWolfen;
					pSVar7 = pSVar7 + 1;
					pcVar2 = pcVar2 + 1;
				} while (levelId < pBLHD->maxElevatorId);
			}
		}
	}

	LoadGame_LoadScenVars();
	LoadGame_LoadGameInfo();
	LoadGame_LoadGameObj();
	
	SaveDataChunk_BSHD* pBSHD = reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSHD) + 1);
	this->gameTime = pBSHD->gameTime;
	SaveGame_CloseChunk();

	return;
}

void CLevelScheduler::Levels_SaveDataToSavedGame()
{
	CChunk* pChunk = this->pSaveData_0x48;
	pChunk->field_0x0 = 0x16660666;
	pChunk->hash = SAVEGAME_CHUNK_BSAV;
	pChunk->size = 0x50000;
	pChunk->offset = 0;

	SaveDataChunk_BSHD* pBSHD = reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_BeginChunk(SAVEGAME_CHUNK_BSHD));
	pBSHD->levelId = 0x10;
	pBSHD->gameTime = 0;
	pBSHD->sectorId = -1;
	pBSHD->musicId = -1;
	pBSHD->field_0xc = -1;
	SaveGame_EndChunk(pBSHD + 1);

	SaveGame_SaveScenVars();
	SaveGame_SaveGameInfo();
	SaveGame_SaveGameObj();

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Episode_LoadFromIniFile(void)
{
	int iVar1;
	bool bVar2;
	EpStruct_80* episodesPtr;
	Episode* episodePtr;
	uint episodeCounter;
	int minWolfen;
	int scenery;
	int monster;
	int bet;
	undefined4 local_120;
	int local_11c;
	char Section[256];
	int local_10;
	int local_c;
	char episodeCounterString[4];

	episodesPtr = _gGameNfo.aEpisodes;

	_gGameNfo.nbEpisodes = 0;
	_gGameNfo.nbMagic = 0;
	_gGameNfo.nbMoney = 0;
	_gGameNfo.scenery = 0;
	_gGameNfo.monster = 0;
	_gGameNfo.bet = 0;
	_gGameNfo.bank = 0;
	_gGameNfo.shop = 0;
	_gGameNfo.health = (float)_gScenVarInfo[20].currentValue;

	iVar1 = 0;
	do {
		local_10 = iVar1;
		local_c = 0;
		bVar2 = true;
		while (bVar2) {
			episodesPtr->aSubObj[local_c].itemId = -1;
			episodesPtr->aSubObj[local_c].field_0x4 = 0;
			local_c = local_c + 1;
			bVar2 = local_c < 0x10;
		}

		episodesPtr = episodesPtr + 1;
		iVar1 = local_10 + 1;
	} while (local_10 + 1 < 2);

	edStrCatMulti(Section, "Episode_", "General", NULL);

	gIniFile.GetKey(Section, "HeroMagic", &_gGameNfo.nbMagic);

	bVar2 = gIniFile.GetKey(Section, "NbEpisodes", &_gGameNfo.nbEpisodes);
	if (((bVar2 != false) && (0 < _gGameNfo.nbEpisodes)) && (episodeCounter = 0, 0 < _gGameNfo.nbEpisodes)) {
		episodePtr = g_EpisodeDataArray_0048eb0;
		do {
			edStrInt2Str(episodeCounter, episodeCounterString, 2, true);
			edStrCatMulti(Section, "Episode_", episodeCounterString, NULL);
			gIniFile.GetKey(Section, "MinWolfen", &minWolfen);
			gIniFile.GetKey(Section, "Scenery", &scenery);
			gIniFile.GetKey(Section, "Monster", &monster);
			gIniFile.GetKey(Section, "Bet", &bet);
			local_120 = 0;
			episodeCounter = episodeCounter + 1;
			episodePtr->minWolfen = minWolfen;
			episodePtr->scenery = scenery;
			episodePtr->monster = monster;
			episodePtr->bet = bet;
			episodePtr->field_0x10 = 0;
			episodePtr->total_0x14 = local_11c;
			episodePtr = episodePtr + 1;
		} while (episodeCounter < _gGameNfo.nbEpisodes);
	}

	CScene::ptable.g_FrontendManager_00451680->SetGlobalFunc_001cf8e0();

	Episode_ComputeCurrent();

	return;
}

void CLevelScheduler::SaveGame_SaveCurLevelState(int param_2)
{
	CChunk* pCVar1;
	CChunk* pChunk;
	uint uVar3;
	CChunk* pCVar4;
	int iVar5;
	undefined4* puVar6;
	ScenarioVariable* pSVar7;
	CChunkDesc* pCVar8;
	int iVar9;
	undefined4* puVar10;
	CAudioManager* pAudioManager;

	if ((this->field_0x80 | this->bShouldLoad | this->bShouldSave) == 0) {
		this->field_0x74 = param_2;

		pChunk = SaveGame_GetLevelChunk(this->currentLevelID);

		if (pChunk != (CChunk*)0x0) {
			this->pSaveData_0x48->DeleteSubChunk(pChunk);
		}

		pChunk = this->pSaveData_0x48->FindNextSubChunk(this->pSaveData_0x48 + 1, SAVEGAME_CHUNK_BSCN);
		if (pChunk != (CChunk*)0x0) {
			this->pSaveData_0x48->DeleteSubChunk(pChunk);
		}

		pChunk = this->pSaveData_0x48->FindNextSubChunk(this->pSaveData_0x48 + 1, SAVEGAME_CHUNK_BGNF);
		if (pChunk != (CChunk*)0x0) {
			this->pSaveData_0x48->DeleteSubChunk(pChunk);
		}

		pChunk = this->pSaveData_0x48->FindNextSubChunk(this->pSaveData_0x48 + 1, SAVEGAME_CHUNK_BOBJ);
		if (pChunk != (CChunk*)0x0) {
			this->pSaveData_0x48->DeleteSubChunk(pChunk);
		}

		int* pLev = (int*)SaveGame_BeginChunk(SAVEGAME_CHUNK_BLEV);
		SaveLevelInfo(&this->aLevelInfo[this->currentLevelID]);
		CScene::_pinstance->Level_SaveContext();
		SaveGame_EndChunk(0x0);

		SaveGame_SaveScenVars();
		SaveGame_SaveGameInfo();
		SaveGame_SaveGameObj();

		SaveDataChunk_BSHD* pBSHD = reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSHD) + 1);
		pBSHD->gameTime = (int)this->gameTime;
		if (this->currentLevelID == 0xe) {
			pBSHD->levelId = this->nextLevelId;
			pBSHD->sectorId = this->aLevelInfo[this->nextLevelId].sectorStartIndex;
		}
		else {
			pBSHD->levelId = this->currentLevelID;
			if (CActorHero::_gThis == (CActorHero*)0x0) {
				pBSHD->sectorId = ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID;
			}
			else {
				pBSHD->sectorId = CActorHero::_gThis->GetLastCheckpointSector();
			}
		}

		IMPLEMENTATION_GUARD_AUDIO(
		pAudioManager = CScene::ptable.g_AudioManager_00451698;
		iVar9 = CAudioManager::FUN_001819b0(CScene::ptable.g_AudioManager_00451698);
		pBSHD->field_0xc = iVar9;
		uVar3 = CAudioManager::GetMusicId(pAudioManager);
		pBSHD->musicId = uVar3;
		)
		SaveGame_CloseChunk();

		pChunk = SaveGame_GetLevelChunk(this->currentLevelID);

		this->field_0x74 = 0;
	}

	return;
}

CChunk* CLevelScheduler::SaveGame_GetLevelChunk(int levelId)
{
	CChunk* pCurChunk = this->pSaveData_0x48->FindNextSubChunk(this->pSaveData_0x48 + 1, SAVEGAME_CHUNK_BLEV);
	CChunk* pLevelChunk = (CChunk*)0x0;
	
	while ((pCurChunk != (CChunk*)0x0 && (pLevelChunk == (CChunk*)0x0))) {
		SaveDataChunk_BLHD* pBLHD = reinterpret_cast<SaveDataChunk_BLHD*>(pCurChunk->FindNextSubChunk(pCurChunk + 1, SAVEGAME_CHUNK_BLHD) + 1);
		if (levelId == pBLHD->levelId) {
			pLevelChunk = pCurChunk;
		}

		pCurChunk = this->pSaveData_0x48->FindNextSubChunk(reinterpret_cast<CChunk*>(reinterpret_cast<char*>(pCurChunk + 1) + pCurChunk->offset), SAVEGAME_CHUNK_BLEV);
	}

	return pLevelChunk;
}

void CLevelScheduler::SaveGame_LoadLevelState(int levelId)
{
	uint uVar1;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	bool bVar6;
	int* piVar7;
	int* piVar8;
	int* piVar9;
	SaveDataChunk_BLHD* pSVar10;
	bool bVar11;
	void* pCVar12;
	SaveDataChunk_BLHD* uVar11;
	CChunk* pCVar14;
	int* piVar15;
	ulong uVar16;
	ulong uVar17;
	int* piVar18;
	int* piVar19;
	int* piVar20;
	int* local_60;
	CChunk* pBLHD;
	CChunk* pBLEV;
	bool bFoundLevelSaveData;
	CAudioManager* pAudioManager;
	CScene* pScene;

	if (this->bShouldLoad != 0) {
		SaveDataChunk_BSHD* pBSHD = reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSHD) + 1);
		IMPLEMENTATION_GUARD_AUDIO(
		CAudioCScene::ptable.g_AudioManager_00451698->FUN_00181970(pBSHD->field_0xc);
		CAudioCScene::ptable.g_AudioManager_00451698->SetMusic(pBSHD->field_x10);)

		SaveGame_CloseChunk();
	}

	pBLEV = SaveGame_GetLevelChunk(levelId);

	if (pBLEV != (CChunk*)0x0) {
		bFoundLevelSaveData = IsACompatibleChunkRecurse(pBLEV);

		pScene = CScene::_pinstance;
		if (bFoundLevelSaveData) {
			this->curChunkIndex = this->curChunkIndex + 1;
			this->aSaveGameChunks[this->curChunkIndex] = (CChunk*)pBLEV;

			CScene::_pinstance->Level_LoadContext();

			this->aSaveGameChunks[this->curChunkIndex] = (CChunk*)0x0;
			this->curChunkIndex = this->curChunkIndex + -1;
			IMPLEMENTATION_GUARD_LOG(
			CMapManager::Func_003f81c0(CScene::ptable.g_MapManager_0045168c);)

			if (this->bShouldLoad != 0) {
				pScene->Level_CheckpointReset();
			}
		}
	}

	Level_UpdateCurLiveLevelInfo();

	return;
}

bool CLevelScheduler::IsACompatibleChunkRecurse(CChunk* pChunk)
{
	CChunk* pBLHD;

	CChunkDesc* pChunkDesc = GetChunkDesc(pChunk->hash);

	bool bFoundLevelSaveData = true;
	if (pChunkDesc != (CChunkDesc*)0x0) {
		bFoundLevelSaveData = pChunkDesc->size >> 0x10 == pChunk->size >> 0x10;
	}

	CChunk* pCurChunk;
	if ((bFoundLevelSaveData) && (pCurChunk = pChunk, pChunk->field_0x0 == 0x16660666)) {
		while ((pBLHD = pCurChunk + 1, reinterpret_cast<char*>(pBLHD) < reinterpret_cast<char*>(pChunk) + pChunk->size + sizeof(CChunk)) && (bFoundLevelSaveData)) {
			if (IsACompatibleChunkRecurse(pBLHD) == 0) {
				bFoundLevelSaveData = false;
			}

			pCurChunk = reinterpret_cast<CChunk*>(reinterpret_cast<char*>(pBLHD) + pBLHD->size);
		}
	}

	return bFoundLevelSaveData;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
int CLevelScheduler::SaveGame_GetMaxBufferSize()
{
	return 0x10000;
}

void CLevelScheduler::UpdateGameInfo(float health, int magic, int money)
{
	_gGameNfo.health = health;
	_gGameNfo.nbMagic = magic;
	_gGameNfo.nbMoney = money;
	return;
}

void CLevelScheduler::GetGameInfo(float* pHealth, float* pMagic, float* pMoney)
{
	*pHealth = _gGameNfo.health;
	*pMagic = (float)_gGameNfo.nbMagic;
	*pMoney = (float)_gGameNfo.nbMoney;

	return;
}

void CLevelScheduler::Game_SaveInventory(CInventoryInterface* pInventory)
{
	bool bVar1;
	EpStruct_80* pEpisode;
	int epIndex;
	InventorySlot* pSlot;
	int slotIndex;

	epIndex = 0;
	pEpisode = _gGameNfo.aEpisodes;
	do {
		slotIndex = 0;
		bVar1 = true;

		while (bVar1) {
			pSlot = pInventory->aSlots[epIndex] + slotIndex;
			pEpisode->aSubObj[slotIndex].itemId = pSlot->itemId;
			pEpisode->aSubObj[slotIndex].field_0x4 = pSlot->field_0x4;
			slotIndex = slotIndex + 1;
			bVar1 = slotIndex < 0x10;
		}

		epIndex = epIndex + 1;
		pEpisode = pEpisode + 1;
	} while (epIndex < 2);

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Game_LoadInventory(CInventoryInterface* pInventory)
{
	bool bVar1;
	EpStruct_80* pEpisode;
	int epIndex;
	InventorySlot* pSlot;
	int slotIndex;

	epIndex = 0;
	pEpisode = _gGameNfo.aEpisodes;
	do {
		slotIndex = 0;
		do {
			const int ItemId = pEpisode->aSubObj[slotIndex].itemId;
			if (ItemId != -1) {
				IMPLEMENTATION_GUARD(
					pInventory->Cmd_AddItem(ItemId, pEpisode->aSubObj[slotIndex].field_0x4, 0);)
			}

			slotIndex = slotIndex + 1;
		} while (slotIndex < 0x10);

		epIndex = epIndex + 1;
		pEpisode = pEpisode + 1;
	} while (epIndex < 2);

	return;
}

ulong gMedallionHashCodes[9] = 
{
	CHAR_TO_UINT64("FIGHT_08"),
	CHAR_TO_UINT64("MED_01\0\0"),
	CHAR_TO_UINT64("MED_02\0\0"),
	CHAR_TO_UINT64("MED_03\0\0"),
	CHAR_TO_UINT64("MED_04\0\0"),
	CHAR_TO_UINT64("MED_05\0\0"),
	CHAR_TO_UINT64("MED_06\0\0"),
	CHAR_TO_UINT64("MED_07\0\0"),
	CHAR_TO_UINT64("MED_08\0\0"),
};

uint CLevelScheduler::GetMedallionLevel()
{
	uint medallionLevel;

	medallionLevel = (uint)(_gScenVarInfo[28].currentValue == 2);

	if (_gScenVarInfo[34].currentValue == 2) {
		medallionLevel = medallionLevel + 1;
	}

	if (_gScenVarInfo[41].currentValue == 2) {
		medallionLevel = medallionLevel + 1;
	}

	if (_gScenVarInfo[47].currentValue == 2) {
		medallionLevel = medallionLevel + 1;
	}

	if (_gScenVarInfo[52].currentValue == 2) {
		medallionLevel = medallionLevel + 1;
	}

	if (_gScenVarInfo[56].currentValue == 2) {
		medallionLevel = medallionLevel + 1;
	}

	if (_gScenVarInfo[63].currentValue == 2) {
		medallionLevel = medallionLevel + 1;
	}

	return medallionLevel;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Episode_ComputeCurrent()
{
	int iVar1;
	int iVar2;

	for (iVar2 = 1; iVar2 < _gGameNfo.nbEpisodes; iVar2 = iVar2 + 1) {
		if ((iVar2 < 0) || (iVar1 = iVar2, _gGameNfo.nbEpisodes <= iVar2)) {
			iVar1 = _gGameNfo.nbEpisodes + -1;
		}

		if (_gScenVarInfo[SCN_GAME_NUM_FREED_WOLFENS].currentValue < g_EpisodeDataArray_0048eb0[iVar1].minWolfen) break;
	}

	iVar1 = iVar2 + -1;

	if (iVar1 != _gScenVarInfo[6].currentValue) {
		if ((iVar1 < 0) || (_gGameNfo.nbEpisodes <= iVar1)) {
			iVar1 = _gGameNfo.nbEpisodes + -1;
		}

		_gScenVarInfo[6].currentValue = iVar2 + -1;
		g_EpisodeDataArray_0048eb0[iVar1].total_0x14 = g_EpisodeDataArray_0048eb0[iVar1].bet +
			g_EpisodeDataArray_0048eb0[iVar1].scenery + g_EpisodeDataArray_0048eb0[iVar1].monster;

		_gGameNfo.scenery = _gGameNfo.scenery + g_EpisodeDataArray_0048eb0[iVar1].scenery;
		_gGameNfo.monster = g_EpisodeDataArray_0048eb0[iVar1].monster;
		_gGameNfo.bet = g_EpisodeDataArray_0048eb0[iVar1].bet;
		_gGameNfo.bank = 0;
		_gGameNfo.shop = 0;
	}

	return;
}

int CLevelScheduler::GetBoomyLevel() 
{
	return _gScenVarInfo[SCN_ABILITY_BOOMY_TYPE].currentValue;
}

ulong gFightHashCodes[8] = 
{
	0x0,
	CHAR_TO_UINT64("FIGHT_01"),
	CHAR_TO_UINT64("FIGHT_02"),
	CHAR_TO_UINT64("FIGHT_03"),
	CHAR_TO_UINT64("FIGHT_04"),
	CHAR_TO_UINT64("FIGHT_05"),
	CHAR_TO_UINT64("FIGHT_06"),
	CHAR_TO_UINT64("FIGHT_07"),
};

int CLevelScheduler::GetFightLevel() 
{
	int fightLevel;
	uint uVar1;

	fightLevel = 0;
	uVar1 = _gScenVarInfo[SCN_ABILITY_FIGHT].currentValue;

	if (_gScenVarInfo[SCN_ABILITY_FIGHT].currentValue != 0) {
		do {
			uVar1 = uVar1 >> 1;
			fightLevel = fightLevel + 1;
		} while (uVar1 != 0);
	}

	return fightLevel;
}

int INT_ARRAY_0048ed60[16] = { 0 };
int INT_ARRAY_0048eda0[16] = { 0 };

CChunkDesc* CLevelScheduler::GetChunkDesc(uint hash)
{
	uint gameChunkHash;
	CChunkDesc* pGameChunk;
	for (pGameChunk = _gGameChunks; (gameChunkHash = pGameChunk->hash, gameChunkHash != hash && (gameChunkHash != 0)); pGameChunk = pGameChunk + 1) {
	}

	if (gameChunkHash == 0) {
		pGameChunk = (CChunkDesc*)0x0;
	}

	return pGameChunk;
}

void* CLevelScheduler::SaveGame_BeginChunk(uint hash)
{
	CChunkDesc* pGameChunk = GetChunkDesc(hash);
	CChunk* pSaveChunk = reinterpret_cast<CChunk*>(reinterpret_cast<char*>(this->aSaveGameChunks[this->curChunkIndex]) + this->aSaveGameChunks[this->curChunkIndex]->offset);

	pSaveChunk = pSaveChunk + 1;

	if (pGameChunk->field_0x4 == 0) {
		pSaveChunk->field_0x0 = 0x16660666;
	}
	else {
		pSaveChunk->field_0x0 = 0x6667666;
	}

	pSaveChunk->hash = hash;
	pSaveChunk->size = pGameChunk->size;
	pSaveChunk->offset = pGameChunk->offset;

	this->curChunkIndex = this->curChunkIndex + 1;
	this->aSaveGameChunks[this->curChunkIndex] = pSaveChunk;

	void* pData = pSaveChunk + 1;

	if (pSaveChunk->field_0x0 != 0x6667666) {
		pData = (void*)0x0;
	}

	return pData;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::SaveGame_EndChunk(void* pDataEnd)
{
	CChunk* pChunk = this->aSaveGameChunks[this->curChunkIndex];
	this->aSaveGameChunks[this->curChunkIndex] = (CChunk*)0x0;
	this->curChunkIndex--;

	if (pChunk->offset == 0) {
		pChunk->offset = reinterpret_cast<char*>(pDataEnd) - reinterpret_cast<char*>(pChunk) - sizeof(CChunk);
	}

	this->aSaveGameChunks[this->curChunkIndex]->offset = 
		reinterpret_cast<char*>(pChunk) + pChunk->offset + sizeof(CChunk) - reinterpret_cast<char*>(this->aSaveGameChunks[this->curChunkIndex] + 1);

	return;
}

CChunk* CLevelScheduler::SaveGame_OpenChunk(uint hash)
{
	CChunk* pChunk;

	pChunk = this->aSaveGameChunks[this->curChunkIndex]->FindNextSubChunk(this->aSaveGameChunks[this->curChunkIndex] + 1, hash);
	if (pChunk != (CChunk*)0x0) {
		this->curChunkIndex = this->curChunkIndex + 1;
		this->aSaveGameChunks[this->curChunkIndex] = pChunk;
	}

	return pChunk;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::SaveGame_CloseChunk()
{
	this->aSaveGameChunks[this->curChunkIndex] = (CChunk*)0x0;
	this->curChunkIndex = this->curChunkIndex + -1;

	assert(this->curChunkIndex >= 0);

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::SaveGame_SaveScenVars()
{
	int* pScenVariables = (int*)SaveGame_BeginChunk(SAVEGAME_CHUNK_BSCN);

	*pScenVariables = 0x62;
	ScenarioVariable* pSVar8 = _gScenVarInfo;
	int* pCurScenVariables = pScenVariables;
	int iVar12 = 0;
	int iVar3;
	do {
		iVar3 = iVar12;
		iVar12 = iVar3 + 8;
		pCurScenVariables[1] = pSVar8->currentValue;
		pCurScenVariables[2] = pSVar8[1].currentValue;
		pCurScenVariables[3] = pSVar8[2].currentValue;
		pCurScenVariables[4] = pSVar8[3].currentValue;
		pCurScenVariables[5] = pSVar8[4].currentValue;
		pCurScenVariables[6] = pSVar8[5].currentValue;
		pCurScenVariables[7] = pSVar8[6].currentValue;
		pCurScenVariables[8] = pSVar8[7].currentValue;
		pSVar8 = pSVar8 + 8;
		pCurScenVariables = pCurScenVariables + 8;
	} while (iVar12 < 0x5a);

	(pScenVariables + iVar12)[1] = _gScenVarInfo[iVar12].currentValue;
	(pScenVariables + iVar12)[2] = _gScenVarInfo[iVar3 + 9].currentValue;


	SaveGame_EndChunk(reinterpret_cast<char*>(pScenVariables) + 0x18c);

	return;
}

struct SaveDataChunk_BGNF
{
	float health;
	float field_0x4;
	int nbMagic;
	int field_0xc;
	int nbMoney;
	int scenery;
	int monster;
	int bet;
	int bank;
	int shop;
	int originalScenery;
	int originalMonster;
	int originalBet;
	EpStruct_80 aEpisodes[2];
};

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::SaveGame_SaveGameInfo()
{
	int iVar4;
	SaveDataChunk_BGNF* pBGNF;

	pBGNF = reinterpret_cast<SaveDataChunk_BGNF*>(SaveGame_BeginChunk(SAVEGAME_CHUNK_BGNF));

	pBGNF->health = _gGameNfo.health;
	pBGNF->nbMagic = _gGameNfo.nbMagic;
	pBGNF->nbMoney = _gGameNfo.nbMoney;
	pBGNF->scenery = _gGameNfo.scenery;
	pBGNF->monster = _gGameNfo.monster;
	pBGNF->bet = _gGameNfo.bet;
	pBGNF->bank = _gGameNfo.bank;
	pBGNF->shop = _gGameNfo.shop;
	if ((_gScenVarInfo[6].currentValue < 0) || (_gGameNfo.nbEpisodes <= _gScenVarInfo[6].currentValue)) {
		pBGNF->originalScenery = 0;
		pBGNF->originalMonster = 0;
		pBGNF->originalBet = 0;
	}
	else {
		if ((_gScenVarInfo[6].currentValue < 0) ||
			(iVar4 = _gScenVarInfo[6].currentValue, _gGameNfo.nbEpisodes <= _gScenVarInfo[6].currentValue)) {
			iVar4 = _gGameNfo.nbEpisodes + -1;
		}

		pBGNF->originalScenery = g_EpisodeDataArray_0048eb0[iVar4].scenery;
		pBGNF->originalMonster = g_EpisodeDataArray_0048eb0[iVar4].monster;
		pBGNF->originalBet = g_EpisodeDataArray_0048eb0[iVar4].bet;
	}

	for (int i = 0; i < 2; i = i + 1) {
		for (int j = 0; j < 0x10; j = j + 1) {
			pBGNF->aEpisodes[i].aSubObj[j] = _gGameNfo.aEpisodes[i].aSubObj[j];
		}
	}

	SaveGame_EndChunk(pBGNF);

	return;
}

struct LoadLoopObject_50SaveData
{
	int field_0x0;
	undefined4 field_0x4;
	undefined4 field_0x8;
	undefined4 field_0xc;
	undefined8 field_0x10;
	edF32VECTOR3 field_0x18;
};

struct NativShopLevelSubObjSaveData
{
	int field_0x0;
	int currentLevelId;
	int field_0x8;
	edF32VECTOR3 currentLocation;
	edF32VECTOR3 rotationQuat;
	NativShopLevelSubObjSubObj aSubObjs[5];
};

struct SaveDataChunk_BOBJ
{
	int field_0x0;
	int field_0x4;
	undefined4 field_0x8;
	NativShopLevelSubObjSaveData field_0xc[10];
	LoadLoopObject_50SaveData field_0x494[64];
};

void CLevelScheduler::SaveGame_SaveGameObj()
{
	int iVar4;
	NativShopLevelSubObj* pShopSubObj;
	NativShopLevelSubObjSaveData* pShopSave;
	LoadLoopObject_50* pLL50;
	SaveDataChunk_BOBJ* pObjSave;

	pObjSave = reinterpret_cast<SaveDataChunk_BOBJ*>(SaveGame_BeginChunk(SAVEGAME_CHUNK_BOBJ));

	pShopSave = pObjSave->field_0xc;
	pShopSubObj = this->aNativShopSubObjs;
	pObjSave->field_0x0 = this->objCount_0x4218;
	pObjSave->field_0x4 = this->nbNativShopSubObjs;
	pObjSave->field_0x8 = this->field_0x5b30;

	iVar4 = 0;
	if (0 < this->nbNativShopSubObjs) {
		do {
			pShopSave->field_0x0 = pShopSubObj->field_0x0;
			pShopSave->currentLevelId = pShopSubObj->currentLevelId;
			pShopSave->field_0x8 = pShopSubObj->field_0x8;
			pShopSave->currentLocation = (pShopSubObj->currentLocation).xyz;
			pShopSave->rotationQuat = (pShopSubObj->rotationQuat).xyz;
			for (int i = 0; i < 5; i = i + 1) {
				pShopSave->aSubObjs[i] = pShopSubObj->aSubObjs[i];
			}
			iVar4 = iVar4 + 1;
		} while (iVar4 < this->nbNativShopSubObjs);
	}

	LoadLoopObject_50SaveData* pLL50Save = pObjSave->field_0x494;
	pLL50 = this->field_0x4220;
	iVar4 = 0;
	if (0 < this->objCount_0x4218) {
		do {
			pLL50Save->field_0x0 = pLL50->field_0x0;
			pLL50Save->field_0x4 = pLL50->field_0x24;
			pLL50Save->field_0x8 = pLL50->field_0x28;
			pLL50Save->field_0xc = pLL50->field_0x2c;
			pLL50Save->field_0x10 = pLL50->field_0x40;
			pLL50Save->field_0x18 = (pLL50->field_0x30).xyz;
			iVar4 = iVar4 + 1;
		} while (iVar4 < this->objCount_0x4218);
	}

	SaveGame_EndChunk(reinterpret_cast<char*>(pObjSave) + pObjSave->field_0x0 * 0x24 + 0x494);

	return;
}

void CLevelScheduler::LoadGame_LoadScenVars()
{
	int* pScenVariables = reinterpret_cast<int*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSCN) + 1);

	uint nbScenVars = *pScenVariables;

	if (nbScenVars > 0x62) {
		nbScenVars = 0x62;
	}

	ScenarioVariable* pSVar8 = _gScenVarInfo;
	int* pCurScenVariables = pScenVariables;
	
	for (int i = 0; i < nbScenVars; i = i + 1) {
		pSVar8->currentValue = pCurScenVariables[1];
		pSVar8 = pSVar8 + 1;
		pCurScenVariables = pCurScenVariables + 1;
	}

	SaveGame_CloseChunk();

	return;
}

void CLevelScheduler::LoadGame_LoadGameInfo()
{
	int iVar3;

	SaveDataChunk_BGNF* pBGNF;

	CChunk* pChunk = SaveGame_OpenChunk(SAVEGAME_CHUNK_BGNF);
	pBGNF = reinterpret_cast<SaveDataChunk_BGNF*>(pChunk + 1);

	if (pBGNF != (SaveDataChunk_BGNF*)0x0) {
		_gGameNfo.health = pBGNF->health;
		_gGameNfo.nbMagic = pBGNF->nbMagic;
		_gGameNfo.nbMoney = pBGNF->nbMoney;
		_gGameNfo.scenery = pBGNF->scenery;
		_gGameNfo.monster = pBGNF->monster;
		_gGameNfo.bet = pBGNF->bet;
		_gGameNfo.bank = pBGNF->bank;
		_gGameNfo.shop = pBGNF->shop;

		if ((_gScenVarInfo[6].currentValue < 0) ||
			(iVar3 = _gScenVarInfo[6].currentValue, _gGameNfo.nbEpisodes <= _gScenVarInfo[6].currentValue)) {
			iVar3 = _gGameNfo.nbEpisodes + -1;
		}

		if (pBGNF->originalScenery != g_EpisodeDataArray_0048eb0[iVar3].scenery) {
			_gGameNfo.scenery = _gGameNfo.scenery + (g_EpisodeDataArray_0048eb0[iVar3].scenery - pBGNF->originalScenery);
		}
		if (pBGNF->originalMonster != g_EpisodeDataArray_0048eb0[iVar3].monster) {
			_gGameNfo.monster = _gGameNfo.monster + (g_EpisodeDataArray_0048eb0[iVar3].monster - pBGNF->originalMonster);
		}
		if (pBGNF->originalBet != g_EpisodeDataArray_0048eb0[iVar3].bet) {
			_gGameNfo.bet = _gGameNfo.bet + (g_EpisodeDataArray_0048eb0[iVar3].bet - pBGNF->originalBet);
		}

		if (pChunk->size == 0x20003) {
			for (int i = 0; i < 2; i = i + 1) {
				for (int j = 0; j < 0x10; j = j + 1) {
					_gGameNfo.aEpisodes[i].aSubObj[j] = pBGNF->aEpisodes[i].aSubObj[j];
				}
			}
		}
		else {
			for (int i = 0; i < 2; i = i + 1) {
				for (int j = 0; j < 0x10; j = j + 1) {
					_gGameNfo.aEpisodes[i].aSubObj[j].itemId = -1;
					_gGameNfo.aEpisodes[i].aSubObj[j].field_0x4 = 0;
				}
			}
		}

		SaveGame_CloseChunk();
	}

	return;
}

void CLevelScheduler::LoadGame_LoadGameObj()
{
	NativShopLevelSubObjSubObj* pNVar1;
	SaveDataChunk_BOBJ* pBOBJ;
	int iVar2;
	NativShopLevelSubObj* pNVar3;
	NativShopLevelSubObjSaveData* pNVar4;
	int iVar5;
	LoadLoopObject_50* pLVar6;
	LoadLoopObject_50SaveData* pLVar7;
	float fVar8;
	float fVar9;

	pBOBJ = reinterpret_cast<SaveDataChunk_BOBJ*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BOBJ) + 1);

	if (pBOBJ != (SaveDataChunk_BOBJ*)0x0) {
		this->nbNativShopSubObjs = pBOBJ->field_0x4;

		if (10 < this->nbNativShopSubObjs) {
			this->nbNativShopSubObjs = 10;
		}

		pNVar4 = pBOBJ->field_0xc;
		pNVar3 = this->aNativShopSubObjs;
		this->field_0x5b30 = pBOBJ->field_0x8;
		iVar2 = 0;
		if (0 < this->nbNativShopSubObjs) {
			do {
				iVar2 = iVar2 + 1;
				pNVar3->field_0x0 = pNVar4->field_0x0;
				pNVar3->currentLevelId = pNVar4->currentLevelId;
				pNVar3->field_0x8 = pNVar4->field_0x8;

				(pNVar3->currentLocation).xyz = pNVar4->currentLocation;
				(pNVar3->currentLocation).w = 1.0f;

				(pNVar3->rotationQuat).xyz = pNVar4->rotationQuat;
				(pNVar3->rotationQuat).w = 0.0f;

				pNVar3->aSubObjs[0].field_0x0 = pNVar4->aSubObjs[0].field_0x0;
				pNVar3->aSubObjs[0].field_0x4 = pNVar4->aSubObjs[0].field_0x4;
				pNVar3->aSubObjs[0].field_0x8 = pNVar4->aSubObjs[0].field_0x8;
				pNVar3->aSubObjs[1].field_0x0 = pNVar4->aSubObjs[1].field_0x0;
				pNVar3->aSubObjs[1].field_0x4 = pNVar4->aSubObjs[1].field_0x4;
				pNVar3->aSubObjs[1].field_0x8 = pNVar4->aSubObjs[1].field_0x8;
				pNVar3->aSubObjs[2].field_0x0 = pNVar4->aSubObjs[2].field_0x0;
				pNVar3->aSubObjs[2].field_0x4 = pNVar4->aSubObjs[2].field_0x4;
				pNVar3->aSubObjs[2].field_0x8 = pNVar4->aSubObjs[2].field_0x8;
				pNVar3->aSubObjs[3].field_0x0 = pNVar4->aSubObjs[3].field_0x0;
				pNVar3->aSubObjs[3].field_0x4 = pNVar4->aSubObjs[3].field_0x4;
				pNVar3->aSubObjs[3].field_0x8 = pNVar4->aSubObjs[3].field_0x8;
				pNVar3->aSubObjs[4].field_0x0 = pNVar4->aSubObjs[4].field_0x0;
				pNVar3->aSubObjs[4].field_0x4 = pNVar4->aSubObjs[4].field_0x4;
				pNVar1 = pNVar4->aSubObjs;
				pNVar4 = pNVar4 + 1;
				pNVar3->aSubObjs[4].field_0x8 = pNVar1[4].field_0x8;
				pNVar3 = pNVar3 + 1;
			} while (iVar2 < this->nbNativShopSubObjs);
		}

		pLVar7 = pBOBJ->field_0x494;
		iVar2 = 0;
		if (0 < pBOBJ->field_0x0) {
			do {
				pLVar6 = this->field_0x4220;
				iVar5 = 0;
				if (0 < this->objCount_0x4218) {
					do {
						if (pLVar6->field_0x0 == pLVar7->field_0x0) goto LAB_002da248;
						iVar5 = iVar5 + 1;
						pLVar6 = pLVar6 + 1;
					} while (iVar5 < this->objCount_0x4218);
				}
				pLVar6 = (LoadLoopObject_50*)0x0;

			LAB_002da248:
				if (pLVar6 != (LoadLoopObject_50*)0x0) {
					pLVar6->field_0x24 = pLVar7->field_0x4;
					pLVar6->field_0x28 = pLVar7->field_0x8;
					pLVar6->field_0x2c = pLVar7->field_0xc;
					pLVar6->field_0x40 = pLVar7->field_0x10;
					(pLVar6->field_0x30).xyz = pLVar7->field_0x18;
					(pLVar6->field_0x30).w = 1.0;
				}

				iVar2 = iVar2 + 1;
				pLVar7 = pLVar7 + 1;
			} while (iVar2 < pBOBJ->field_0x0);
		}
		
		SaveGame_CloseChunk();
	}

	return;
}

void SaveGame_SaveDesc(SaveDataDesc* pDesc, uint levelId)
{
	CLevelScheduler* pCVar1;
	undefined4 uVar2;
	int iVar3;

	pCVar1 = CLevelScheduler::gThis;
	pDesc->levelId = levelId;
	pDesc->gameTime = pCVar1->gameTime;
	uVar2 = CLevelScheduler::ScenVar_Get(0);
	pDesc->nbFreedWolfen = (short)uVar2;
	pDesc->nbMagic = CLevelScheduler::_gGameNfo.nbMagic;
	pDesc->nbMoney = CLevelScheduler::_gGameNfo.nbMoney;
	iVar3 = CLevelScheduler::ScenVar_Get(0x3d);
	if (iVar3 == 2) {
		pDesc->bGameCompleted = 1;
	}
	else {
		pDesc->bGameCompleted = 0;
	}
	return;
}

void CLevelScheduler::ResetAutoSaveTriggerTime()
{
	this->autoSaveTriggerTime = 0.0f;

	return;
}

uint CLevelScheduler::SaveGame_SaveToBuffer(SaveBigAlloc* pSaveData, SaveDataDesc* pSaveDesc)
{
	uint size;

	SaveGame_SaveCurLevelState(1);

	size = this->pSaveData_0x48->offset + 0x10;
	memcpy(pSaveData, this->pSaveData_0x48, size);
	SaveGame_SaveDesc(pSaveDesc, reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSHD) + 1)->levelId);
	SaveGame_CloseChunk();
	return size;
}

void CLevelScheduler::SaveGame_LoadFromBuffer(SaveBigAlloc* pSaveData, uint size)
{
	uint levelID;

	memcpy(this->pSaveData_0x48, pSaveData, size);
	this->bShouldLoad = 1;

	SaveDataChunk_BSHD* pBSHD = reinterpret_cast<SaveDataChunk_BSHD*>(SaveGame_OpenChunk(SAVEGAME_CHUNK_BSHD) + 1);
	levelID = pBSHD->levelId;
	SaveGame_CloseChunk();

	if (levelID == 0x10) {
		levelID = this->nextLevelId;
	}

	Level_FillRunInfo(levelID, -1, -1);
	CScene::_pinstance->SetFadeStateTerm(true);

	return;
}

void CLevelScheduler::Level_WolfenChanged()
{
	Level_UpdateCurLiveLevelInfo();

	return;
}

#ifdef PLATFORM_WIN
int gDebugLevelLoadOverride = -1;
#endif

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Game_Init()
{
	CChunk* pSVar1;
	bool bVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	undefined4* puVar6;
	uint* puVar7;
	undefined4* puVar8;
	int* piVar9;
	int* piVar10;
	int iVar11;
	S_LEVEL_INFO* pLVar12;
	char local_80[128];

	this->pSaveData_0x48 = reinterpret_cast<CChunk*>(edMemAlloc(TO_HEAP(H_MAIN), 0x10000));
	this->pSaveDataEnd_0x4c = pSaveData_0x48 + 1;
	memset(pSaveData_0x48, 0, 0x10000);
	this->aSaveGameChunks[0] = (CChunk*)0x0;
	this->aSaveGameChunks[1] = (CChunk*)0x0;
	this->aSaveGameChunks[2] = (CChunk*)0x0;
	this->aSaveGameChunks[3] = (CChunk*)0x0;
	pLVar12 = this->aLevelInfo;
	this->aSaveGameChunks[4] = (CChunk*)0x0;
	iVar11 = 0;
	this->aSaveGameChunks[5] = (CChunk*)0x0;
	this->aSaveGameChunks[6] = (CChunk*)0x0;
	this->aSaveGameChunks[7] = (CChunk*)0x0;
	curChunkIndex = 0;
	this->aSaveGameChunks[curChunkIndex] = pSaveData_0x48;
	this->pObjectiveStreamBegin = 0;
	this->pObjectiveStreamEnd = 0;
	this->field_0x74 = 0;
	this->bShouldLoad = 0;
	this->bShouldSave = 0;
	this->field_0x80 = 0;
	this->pLevelBankBufferEntry = (edCBankBufferEntry*)0x0;
	this->pIOPBankBufferEntry = (edCBankBufferEntry*)0x0;
	this->loadStage_0x5b48 = 4;
	this->autoSaveTriggerTime = 0.0f;
	this->curAutoSaveTime = 0.0f;
	this->nextLevelId = 0;
	this->currentLevelID = 0x10;
	this->currentElevatorID = -1;
	this->level_0x5b50 = 0x10;
	this->level_0x5b54 = -1;
	do {
		pLVar12->aCompanionInfo = 0;
		pLVar12->pSimpleConditionData = 0;
		iVar11 = iVar11 + 8;
		pLVar12[1].aCompanionInfo = 0;
		pLVar12[1].pSimpleConditionData = 0;
		pLVar12[2].aCompanionInfo = 0;
		pLVar12[2].pSimpleConditionData = 0;
		pLVar12[3].aCompanionInfo = 0;
		pLVar12[3].pSimpleConditionData = 0;
		pLVar12[4].aCompanionInfo = 0;
		pLVar12[4].pSimpleConditionData = 0;
		pLVar12[5].aCompanionInfo = 0;
		pLVar12[5].pSimpleConditionData = 0;
		pLVar12[6].aCompanionInfo = 0;
		pLVar12[6].pSimpleConditionData = 0;
		pLVar12[7].aCompanionInfo = 0;
		pLVar12[7].pSimpleConditionData = 0;
		pLVar12 = pLVar12 + 8;
	} while (iVar11 < 0x10);

	iVar11 = 0xf;
	piVar10 = INT_ARRAY_0048ed60 + 0xf;
	piVar9 = INT_ARRAY_0048eda0 + 0xf;
	do {
		*piVar10 = iVar11;
		*piVar9 = iVar11;
		piVar10[-1] = iVar11 + -1;
		piVar9[-1] = iVar11 + -1;
		piVar10[-2] = iVar11 + -2;
		piVar9[-2] = iVar11 + -2;
		iVar5 = iVar11 + -5;
		piVar10[-3] = iVar11 + -3;
		iVar4 = iVar11 + -6;
		piVar9[-3] = iVar11 + -3;
		piVar10[-4] = iVar11 + -4;
		iVar3 = iVar11 + -7;
		piVar9[-4] = iVar11 + -4;
		iVar11 = iVar11 + -8;
		piVar10[-5] = iVar5;
		piVar9[-5] = iVar5;
		piVar10[-6] = iVar4;
		piVar9[-6] = iVar4;
		piVar10[-7] = iVar3;
		piVar9[-7] = iVar3;
		piVar10 = piVar10 + -8;
		piVar9 = piVar9 + -8;
	} while (-1 < iVar11);
	INT_ARRAY_0048ed60[4] = 0;
	INT_ARRAY_0048eda0[0] = 4;
	INT_ARRAY_0048ed60[11] = 5;
	INT_ARRAY_0048eda0[5] = 0xb;
	INT_ARRAY_0048ed60[9] = 8;
	INT_ARRAY_0048eda0[8] = 9;
	/* Copy 'CDEURO/Level/' into load loop object after this ptr */
	INT_ARRAY_0048ed60[6] = 0;
	strcpy(this->levelPath, g_CD_LevelPath_00433bf8);
	/* Try read [Router] - SetPath from INI file */
	bVar2 = gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szSetPath_00433c10, this->levelPath);
	if (bVar2 == false) {
		/* No path found in .INI -> use default !\n */
		edDebugPrintf(g_szNoPathError_00433c20);
	}
	/* Try read [Router] - AddLevel from INI file  */
	local_80[0] = '\0';
	gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szAddLevel_00433c48, local_80);
	pLVar12 = this->aLevelInfo;
	iVar11 = 0;
	do {
		if (pLVar12->aCompanionInfo != (S_COMPANION_INFO*)0x0) {
			IMPLEMENTATION_GUARD();
			//FreeArray_00217340((undefined*)pLVar12->field_0x48, Destructor_002dd1c0);
			pLVar12->aCompanionInfo = 0;
		}
		if ((void*)pLVar12->pSimpleConditionData != (void*)0x0) {
			edMemFree((void*)pLVar12->pSimpleConditionData);
			pLVar12->pSimpleConditionData = 0;
		}
		iVar11 = iVar11 + 1;
		pLVar12 = pLVar12 + 1;
	} while (iVar11 < 0x10);

	MoreLoadLoopObjectSetup(true);
	Levels_LoadInfoBank();
	Episode_LoadFromIniFile();

	pLVar12 = this->aLevelInfo;
	iVar11 = 0;
	do {
		iVar3 = edStrICmp(local_80, pLVar12->levelName);

		if (iVar3 == 0) goto LAB_002e26c8;

		iVar11 = iVar11 + 1;
		pLVar12 = pLVar12 + 1;
	} while (iVar11 < 0x10);

	iVar11 = 0;
LAB_002e26c8:
	nextLevelId = iVar11;

	Levels_SaveDataToSavedGame();

	if (this->aLevelInfo[0xe].levelName[0] == '\0') {
		Level_FillRunInfo(nextLevelId, -1, -1);
	}
	else {
#ifdef PLATFORM_WIN
		if (gDebugLevelLoadOverride != -1) {
			Level_FillRunInfo(gDebugLevelLoadOverride, -1, -1);
		}
		else {
			Level_FillRunInfo(0xe, -1, -1);
		}
#else
		Level_FillRunInfo(0xe, -1, -1);
#endif
	}
	return;
}

void NativShopLevelSubObj::FUN_002d8d10(CActorNativShop* pNativ)
{
	CInventoryInfo* pInventoryInfo;
	int currentIndex;

	if (pNativ->field_0x17c == 0) {
		this->field_0x8 = 0;
	}
	else {
		this->field_0x8 = 1;
	}

	currentIndex = 0;
	do {
		pInventoryInfo = pNativ->GetInventoryInfoForPurchase(currentIndex);
		if (pInventoryInfo == (CInventoryInfo*)0x0) {
			this->aSubObjs[currentIndex].field_0x0 = 0x20;
			this->aSubObjs[currentIndex].field_0x4 = 0;
			this->aSubObjs[currentIndex].field_0x8 = 0;
		}
		else {
			this->aSubObjs[currentIndex].field_0x0 = pInventoryInfo->purchaseId;
			this->aSubObjs[currentIndex].field_0x4 = pInventoryInfo->moneyCost;
			this->aSubObjs[currentIndex].field_0x8 = pInventoryInfo->field_0x8;
		}

		currentIndex = currentIndex + 1;
	} while (currentIndex < 5);

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Game_Term()
{
	IMPLEMENTATION_GUARD();
}

bool BnkInstallScene(char* pFileData, int size)
{
	ByteCode byteCode;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallScene\n");

	/* Origin: 0040e860 */
	byteCode.Init(pFileData);
	byteCode.GetChunk();
	CScene::ptable.g_C3DFileManager_00451664->Level_AddAll(&byteCode);
	CScene::_pinstance->Level_Setup(&byteCode);
	CScene::ptable.g_WayPointManager_0045169c->Level_AddAll(&byteCode);
	CScene::ptable.g_PathManager_004516a0->Level_AddAll(&byteCode);
	CScene::ptable.g_CollisionManager_00451690->Level_AddAll(&byteCode);
	CScene::ptable.g_ActorManager_004516a4->Level_AddAll(&byteCode);
	CScene::ptable.g_SectorManager_00451670->Level_AddAll(&byteCode);
	if (CScene::ptable.g_GlobalDListManager_004516bc != (CGlobalDListManager*)0x0) {
		CScene::ptable.g_GlobalDListManager_004516bc->Level_Create();
	}
	byteCode.Term();
	return false;
}

bool BnkInstallSceneCfg(char* pFileData, int size)
{
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallSceneCfg\n");

	CScene* pScene;
	uint uVar2;
	float fVar3;
	ByteCode byteCode;
	byteCode.Init(pFileData);
	byteCode.GetChunk();
	pScene = CScene::_pinstance;
	fVar3 = byteCode.GetF32();
	pScene->field_0x1c = fVar3;
	uVar2 = byteCode.GetU32();
	pScene->field_0x20 = uVar2;
	fVar3 = byteCode.GetF32();
	pScene->field_0x24 = fVar3;
	byteCode.GetU32();
	byteCode.GetU32();
	CScene::ptable.g_C3DFileManager_00451664->Level_Create(&byteCode);
	CScene::ptable.g_CollisionManager_00451690->Level_Create(&byteCode);
	CScene::ptable.g_AnimManager_00451668->Level_Create(&byteCode);
	byteCode.GetU32();
	byteCode.GetU32();
	CScene::ptable.g_SectorManager_00451670->Level_Create(&byteCode);
	CScene::ptable.g_ActorManager_004516a4->Level_LoadClassesInfo(&byteCode);
	CScene::ptable.g_CollisionManager_00451690->Level_PostCreate();
	CFxParticleManager::Level_Create(&byteCode);

	return false;
}

bool BnkInstallSoundCfg(char* pFileData, int param_2)
{
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "MISSING HANDLER OnSoundLoaded_00180ef0\n");
	return false;
}

bool BnkInstallG2D(char* pFileData, int length)
{
	C3DFileManager* pFVar1;
	int iStack4;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallG2D: {}\n", ObjectNaming::CopyObjectName());

	pFVar1 = CScene::ptable.g_C3DFileManager_00451664;
	ed3DInstallG2D(pFileData, length, &iStack4, &pFVar1->aCommonLevelTextures[pFVar1->textureLoadedCount].manager, 1);
	pFVar1->aCommonLevelTextures[pFVar1->textureLoadedCount].pFileBuffer = pFileData;
	pFVar1->textureLoadedCount = pFVar1->textureLoadedCount + 1;
	return false;
}

bool BnkInstallG3D(char* pFileData, int length)
{
	int newMeshIndex;
	Mesh* aMeshes;
	C3DFileManager* pFileManager;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallG3D: {}\n", ObjectNaming::CopyObjectName());

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	newMeshIndex = pFileManager->meshLoadedCount;

	aMeshes = pFileManager->aCommonLevelMeshes;
	aMeshes[newMeshIndex].fileLength = length;
	aMeshes[newMeshIndex].pFileData = pFileData;

#ifdef PLATFORM_WIN
	aMeshes[newMeshIndex].name = ObjectNaming::CopyObjectName();
#endif

	pFileManager->meshLoadedCount = pFileManager->meshLoadedCount + 1;
	return false;
}

bool BnkInstallCol(char* pFileBuffer, int length)
{
	CCollisionManager* pCollisionManager;
	edColG3D_OBB_TREE* pObbTree;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallCol\n");

	pCollisionManager = CScene::ptable.g_CollisionManager_00451690;
	pObbTree = edColLoadStatic(pFileBuffer, length, 0);
	pCollisionManager->aStaticCollisionRefs[pCollisionManager->staticCollisionCount] = (edObbTREE_DYN*)LOAD_POINTER(pObbTree->pObbTree);
	pCollisionManager->staticCollisionCount = pCollisionManager->staticCollisionCount + 1;
	return false;
}

bool BnkInstallEvents(char* pFileData, int length)
{
	CEventManager* pCVar1;
	uint* pFileData_00;
	uint uVar2;
	undefined4 uVar3;
	ByteCode BStack16;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallEvents\n");

	BStack16.Init(pFileData);
	BStack16.GetChunk();
	pCVar1 = CScene::ptable.g_EventManager_006f5080;
	pFileData_00 = (uint*)BStack16.GetPosition();
	uVar2 = edEventAddChunk(pFileData_00, 0);
	pCVar1->activeChunkId = uVar2;
	uVar3 = edEventGetChunkNbEvents(pCVar1->activeChunkId);
	pCVar1->nbEvents = uVar3;
	//edEventRegisterDrawingFunctions((int*)event_draw_func);
	BStack16.Term();
	return false;
}

bool BnkInstallCameras(char* pFileData, int length)
{
	ByteCode MStack16;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallCameras\n");

	MStack16.Init(pFileData);
	MStack16.GetChunk();

	CCameraManager::_gThis->Level_AddAll(&MStack16);

	MStack16.Term();
	return false;
}

bool BnkInstallLights(char* pFileData, int length)
{
	ByteCode MStack16;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallLights\n");

	MStack16.Init(pFileData);
	MStack16.GetChunk();
	CScene::ptable.g_LightManager_004516b0->Level_AddAll(&MStack16);
	MStack16.Term();
	return false;
}

bool BnkInstallFxCfg(char* pFileData, int length)
{
	ByteCode MStack16;
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallFxCfg\n");

	MStack16.Init(pFileData);
	CScene::ptable.g_EffectsManager_004516b8->Level_AddAll(&MStack16);
	MStack16.Term();
	return false;
}

bool BnkInstallCinematic(char* pFileData, int length)
{
	ByteCode MStack16;
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallCinematic\n");

	MStack16.Init(pFileData);
	g_CinematicManager_0048efc->Level_AddAll(&MStack16);
	MStack16.Term();
	return false;
}

bool BnkInstallAnim(char* pFileData, int length)
{
	CAnimationManager* pCVar1;
	edANM_HDR* peVar2;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallAnim\n");

	pCVar1 = CScene::ptable.g_AnimManager_00451668;
	peVar2 = edAnmAnim::LoadFromMem(pFileData, length);
	pCVar1->pAnimKeyTable[pCVar1->loadedAnimKeyDataCount] = peVar2;
	pCVar1->loadedAnimKeyDataCount = pCVar1->loadedAnimKeyDataCount + 1;
	return false;
}

bool BnkInstallSample(char* pFileData, int length)
{
	CAudioManager* pGVar1;
	uint uVar2;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallSample\n");

	IMPLEMENTATION_GUARD_AUDIO(

	pGVar1 = CScene::ptable.g_AudioManager_00451698;
	if (DAT_00448ef0 == 0) {
		uVar2 = length - 0x30;
		if ((uVar2 & 0x3f) != 0) {
			uVar2 = (uVar2 & 0xffffffc0) + 0x40;
		}
		if (0x1ee3c0 - (CScene::ptable.g_AudioManager_00451698)->field_0xb4 < (int)uVar2) {
			*(undefined4*)
				((CScene::ptable.g_AudioManager_00451698)->field_0x14 +
					(CScene::ptable.g_AudioManager_00451698)->field_0xc * 0x18) = 0;
		}
		else {
			(CScene::ptable.g_AudioManager_00451698)->field_0xb4 =
				(CScene::ptable.g_AudioManager_00451698)->field_0xb4 + uVar2;
			edSoundSampleLoadNoWait(pFileData, (ed_sound_sample*)(pGVar1->field_0x14 + pGVar1->field_0xc * 0x18), 0);
		}
	}
	pGVar1->field_0xc = pGVar1->field_0xc + 1;
	if (pGVar1->field_0xc == pGVar1->field_0x8) {
		pGVar1->field_0xc = 0;
	})
	return false;
}

bool BnkInstallBank(char* pFileData, int length)
{
	CAudioManager* pGVar1;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallBank\n");

	IMPLEMENTATION_GUARD_AUDIO(

	pGVar1 = CScene::ptable.g_AudioManager_00451698;
	if (DAT_00448ef0 == 0) {
		*(char**)(*(int*)&(CScene::ptable.g_AudioManager_00451698)->instanceIndex +
			(CScene::ptable.g_AudioManager_00451698)->field_0xc * 8) = pFileData;
		*(int*)(pGVar1->field_0xc * 8 + *(int*)&pGVar1->instanceIndex + 4) = length;
	}
	pGVar1->field_0xc = pGVar1->field_0xc + 1;
	if (pGVar1->field_0xc == pGVar1->field_0x48) {
		pGVar1->field_0xc = 0;
	})
	return false;
}

bool BnkInstallBankHeader(char* pFileData, int length)
{
	CAudioManager* pGVar1;
	undefined4* puVar2;
	uint uVar3;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallBank\n");

	IMPLEMENTATION_GUARD_AUDIO(

	pGVar1 = CScene::ptable.g_AudioManager_00451698;
	if (DAT_00448ef0 == 0) {
		puVar2 = (undefined4*)
			(*(int*)&(CScene::ptable.g_AudioManager_00451698)->instanceIndex +
				(CScene::ptable.g_AudioManager_00451698)->field_0xc * 8);
		uVar3 = FUN_00267e20(*puVar2, pFileData, (long)(int)puVar2[1], (long)length);
		*(uint*)(pGVar1->field_0x50 + pGVar1->field_0xc * 4) = uVar3;
		*(undefined4*)&pGVar1->field_0xb8 = *(undefined4*)(pGVar1->field_0xc * 8 + *(int*)&pGVar1->instanceIndex + 4);
		if ((*(uint*)&pGVar1->field_0xb8 & 0x3f) != 0) {
			*(uint*)&pGVar1->field_0xb8 = (*(uint*)&pGVar1->field_0xb8 & 0xffffffc0) + 0x40;
		}
		pGVar1->field_0xb4 = pGVar1->field_0xb4 + *(int*)&pGVar1->field_0xb8;
	}
	pGVar1->field_0xc = pGVar1->field_0xc + 1;
	if (pGVar1->field_0xc == pGVar1->field_0x48) {
		pGVar1->field_0xc = 0;
		edMemFree(*(void**)&pGVar1->instanceIndex);
	})
	return false;
}

bool BnkInstallSong(char* pFileData, int length)
{
	CAudioManager* pGVar1;
	uint uVar2;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallBank\n");

	IMPLEMENTATION_GUARD_AUDIO(

	pGVar1 = CScene::ptable.g_AudioManager_00451698;
	if (DAT_00448ef0 == 0) {
		uVar2 = FUN_00267d30(pFileData, (long)length);
		*(uint*)(pGVar1->field_0x4c + pGVar1->field_0xc * 4) = uVar2;
	}
	pGVar1->field_0xc = pGVar1->field_0xc + 1;
	if (pGVar1->field_0xc == pGVar1->field_0x44) {
		pGVar1->field_0xc = 0;
	})
	return false;
}

bool BnkInstallDynCol(char* pFileData, int length)
{
	int nextIndex;
	BankCollision_14* pBankCollision;

	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallDynCol\n");

	nextIndex = (CScene::ptable.g_CollisionManager_00451690)->loadedBankCount_0x8;
	(CScene::ptable.g_CollisionManager_00451690)->loadedBankCount_0x8 = nextIndex + 1;
	pBankCollision = &CScene::ptable.g_CollisionManager_00451690->pBankCollisionData[nextIndex];

	pBankCollision->pBase = (DynColEntry*)pFileData;
	pBankCollision->dynColSize = length;
	pBankCollision->useCount_0xc = 0;
	pBankCollision->pNextFree = (DynColEntry*)0x0;
	return false;
}

bool BnkInstallParticleManager(char* pFileData, int length)
{
	return false;
}

bool BnkInstallAnimMacro(char* pFileData, int length)
{
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallAnimMacro\n");

	if (*(int*)pFileData != 0) {
		(CScene::ptable.g_AnimManager_00451668)->pAnimKeyEntryData = pFileData + 4;
	}
	return false;
}

bool BnkInstallAstar(char* pFileData, int length)
{
	ByteCode byteCode;
	byteCode.Init(pFileData);
	byteCode.GetChunk();
	CScene::ptable.g_PathManager_004516a0->pBasicPathFinder->Create(&byteCode);
	byteCode.Term();

	return false;
}

bool BnkInstallLiptracks(char* pFileData, int length)
{
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "BnkInstallLiptracks\n");
	return false;
}

edCBankCallback TableBankCallback[24] = {
	{ 0x02,						0x1, { BnkInstallAnim, 0, 0, 0, 0, 0 } },
	{ 0x02,						0x2, { BnkInstallAnimMacro, 0, 0, 0, 0, 0 } },
	{ 0x03,						0x4, { BnkInstallSoundCfg, 0, 0, 0, 0, 0 } },
	{ 0x03,						0x1, { BnkInstallSample, 0, 0, 0, 0, 0 } },
	{ 0x03,						0x5, { BnkInstallSong, 0, 0, 0, 0, 0 } },
	{ 0x03,						0x6, { BnkInstallBank, 0, 0, 0, 0, 0 } },
	{ 0x03,						0x7, { BnkInstallBankHeader, 0, 0, 0, 0, 0 } },
	{ 0x06,						0x1, { BnkInstallScene, 0, 0, 0, 0, 0 } },
	{ 0x06,						0x2, { BnkInstallSceneCfg, 0, 0, 0, 0, 0 } },
	{ BANK_CALLBACK_MESH,		0x1, { BnkInstallG3D, 0, 0, 0, 0, 0 } },
	{ BANK_CALLBACK_TEXTURE,	0x1, { BnkInstallG2D, 0, 0, 0, 0, 0 } },
	{ 0x07,						0x1, { BnkInstallCol, 0, 0, 0, 0, 0 } },
	{ 0x07,						0x2, { BnkInstallDynCol, 0, 0, 0, 0, 0 } },
	{ 0x08,						0x1, { BnkInstallEvents, 0, 0, 0, 0, 0 } },
	{ 0x0A,						0x1, { BnkInstallTrack, 0, 0, 0, 0, 0 } },
	{ 0x0B,						0x1, { BnkInstallCameras, 0, 0, 0, 0, 0 } },
	{ 0x0C,						0x1, { NULL, 0, 0, 0, 0, 0 } },
	{ 0x0F,						0x1, { BnkInstallAstar, 0, 0, 0, 0, 0 } },
	{ 0x10,						0x1, { BnkInstallLights, 0, 0, 0, 0, 0 } },
	{ 0x11,						0x1, { BnkInstallLiptracks, 0, 0, 0, 0, 0 } },
	{ 0x09,						0x1, { BnkInstallFxCfg, 0, 0, 0, 0, 0 } },
	{ 0x13,						0x1, { BnkInstallCinematic, 0, 0, 0, 0, 0 } },
	{ 0x09,						0x2, { BnkInstallParticleManager, 0, 0, 0, 0, 0 } },
	{ 0xFFFFFFFF,				0xFFFFFFFF, { NULL, 0, 0, 0, 0, 0 } },
};

void WillLoadFileFromBank(struct CAudioManager* param_1, edCBankBufferEntry* pBankBuffer)
{
	bool bVar1;
	uint uVar2;
	int iVar3;
	undefined* puVar4;
	char* pcVar5;
	int iVar6;
	int iVar7;
	uint uVar8;
	edBANK_ENTRY_INFO local_220;
	char acStack512[512];

	uVar2 = pBankBuffer->get_element_count();
	iVar6 = 0;
	iVar7 = 0;
	uVar8 = 0;
	if (uVar2 != 0) {
		do {
			bVar1 = pBankBuffer->get_info(uVar8, &local_220, acStack512);
			if (bVar1 == false) break;
			if ((local_220.type << 0x10 | local_220.stype) == 0x30001) {
				iVar3 = edStrLength(acStack512);
				iVar7 = iVar7 + 1;
				iVar6 = iVar6 + iVar3 + 1;
			}
			uVar8 = uVar8 + 1;
		} while (uVar8 < uVar2);
	}
	//if (iVar7 == 0) {
	//	param_1->field_0x18 = (undefined*)0x0;
	//}
	//else {
	//	puVar4 = (undefined*)edMemAlloc(H_MAIN, iVar7 << 2);
	//	param_1->field_0x18 = puVar4;
	//}
	//if (iVar6 == 0) {
	//	param_1->pString_0x1c = (char*)0x0;
	//}
	//else {
	//	pcVar5 = (char*)edMemAlloc(H_MAIN, iVar6);
	//	param_1->pString_0x1c = pcVar5;
	//}
	//pcVar5 = param_1->pString_0x1c;
	//if ((pcVar5 != (char*)0x0) && (uVar8 = 0, uVar2 != 0)) {
	//	iVar6 = 0;
	//	do {
	//		bVar1 = GetFileDataForIndex(pBankBuffer, uVar8, &local_220, acStack512);
	//		if (bVar1 == false) {
	//			return;
	//		}
	//		if ((local_220.unknownA << 0x10 | local_220.unknownB) == 0x30001) {
	//			edStrCopy(pcVar5, acStack512);
	//			*(char**)(param_1->field_0x18 + iVar6) = pcVar5;
	//			iVar6 = iVar6 + 4;
	//			iVar7 = edStrLength(pcVar5);
	//			pcVar5 = pcVar5 + iVar7 + 1;
	//		}
	//		uVar8 = uVar8 + 1;
	//	} while (uVar8 < uVar2);
	//}
	return;
}

void WillLoadFilefromBank(bool param_1, void* pObj)
{
	CLevelScheduler* pLVar1;

	pLVar1 = CLevelScheduler::gThis;
	if (param_1 != false) {
		WillLoadFileFromBank(CScene::ptable.g_AudioManager_00451698, CLevelScheduler::gThis->pIOPBankBufferEntry);
		pLVar1->loadStage_0x5b48 = 1;
	}
	return;
}

const char* sz_bankSlash = "/";
const char* sz_LevelBank_00433bd8 = "Level.bnk";
const char* sz_LevelIOPBankName = "LevelIOP.bnk";

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::LevelLoading_Begin()
{
	int levelId;
	char filePath[128];
	edCBankInstall bankContainer;

	/* This is part of the level load process
	   It loads in the level IOP bank for the specific level found in the level load
	   master.
	   Example: CDEURO/LEVEL/PREINTRO/LevelIOP.bnk */

#ifdef PLATFORM_WIN
	if (this->nextLevelID == 0x10) {
		// Trying to load an invalid level. We probably termed the level via the debug menu. Wait for the debug menu to change our next level ID.
		while (this->nextLevelID == 0x10) {
			Renderer::WaitUntilReady();
			Renderer::Present();
		}
	}
#endif

	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	levelId = this->nextLevelID;
	memset(&bankContainer, 0, sizeof(edCBankInstall));
	levelIOPBank.initialize(aLevelInfo[levelId].bankSizeIOP + 0x1000, 1, &bankContainer);
	levelIOPBank.bank_buffer_setcb(TableBankCallback);
	/* / + LevelIOP.bnk */
	edStrCatMulti(filePath, levelPath, aLevelInfo[levelId].levelName, sz_bankSlash, sz_LevelIOPBankName, 0);
	bankContainer.filePath = filePath;
	bankContainer.pObjectReference = (void*)0x0;
	bankContainer.fileFlagA = 4;
	bankContainer.fileFunc = WillLoadFilefromBank;
	pIOPBankBufferEntry = levelIOPBank.get_free_entry();
	loadStage_0x5b48 = 0;
	pIOPBankBufferEntry->load(&bankContainer);
	edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
}

uint _edSysTransferIndex = 0;
uint _edSoundLastTransferIndex = 0;
uint _edMusicLastTransferIndex = 0;
uint _edSysCompletedTransferIndex = 0;

bool _edSoundAreAllSoundDataLoaded(uint lastIndex)
{
	bool bVar1;

	bVar1 = true;
	if ((_edSysTransferIndex != 0) && ((lastIndex == 0 || (bVar1 = false, lastIndex <= _edSysCompletedTransferIndex)))) {
		bVar1 = true;
	}
	return bVar1;
}

// Should be in: D:/Projects/EdenLib/edSound/sources/edSoundPlay.cpp
bool edSoundAreAllSoundDataLoaded()
{
	return _edSoundAreAllSoundDataLoaded(_edSoundLastTransferIndex);
}

// Should be in: D:/projects/EdenLib/edMusic/sources/edMusicPlay.cpp
bool edMusicAreAllMusicDataLoaded()
{
	return _edSoundAreAllSoundDataLoaded(_edMusicLastTransferIndex);
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
bool CLevelScheduler::LevelLoading_Manage()
{
	bool bVar1;
	char filePath[128];
	edCBankInstall bankFilePathContainer;
	int levelToLoadID;
	int loadStage;

	/* This loads the level bank into memory, based on which level you want to load.
	   Example: CDEURO/LEVEL/PREINTRO/Level.bnk */
	loadStage = this->loadStage_0x5b48;
	if (loadStage != 4) {
		if (loadStage == 3) {
			bVar1 = this->pLevelBankBufferEntry->is_loaded();
			if (bVar1 != false) {
				this->loadStage_0x5b48 = 4;
			}
		}
		else {
			if (loadStage == 2) {
				bVar1 = edSoundAreAllSoundDataLoaded();
				if ((bVar1 != false) && (bVar1 = edMusicAreAllMusicDataLoaded(), bVar1 != false)) {
					this->pIOPBankBufferEntry->close();
					this->pIOPBankBufferEntry = (edCBankBufferEntry*)0x0;
					this->levelIOPBank.terminate();
					this->loadStage_0x5b48 = 3;
				}
			}
			else {
				if (loadStage == 1) {
					levelToLoadID = this->nextLevelID;
					memset(&bankFilePathContainer, 0, sizeof(edCBankInstall));
					this->levelBank.initialize(this->aLevelInfo[levelToLoadID].bankSizeLevel + 0x1000, 1, &bankFilePathContainer);
					this->levelBank.bank_buffer_setcb(TableBankCallback);
					/* / + level.bnk */
					edStrCatMulti(filePath, this->levelPath, this->aLevelInfo[levelToLoadID].levelName, sz_bankSlash, sz_LevelBank_00433bd8, 0);
					bankFilePathContainer.filePath = filePath;
					bankFilePathContainer.fileFlagA = 0xc;
					this->pLevelBankBufferEntry = this->levelBank.get_free_entry();
					this->pLevelBankBufferEntry->load(&bankFilePathContainer);
					this->loadStage_0x5b48 = 2;
				}
			}
		}
	}
	return this->loadStage_0x5b48 != 4;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_Install()
{
	edCBankInstall SStack32;

	memset(&SStack32, 0, sizeof(edCBankInstall));
	SStack32.fileFlagA = 0;
	this->pLevelBankBufferEntry->install(&SStack32);
	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_Init()
{
	uint** ppuVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	//sceVu0FMATRIX* m2;
	ulong uVar5;
	int iVar6;
	int* piVar7;
	LoadLoopObject_50* pLVar8;
	uint uVar9;
	int* piVar10;
	float afStack144[16];
	undefined auStack80[48];
	float local_20;
	float local_1c;
	float local_18;
	float local_14;
	uint uStack4;
	int episodeNumber;

	//piVar7 = (int*)this->field_0x5b58;
	//if (piVar7 != (int*)0x0) {
	//	piVar10 = piVar7 + 1;
	//	uVar9 = 0;
	//	if (*piVar7 != 0) {
	//		do {
	//			episodeNumber = 0;
	//			if (0 < piVar10[7]) {
	//				piVar7 = piVar10 + 7;
	//				do {
	//					FUN_00119a40(piVar7 + 3);
	//					FUN_00119940(piVar7 + 4);
	//					episodeNumber = episodeNumber + 1;
	//					piVar7 = piVar7 + 4;
	//				} while (episodeNumber < piVar10[7]);
	//			}
	//			if (*piVar10 != 0x62) {
	//				episodeNumber = 0;
	//				pLVar8 = LevelScheduleManager::gThis->field_0x4220;
	//				if (0 < LevelScheduleManager::gThis->objCount_0x4218) {
	//					do {
	//						if (pLVar8->field_0x0 == *piVar10) goto LAB_002dd540;
	//						episodeNumber = episodeNumber + 1;
	//						pLVar8 = pLVar8 + 1;
	//					} while (episodeNumber < LevelScheduleManager::gThis->objCount_0x4218);
	//				}
	//				pLVar8 = (LoadLoopObject_50*)0x0;
	//			LAB_002dd540:
	//				if (pLVar8 != (LoadLoopObject_50*)0x0) {
	//					pLVar8->field_0x24 = piVar10[5];
	//					pLVar8->field_0x28 = piVar10[6];
	//					if ((int)pLVar8->field_0x2c < piVar10[7]) {
	//						piVar7 = piVar10 + pLVar8->field_0x2c * 4;
	//						if (piVar7 + 8 == (int*)0x0) {
	//							pLVar8->field_0x40 = 0;
	//							fVar4 = gF32Vertex4Zero.w;
	//							fVar3 = gF32Vertex4Zero.z;
	//							fVar2 = gF32Vertex4Zero.y;
	//							pLVar8->field_0x30 = gF32Vertex4Zero.x;
	//							pLVar8->field_0x34 = fVar2;
	//							pLVar8->field_0x38 = fVar3;
	//							pLVar8->field_0x3c = fVar4;
	//						}
	//						else {
	//							uVar5 = FUN_00189a30(piVar7[8], piVar7[9]);
	//							pLVar8->field_0x40 = uVar5;
	//							fVar4 = gF32Vertex4Zero.w;
	//							fVar3 = gF32Vertex4Zero.z;
	//							fVar2 = gF32Vertex4Zero.y;
	//							ppuVar1 = (uint**)piVar7[0xb];
	//							if (ppuVar1 == (uint**)0x0) {
	//								pLVar8->field_0x30 = gF32Vertex4Zero.x;
	//								pLVar8->field_0x34 = fVar2;
	//								pLVar8->field_0x38 = fVar3;
	//								pLVar8->field_0x3c = fVar4;
	//							}
	//							else {
	//								m2 = (sceVu0FMATRIX*)
	//									FUN_002596a0((Scene::ptable.g_EventManager_006f5080)->activeEventChunkID_0x8,
	//										ppuVar1, 0, &uStack4);
	//								if ((sceVu0FMATRIX*)ppuVar1[8] != (sceVu0FMATRIX*)0x0) {
	//									sceVu0MulMatrix((sceVu0FMATRIX*)afStack144, (sceVu0FMATRIX*)ppuVar1[8], m2);
	//									FUN_00264910((int)auStack80, (int)afStack144);
	//									m2 = (sceVu0FMATRIX*)afStack144;
	//								}
	//								FUN_00264910((int)auStack80, (int)m2);
	//								pLVar8->field_0x30 = local_20;
	//								pLVar8->field_0x34 = local_1c;
	//								pLVar8->field_0x38 = local_18;
	//								pLVar8->field_0x3c = local_14;
	//								pLVar8->field_0x3c = 1.0;
	//							}
	//						}
	//					}
	//					else {
	//						pLVar8->field_0x40 = 0;
	//						fVar4 = gF32Vertex4Zero.w;
	//						fVar3 = gF32Vertex4Zero.z;
	//						fVar2 = gF32Vertex4Zero.y;
	//						pLVar8->field_0x30 = gF32Vertex4Zero.x;
	//						pLVar8->field_0x34 = fVar2;
	//						pLVar8->field_0x38 = fVar3;
	//						pLVar8->field_0x3c = fVar4;
	//					}
	//				}
	//			}
	//			uVar9 = uVar9 + 1;
	//			piVar10 = piVar10 + piVar10[7] * 4 + 8;
	//		} while (uVar9 < *(uint*)this->field_0x5b58);
	//	}
	//}
	//piVar7 = (int*)this->field_0x5b5c;
	//episodeNumber = 0;
	//if (piVar7 != (int*)0x0) {
	//	episodeNumber = *piVar7;
	//}
	//if (episodeNumber != 0) {
	//	piVar10 = piVar7 + 1;
	//	for (episodeNumber = *piVar7; episodeNumber != 0; episodeNumber = episodeNumber + -1) {
	//		FUN_00119940(piVar10 + 1);
	//		piVar10 = piVar10 + 2;
	//	}
	//}
	//if ((g_Count_00425404 < 0) || (episodeNumber = g_Count_00425404, _gGameNfo.count <= g_Count_00425404)) {
	//	episodeNumber = _gGameNfo.count + -1;
	//}
	//episodeNumber = _gGameNfo.monster - g_EpisodeDataArray_0048eb0[episodeNumber].monster;
	//if (episodeNumber < 1) {
	//	if (episodeNumber < 0) {
	//		iVar6 = _gGameNfo.field_0x1c;
	//		if (-episodeNumber <= _gGameNfo.field_0x1c) {
	//			iVar6 = -episodeNumber;
	//		}
	//		if (0 < _gGameNfo.field_0x1c) {
	//			_gGameNfo.field_0x1c = _gGameNfo.field_0x1c - iVar6;
	//			_gGameNfo.monster = _gGameNfo.monster + iVar6;
	//		}
	//	}
	//}
	//else {
	//	_gGameNfo.monster = _gGameNfo.monster - episodeNumber;
	//	_gGameNfo.field_0x1c = _gGameNfo.field_0x1c + episodeNumber;
	//}
	//if ((g_Count_00425404 < 0) || (episodeNumber = g_Count_00425404, _gGameNfo.count <= g_Count_00425404)) {
	//	episodeNumber = _gGameNfo.count + -1;
	//}
	//episodeNumber = _gGameNfo.bet - g_EpisodeDataArray_0048eb0[episodeNumber].bet;
	//if (episodeNumber < 1) {
	//	if (episodeNumber < 0) {
	//		iVar6 = _gGameNfo.field_0x1c;
	//		if (-episodeNumber <= _gGameNfo.field_0x1c) {
	//			iVar6 = -episodeNumber;
	//		}
	//		if (0 < _gGameNfo.field_0x1c) {
	//			_gGameNfo.field_0x1c = _gGameNfo.field_0x1c - iVar6;
	//			_gGameNfo.bet = _gGameNfo.bet + iVar6;
	//		}
	//	}
	//}
	//else {
	//	_gGameNfo.bet = _gGameNfo.bet - episodeNumber;
	//	_gGameNfo.field_0x1c = _gGameNfo.field_0x1c + episodeNumber;
	//}

	if (this->currentLevelID == 0xe) {
		GameFlags = GameFlags | 0x40;
	}

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.h
void CLevelScheduler::Level_Term()
{
	if (this->currentLevelID == 0xe) {
		GameFlags = GameFlags & 0xffffffbf;
	}

	return;
}

void CLevelScheduler::Level_AddAll(struct ByteCode* pByteCode)
{
	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_ClearAll()
{
	int iVar4;
	CChunk* pBSAV;
	int iVar10;
	S_LEVEL_INFO* pLevelInfo;
	char local_80[128];

	this->pLevelBankBufferEntry->close();
	this->pLevelBankBufferEntry = (edCBankBufferEntry*)0x0;

	this->levelBank.terminate();

	if ((this->field_0x80 | this->bShouldLoad | this->bShouldSave) == 0) {
		_gScenVarInfo[1].currentValue = this->currentLevelID;
	}

	this->currentLevelID = 0x10;
	this->autoSaveTriggerTime = 0.0f;
	this->curAutoSaveTime = 0.0f;
	this->pObjectiveStreamBegin = (int*)0x0;
	this->pObjectiveStreamEnd = (int*)0x0;

	if (this->bShouldLoad != 0) {
		MoreLoadLoopObjectSetup(false);
		Levels_UpdateDataFromSavedGame();
	}

	if (this->bShouldSave != 0) {
		MoreLoadLoopObjectSetup(false);
		Episode_LoadFromIniFile();
		Levels_SaveDataToSavedGame();
	}

	if (this->field_0x80 != 0) {
		edStrCopy(this->levelPath, g_CD_LevelPath_00433bf8);
		/* Router - Set Path */
		if (gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szSetPath_00433c10, this->levelPath) == false) {
			/* No path found in .INI -> use default !\n */
			edDebugPrintf(g_szNoPathError_00433c20);
		}

		/* Router - Add Level */
		local_80[0] = 0;
		gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szAddLevel_00433c48, local_80);
		pLevelInfo = this->aLevelInfo;
		iVar10 = 0;
		do {
			if (pLevelInfo->aCompanionInfo != (S_COMPANION_INFO*)0x0) {
				delete[] pLevelInfo->aCompanionInfo;
				pLevelInfo->aCompanionInfo = (S_COMPANION_INFO*)0x0;
			}
			if (pLevelInfo->pSimpleConditionData != (void*)0x0) {
				edMemFree(pLevelInfo->pSimpleConditionData);
				pLevelInfo->pSimpleConditionData = 0;
			}

			iVar10 = iVar10 + 1;
			pLevelInfo = pLevelInfo + 1;
		} while (iVar10 < 0x10);

		MoreLoadLoopObjectSetup(true);
		Levels_LoadInfoBank();
		Episode_LoadFromIniFile();

		pLevelInfo = this->aLevelInfo;
		iVar10 = 0;
		do {
			iVar4 = edStrICmp(local_80, pLevelInfo->levelName);
			if (iVar4 == 0) goto LAB_002dce40;
			iVar10 = iVar10 + 1;
			pLevelInfo = pLevelInfo + 1;
		} while (iVar10 < 0x10);
		iVar10 = 0;
	LAB_002dce40:
		this->nextLevelId = iVar10;

		Levels_SaveDataToSavedGame();
	}

	Episode_ComputeCurrent();

	return;
}

#ifdef PLATFORM_WIN
QueuedTaskList gQueuedLevelManageTasks;

void EnqueueLevelManageTask(std::function<void()> task)
{
	gQueuedLevelManageTasks.emplace_back(std::async(std::launch::deferred, task));
}
#endif

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_Manage()
{
	int* piVar1;
	bool bVar2;
	CActorHero* pCVar3;
	Timer* pTVar4;
	CLifeInterface* pCVar5;
	StateConfig* pSVar6;
	ulong uVar7;
	uint uVar8;
	uint* puVar9;
	CActorsTable* pCVar10;
	uint** ppuVar11;
	int iVar12;
	int iVar13;
	float fVar14;
	float fVar15;
	CActorsTable local_110;

#ifdef PLATFORM_WIN
	for (auto& task : gQueuedLevelManageTasks) {
		task.wait();
	}
	gQueuedLevelManageTasks.clear();
#endif

	pTVar4 = Timer::GetTimer();
	fVar15 = pTVar4->cutsceneDeltaTime;
	this->gameTime = this->gameTime + pTVar4->lastFrameTime;

	IMPLEMENTATION_GUARD_OBJECTIVE(
	piVar1 = this->pObjectiveStreamBegin;
	if (piVar1 != (int*)0x0) {
		ppuVar11 = (uint**)(piVar1 + 1);
		uVar8 = 0;
		if (*piVar1 != 0) {
			do {
				FUN_002d8970(ppuVar11);
				uVar8 = uVar8 + 1;
				ppuVar11 = ppuVar11 + (int)ppuVar11[7] * 4 + 8;
			} while (uVar8 < (uint)*this->pObjectiveStreamBegin);
		}
	}

	piVar1 = this->pObjectiveStreamEnd;
	iVar12 = 0;
	if (piVar1 != (int*)0x0) {
		iVar12 = *piVar1;
	}

	if (iVar12 != 0) {
		puVar9 = (uint*)(piVar1 + 1);
		for (iVar12 = *piVar1; iVar12 != 0; iVar12 = iVar12 + -1) {
			if ((((*puVar9 != 0x20) && ((ed_zone_3d*)puVar9[1] != (ed_zone_3d*)0x0)) &&
				(CActorHero::_gThis != (CActorHero*)0x0)) &&
				(uVar8 = edEventComputeZoneAgainstVertex
				((CScene::ptable.g_EventManager_006f5080)->activeChunkId, (ed_zone_3d*)puVar9[1],
					&CActorHero::_gThis->currentLocation, 0),
					(uVar8 & 1) != 0)) {
				gThis->field_0x5b30 = gThis->field_0x5b30 | 1 << (*puVar9 & 0x1f);
			}
			puVar9 = puVar9 + 2;
		}
	})

	if (this->currentLevelID == 0) {
		local_110.nbEntries = 0;
		CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(NATIV_SHOP, &local_110);

		this->nbNativShopSubObjs = local_110.nbEntries;
		if (10 < this->nbNativShopSubObjs) {
			this->nbNativShopSubObjs = 10;
		}

		NativShopLevelSubObj* pSubObj = this->aNativShopSubObjs;
		iVar12 = 0;
		if (0 < this->nbNativShopSubObjs) {
			CActor** pActor = local_110.aEntries;
			do {
				pSubObj->FUN_002d8d10(static_cast<CActorNativShop*>(*pActor));
				iVar12 = iVar12 + 1;
				pActor = pActor + 1;
				pSubObj = pSubObj + 1;
			} while (iVar12 < this->nbNativShopSubObjs);
		}
	}

	if (this->currentElevatorID != -1) {
		FUN_002dc200(this->currentElevatorID, this->level_0x5b50, this->level_0x5b54);
		this->currentElevatorID = -1;
		this->level_0x5b50 = 0x10;
		this->level_0x5b54 = -1;
	}

	if (this->autoSaveTriggerTime <= 0.0f) {
		return;
	}

	fVar14 = this->autoSaveTriggerTime - fVar15;
	this->autoSaveTriggerTime = fVar14;
	if (0.0 < fVar14) {
		return;
	}

	if (((GameFlags & 0x1c) != 0) || (g_CinematicManager_0048efc->FUN_001c5c60() != false)) {
		this->autoSaveTriggerTime = this->autoSaveTriggerTime + fVar15;
		return;
	}

	IMPLEMENTATION_GUARD_LOG(
	if (((gSaveManagement.field_0x0 != 0) && (gSaveManagement.slotID_0x28 != -1)) &&
		((uVar7 = FUN_002f39c0((int)&gSaveManagement), uVar7 == 0 &&
			(uVar7 = FUN_001b92f0(CScene::_pinstance), pCVar3 = CActorHero::_gThis, uVar7 == 0)))) {
		if (CActorHero::_gThis != (CActorHero*)0x0) {
			pCVar5 = (*(CActorHero::_gThis->pVTable)->GetLifeInterface)
				((CActor*)CActorHero::_gThis);
			fVar15 = (float)(**(code**)((int)pCVar5->pVtable + 0x24))(pCVar5);
			bVar2 = fVar15 - (pCVar3->character).characterBase.field_0x2e4 <= 0.0;
			if (!bVar2) {
				iVar12 = (pCVar3->character).characterBase.base.base.actorState;
				if (iVar12 == -1) {
					uVar8 = 0;
				}
				else {
					pSVar6 = (*((pCVar3->character).characterBase.base.base.pVTable)->GetStateCfg)((CActor*)pCVar3, iVar12);
					uVar8 = pSVar6->flags_0x4 & 1;
				}
				bVar2 = uVar8 != 0;
			}
			if (bVar2) goto LAB_002dc8d8;
		}
		SaveManagement_MemCardAutoSave();
		this->curAutoSaveTime = pTVar4->scaledTotalTime;
	})

LAB_002dc8d8:
	this->autoSaveTriggerTime = 0.0f;
	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_ManagePaused()
{
	if ((GameFlags & 0x1c) == 0) {
		Level_Manage();
	}

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.h
void CLevelScheduler::Level_Draw()
{
	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::LevelLoading_End()
{
	this->currentLevelID = this->nextLevelID;
	this->nextLevelID = 0x10;
	this->outroCutsceneId = -1;
	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_PostInit()
{
	CActorNativShop* pNativ;
	CLevelScheduler* pThis;
	CActor** pActor;
	int iVar2;
	NativShopLevelSubObj* pSubObj;
	float fVar3;
	float fVar4;
	float fVar5;
	CActorsTable nativShopsTable;

	SaveGame_LoadLevelState(this->currentLevelID);

	if (this->currentLevelID == 0) {
		nativShopsTable.nbEntries = 0;
		CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(NATIV_SHOP, &nativShopsTable);

		this->nbNativShopSubObjs = nativShopsTable.nbEntries;
		if (10 < this->nbNativShopSubObjs) {
			this->nbNativShopSubObjs = 10;
		}

		pSubObj = this->aNativShopSubObjs;
		iVar2 = 0;
		if (0 < this->nbNativShopSubObjs) {
			pActor = nativShopsTable.aEntries;
			do {
				pThis = gThis;
				pNativ = static_cast<CActorNativShop*>(*pActor);
				pSubObj->field_0x0 = pNativ->field_0x170;
				pSubObj->currentLevelId = pThis->currentLevelID;
				pSubObj->currentLocation = pNativ->currentLocation;
				pSubObj->rotationQuat = pNativ->rotationQuat;
				pSubObj->FUN_002d8d10(pNativ);
				iVar2 = iVar2 + 1;
				pActor = pActor + 1;
				pSubObj = pSubObj + 1;
			} while (iVar2 < this->nbNativShopSubObjs);
		}
	}

	if ((((this->field_0x80 | this->bShouldLoad | this->bShouldSave) == 0) && (this->currentLevelID != 0xe)) &&
		(this->currentLevelID != 0xf)) {
		if (this->autoSaveTriggerTime < 0.1f) {
			this->autoSaveTriggerTime = 0.1f;
		}

		this->curAutoSaveTime = 0.0f;
	}

	this->currentElevatorID = this->nextElevatorID;
	this->nextElevatorID = -1;
	this->baseSectorIndex = -1;
	this->bShouldLoad = 0;
	this->bShouldSave = 0;
	this->field_0x80 = 0;

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_PreTerm()
{
	S_SUBSECTOR_INFO* pLVar1;
	int iVar2;

	iVar2 = 0;
	pLVar1 = this->aLevelInfo[this->currentLevelID].aSubSectorInfo;
	do {
		pLVar1[0].field_0x20 = (edDList_material*)0x0;
		pLVar1[1].field_0x20 = (edDList_material*)0x0;
		pLVar1[2].field_0x20 = (edDList_material*)0x0;
		pLVar1[3].field_0x20 = (edDList_material*)0x0;
		pLVar1[4].field_0x20 = (edDList_material*)0x0;
		pLVar1[5].field_0x20 = (edDList_material*)0x0;
		iVar2 = iVar2 + 6;
		pLVar1 = pLVar1 + 6;
	} while (iVar2 < 0xc);

	g_CinematicManager_0048efc->PlayOutroCinematic(this->outroCutsceneId, (CActor*)0x0);
	SaveGame_SaveCurLevelState(0);

	this->autoSaveTriggerTime = 0.0f;

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
int CLevelScheduler::ScenVar_Get(SCENARIC_VARIABLE scenVarId)
{
	return _gScenVarInfo[scenVarId].currentValue;
}

void CLevelScheduler::ScenVar_Set(SCENARIC_VARIABLE scenVarId, int newValue)
{
	_gScenVarInfo[scenVarId].currentValue = newValue;
	return;
}

void CLevelScheduler::OnSceneVarSet()
{
	CActorHero* pHero;

	pHero = CActorHero::_gThis;
	if (CActorHero::_gThis != (CActorHero*)0x0) {
		if (CActorHero::_gThis->pActorBoomy != (CActorBoomy*)0x0) {
			pHero->DoMessage(pHero, MESSAGE_BOOMY_CHANGED, (MSG_PARAM)_gScenVarInfo[SCN_ABILITY_BOOMY_TYPE].currentValue);
			pHero->DoMessage(pHero->pActorBoomy, MESSAGE_BOOMY_CHANGED, (MSG_PARAM)_gScenVarInfo[SCN_ABILITY_BOOMY_TYPE].currentValue);
		}

		pHero->DoMessage(pHero, MESSAGE_FIGHT_RING_CHANGED, (MSG_PARAM)_gScenVarInfo[SCN_ABILITY_FIGHT].currentValue);
		pHero->DoMessage(pHero, MESSAGE_RECEPTACLE_CHANGED, 0);
	}
	return;
}

void CLevelScheduler::Level_Run(undefined8 param_2, int levelID, int elevatorID, int param_5, int param_6, bool bFadeOut)
{
	bool bVar1;

	if ((levelID != 0x10) && (bVar1 = CScene::_pinstance->IsFadeTermActive(), bVar1 == false)) {
		if ((levelID == 0) && (_gScenVarInfo[5].currentValue != 0)) {
			levelID = 6;
		}

		Level_FillRunInfo(levelID, elevatorID, param_6);
		this->outroCutsceneId = param_5;
		CScene::_pinstance->SetFadeStateTerm(bFadeOut);
	}

	return;
}

void CLevelScheduler::Level_Teleport(CActor* pActor, int levelId, int elevatorId, int cutsceneId, int param_6)
{
	int iVar1;
	char cVar2;
	bool bVar3;
	CActor* pCVar4;
	CSectorManager* pSectorManager;

	if ((levelId == 0x10) || (levelId != this->currentLevelID)) {
		if ((levelId != 0x10) && (CScene::_pinstance->IsFadeTermActive() == false)) {
			if ((levelId == 0) && (_gScenVarInfo[5].currentValue != 0)) {
				levelId = 6;
			}

			Level_FillRunInfo(levelId, elevatorId, param_6);

			this->outroCutsceneId = cutsceneId;
			CScene::_pinstance->SetFadeStateTerm(false);
		}
	}
	else {
		if ((-1 < elevatorId) && (elevatorId < this->aLevelInfo[this->currentLevelID].maxElevatorId)) {
			IMPLEMENTATION_GUARD(
			iVar1 = *(int*)(this->levelPath + this->currentLevelID * 0x418 + -8 + elevatorId * 0x28 + 0xf0);
			if ((iVar1 != -1) &&
				(pCVar4 = CScene::ptable.g_ActorManager_004516a4->GetActorByHashcode(iVar1),
					pSectorManager = CScene::ptable.g_SectorManager_00451670, pCVar4 != (CActor*)0x0)) {
				iVar1 = (pCVar4->data).objectId;
				cVar2 = '\0';
				if ((iVar1 == ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID) || (iVar1 == -1)) {
					if (cutsceneId != -1) {
						cVar2 = CCinematicManager::RunSectorLoadingCinematic (g_CinematicManager_0048efc, cutsceneId, pActor, elevatorId, param_6);
					}
				}
				else {
					cVar2 = CCinematicManager::RunSectorLoadingCinematic
					(g_CinematicManager_0048efc, cutsceneId, pActor, elevatorId, param_6);
					CSectorManager::SwitchToSector(pSectorManager, iVar1, (int)cVar2);
				}
				if (cVar2 == '\0') {
					this->currentElevatorID = elevatorId;
					this->level_0x5b50 = this->currentLevelID;
					this->level_0x5b54 = param_6;
				}
			})
		}
	}

	return;
}

void CLevelScheduler::Level_GetSubSectorInfo(int levelIndex, int elevatorId, S_SUBSECTOR_INFO* pSubSectorInfo)
{
	pSubSectorInfo->field_0x0 = 0;
	pSubSectorInfo->teleporterActorHashCode = -1;
	pSubSectorInfo->field_0xc = -1;
	pSubSectorInfo->field_0x10 = 0;
	pSubSectorInfo->field_0x14 = 0;
	pSubSectorInfo->nbExorcisedWolfen = 0;
	pSubSectorInfo->flags = 0;
	pSubSectorInfo->field_0x20 = 0;

	if ((pSubSectorInfo->field_0x10 & 1) != 0) {
		pSubSectorInfo->flags = pSubSectorInfo->flags | 1;
	}

	if ((-1 < elevatorId) && (elevatorId < this->aLevelInfo[levelIndex].maxElevatorId)) {
		*pSubSectorInfo = this->aLevelInfo[levelIndex].aSubSectorInfo[elevatorId];
	}

	return;
}

// Should be in: D:/Projects/b-witch/LevelScheduler.cpp
void CLevelScheduler::Level_UpdateCurLiveLevelInfo()
{
	int iVar1;
	S_SUBSECTOR_INFO* pWolfenSubSectorInfo;
	int iVar3;
	bool bVar4;
	CActorTeleporter* this_00;
	edDList_material* peVar5;
	S_LEVEL_INFO* pLevelInfo;
	int iVar6;
	int iVar8;
	CActor* piVar9;
	int iVar9;
	S_SUBSECTOR_INFO* pSubSectorInfo;
	CActorsTable teleportersTable;
	CActorManager* pActorManager;

	pActorManager = CScene::ptable.g_ActorManager_004516a4;
	iVar3 = _gScenVarInfo[SCN_GAME_NUM_FREED_WOLFENS].currentValue;
	iVar8 = 0;
	pLevelInfo = this->aLevelInfo + this->currentLevelID;
	iVar9 = pLevelInfo->nbExorcisedWolfen;
	pSubSectorInfo = pLevelInfo->aSubSectorInfo;
	pLevelInfo->nbExorcisedWolfen = 0;
	do {
		pSubSectorInfo->nbExorcisedWolfen = 0;
		pSubSectorInfo->flags = 0;
		pSubSectorInfo->field_0x20 = (edDList_material*)0x0;

		if ((pSubSectorInfo->field_0x10 & 1) != 0) {
			pSubSectorInfo->flags = pSubSectorInfo->flags | 1;
		}

		if ((pSubSectorInfo->teleporterActorHashCode != -1) &&
			(this_00 = (CActorTeleporter*)pActorManager->GetActorByHashcode(pSubSectorInfo->teleporterActorHashCode), this_00 != (CActorTeleporter*)0x0)) {
			iVar6 = this_00->field_0x168;
			if (iVar6 < 0) {
				iVar6 = 0;
			}

			pSubSectorInfo->field_0x20 = this_00->GetMySubSectorMaterial(this->currentLevelID, iVar6);
			if (this_00->bOpen != 0) {
				pSubSectorInfo->flags = pSubSectorInfo->flags | 1;
			}
		}

		iVar8 = iVar8 + 1;
		pSubSectorInfo = pSubSectorInfo + 1;
	} while (iVar8 < 0xc);

	iVar8 = 0;
	if (0 < pActorManager->nbActors) {
		do {
			if (iVar8 == -1) {
				piVar9 = (CActorWolfen*)0x0;
			}
			else {
				piVar9 = CScene::ptable.g_ActorManager_004516a4->aActors[iVar8];
			}
			bVar4 = piVar9->IsKindOfObject(OBJ_TYPE_WOLFEN);
			CActorWolfen* pWolfen = static_cast<CActorWolfen*>(piVar9);
			if ((bVar4 != false) && (pWolfen->exorcisedState == 2)) {
				iVar1 = pWolfen->startSectorId;
				pWolfenSubSectorInfo = pLevelInfo->aSubSectorInfo;
				if (-1 < iVar1) {
					pWolfenSubSectorInfo = pWolfenSubSectorInfo + iVar1;
				}

				pWolfenSubSectorInfo->nbExorcisedWolfen = pWolfenSubSectorInfo->nbExorcisedWolfen + 1;

				if (0 < iVar1) {
					pLevelInfo->nbExorcisedWolfen = pLevelInfo->nbExorcisedWolfen + 1;
				}
			}

			iVar8 = iVar8 + 1;
		} while (iVar8 < pActorManager->nbActors);
	}

	_gScenVarInfo[SCN_GAME_NUM_FREED_WOLFENS].currentValue = (iVar3 - iVar9) + pLevelInfo->nbExorcisedWolfen;

	teleportersTable.nbEntries = 0;
	pActorManager->GetActorsByClassID(TELEPORTER, &teleportersTable);
	iVar9 = 0;
	if (0 < teleportersTable.nbEntries) {
		do {
			CActorTeleporter* pTeleporter = static_cast<CActorTeleporter*>(teleportersTable.aEntries[iVar9]);
			pTeleporter->NotifyLevelTeleporterChanged();
			iVar9 = iVar9 + 1;
		} while (iVar9 < teleportersTable.nbEntries);
	}

	return;
}

void CLevelScheduler::Level_TeleporterChanged()
{
	Level_UpdateCurLiveLevelInfo();

	return;
}

void CLevelScheduler::SetLevelTimerFunc_002df450(float param_1, int mode)
{
	Timer* pTVar1;

	if (((this->curAutoSaveTime == 0.0f) || (mode != 0)) || (pTVar1 = Timer::GetTimer(), 180.0f < (param_1 + pTVar1->scaledTotalTime) - this->curAutoSaveTime)) {
		if (param_1 == 0.0f) {
			param_1 = 0.001f;
		}

		if (this->autoSaveTriggerTime < param_1) {
			this->autoSaveTriggerTime = param_1;
		}

		if (mode != 0) {
			this->curAutoSaveTime = 0.0f;
		}
	}
	return;
}

int CLevelScheduler::GetNbAreas(int currentLevelID)
{
	bool bVar1;
	CActorTeleporter* this_00;
	int iVar2;
	S_LEVEL_INFO* pcVar3;
	int iVar4;
	S_SUBSECTOR_INFO* pcVar5;
	int iVar6;
	int iVar7;
	int nbAreas;
	CActorManager* pActorManager;

	bVar1 = this->currentLevelID == 0 || this->currentLevelID == INT_ARRAY_0048ed60[0];

	if (this->currentLevelID != 0 && this->currentLevelID != INT_ARRAY_0048ed60[0]) {
		bVar1 = INT_ARRAY_0048ed60[this->currentLevelID] == 0;
	}

	if (bVar1) {
		nbAreas = 2;
	}
	else {
		pcVar3 = this->aLevelInfo + currentLevelID;
		nbAreas = this->aLevelInfo[INT_ARRAY_0048ed60[currentLevelID]].maxElevatorId;
		iVar6 = pcVar3->maxElevatorId;
		if (pcVar3->maxElevatorId < nbAreas) {
			iVar6 = nbAreas;
		}

		nbAreas = this->aLevelInfo[INT_ARRAY_0048eda0[currentLevelID]].maxElevatorId;
		if (iVar6 < nbAreas) {
			iVar6 = nbAreas;
		}

		iVar7 = 1;
		for (nbAreas = 1; pActorManager = CScene::ptable.g_ActorManager_004516a4, nbAreas < iVar6; nbAreas = nbAreas + 1) {
			edDList_material* pSubSectorMaterial = pcVar3->aSubSectorInfo[iVar7].field_0x20;
			if (pSubSectorMaterial == (edDList_material*)0x0) {
				iVar4 = 0;
				pcVar5 = pcVar3->aSubSectorInfo;
				while ((iVar4 < 0xc && (pSubSectorMaterial == (edDList_material*)0x0))) {
					if ((pcVar5->teleporterActorHashCode != 0xffffffff) &&
						(this_00 = (CActorTeleporter*)pActorManager->GetActorByHashcode(pcVar5->teleporterActorHashCode), this_00 != (CActorTeleporter*)0x0)) {
						pSubSectorMaterial = this_00->GetMySubSectorMaterial(currentLevelID, nbAreas);
					}

					iVar4 = iVar4 + 1;
					pcVar5 = pcVar5 + 1;
				}
			}

			if (pSubSectorMaterial == (edDList_material*)0x0) {
				return nbAreas;
			}

			iVar7 = iVar7 + 1;
		}
	}

	return nbAreas;
}

void CLevelScheduler::ExitLevel(int param_2)
{
	int levelID;

	this->bShouldSave = 1;

	gSaveManagement.clear_slot();

	if (this->aLevelInfo[0xe].levelName[0] == '\0') {
		levelID = this->nextLevelId;

		if ((levelID != 0x10) && (CScene::_pinstance->IsFadeTermActive() == false)) {
			if ((levelID == 0) && (_gScenVarInfo[5].currentValue != 0)) {
				levelID = 6;
			}

			Level_FillRunInfo(levelID, -1, -1);
			this->outroCutsceneId = -1;
			CScene::_pinstance->SetFadeStateTerm(true);
		}
	}
	else {
		if (CScene::_pinstance->IsFadeTermActive() == false) {
			Level_FillRunInfo(0xe, -1, -1);
			this->outroCutsceneId = -1;
			CScene::_pinstance->SetFadeStateTerm(true);
		}
	}

	if (param_2 != 0) {
		if ((GameFlags & 8) != 0) {
			HelpLeave();
		}
		if ((GameFlags & 0x10) != 0) {
			MapLeave();
		}
		if ((GameFlags & 4) != 0) {
			ResumeGame(0);
		}
	}

	return;
}

void CLevelScheduler::FUN_002dc200(int elevatorId, int levelId, int param_4)
{
	CActorTeleporter* pTeleporter;
	CCameraManager* pCameraManager;

	if ((((elevatorId != -1) && (-1 < elevatorId)) && (elevatorId < this->aLevelInfo[this->currentLevelID].maxElevatorId) &&
		(((pTeleporter = (CActorTeleporter*)
			CScene::ptable.g_ActorManager_004516a4->GetActorByHashcode(this->aLevelInfo[this->currentLevelID].aSubSectorInfo[elevatorId].field_0x0)),
			pTeleporter != (CActorTeleporter*)0x0 && (pTeleporter->bOpen != 0)) &&
			(pTeleporter->UpdateCurTeleporterState(levelId, param_4),
				(CActorHero::_gThis->flags & 0x800000) == 0)))) {
		pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
		pCameraManager->Level_Reset();
		pCameraManager->SetMainCamera(CActorHero::_gThis->pMainCamera);
		pCameraManager->AlertCamera(2, 1);
	}

	return;
}

CChunk* CChunk::FindNextSubChunk(CChunk* pChunk, uint hash)
{
	char* pEnd = reinterpret_cast<char*>(this + 1) + this->offset;

	for (; (reinterpret_cast<char*>(pChunk) < pEnd && (hash != pChunk->hash));
		pChunk = (CChunk*)(reinterpret_cast<char*>(pChunk + 1) + pChunk->offset)) {
	}

	if ((pEnd <= reinterpret_cast<char*>(pChunk)) || (hash != pChunk->hash)) {
		pChunk = (CChunk*)0x0;
	}

	return pChunk;
}

// Should be in: D:/Projects/b-witch/Chunk.cpp
void* CChunk::DeleteSubChunk(CChunk* pChunk)
{
	char* pDeleteChunkEnd;
	char* pRootChunkEnd;

	pDeleteChunkEnd = reinterpret_cast<char*>(pChunk + 1) + pChunk->offset;
	pRootChunkEnd = reinterpret_cast<char*>(this + 1) + this->offset;

	memmove(pChunk, pDeleteChunkEnd, pRootChunkEnd - pDeleteChunkEnd);

	void* pRepvVar1 = memset(pRootChunkEnd + -(pDeleteChunkEnd - reinterpret_cast<char*>(pChunk)),
		0, 
		pDeleteChunkEnd - reinterpret_cast<char*>(pChunk));

	this->offset = pRootChunkEnd + -(pDeleteChunkEnd - reinterpret_cast<char*>(pChunk)) - reinterpret_cast<char*>(this + 1);
	return pRepvVar1;
}