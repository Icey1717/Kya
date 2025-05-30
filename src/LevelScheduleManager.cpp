#include "LevelScheduleManager.h"
#include "IniFile.h"
#include "edMem.h"
#include "edBankFile.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#include <libvu0.h>
#else
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

CLevelScheduler* CLevelScheduler::gThis = NULL;

bool CLevelScheduler::AddMoneyB(int amount)
{
	_gGameNfo.nbMoney = _gGameNfo.nbMoney + amount;
	//_gGameNfo.scenery = _gGameNfo.scenery - amount;

	return true;
}

bool CLevelScheduler::AddMoneyA(int amount)
{
	_gGameNfo.nbMoney = _gGameNfo.nbMoney + amount;
	//_gGameNfo.field_0x1c = _gGameNfo.field_0x1c - amount;

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
	pSaveData_0x48 = (SaveBigAlloc*)0x0;
	pSaveDataEnd_0x4c = 0;
	currentSaveIndex = 0;
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
	if (this->field_0x78 == 0) {
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
		IMPLEMENTATION_GUARD();
		pCVar2 = this->aSaveDataArray[this->currentSaveIndex]->FindNextSubChunk(this->aSaveDataArray[this->currentSaveIndex] + 1, 0x44485342);
		if (pCVar2 != (CChunk*)0x0) {
			this->currentSaveIndex = this->currentSaveIndex + 1;
			this->aSaveDataArray[this->currentSaveIndex] = pCVar2;
		}

		this->baseSectorIndex = pCVar2[1].field_0x4;
		this->aSaveDataArray[this->currentSaveIndex] = (CChunk*)0x0;
		this->currentSaveIndex = this->currentSaveIndex + -1;
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

struct LoadLoopObject_418_18 {
	SectorManagerSubObj aSubObj[6];
};

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
		pLevelInfo->sectorCount_0x14 = 0;
		pLevelInfo->maxElevatorId = 0;
		pLevelInfo->field_0x20 = 0;
		pLevelInfo->levelName[0] = '\0';
		pLevelInfo->aCompanionInfo = (S_COMPANION_INFO*)0x0;
		pLevelInfo->pSimpleConditionData = 0;
	}

	pLevelInfo->field_0x50 = 0;

	pLevelInfoSubObj = pLevelInfo->aSubSectorInfo;
	iVar2 = 0;
	do {
		if (param_2 != false) {
			pLevelInfoSubObj->field_0x0 = 0;
			pLevelInfoSubObj->field_0x8 = 0xffffffff;
			pLevelInfoSubObj->field_0xc = -1;
			pLevelInfoSubObj->field_0x10 = 0;
			pLevelInfoSubObj->field_0x14 = 0;
		}

		pLevelInfoSubObj->field_0x18 = 0;
		pLevelInfoSubObj->field_0x1c = 0;
		pLevelInfoSubObj->field_0x20 = (edDList_material*)0x0;

		if ((pLevelInfoSubObj->field_0x10 & 1) != 0) {
			pLevelInfoSubObj->field_0x1c = pLevelInfoSubObj->field_0x1c | 1;
		}

		iVar2 = iVar2 + 1;
		pLevelInfoSubObj = pLevelInfoSubObj + 1;
	} while (iVar2 < 0xc);
	
	//pLVar1 = pLevelInfo->field_0x238;
	//iVar2 = 0;
	//do {
	//	if (param_2 != false) {
	//		pLVar1->field_0x4 = 0;
	//		pLVar1->field_0x0 = 0;
	//		pLVar1->field_0x8 = 0;
	//		pLVar1->field_0xc = 0;
	//	}
	//	iVar2 = iVar2 + 1;
	//	pLVar1 = (LoadLoopObject_418_18*)&pLVar1->field_0x10;
	//} while (iVar2 < 0x1e);
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
	this->field_0x4214 = 0.0f;


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

	//iVar9 = 0;
	//if (0 < this->objCount_0x4218) {
	//	do {
	//		pLVar7->field_0x24 = 0x20;
	//		pLVar7->field_0x28 = 0;
	//		iVar9 = iVar9 + 1;
	//		pLVar7->field_0x2c = 0;
	//		pLVar7->field_0x40 = 0;
	//		fVar3 = gF32Vertex4Zero.w;
	//		fVar2 = gF32Vertex4Zero.z;
	//		fVar1 = gF32Vertex4Zero.y;
	//		(pLVar7->field_0x30).x = gF32Vertex4Zero.x;
	//		(pLVar7->field_0x30).y = fVar1;
	//		(pLVar7->field_0x30).z = fVar2;
	//		(pLVar7->field_0x30).w = fVar3;
	//		pLVar7 = pLVar7 + 1;
	//	} while (iVar9 < this->objCount_0x4218);
	//}
	puVar8 = this->aNativShopSubObjs;
	iVar9 = 0;
	if (0 < this->nbNativShopSubObjs) {
		do {
			if (param_2 != false) {
				puVar8->field_0x0 = -1;
				puVar8->field_0x4 = 0x10;
				puVar8->field_0x8 = 0;

				puVar8->field_0x10 = gF32Vertex4Zero;
				puVar8->field_0x20 = gF32Vector4UnitZ;

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

void CLevelScheduler::LevelsInfo_ReadHeader_V7_V9(char* fileData, S_LEVEL_INFO* pLevelInfo)
{
	int iVar2;
	SectorManagerSubObjOther* pLVar3;

	pLevelInfo->titleMsgHash = ByteCode::BuildU64(*(uint*)(fileData + 0x18), *(uint*)(fileData + 0x1c));
	pLVar3 = pLevelInfo->aSectorSubObj;
	iVar2 = 0;
	pLevelInfo->field_0x20 = *(undefined4*)(fileData + 0x28);
	pLevelInfo->bankSizeLevel = *(int*)(fileData + 0xc);
	pLevelInfo->bankSizeSect = *(int*)(fileData + 0x10);
	pLevelInfo->bankSizeIOP = *(int*)(fileData + 0x14);
	pLevelInfo->sectorCount_0x14 = *(int*)(fileData + 0x24);
	pLevelInfo->field_0x30 = *(int*)(fileData + 0x30);
	pLevelInfo->sectorStartIndex = *(int*)(fileData + 0x20);
	do {
		pLVar3->field_0x4 = 0;
		pLVar3->flags = 0;
		iVar2 = iVar2 + 6;
		pLVar3->nbSectorConditions = 0x0;
		pLVar3->aCompanionInfo = 0;
		pLVar3[1].field_0x4 = 0;
		pLVar3[1].flags = 0;
		pLVar3[1].nbSectorConditions = 0x0;
		pLVar3[1].aCompanionInfo = 0;
		pLVar3[2].field_0x4 = 0;
		pLVar3[2].flags = 0;
		pLVar3[2].nbSectorConditions = 0x0;
		pLVar3[2].aCompanionInfo = 0;
		pLVar3[3].field_0x4 = 0;
		pLVar3[3].flags = 0;
		pLVar3[3].nbSectorConditions = 0x0;
		pLVar3[3].aCompanionInfo = 0;
		pLVar3[4].field_0x4 = 0;
		pLVar3[4].flags = 0;
		pLVar3[4].nbSectorConditions = 0x0;
		pLVar3[4].aCompanionInfo = 0;
		pLVar3[5].field_0x4 = 0;
		pLVar3[5].flags = 0;
		pLVar3[5].nbSectorConditions = 0x0;
		pLVar3[5].aCompanionInfo = 0;
		pLVar3 = pLVar3 + 6;
	} while (iVar2 < 0x1e);
	iVar2 = 8;
	/* Set the name of the level we will load to memory */
	pLevelInfo->levelName[0] = fileData[4];
	pLevelInfo->levelName[1] = fileData[5];
	pLevelInfo->levelName[2] = fileData[6];
	pLevelInfo->levelName[3] = fileData[7];
	pLevelInfo->levelName[4] = fileData[8];
	pLevelInfo->levelName[5] = fileData[9];
	pLevelInfo->levelName[6] = fileData[10];
	pLevelInfo->levelName[7] = fileData[0xb];
	do {
		/* Make sure the 12 characters after the level name are clear */
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
			if (pLevelInfo->sectorCount_0x14 < pLvlNfoSector->sectorIndex) {
				pLevelInfo->sectorCount_0x14 = pLvlNfoSector->sectorIndex;
			}

			LEVEL_SCHEDULER_LOG(LogLevel::Info, "CSectorManager::LevelsInfo_ReadSectors_V7_V9 sectorIndex: 0x{:x}, field_0x4: 0x{:x}, nbConditions: 0x{:x}",
				pLvlNfoSector->sectorIndex, pLvlNfoSector->field_0x4, pLvlNfoSector->nbConditions);

			pCurSectorSubObj = pLevelInfo->aSectorSubObj + pLvlNfoSector->sectorIndex;
			pCurSectorSubObj->field_0x4 = pLvlNfoSector->field_0x4;

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
				if (pLevelInfo->aSectorSubObj[pLvlNfoSector->sectorIndex].nbSectorConditions != 0) {
					pLevelInfo->aSectorSubObj[pLvlNfoSector->sectorIndex].aCompanionInfo = pCurCompanionInfo;
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
		(pLevelInfo->aSectorSubObj[pLevelInfo->sectorStartIndex].field_0x4 == 0)) {
		curSectorIndex = 1;
		pLVar3 = pLevelInfo;
		while ((curSectorIndex <= pLevelInfo->sectorCount_0x14 &&
			(pLVar3->aSectorSubObj[curSectorIndex].field_0x4 == 0))) {
			curSectorIndex = curSectorIndex + 1;
		}
		if (pLevelInfo->sectorCount_0x14 < curSectorIndex) {
			pLevelInfo->sectorStartIndex = -1;
		}
		else {
			pLevelInfo->sectorStartIndex = curSectorIndex;
		}
	}

	return pCurConditionData;
}

void CLevelScheduler::LevelsInfo_ReadLanguageFileNames_V7_V9(S_LVLNFO_LANGUAGE_V7_V9* param_2, int nbObj, undefined4 param_4)
{
	int iVar1;
	float fVar2;
	ulong uVar3;
	int iVar4;
	LoadLoopObject_50* pLVar6;
	float fVar5;
	float fVar6;

	iVar1 = this->objCount_0x4218;
	iVar4 = iVar1 + nbObj;
	if (iVar4 < 0x41) {
		this->objCount_0x4218 = iVar4;
		iVar4 = 0;
		pLVar6 = this->field_0x4220 + iVar1;
		if (0 < nbObj) {
			do {
				pLVar6->field_0x24 = 0x20;
				pLVar6->field_0x28 = 0;
				pLVar6->field_0x2c = 0;
				pLVar6->field_0x40 = 0;
				pLVar6->field_0x30 = gF32Vertex4Zero;

				pLVar6->field_0x0 = param_2->field_0x0;
				pLVar6->field_0x20 = param_4;
				pLVar6->field_0x4 = param_2->field_0xc;
				pLVar6->messageKey = ByteCode::BuildU64(param_2->keyA, param_2->keyB);
				iVar4 = iVar4 + 1;
				fVar5 = param_2->field_0x14;
				fVar6 = param_2->field_0x18;
				pLVar6->field_0x10 = param_2->field_0x10;
				param_2 = param_2 + 1;
				pLVar6->field_0x14 = fVar5;
				pLVar6->field_0x18 = fVar6;
				pLVar6->field_0x1c = 1.0f;
				pLVar6 = pLVar6 + 1;
			} while (iVar4 < nbObj);
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
			pLevelInfoSubObj->field_0x8 = pFileData->field_0x0;
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
				pLevelInfoSubObj->field_0x1c = pLevelInfoSubObj->field_0x1c | 1;
			}

			iVar1 = iVar1 + 1;
			pLevelInfoSubObj = pLevelInfoSubObj + 1;
		} while (iVar1 < pLevelInfo->maxElevatorId);
	}

	pLevelInfo->aSubSectorInfo[0].field_0x14 = pLevelInfo->aSubSectorInfo[0].field_0x14 + (pLevelInfo->field_0x20 - iVar4);
	pLevelInfo->field_0x20 = pLevelInfo->field_0x20 - pLevelInfo->aSubSectorInfo[0].field_0x14;

	return;
}

void CLevelScheduler::Levels_LoadInfoBank()
{
	LEVEL_SCHEDULER_LOG(LogLevel::Info, "LevelScheduleManager::Levels_LoadInfoBank\n");

	char cVar1;
	int iVar2;
	undefined4* puVar3;
	edCBankBufferEntry* infoLevelsFileBuffer;
	int curFileIndex;
	int* puVar5;
	S_LEVEL_INFO* pLevelInfo;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	int* fileData;
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
				fileData = (int*)(outHeader.fileBufferStart + 4);
				if (true) {
					switch (*(int*)outHeader.fileBufferStart) {
					case 9:
						iVar2 = *fileData;
						if ((-1 < iVar2) && (iVar2 < 0x10)) {
							pLevelInfo = &aLevelInfo[iVar2];
							LevelsInfo_ReadHeader_V7_V9((char*)fileData, pLevelInfo);
							LevelsInfo_ReadTeleporters_V7_V9(reinterpret_cast<S_LVLNFO_TELEPORTERS_V7_V9*>(puVar3 + 0xf), puVar3[0xc], pLevelInfo);
							puVar5 = LevelsInfo_ReadSectors_V7_V9(reinterpret_cast<S_LVLNFO_SECTOR_V7_V9*>(puVar3 + 0xf + puVar3[0xc] * 7), puVar3[10], pLevelInfo);
							LevelsInfo_ReadLanguageFileNames_V7_V9(reinterpret_cast<S_LVLNFO_LANGUAGE_V7_V9*>(puVar5), puVar3[0xe], *fileData);
							iVar9 = puVar3[0xd];
							puVar5 = puVar5 + puVar3[0xe] * 7;

							char* pLevelPath = reinterpret_cast<char*>(puVar5);

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

							this->field_0x4210 = this->field_0x4210 + *(int*)(this->levelPath + iVar2 * 0x418 + -8 + 0xb0);
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

void CLevelScheduler::Episode_LoadFromIniFile(void)
{
	//int iVar1;
	//bool bVar2;
	//int* episodesPtr;
	//Episode* episodePtr;
	//uint episodeCounter;
	//int minWolfen;
	//int scenery;
	//int monster;
	//int bet;
	//undefined4 local_120;
	//int local_11c;
	//char Section[256];
	//int local_10;
	//int local_c;
	//char episodeCounterString[4];
	//
	//episodesPtr = &_gGameNfo;
	//_gGameNfo.count = 0;
	_gGameNfo.nbMagic = 0;
	_gGameNfo.nbMoney = 0;
	//_gGameNfo.scenery = 0;
	//_gGameNfo.monster = 0;
	//_gGameNfo.bet = 0;
	//_gGameNfo.field_0x1c = 0;
	//_gGameNfo.field_0x20 = 0;
	_gGameNfo.health = (float)_gScenVarInfo[20].currentValue;
	//iVar1 = 0;
	//do {
	//	local_10 = iVar1;
	//	local_c = 0;
	//	bVar2 = true;
	//	while (bVar2) {
	//		episodesPtr->field_0x24[local_c].field_0x0 = -1;
	//		episodesPtr->field_0x24[local_c].field_0x4 = 0;
	//		local_c = local_c + 1;
	//		bVar2 = local_c < 0x10;
	//	}
	//	episodesPtr = (EpStructComplete*)&episodesPtr->field_0x24[0xb].field_0x4;
	//	iVar1 = local_10 + 1;
	//} while (local_10 + 1 < 2);
	///* Episode_ + General */
	//edStrCatMulti(Section, s_Episode__00433b70, 0x433b80, 0);
	///* HeroMagic */
	//IniFile::ReadInt_001a9830(&gIniFile, Section, s_HeroMagic_00433b88, &_gGameNfo.nbMagic);
	//bVar2 = IniFile::ReadInt_001a9830(&gIniFile, Section, s_NbEpisodes_00433b98, (int*)&_gGameNfo);
	//if (((bVar2 != false) && (0 < _gGameNfo.count)) && (episodeCounter = 0, 0 < _gGameNfo.count)) {
	//	episodePtr = g_EpisodeDataArray_0048eb0;
	//	do {
	//		edStrInt2Str(episodeCounter, episodeCounterString, 2, true);
	//		edStrCatMulti(Section, s_Episode__00433b70, episodeCounterString);
	//		/* MinWolfen */
	//		IniFile::ReadInt_001a9830(&gIniFile, Section, s_MinWolfen_00433ba8, &minWolfen);
	//		/* Scenery */
	//		IniFile::ReadInt_001a9830(&gIniFile, Section, s_Scenery_00433bb8, &scenery);
	//		/* Monster */
	//		IniFile::ReadInt_001a9830(&gIniFile, Section, s_Monster_00433bc0, &monster);
	//		/* Bet */
	//		IniFile::ReadInt_001a9830(&gIniFile, Section, s_Bet_00433bc8, &bet);
	//		local_120 = 0;
	//		episodeCounter = episodeCounter + 1;
	//		episodePtr->minWolfen = minWolfen;
	//		episodePtr->scenery = scenery;
	//		episodePtr->monster = monster;
	//		episodePtr->bet = bet;
	//		episodePtr->field_0x10 = 0;
	//		episodePtr->total_0x14 = local_11c;
	//		episodePtr = episodePtr + 1;
	//	} while ((int)episodeCounter < _gGameNfo.count);
	//}
	CScene::ptable.g_FrontendManager_00451680->SetGlobalFunc_001cf8e0();
	//FUN_002daf70();
	return;
}

void CLevelScheduler::SaveGame_SaveCurLevelState(int param_2)
{
	IMPLEMENTATION_GUARD_SAVE();
}

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

int CLevelScheduler::GetBoomyLevel() 
{
	return _gScenVarInfo[9].currentValue;
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
	uVar1 = _gScenVarInfo[10].currentValue;

	if (_gScenVarInfo[10].currentValue != 0) {
		do {
			uVar1 = uVar1 >> 1;
			fightLevel = fightLevel + 1;
		} while (uVar1 != 0);
	}

	return fightLevel;
}

int INT_ARRAY_0048ed60[16] = { 0 };
int INT_ARRAY_0048eda0[16] = { 0 };

void CLevelScheduler::Game_Init()
{
	SaveBigAlloc* pSVar1;
	bool bVar2;
	SaveBigAlloc* lVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	undefined4* puVar6;
	undefined4* puVar7;
	undefined4* puVar8;
	int* piVar9;
	int* piVar10;
	int iVar11;
	S_LEVEL_INFO* pLVar12;
	char local_80[128];

	lVar2 = (SaveBigAlloc*)edMemAlloc(TO_HEAP(H_MAIN), 0x10000);
	/* Zero out most of load loop */
	pSaveData_0x48 = lVar2;
	pSaveDataEnd_0x4c = (int)(pSaveData_0x48 + 1);
	memset(pSaveData_0x48, 0, 0x10000);
	aSaveDataArray[0] = (CChunk*)0x0;
	aSaveDataArray[1] = (CChunk*)0x0;
	aSaveDataArray[2] = (CChunk*)0x0;
	aSaveDataArray[3] = (CChunk*)0x0;
	pLVar12 = aLevelInfo;
	aSaveDataArray[4] = (CChunk*)0x0;
	iVar11 = 0;
	aSaveDataArray[5] = (CChunk*)0x0;
	aSaveDataArray[6] = (CChunk*)0x0;
	aSaveDataArray[7] = (CChunk*)0x0;
	currentSaveIndex = 0;
	aSaveDataArray[currentSaveIndex] = (CChunk*)pSaveData_0x48;
	pObjectiveStreamBegin = 0;
	pObjectiveStreamEnd = 0;
	field_0x74 = 0;
	field_0x78 = 0;
	field_0x7c = 0;
	field_0x80 = 0;
	pLevelBankBufferEntry = (edCBankBufferEntry*)0x0;
	pIOPBankBufferEntry = (edCBankBufferEntry*)0x0;
	loadStage_0x5b48 = 4;
	field_0x84 = 0.0;
	field_0x88 = 0.0;
	nextLevelId = 0;
	currentLevelID = 0x10;
	currentElevatorID = -1;
	level_0x5b50 = 0x10;
	level_0x5b54 = -1;
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
	strcpy(levelPath, g_CD_LevelPath_00433bf8);
	/* Try read [Router] - SetPath from INI file */
	bVar2 = gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szSetPath_00433c10, levelPath);
	if (bVar2 == false) {
		/* No path found in .INI -> use default !\n */
		edDebugPrintf(g_szNoPathError_00433c20);
	}
	/* Try read [Router] - AddLevel from INI file  */
	local_80[0] = '\0';
	gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szAddLevel_00433c48, local_80);
	pLVar12 = aLevelInfo;
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
	pLVar12 = aLevelInfo;
	iVar11 = 0;
	do {
		iVar3 = edStrICmp((byte*)local_80, (byte*)pLVar12->levelName);
		if (iVar3 == 0) goto LAB_002e26c8;
		iVar11 = iVar11 + 1;
		pLVar12 = pLVar12 + 1;
	} while (iVar11 < 0x10);
	iVar11 = 0;
LAB_002e26c8:
	nextLevelId = iVar11;
	pSVar1 = pSaveData_0x48;
	pSVar1->field_0x0 = 0x16660666;
	pSVar1->field_0x4 = 0x56415342;
	pSVar1->field_0x8 = 0x50000;
	pSVar1->size_0xc = 0;
	iVar11 = *(int*)(aLevelInfo[0].levelName + currentSaveIndex * 4 + -100);
	//for (piVar9 = &DAT_00433aa0; (iVar3 = *piVar9, iVar3 != 0x44485342 && (iVar3 != 0)); piVar9 = piVar9 + 4) {
	//}
	//if (iVar3 == 0) {
	//	piVar9 = (int*)0x0;
	//}
	//iVar3 = piVar9[3];
	//iVar4 = piVar9[2];
	//iVar11 = iVar11 + *(int*)(iVar11 + 0xc);
	//piVar10 = (int*)(iVar11 + 0x10);
	//if (piVar9[1] == 0) {
	//	*piVar10 = 0x16660666;
	//}
	//else {
	//	*piVar10 = 0x6667666;
	//}
	//*(undefined4*)(iVar11 + 0x14) = 0x44485342;
	//*(int*)(iVar11 + 0x18) = iVar4;
	//*(int*)(iVar11 + 0x1c) = iVar3;
	//count_0x70 = count_0x70 + 1;
	//*(int**)(loadLoopObjectArray[0].levelName + count_0x70 * 4 + -100) = piVar10;
	//if (*piVar10 == 0x6667666) {
	//	puVar7 = (undefined4*)(iVar11 + 0x20);
	//}
	//else {
	//	puVar7 = (undefined4*)0x0;
	//}
	//*puVar7 = 0x10;
	//puVar7[2] = 0;
	//puVar7[1] = 0xffffffff;
	//puVar7[3] = 0xffffffff;
	//puVar7[4] = 0xffffffff;
	//iVar11 = *(int*)(levelPath + count_0x70 * 4 + -8 + 0x50);
	//*(undefined4*)(levelPath + count_0x70 * 4 + -8 + 0x50) = 0;
	//count_0x70 = count_0x70 + -1;
	//iVar3 = *(int*)(loadLoopObjectArray[0].levelName + count_0x70 * 4 + -100);
	//*(int*)(iVar3 + 0xc) = (iVar11 + *(int*)(iVar11 + 0xc) + 0x10) - (iVar3 + 0x10);
	//iVar11 = *(int*)(loadLoopObjectArray[0].levelName + count_0x70 * 4 + -100);
	//for (piVar9 = &DAT_00433aa0; (iVar3 = *piVar9, iVar3 != 0x4e435342 && (iVar3 != 0)); piVar9 = piVar9 + 4) {
	//}
	//if (iVar3 == 0) {
	//	piVar9 = (int*)0x0;
	//}
	//iVar3 = piVar9[3];
	//iVar4 = piVar9[2];
	//iVar11 = iVar11 + *(int*)(iVar11 + 0xc);
	//piVar10 = (int*)(iVar11 + 0x10);
	//if (piVar9[1] == 0) {
	//	*piVar10 = 0x16660666;
	//}
	//else {
	//	*piVar10 = 0x6667666;
	//}
	//*(undefined4*)(iVar11 + 0x14) = 0x4e435342;
	//*(int*)(iVar11 + 0x18) = iVar4;
	//*(int*)(iVar11 + 0x1c) = iVar3;
	//count_0x70 = count_0x70 + 1;
	//*(int**)(loadLoopObjectArray[0].levelName + count_0x70 * 4 + -100) = piVar10;
	//puVar7 = (undefined4*)0x0;
	//if (*piVar10 == 0x6667666) {
	//	puVar7 = (undefined4*)(iVar11 + 0x20);
	//}
	//iVar11 = 0;
	//*puVar7 = 0x62;
	//puVar8 = &DAT_004253d0;
	//puVar6 = puVar7;
	//do {
	//	iVar11 = iVar11 + 8;
	//	puVar6[1] = puVar8[1];
	//	puVar6[2] = puVar8[3];
	//	puVar6[3] = puVar8[5];
	//	puVar6[4] = puVar8[7];
	//	puVar6[5] = puVar8[9];
	//	puVar6[6] = puVar8[0xb];
	//	puVar6[7] = puVar8[0xd];
	//	puVar6[8] = puVar8[0xf];
	//	puVar8 = puVar8 + 0x10;
	//	puVar6 = puVar6 + 8;
	//} while (iVar11 < 0x5a);
	//(puVar7 + iVar11)[1] = (&DAT_004253d4)[iVar11 * 2];
	//(puVar7 + iVar11)[2] = (&DAT_004253dc)[iVar11 * 2];
	//iVar11 = *(int*)(levelPath + count_0x70 * 4 + -8 + 0x50);
	//*(undefined4*)(levelPath + count_0x70 * 4 + -8 + 0x50) = 0;
	//count_0x70 = count_0x70 + -1;
	//*(int*)(iVar11 + 0xc) = (int)puVar7 + (0x18c - (iVar11 + 0x10));
	//iVar3 = *(int*)(loadLoopObjectArray[0].levelName + count_0x70 * 4 + -100);
	//*(int*)(iVar3 + 0xc) = (iVar11 + *(int*)(iVar11 + 0xc) + 0x10) - (iVar3 + 0x10);
	//FUN_002da9b0(inLoapLoopObject);
	//FUN_002da2e0(inLoapLoopObject);
	if (aLevelInfo[0xe].levelName[0] == '\0') {
		Level_FillRunInfo(nextLevelId, -1, -1);
	}
	else {
		//Level_FillRunInfo(0xe, -1, -1);
		// #HACK
		//Level_FillRunInfo(0x4, -1, -1);
		//Level_FillRunInfo(0x4, 9, -1);
		Level_FillRunInfo(0x0, 9, 0xb);
	}
	return;
}

void NativShopLevelSubObj::FUN_002d8d10(CActorNativ* pNativ)
{
	int iVar1;
	void* lVar2;
	int iVar3;

	if (pNativ->dynamic.field_0x10.w == 0.0f) {
		this->field_0x8 = 0;
	}
	else {
		this->field_0x8 = 1;
	}

	iVar3 = 0;
	do {
		lVar2 = pNativ->FUN_0036f330(iVar3);
		if (lVar2 == 0) {
			this->aSubObjs[iVar3].field_0x0 = 0x20;
			this->aSubObjs[iVar3].field_0x4 = 0;
			this->aSubObjs[iVar3].field_0x8 = 0;
		}
		else {
			IMPLEMENTATION_GUARD(
			iVar1 = (int)lVar2;
			this->aSubObjs[iVar3].field_0x0 = *(undefined4*)(iVar1 + 0x20);
			this->aSubObjs[iVar3].field_0x4 = *(undefined4*)(iVar1 + 0x30);
			this->aSubObjs[iVar3].field_0x8 = *(undefined8*)(iVar1 + 8);)
		}

		iVar3 = iVar3 + 1;
	} while (iVar3 < 5);
	return;
}

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
	pCollisionManager->aStaticCollisionRefs[pCollisionManager->staticCollisionCount] = (edObbTREE_DYN*)LOAD_SECTION(pObbTree->pObbTree);
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
		*(char**)(*(int*)&(CScene::ptable.g_AudioManager_00451698)->field_0x60 +
			(CScene::ptable.g_AudioManager_00451698)->field_0xc * 8) = pFileData;
		*(int*)(pGVar1->field_0xc * 8 + *(int*)&pGVar1->field_0x60 + 4) = length;
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
			(*(int*)&(CScene::ptable.g_AudioManager_00451698)->field_0x60 +
				(CScene::ptable.g_AudioManager_00451698)->field_0xc * 8);
		uVar3 = FUN_00267e20(*puVar2, pFileData, (long)(int)puVar2[1], (long)length);
		*(uint*)(pGVar1->field_0x50 + pGVar1->field_0xc * 4) = uVar3;
		*(undefined4*)&pGVar1->field_0xb8 = *(undefined4*)(pGVar1->field_0xc * 8 + *(int*)&pGVar1->field_0x60 + 4);
		if ((*(uint*)&pGVar1->field_0xb8 & 0x3f) != 0) {
			*(uint*)&pGVar1->field_0xb8 = (*(uint*)&pGVar1->field_0xb8 & 0xffffffc0) + 0x40;
		}
		pGVar1->field_0xb4 = pGVar1->field_0xb4 + *(int*)&pGVar1->field_0xb8;
	}
	pGVar1->field_0xc = pGVar1->field_0xc + 1;
	if (pGVar1->field_0xc == pGVar1->field_0x48) {
		pGVar1->field_0xc = 0;
		edMemFree(*(void**)&pGVar1->field_0x60);
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

void CLevelScheduler::LevelLoading_Begin()
{
	int cachedNextLevelID;
	char filePath[128];
	edCBankInstall bankContainer;

	/* This is part of the level load process
	   It loads in the level IOP bank for the specific level found in the level load
	   master.
	   Example: CDEURO/LEVEL/PREINTRO/LevelIOP.bnk */
	edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
	cachedNextLevelID = nextLevelID;
	memset(&bankContainer, 0, sizeof(edCBankInstall));
	levelIOPBank.initialize(aLevelInfo[cachedNextLevelID].bankSizeIOP + 0x1000, 1,
		&bankContainer);
	levelIOPBank.bank_buffer_setcb(TableBankCallback);
	/* / + LevelIOP.bnk */
	edStrCatMulti(filePath, levelPath,
		aLevelInfo[cachedNextLevelID].levelName, sz_bankSlash, sz_LevelIOPBankName, 0);
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

bool edSoundAreAllSoundDataLoaded()
{
	return _edSoundAreAllSoundDataLoaded(_edSoundLastTransferIndex);
}

bool edMusicAreAllMusicDataLoaded()
{
	return _edSoundAreAllSoundDataLoaded(_edMusicLastTransferIndex);
}

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

void CLevelScheduler::Level_Install()
{
	edCBankInstall SStack32;

	memset(&SStack32, 0, sizeof(edCBankInstall));
	SStack32.fileFlagA = 0;
	this->pLevelBankBufferEntry->install(&SStack32);
	return;
}

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

void CLevelScheduler::Level_ClearAll()
{
	undefined4 uVar1;
	undefined4 uVar2;
	bool bVar3;
	int iVar4;
	undefined4* puVar5;
	undefined4* puVar6;
	ScenarioVariable* pSVar7;
	//SaveStruct_16* pSVar8;
	CChunk* pCVar9;
	int iVar10;
	S_LEVEL_INFO* pSVar11;
	char local_80[128];

	this->pLevelBankBufferEntry->close();
	this->pLevelBankBufferEntry = (edCBankBufferEntry*)0x0;

	this->levelBank.terminate();

	if ((this->field_0x80 | this->field_0x78 | this->field_0x7c) == 0) {
		_gScenVarInfo[1].currentValue = this->currentLevelID;
	}

	this->currentLevelID = 0x10;
	this->field_0x84 = 0.0f;
	this->field_0x88 = 0.0f;
	this->pObjectiveStreamBegin = (int*)0x0;
	this->pObjectiveStreamEnd = (int*)0x0;

	if (this->field_0x78 != 0) {
		MoreLoadLoopObjectSetup(false);

		IMPLEMENTATION_GUARD_LOG(
		Levels_UpdateDataFromSavedGame();)
	}

	if (this->field_0x7c != 0) {
		MoreLoadLoopObjectSetup(false);
		Episode_LoadFromIniFile();

		IMPLEMENTATION_GUARD_LOG(
		pCVar9 = this->pSaveData_0x48;
		pCVar9->field_0x0 = 0x16660666;
		pCVar9->field_0x4 = 0x56415342;
		pCVar9->size = 0x50000;
		pCVar9->offset = 0;
		for (pSVar8 = SaveStruct_16_ARRAY_00433aa0; (iVar10 = *(int*)pSVar8->header, iVar10 != 0x44485342 && (iVar10 != 0))
			; pSVar8 = pSVar8 + 1) {
		}
		if (iVar10 == 0) {
			pSVar8 = (SaveStruct_16*)0x0;
		}
		uVar1 = pSVar8->field_0xc;
		uVar2 = pSVar8->field_0x8;
		iVar10 = (int)&this->aSaveDataArray[this->currentSaveIndex]->field_0x0 +
			this->aSaveDataArray[this->currentSaveIndex]->offset;
		pCVar9 = (CChunk*)(iVar10 + 0x10);
		if (pSVar8->field_0x4 == 0) {
			pCVar9->field_0x0 = 0x16660666;
		}
		else {
			pCVar9->field_0x0 = 0x6667666;
		}
		*(undefined4*)(iVar10 + 0x14) = 0x44485342;
		*(undefined4*)(iVar10 + 0x18) = uVar2;
		*(undefined4*)(iVar10 + 0x1c) = uVar1;
		this->currentSaveIndex = this->currentSaveIndex + 1;
		this->aSaveDataArray[this->currentSaveIndex] = pCVar9;
		if (pCVar9->field_0x0 == 0x6667666) {
			puVar6 = (undefined4*)(iVar10 + 0x20);
		}
		else {
			puVar6 = (undefined4*)0x0;
		}
		*puVar6 = 0x10;
		puVar6[2] = 0;
		puVar6[1] = 0xffffffff;
		puVar6[3] = 0xffffffff;
		puVar6[4] = 0xffffffff;
		iVar10 = *(int*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50);
		*(undefined4*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50) = 0;
		this->currentSaveIndex = this->currentSaveIndex + -1;
		this->aSaveDataArray[this->currentSaveIndex]->offset =
			iVar10 + ((*(int*)(iVar10 + 0xc) + 0x10) - (int)(this->aSaveDataArray[this->currentSaveIndex] + 1));
		for (pSVar8 = SaveStruct_16_ARRAY_00433aa0; (iVar10 = *(int*)pSVar8->header, iVar10 != 0x4e435342 && (iVar10 != 0))
			; pSVar8 = pSVar8 + 1) {
		}
		if (iVar10 == 0) {
			pSVar8 = (SaveStruct_16*)0x0;
		}
		uVar1 = pSVar8->field_0xc;
		uVar2 = pSVar8->field_0x8;
		iVar10 = (int)&this->aSaveDataArray[this->currentSaveIndex]->field_0x0 +
			this->aSaveDataArray[this->currentSaveIndex]->offset;
		pCVar9 = (CChunk*)(iVar10 + 0x10);
		if (pSVar8->field_0x4 == 0) {
			pCVar9->field_0x0 = 0x16660666;
		}
		else {
			pCVar9->field_0x0 = 0x6667666;
		}
		*(undefined4*)(iVar10 + 0x14) = 0x4e435342;
		*(undefined4*)(iVar10 + 0x18) = uVar2;
		*(undefined4*)(iVar10 + 0x1c) = uVar1;
		this->currentSaveIndex = this->currentSaveIndex + 1;
		this->aSaveDataArray[this->currentSaveIndex] = pCVar9;
		puVar6 = (undefined4*)0x0;
		if (pCVar9->field_0x0 == 0x6667666) {
			puVar6 = (undefined4*)(iVar10 + 0x20);
		}
		*puVar6 = 0x62;
		pSVar7 = _gScenVarInfo;
		puVar5 = puVar6;
		iVar10 = 0;
		do {
			iVar4 = iVar10;
			iVar10 = iVar4 + 8;
			puVar5[1] = pSVar7->currentValue;
			puVar5[2] = pSVar7[1].currentValue;
			puVar5[3] = pSVar7[2].currentValue;
			puVar5[4] = pSVar7[3].currentValue;
			puVar5[5] = pSVar7[4].currentValue;
			puVar5[6] = pSVar7[5].currentValue;
			puVar5[7] = pSVar7[6].currentValue;
			puVar5[8] = pSVar7[7].currentValue;
			pSVar7 = pSVar7 + 8;
			puVar5 = puVar5 + 8;
		} while (iVar10 < 0x5a);
		(puVar6 + iVar10)[1] = _gScenVarInfo[iVar10].currentValue;
		(puVar6 + iVar10)[2] = _gScenVarInfo[iVar4 + 9].currentValue;
		iVar10 = *(int*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50);
		*(undefined4*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50) = 0;
		this->currentSaveIndex = this->currentSaveIndex + -1;
		*(int*)(iVar10 + 0xc) = (int)puVar6 + (0x18c - (iVar10 + 0x10));
		this->aSaveDataArray[this->currentSaveIndex]->offset =
			iVar10 + ((*(int*)(iVar10 + 0xc) + 0x10) - (int)(this->aSaveDataArray[this->currentSaveIndex] + 1));
		SaveGame_SaveScenVars(this);
		SaveGame_SaveGameInfo(this);)
	}

	if (this->field_0x80 != 0) {
		edStrCopy(this->levelPath, g_CD_LevelPath_00433bf8);
		/* Router - Set Path */
		bVar3 = gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szSetPath_00433c10, this->levelPath);
		if (bVar3 == false) {
			/* No path found in .INI -> use default !\n */
			edDebugPrintf(g_szNoPathError_00433c20);
		}

		/* Router - Add Level */
		local_80[0] = 0;
		gIniFile.ReadString_001aa520(g_szRouter_00433c08, g_szAddLevel_00433c48, local_80);
		pSVar11 = this->aLevelInfo;
		iVar10 = 0;
		do {
			if (pSVar11->aCompanionInfo != (S_COMPANION_INFO*)0x0) {
				delete[] pSVar11->aCompanionInfo;
				pSVar11->aCompanionInfo = (S_COMPANION_INFO*)0x0;
			}
			if (pSVar11->pSimpleConditionData != (void*)0x0) {
				edMemFree(pSVar11->pSimpleConditionData);
				pSVar11->pSimpleConditionData = 0;
			}

			iVar10 = iVar10 + 1;
			pSVar11 = pSVar11 + 1;
		} while (iVar10 < 0x10);

		MoreLoadLoopObjectSetup(true);
		Levels_LoadInfoBank();
		Episode_LoadFromIniFile();

		IMPLEMENTATION_GUARD_LOG(
		pSVar11 = this->aLevelInfo;
		iVar10 = 0;
		do {
			iVar4 = edStrICmp((byte*)local_80, (byte*)pSVar11->levelName);
			if (iVar4 == 0) goto LAB_002dce40;
			iVar10 = iVar10 + 1;
			pSVar11 = pSVar11 + 1;
		} while (iVar10 < 0x10);
		iVar10 = 0;
	LAB_002dce40:
		this->nextLevelId = iVar10;
		pCVar9 = this->pSaveData_0x48;
		pCVar9->field_0x0 = 0x16660666;
		pCVar9->field_0x4 = 0x56415342;
		pCVar9->size = 0x50000;
		pCVar9->offset = 0;
		for (pSVar8 = SaveStruct_16_ARRAY_00433aa0; (iVar10 = *(int*)pSVar8->header, iVar10 != 0x44485342 && (iVar10 != 0))
			; pSVar8 = pSVar8 + 1) {
		}
		if (iVar10 == 0) {
			pSVar8 = (SaveStruct_16*)0x0;
		}
		uVar1 = pSVar8->field_0xc;
		uVar2 = pSVar8->field_0x8;
		iVar10 = (int)&this->aSaveDataArray[this->currentSaveIndex]->field_0x0 +
			this->aSaveDataArray[this->currentSaveIndex]->offset;
		pCVar9 = (CChunk*)(iVar10 + 0x10);
		if (pSVar8->field_0x4 == 0) {
			pCVar9->field_0x0 = 0x16660666;
		}
		else {
			pCVar9->field_0x0 = 0x6667666;
		}
		*(undefined4*)(iVar10 + 0x14) = 0x44485342;
		*(undefined4*)(iVar10 + 0x18) = uVar2;
		*(undefined4*)(iVar10 + 0x1c) = uVar1;
		this->currentSaveIndex = this->currentSaveIndex + 1;
		this->aSaveDataArray[this->currentSaveIndex] = pCVar9;
		if (pCVar9->field_0x0 == 0x6667666) {
			puVar6 = (undefined4*)(iVar10 + 0x20);
		}
		else {
			puVar6 = (undefined4*)0x0;
		}
		*puVar6 = 0x10;
		puVar6[2] = 0;
		puVar6[1] = 0xffffffff;
		puVar6[3] = 0xffffffff;
		puVar6[4] = 0xffffffff;
		iVar10 = *(int*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50);
		*(undefined4*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50) = 0;
		this->currentSaveIndex = this->currentSaveIndex + -1;
		this->aSaveDataArray[this->currentSaveIndex]->offset =
			iVar10 + ((*(int*)(iVar10 + 0xc) + 0x10) - (int)(this->aSaveDataArray[this->currentSaveIndex] + 1));
		for (pSVar8 = SaveStruct_16_ARRAY_00433aa0; (iVar10 = *(int*)pSVar8->header, iVar10 != 0x4e435342 && (iVar10 != 0))
			; pSVar8 = pSVar8 + 1) {
		}
		if (iVar10 == 0) {
			pSVar8 = (SaveStruct_16*)0x0;
		}
		uVar1 = pSVar8->field_0xc;
		uVar2 = pSVar8->field_0x8;
		iVar10 = (int)&this->aSaveDataArray[this->currentSaveIndex]->field_0x0 +
			this->aSaveDataArray[this->currentSaveIndex]->offset;
		pCVar9 = (CChunk*)(iVar10 + 0x10);
		if (pSVar8->field_0x4 == 0) {
			pCVar9->field_0x0 = 0x16660666;
		}
		else {
			pCVar9->field_0x0 = 0x6667666;
		}
		*(undefined4*)(iVar10 + 0x14) = 0x4e435342;
		*(undefined4*)(iVar10 + 0x18) = uVar2;
		*(undefined4*)(iVar10 + 0x1c) = uVar1;
		this->currentSaveIndex = this->currentSaveIndex + 1;
		this->aSaveDataArray[this->currentSaveIndex] = pCVar9;
		puVar6 = (undefined4*)0x0;
		if (pCVar9->field_0x0 == 0x6667666) {
			puVar6 = (undefined4*)(iVar10 + 0x20);
		}
		*puVar6 = 0x62;
		pSVar7 = _gScenVarInfo;
		puVar5 = puVar6;
		iVar10 = 0;
		do {
			iVar4 = iVar10;
			iVar10 = iVar4 + 8;
			puVar5[1] = pSVar7->currentValue;
			puVar5[2] = pSVar7[1].currentValue;
			puVar5[3] = pSVar7[2].currentValue;
			puVar5[4] = pSVar7[3].currentValue;
			puVar5[5] = pSVar7[4].currentValue;
			puVar5[6] = pSVar7[5].currentValue;
			puVar5[7] = pSVar7[6].currentValue;
			puVar5[8] = pSVar7[7].currentValue;
			pSVar7 = pSVar7 + 8;
			puVar5 = puVar5 + 8;
		} while (iVar10 < 0x5a);
		(puVar6 + iVar10)[1] = _gScenVarInfo[iVar10].currentValue;
		(puVar6 + iVar10)[2] = _gScenVarInfo[iVar4 + 9].currentValue;
		iVar10 = *(int*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50);
		*(undefined4*)(this->levelPath + this->currentSaveIndex * 4 + -8 + 0x50) = 0;
		this->currentSaveIndex = this->currentSaveIndex + -1;
		*(int*)(iVar10 + 0xc) = (int)puVar6 + (0x18c - (iVar10 + 0x10));
		this->aSaveDataArray[this->currentSaveIndex]->offset =
			iVar10 + ((*(int*)(iVar10 + 0xc) + 0x10) - (int)(this->aSaveDataArray[this->currentSaveIndex] + 1));
		SaveGame_SaveScenVars(this);
		SaveGame_SaveGameInfo(this);)
	}

	//FUN_002daf70();

	return;
}

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

	pTVar4 = Timer::GetTimer();
	fVar15 = pTVar4->cutsceneDeltaTime;
	this->field_0x4214 = this->field_0x4214 + pTVar4->lastFrameTime;

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
				pSubObj->FUN_002d8d10(static_cast<CActorNativ*>(*pActor));
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

	if (this->field_0x84 <= 0.0f) {
		return;
	}

	fVar14 = this->field_0x84 - fVar15;
	this->field_0x84 = fVar14;
	if (0.0 < fVar14) {
		return;
	}

	if (((GameFlags & 0x1c) != 0) || (g_CinematicManager_0048efc->FUN_001c5c60() != false)) {
		this->field_0x84 = this->field_0x84 + fVar15;
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
		this->field_0x88 = pTVar4->scaledTotalTime;
	})

LAB_002dc8d8:
	this->field_0x84 = 0.0f;
	return;
}

void CLevelScheduler::Level_ManagePaused()
{
	if ((GameFlags & 0x1c) == 0) {
		Level_Manage();
	}

	return;
}

void CLevelScheduler::Level_Draw()
{
	return;
}

void CLevelScheduler::LevelLoading_End()
{
	this->currentLevelID = this->nextLevelID;
	this->nextLevelID = 0x10;
	this->outroCutsceneId = -1;
	return;
}

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

	this->field_0x84 = 0.0f;

	return;
}

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
			pHero->DoMessage(pHero, (ACTOR_MESSAGE)0x62, (MSG_PARAM)_gScenVarInfo[9].currentValue);
			pHero->DoMessage(pHero->pActorBoomy, (ACTOR_MESSAGE)0x62, (MSG_PARAM)_gScenVarInfo[9].currentValue);
		}

		pHero->DoMessage(pHero, (ACTOR_MESSAGE)0x6b, (MSG_PARAM)_gScenVarInfo[10].currentValue);
		pHero->DoMessage(pHero, (ACTOR_MESSAGE)0x79, 0);
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

void CLevelScheduler::SetLevelTimerFunc_002df450(float param_1, int mode)
{
	Timer* pTVar1;

	if (((this->field_0x88 == 0.0f) || (mode != 0)) || (pTVar1 = Timer::GetTimer(), 180.0f < (param_1 + pTVar1->scaledTotalTime) - this->field_0x88)) {
		if (param_1 == 0.0f) {
			param_1 = 0.001f;
		}

		if (this->field_0x84 < param_1) {
			this->field_0x84 = param_1;
		}

		if (mode != 0) {
			this->field_0x88 = 0.0f;
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
					if ((pcVar5->field_0x8 != 0xffffffff) &&
						(this_00 = (CActorTeleporter*)pActorManager->GetActorByHashcode(pcVar5->field_0x8), this_00 != (CActorTeleporter*)0x0)) {
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

	this->field_0x7c = 1;

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
			pTeleporter != (CActorTeleporter*)0x0 && (pTeleporter->field_0x2a0 != 0)) &&
			(pTeleporter->UpdateCurTeleporterState(levelId, param_4),
				(CActorHero::_gThis->flags & 0x800000) == 0)))) {
		pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
		pCameraManager->Level_Reset();
		pCameraManager->SetMainCamera(CActorHero::_gThis->pMainCamera);
		pCameraManager->AlertCamera(2, 1);
	}

	return;
}

CChunk* CChunk::FindNextSubChunk(CChunk* pChunk, uint param_3)
{
	for (; (pChunk < (CChunk*)((char*)&this[1].field_0x0 + this->offset) && (param_3 != pChunk->field_0x4));
		pChunk = (CChunk*)((char*)&pChunk[1].field_0x0 + pChunk->offset)) {
	}
	if (((CChunk*)((char*)&this[1].field_0x0 + this->offset) <= pChunk) || (param_3 != pChunk->field_0x4)) {
		pChunk = (CChunk*)0x0;
	}
	return pChunk;
}
