#include "LevelScheduleManager.h"
#include "IniFile.h"
#include "edMem.h"
#include "edC/edCBank.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#include <libvu0.h>
#endif

#include "edSystem.h"
#include "MemoryStream.h"
#include "ed3D.h"
#include "FileManager3D.h"
#include "edStr.h"
#include "CinematicManager.h"
#include "ScenaricCondition.h"
#include "WayPoint.h"
#include "PathManager.h"
#include "ActorManager.h"
#include "SectorManager.h"
#include "DlistManager.h"


LevelScheduleManager* LevelScheduleManager::gThis = NULL;

const char* g_CD_LevelPath_00433bf8 = "CDEURO/Level/";
const char* g_szRouter_00433c08 = "Router";
const char* g_szSetPath_00433c10 = "SetPath";
const char* g_szAddLevel_00433c48 = "AddLevel";
const char* g_szNoPathError_00433c20 = "No path found in .INI -> use default !\n";

LevelScheduleManager::LevelScheduleManager()
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

void LevelScheduleManager::SetLevelToLoad_002dba90(int levelID, int elevatorID, int param_4)
{
	undefined* puVar1;
	int iVar2;
	uint uVar3;
	LevelInfo* pcVar3;

	if (aLevelInfo[levelID].levelName[0] == '\0') {
		levelID = field_0x4;
		elevatorID = -1;
	}
	nextLevelID = levelID;
	level_0x5b3c = -1;
	nextElevatorID = elevatorID;
	pcVar3 = &aLevelInfo[levelID];
	level_0x5b40 = pcVar3->sectorStartIndex;
	currentElevatorID = -1;
	level_0x5b50 = 0x10;
	level_0x5b54 = -1;
	if (field_0x78 == 0) {
		// Check maxElevatorID_0xa8 and field_0xf4
		iVar2 = nextElevatorID;
		if (((iVar2 != -1) && (-1 < iVar2)) && (iVar2 < pcVar3->maxElevatorID_0xa8)) {
			IMPLEMENTATION_GUARD(
			iVar2 = *(int*)(&pcVar3->field_0xf4 + iVar2 * 0x28);
			if (iVar2 != -1) {
				level_0x5b40 = iVar2;
			}
			level_0x5b50 = currentLevelID;
			level_0x5b54 = param_4;)
		}
	}
	else {
		puVar1 = *(undefined**)(aLevelInfo[0].levelName + currentSaveIndex * 4 + -100);
		uVar3 = SearchForSection_002e3bf0(puVar1, (uint)(puVar1 + 0x10), 0x44485342);
		if (uVar3 != 0) {
			currentSaveIndex = currentSaveIndex + 1;
			*(uint*)(aLevelInfo[0].levelName + currentSaveIndex * 4 + -100) = uVar3;
		}
		level_0x5b40 = *(int*)(uVar3 + 0x14);
		*(undefined4*)(aLevelInfo[0].levelName + currentSaveIndex * 4 + -100) = 0;
		currentSaveIndex = currentSaveIndex + -1;
	}
	return;
}

void LevelScheduleManager::Level_LoadObjectives(ByteCode* pMemoryStream)
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

void SetupLevelInfo_002d97c0(LevelInfo* pLevelInfo, bool param_2)
{
	LoadLoopObject_418_18* pLVar1;
	int iVar2;
	undefined8* puVar3;

	/* This will clear our current level name */
	if (param_2 != false) {
		pLevelInfo->field_0x0 = 0;
		pLevelInfo->bankSizeLevel = 0;
		pLevelInfo->bankSizeSect = 0;
		pLevelInfo->sectorCount_0x14 = 0;
		pLevelInfo->field_0x18 = 0;
		pLevelInfo->field_0x20 = 0;
		pLevelInfo->levelName[0] = '\0';
		pLevelInfo->field_0x48 = 0;
		pLevelInfo->field_0x4c = 0;
	}
	pLevelInfo->field_0x50 = 0;
	//puVar3 = &pLevelInfo->field_0x58;
	//iVar2 = 0;
	//do {
	//	if (param_2 != false) {
	//		*puVar3 = 0;
	//		*(undefined4*)(puVar3 + 1) = 0xffffffff;
	//		*(undefined4*)((int)puVar3 + 0xc) = 0xffffffff;
	//		*(undefined4*)(puVar3 + 2) = 0;
	//		*(undefined4*)((int)puVar3 + 0x14) = 0;
	//	}
	//	*(undefined4*)(puVar3 + 3) = 0;
	//	*(undefined4*)((int)puVar3 + 0x1c) = 0;
	//	*(undefined4*)(puVar3 + 4) = 0;
	//	if ((*(uint*)(puVar3 + 2) & 1) != 0) {
	//		*(uint*)((int)puVar3 + 0x1c) = *(uint*)((int)puVar3 + 0x1c) | 1;
	//	}
	//	iVar2 = iVar2 + 1;
	//	puVar3 = puVar3 + 5;
	//} while (iVar2 < 0xc);
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

void LevelScheduleManager::MoreLoadLoopObjectSetup(bool param_2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	//MapManager* pMVar4;
	undefined4* puVar5;
	LoadLoopObject_50* pLVar6;
	int iVar7;
	LevelInfo* pLVar8;

	iVar7 = 0;
	pLVar8 = aLevelInfo;
	do {
		SetupLevelInfo_002d97c0(pLVar8, param_2);
		iVar7 = iVar7 + 1;
		pLVar8 = pLVar8 + 1;
	} while (iVar7 < 0x10);
	if (param_2 != false) {
		field_0x4210 = 0;
		objCount_0x4218 = 0;
		field_0x5620 = 0;
	}
	field_0x5b30 = 0;
	*(undefined4*)&field_0x4214 = 0;
	iVar7 = 0;
	//puVar5 = &DAT_004253d0;
	//do {
	//	iVar7 = iVar7 + 8;
	//	puVar5[1] = *puVar5;
	//	puVar5[3] = puVar5[2];
	//	puVar5[5] = puVar5[4];
	//	puVar5[7] = puVar5[6];
	//	puVar5[9] = puVar5[8];
	//	puVar5[0xb] = puVar5[10];
	//	puVar5[0xd] = puVar5[0xc];
	//	puVar5[0xf] = puVar5[0xe];
	//	puVar5 = puVar5 + 0x10;
	//} while (iVar7 < 0x5a);
	//pLVar6 = field_0x4220;
	//(&DAT_004253d4)[iVar7 * 2] = (&DAT_004253d0)[iVar7 * 2];
	//(&DAT_004253dc)[iVar7 * 2] = (&DAT_004253d8)[iVar7 * 2];
	//iVar7 = 0;
	//if (0 < objCount_0x4218) {
	//	do {
	//		pLVar6->field_0x24 = 0x20;
	//		pLVar6->field_0x28 = 0;
	//		iVar7 = iVar7 + 1;
	//		pLVar6->field_0x2c = 0;
	//		pLVar6->field_0x40 = 0;
	//		fVar3 = sceVu0FVECTOR_00431710.w;
	//		fVar2 = sceVu0FVECTOR_00431710.z;
	//		fVar1 = sceVu0FVECTOR_00431710.y;
	//		pLVar6->field_0x30 = sceVu0FVECTOR_00431710.x;
	//		pLVar6->field_0x34 = fVar1;
	//		pLVar6->field_0x38 = fVar2;
	//		pLVar6->field_0x3c = fVar3;
	//		pLVar6 = pLVar6 + 1;
	//	} while (iVar7 < objCount_0x4218);
	//}
	//puVar5 = (undefined4*)&field_0x5630;
	//iVar7 = 0;
	//if (0 < field_0x5620) {
	//	do {
	//		if (param_2 != false) {
	//			*puVar5 = 0xffffffff;
	//			puVar5[1] = 0x10;
	//			puVar5[2] = 0;
	//			fVar3 = sceVu0FVECTOR_00431710.w;
	//			fVar2 = sceVu0FVECTOR_00431710.z;
	//			fVar1 = sceVu0FVECTOR_00431710.y;
	//			puVar5[4] = sceVu0FVECTOR_00431710.x;
	//			puVar5[5] = fVar1;
	//			puVar5[6] = fVar2;
	//			puVar5[7] = fVar3;
	//			fVar3 = g_ZVector.w;
	//			fVar2 = g_ZVector.z;
	//			fVar1 = g_ZVector.y;
	//			puVar5[8] = g_ZVector.x;
	//			puVar5[9] = fVar1;
	//			puVar5[10] = fVar2;
	//			puVar5[0xb] = fVar3;
	//			puVar5[0xc] = 0x20;
	//			puVar5[0xd] = 0;
	//			*(undefined8*)(puVar5 + 0xe) = 0;
	//			puVar5[0x10] = 0x20;
	//			puVar5[0x11] = 0;
	//			*(undefined8*)(puVar5 + 0x12) = 0;
	//			puVar5[0x14] = 0x20;
	//			puVar5[0x15] = 0;
	//			*(undefined8*)(puVar5 + 0x16) = 0;
	//			puVar5[0x18] = 0x20;
	//			puVar5[0x19] = 0;
	//			*(undefined8*)(puVar5 + 0x1a) = 0;
	//			puVar5[0x1c] = 0x20;
	//			puVar5[0x1d] = 0;
	//			*(undefined8*)(puVar5 + 0x1e) = 0;
	//		}
	//		iVar7 = iVar7 + 1;
	//		puVar5 = puVar5 + 0x20;
	//	} while (iVar7 < field_0x5620);
	//}
	//pMVar4 = (MapManager*)GetManagerObject(MO_Map);
	//MapManager::SetupFunc_003f8150(pMVar4);
	return;
}

TypePairData g_LevelInfoTypePairData_004256e0 = { -1, -1, 0x0, 0, 0, 0, 0, 0 };
char* g_szLevelInfoBnkPath_00433c60 = "Info/levels.bnk";

void LevelScheduleManager::LevelsInfo_ReadHeader_V7_V9(char* fileData, LevelInfo* pLevelInfo)
{
	ulong uVar1;
	int iVar2;
	SectorManagerSubObj* pLVar3;

	uVar1 = ByteCode::BuildU64(*(int*)(fileData + 0x18), *(int*)(fileData + 0x1c));
	pLevelInfo->field_0x0 = uVar1;
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
		pLVar3->pFileData = (undefined*)0x0;
		pLVar3->flags = 0;
		iVar2 = iVar2 + 6;
		pLVar3->pWindSectorObj = (CSectorHierarchy*)0x0;
		pLVar3->field_0xc = 0;
		pLVar3[1].pFileData = (undefined*)0x0;
		pLVar3[1].flags = 0;
		pLVar3[1].pWindSectorObj = (CSectorHierarchy*)0x0;
		pLVar3[1].field_0xc = 0;
		pLVar3[2].pFileData = (undefined*)0x0;
		pLVar3[2].flags = 0;
		pLVar3[2].pWindSectorObj = (CSectorHierarchy*)0x0;
		pLVar3[2].field_0xc = 0;
		pLVar3[3].pFileData = (undefined*)0x0;
		pLVar3[3].flags = 0;
		pLVar3[3].pWindSectorObj = (CSectorHierarchy*)0x0;
		pLVar3[3].field_0xc = 0;
		pLVar3[4].pFileData = (undefined*)0x0;
		pLVar3[4].flags = 0;
		pLVar3[4].pWindSectorObj = (CSectorHierarchy*)0x0;
		pLVar3[4].field_0xc = 0;
		pLVar3[5].pFileData = (undefined*)0x0;
		pLVar3[5].flags = 0;
		pLVar3[5].pWindSectorObj = (CSectorHierarchy*)0x0;
		pLVar3[5].field_0xc = 0;
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

uint* LevelScheduleManager::LevelsInfo_ReadSectors_V7_V9(uint* pFileBuffer, int count, LevelInfo* pLevelInfo)
{
	uint uVar1;
	uint uVar2;
	LevelInfo* pLVar3;
	int iVar4;
	uint* puVar5;
	int* piVar6;
	void* pvVar7;
	void* __src;
	SectorManagerSubObj* pcVar8;
	int iVar9;
	int iVar10;
	uint* puVar11;
	uint uVar12;
	int iVar13;
	uint* local_10;
	int* local_8;
	ScenaricCondition local_4;

	iVar13 = 0;
	uVar12 = 0;
	puVar5 = pFileBuffer;
	iVar10 = count;
	if (count != 0) {
		do {
			uVar1 = *puVar5;
			if (pLevelInfo->sectorCount_0x14 < (int)uVar1) {
				pLevelInfo->sectorCount_0x14 = uVar1;
			}
			pcVar8 = &pLevelInfo->aSectorSubObj[uVar1];
			puVar11 = puVar5 + 3;
			pcVar8->pFileData = (undefined*)puVar5[1];
			uVar1 = puVar5[2];
			iVar9 = 0;
			puVar5 = puVar11;
			if (0 < (int)uVar1) {
				do {
					uVar2 = *puVar5;
					memset(&local_4, 0, sizeof(ScenaricCondition));
					local_4.Create((char*)(puVar5 + 1));
					if ((0 < (int)uVar2) && ((int)uVar2 < 0x1e)) {
						pcVar8->flags = pcVar8->flags | 1 << (uVar2 & 0x1f);
						iVar4 = local_4.GetNumSimpleConds();
						if (iVar4 != 0) {
							uVar12 = uVar12 + 1;
							pcVar8->pWindSectorObj = (CSectorHierarchy*)((int)pcVar8->pWindSectorObj + 1);
							iVar4 = local_4.GetDataSize();
							iVar13 = iVar13 + iVar4;
						}
					}
					puVar5 = local_4.GetEndPtr();
					iVar9 = iVar9 + 1;
				} while (iVar9 < (int)uVar1);
			}
			iVar10 = iVar10 + -1;
		} while (iVar10 != 0);
	}
	if (uVar12 != 0) {
		//IMPLEMENTATION_GUARD(
		//piVar6 = (int*)AllocateFunc_001002a0((long)(int)(uVar12 * 8 + 0x10));
		//piVar6 = RunActorInitFunctions_00217580(piVar6, (ActorConstructorA*)&LAB_002e1a80, Destructor_002dd1c0, 8, uVar12);
		//pLevelInfo->field_0x48 = piVar6;
		//pvVar7 = edMemAlloc(H_MAIN, iVar13);
		//pLevelInfo->field_0x4c = pvVar7;
		//pvVar7 = (void*)pLevelInfo->field_0x4c;
		//puVar11 = (uint*)pLevelInfo->field_0x48;
		//local_10 = pFileBuffer;
		//if (count != 0) {
		//	do {
		//		if (*(int*)(pLevelInfo->levelName + *local_10 * 0x10 + -0x24 + 0x240) != 0) {
		//			*(uint**)(pLevelInfo->levelName + *local_10 * 0x10 + -0x24 + 0x244) = puVar11;
		//		}
		//		iVar10 = 0;
		//		uVar12 = local_10[2];
		//		local_10 = local_10 + 3;
		//		if (0 < (int)uVar12) {
		//			do {
		//				uVar1 = *local_10;
		//				local_8 = (int*)0x0;
		//				SetDereferenceParam1ToParam2(&local_8, local_10 + 1);
		//				if ((0 < (int)uVar1) && ((int)uVar1 < 0x1e)) {
		//					iVar13 = FUN_001173d0((int*)&local_8);
		//					if (iVar13 != 0) {
		//						iVar13 = FUN_001173f0(&local_8);
		//						__src = (void*)GetDereferenceParam1(&local_8);
		//						memcpy(pvVar7, __src, iVar13);
		//						*puVar11 = uVar1;
		//						SetDereferenceParam1ToParam2(puVar11 + 1, pvVar7);
		//						pvVar7 = (void*)((int)pvVar7 + iVar13);
		//						puVar11 = puVar11 + 2;
		//					}
		//				}
		//				local_10 = (uint*)SeekForward_00117410(&local_8);
		//				iVar10 = iVar10 + 1;
		//			} while (iVar10 < (int)uVar12);
		//		}
		//		count = count + -1;
		//	} while (count != 0);
		//})
	}
	if ((pLevelInfo->sectorStartIndex == -1) ||
		(pLevelInfo->aSectorSubObj[pLevelInfo->sectorStartIndex].pFileData == (undefined*)0x0)) {
		iVar10 = 1;
		pLVar3 = pLevelInfo;
		while ((iVar10 <= pLevelInfo->sectorCount_0x14 &&
			(pLVar3->aSectorSubObj[iVar10].pFileData == (undefined*)0x0))) {
			iVar10 = iVar10 + 1;
		}
		if (pLevelInfo->sectorCount_0x14 < iVar10) {
			pLevelInfo->sectorStartIndex = -1;
		}
		else {
			pLevelInfo->sectorStartIndex = iVar10;
		}
	}
	return puVar5;
}

void LevelScheduleManager::LevelsInfo_ReadTeleporters_V7_V9(char* pFileData, int count, LevelInfo* pLevelInfo)
{
	int iVar1;
	undefined8* puVar2;
	char* pcVar3;
	int iVar4;

	pLevelInfo->field_0x18 = 0;
	if (count != 0) {
		do {
			iVar1 = *(int*)(pFileData + 8);
			if (iVar1 < 0) {
				iVar1 = 0;
			}
			if (pLevelInfo->field_0x18 <= iVar1) {
				pLevelInfo->field_0x18 = iVar1 + 1;
			}
			count = count + -1;
			pcVar3 = pLevelInfo->levelName + iVar1 * 0x28 + -0x24;
			*(undefined8*)(pcVar3 + 0x58) = *(undefined8*)(pFileData + 0x14);
			*(undefined4*)(pcVar3 + 0x60) = *(undefined4*)pFileData;
			*(undefined4*)(pcVar3 + 100) = *(undefined4*)(pFileData + 4);
			*(undefined4*)(pcVar3 + 0x68) = *(undefined4*)(pFileData + 0xc);
			*(undefined4*)(pcVar3 + 0x6c) = *(undefined4*)(pFileData + 0x10);
			pFileData = (char*)(pFileData + 0x1c);
		} while (count != 0);
	}
	iVar4 = 0;
	puVar2 = &pLevelInfo->field_0x58;
	iVar1 = 0;
	if (0 < pLevelInfo->field_0x18) {
		do {
			iVar4 = iVar4 + *(int*)(puVar2 + 0x14);
			if ((*(uint*)(puVar2 + 2) & 1) != 0) {
				*(uint*)(puVar2 + 0x1c) = *(uint*)(puVar2 + 0x1c) | 1;
			}
			iVar1 = iVar1 + 1;
			puVar2 = puVar2 + 5;
		} while (iVar1 < pLevelInfo->field_0x18);
	}
	pLevelInfo->field_0x6c = pLevelInfo->field_0x6c + (pLevelInfo->field_0x20 - iVar4);
	pLevelInfo->field_0x20 = pLevelInfo->field_0x20 - pLevelInfo->field_0x6c;
	return;
}

void LevelScheduleManager::Levels_LoadInfoBank()
{
	MY_LOG("LevelScheduleManager::Levels_LoadInfoBank\n");

	char cVar1;
	int iVar2;
	undefined4* puVar3;
	bool bVar4;
	edCBankBufferEntry* infoLevelsFileBuffer;
	int inFileIndex;
	uint* puVar5;
	LevelInfo* pLevelInfo;
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
	bank.bank_buffer_setcb(&g_LevelInfoTypePairData_004256e0);
	/* CDEURO/LEVEL/ + Info/levels.bnk */
	levelInfoFilePath[0] = '\0';
	edStrCatMulti(levelInfoFilePath, levelPath, "Info/levels.bnk", 0);
	infoLevelsFileBuffer = bank.get_free_entry();
	infoLevelsPathPtr.filePath = levelInfoFilePath;
	bVar4 = infoLevelsFileBuffer->load(&infoLevelsPathPtr);
	if (bVar4 != false) {
		inFileIndex = infoLevelsFileBuffer->get_element_count();
		while (inFileIndex != 0) {
			inFileIndex = inFileIndex + -1;
			bVar4 = infoLevelsFileBuffer->get_info(inFileIndex, &outHeader, (char*)0x0);
			puVar3 = (undefined4*)outHeader.fileBufferStart;
			if (bVar4 != false) {
				fileData = (int*)(outHeader.fileBufferStart + 4);
				if (true) {
					switch (*(undefined4*)outHeader.fileBufferStart) {
					case 9:
						iVar2 = *fileData;
						if ((-1 < iVar2) && (iVar2 < 0x10)) {
							pLevelInfo = &aLevelInfo[iVar2];
							LevelsInfo_ReadHeader_V7_V9((char*)fileData, pLevelInfo);
							LevelsInfo_ReadTeleporters_V7_V9((char*)(puVar3 + 0xf), puVar3[0xc], pLevelInfo);
							puVar5 = LevelsInfo_ReadSectors_V7_V9(puVar3 + 0xf + puVar3[0xc] * 7, puVar3[10], pLevelInfo);
							//LevelsInfo_ReadLanguageFileNames_V7_V9(this, puVar5, puVar3[0xe], *fileData);
							//iVar9 = puVar3[0xd];
							//puVar5 = puVar5 + puVar3[0xe] * 7;
							//if (1 < iVar9) {
							//	iVar9 = 1;
							//}
							//iVar8 = 0;
							//if (0 < iVar9) {
							//	do {
							//		iVar6 = 0;
							//		do {
							//			iVar7 = iVar6;
							//			if (iVar6 < 0x14) {
							//				iVar7 = iVar6 + 1;
							//				levelNameHolderObj->levelPath[iVar6] = *(char*)puVar5;
							//			}
							//			cVar1 = *(char*)puVar5;
							//			puVar5 = (uint*)((int)puVar5 + 1);
							//			iVar6 = iVar7;
							//		} while (cVar1 != '\0');
							//		iVar8 = iVar8 + 1;
							//		levelNameHolderObj = (LevelInfo*)&levelNameHolderObj->field_0x14;
							//	} while (iVar8 < iVar9);
							//}
							//this->field_0x4210 = this->field_0x4210 + *(int*)(this->levelPath + iVar2 * 0x418 + -8 + 0xb0);
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

int LevelScheduleManager::SaveGame_GetMaxBufferSize()
{
	return 0x10000;
}

int INT_ARRAY_0048ed60[16] = { 0 };
int INT_ARRAY_0048eda0[16] = { 0 };

void LevelScheduleManager::Game_Init()
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
	LevelInfo* pLVar12;
	char local_80[128];

	lVar2 = (SaveBigAlloc*)edMemAlloc(TO_HEAP(H_MAIN), 0x10000);
	/* Zero out most of load loop */
	pSaveData_0x48 = lVar2;
	pSaveDataEnd_0x4c = (int)(pSaveData_0x48 + 1);
	memset(pSaveData_0x48, 0, 0x10000);
	aSaveDataArray[0] = (SaveBigAlloc*)0x0;
	aSaveDataArray[1] = (SaveBigAlloc*)0x0;
	aSaveDataArray[2] = (SaveBigAlloc*)0x0;
	aSaveDataArray[3] = (SaveBigAlloc*)0x0;
	pLVar12 = aLevelInfo;
	aSaveDataArray[4] = (SaveBigAlloc*)0x0;
	iVar11 = 0;
	aSaveDataArray[5] = (SaveBigAlloc*)0x0;
	aSaveDataArray[6] = (SaveBigAlloc*)0x0;
	aSaveDataArray[7] = (SaveBigAlloc*)0x0;
	currentSaveIndex = 0;
	aSaveDataArray[currentSaveIndex] = pSaveData_0x48;
	pObjectiveStreamBegin = 0;
	pObjectiveStreamEnd = 0;
	field_0x74 = 0;
	field_0x78 = 0;
	field_0x7c = 0;
	field_0x80 = 0;
	(levelBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
	(levelIOPBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
	loadStage_0x5b48 = 4;
	field_0x84 = 0.0;
	field_0x88 = 0.0;
	field_0x4 = 0;
	currentLevelID = 0x10;
	currentElevatorID = -1;
	level_0x5b50 = 0x10;
	level_0x5b54 = -1;
	do {
		pLVar12->field_0x48 = 0;
		pLVar12->field_0x4c = 0;
		iVar11 = iVar11 + 8;
		pLVar12[1].field_0x48 = 0;
		pLVar12[1].field_0x4c = 0;
		pLVar12[2].field_0x48 = 0;
		pLVar12[2].field_0x4c = 0;
		pLVar12[3].field_0x48 = 0;
		pLVar12[3].field_0x4c = 0;
		pLVar12[4].field_0x48 = 0;
		pLVar12[4].field_0x4c = 0;
		pLVar12[5].field_0x48 = 0;
		pLVar12[5].field_0x4c = 0;
		pLVar12[6].field_0x48 = 0;
		pLVar12[6].field_0x4c = 0;
		pLVar12[7].field_0x48 = 0;
		pLVar12[7].field_0x4c = 0;
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
	bVar2 = g_IniFile_00450750.ReadString_001aa520(g_szRouter_00433c08, g_szSetPath_00433c10, levelPath);
	if (bVar2 == false) {
		/* No path found in .INI -> use default !\n */
		edDebugPrintf(g_szNoPathError_00433c20);
	}
	/* Try read [Router] - AddLevel from INI file  */
	local_80[0] = '\0';
	g_IniFile_00450750.ReadString_001aa520(g_szRouter_00433c08, g_szAddLevel_00433c48, local_80);
	pLVar12 = aLevelInfo;
	iVar11 = 0;
	do {
		if ((undefined*)pLVar12->field_0x48 != (undefined*)0x0) {
			//FreeArray_00217340((undefined*)pLVar12->field_0x48, Destructor_002dd1c0);
			pLVar12->field_0x48 = 0;
		}
		if ((void*)pLVar12->field_0x4c != (void*)0x0) {
			edMemFree((void*)pLVar12->field_0x4c);
			pLVar12->field_0x4c = 0;
		}
		iVar11 = iVar11 + 1;
		pLVar12 = pLVar12 + 1;
	} while (iVar11 < 0x10);
	MoreLoadLoopObjectSetup(true);
	Levels_LoadInfoBank();
	//Episode_LoadFromIniFile();
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
	field_0x4 = iVar11;
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
		SetLevelToLoad_002dba90(field_0x4, -1, -1);
	}
	else {
		SetLevelToLoad_002dba90(0xe, -1, -1);
	}
	return;
}

void BnkInstallScene(char* pFileData, int size)
{
	ByteCode byteCode;

	/* Origin: 0040e860 */
	byteCode.Init(pFileData);
	byteCode.GetChunk();
	CScene::ptable.g_FileManager3D_00451664->Level_AddAll(&byteCode);
	CScene::_pinstance->Level_Setup(&byteCode);
	CScene::ptable.g_WayPointManager_0045169c->Level_AddAll(&byteCode);
	CScene::ptable.g_PathManager_004516a0->Level_AddAll(&byteCode);
	//(*(code*)(Scene::ptable.g_CollisionManager_00451690)->pManagerFunctionData->Level_AddAll)();
	CScene::ptable.g_ActorManager_004516a4->Level_AddAll(&byteCode);
	CScene::ptable.g_SectorManager_00451670->Level_AddAll(&byteCode);
	if (CScene::ptable.g_GlobalDListManager_004516bc != (CGlobalDListManager*)0x0) {
		CScene::ptable.g_GlobalDListManager_004516bc->Level_Create();
	}
	byteCode.Term();
	return;
}

void BnkInstallSceneCfg(char* pFileData, int size)
{
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
	CScene::ptable.g_FileManager3D_00451664->Level_Create(&byteCode);
	//CCollisionManager::Level_Create(Scene::ptable.g_CollisionManager_00451690, &MStack16);
	//CAnimManager::Level_Create(Scene::ptable.g_AnimManager_00451668, &MStack16);
	byteCode.GetU32();
	byteCode.GetU32();
	CScene::ptable.g_SectorManager_00451670->Level_Create(&byteCode);
	//CActorManager::Level_LoadClassesInfo(Scene::ptable.g_ActorManager_004516a4, &MStack16);
	//CCollisionManager::Level_PostCreate(Scene::ptable.g_CollisionManager_00451690);
	//CFxParticleManager::Level_Create(&BStack16);
	//ByteCodeDestructor(&MStack16, -1);
	return;
}

void OnSoundLoaded_00180ef0(char* pFileData, int param_2)
{
	MY_LOG("MISSING HANDLER OnSoundLoaded_00180ef0\n");
}

void BnkInstallG2D(char* pFileData, int length)
{
	FileManager3D* pFVar1;
	int iStack4;

	pFVar1 = CScene::ptable.g_FileManager3D_00451664;
	ed3DInstallG2D
	(pFileData, length, &iStack4,
		(ed_g2d_manager*)
		((CScene::ptable.g_FileManager3D_00451664)->pTextureInfoArray +
			(CScene::ptable.g_FileManager3D_00451664)->textureLoadedCount), 1);
	pFVar1->pTextureInfoArray[pFVar1->textureLoadedCount].pFileBuffer = pFileData;
	pFVar1->textureLoadedCount = pFVar1->textureLoadedCount + 1;
	return;
}

void OnMeshLoaded_001a6380(char* pFileData, int length)
{
	int iVar1;
	Mesh* pMVar2;
	FileManager3D* pFVar3;

	pFVar3 = CScene::ptable.g_FileManager3D_00451664;
	iVar1 = (CScene::ptable.g_FileManager3D_00451664)->meshLoadedCount;
	pMVar2 = (CScene::ptable.g_FileManager3D_00451664)->pMeshDataArray;
	pMVar2[iVar1].fileLength = length;
	pMVar2[iVar1].pFileData = pFileData;
	pFVar3->meshLoadedCount = pFVar3->meshLoadedCount + 1;
	return;
}

void OnLoadedFunc_00211480(char* pFileBuffer, int length)
{
	MY_LOG("MISSING HANDLER OnLoadedFunc_00211480\n");
	//CollisionManager* pCVar1;
	//int iVar2;
	//
	//pCVar1 = Scene::ptable.g_CollisionManager_00451690;
	//iVar2 = LoadCollision_00251570(pFileBuffer, length, 0);
	//*(undefined4*)(&pCVar1->field_0x20 + pCVar1->count_0x60 * 4) = *(undefined4*)(iVar2 + 0x44);
	//pCVar1->count_0x60 = pCVar1->count_0x60 + 1;
	return;
}

void OnEventLoaded_0019e750(char* pFileData, int length)
{
	MY_LOG("MISSING HANDLER OnEventLoaded_0019e750\n");
	//EventManager* pEVar1;
	//uint* pFileData_00;
	//uint uVar2;
	//undefined4 uVar3;
	//MemoryStream MStack16;
	//
	//MemoryStream::MemoryStream(&MStack16);
	//MemoryStream::Init(&MStack16, pFileData);
	//MemoryStream:::Func_00189a90(&MStack16);
	//pEVar1 = Scene::ptable.g_EventManager_006f5080;
	//pFileData_00 = (uint*)MemoryStream:::GetPosition(&MStack16);
	//uVar2 = edEvent::AddChunk_002590c0(pFileData_00, 0);
	//pEVar1->activeEventChunkID_0x8 = uVar2;
	//uVar3 = edEvent::GetEventObjCount_00259610(pEVar1->activeEventChunkID_0x8);
	//pEVar1->field_0x4 = uVar3;
	//edEvent::SetFunctionData_0025a0c0((int*)edEvent::FunctionDataA_0040eba0);
	//::EmptyFunction();
	//ByteCodeDestructor(&MStack16, -1);
	return;
}

void OnViewLoaded_0019a9e0(char* pFileData, int length)
{
	ByteCode MStack16;

	MStack16.Init(pFileData);
	MStack16.GetChunk();

	MY_LOG("MISSING HANDLER OnViewLoaded_0019a9e0\n");
	//(*(code*)g_CameraViewManager_00448e98->pManagerFunctionData->deserializeFunc)();
	//::EmptyFunction();
	//ByteCodeDestructor(&MStack16, -1);
	return;
}

void OnLightLoaded_002172d0(char* pFileData, int length)
{
	ByteCode MStack16;

	MStack16.Init(pFileData);
	MStack16.GetChunk();
	MY_LOG("MISSING HANDLER OnLightLoaded_002172d0\n");
	//(*(code*)(Scene::ptable.g_LightManager_004516b0)->pManagerFunctionData->deserializeFunc)();
	//::EmptyFunction();
	//ByteCodeDestructor(&MStack16, -1);
	return;
}

void OnEffectLoaded_001a0870(char* pFileData, int length)
{
	ByteCode MStack16;
	MY_LOG("MISSING HANDLER OnEffectLoaded_001a0870\n");

	MStack16.Init(pFileData);
	//(*(code*)(Scene::ptable.g_EffectsManager_004516b8)->pManagerFunctionData->deserializeFunc)();
	//::EmptyFunction();
	//ByteCodeDestructor(&MStack16, -1);
	return;
}

void BnkInstallCinematic(char* pFileData, int length)
{
	ByteCode MStack16;
	MY_LOG("BnkInstallCinematic\n");

	MStack16.Init(pFileData);
	g_CinematicManager_0048efc->Level_AddAll(&MStack16);
	MStack16.Term();
	return;
}

void NullTypePairFunc(char* pFileData, int param_2)
{
	IMPLEMENTATION_GUARD();
}

TypePairData TableBankCallback[24] = {
	{ 0x02, 1, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x02, 2, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x03, 4, { OnSoundLoaded_00180ef0, 0, 0, 0, 0, 0 } },
	{ 0x03, 1, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x03, 5, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x03, 6, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x03, 7, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x06, 1, { BnkInstallScene, 0, 0, 0, 0, 0 } },
	{ 0x06, 2, { BnkInstallSceneCfg, 0, 0, 0, 0, 0 } },
	{ 0x04, 1, { OnMeshLoaded_001a6380, 0, 0, 0, 0, 0 } },
	{ 0x05, 1, { BnkInstallG2D, 0, 0, 0, 0, 0 } },
	{ 0x07, 1, { OnLoadedFunc_00211480, 0, 0, 0, 0, 0 } },
	{ 0x07, 2, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x08, 1, { OnEventLoaded_0019e750, 0, 0, 0, 0, 0 } },
	{ 0x0A, 1, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x0B, 1, { OnViewLoaded_0019a9e0, 0, 0, 0, 0, 0 } },
	{ 0x0C, 1, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x0F, 1, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x10, 1, { OnLightLoaded_002172d0, 0, 0, 0, 0, 0 } },
	{ 0x11, 1, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0x09, 1, { OnEffectLoaded_001a0870, 0, 0, 0, 0, 0 } },
	{ 0x13, 1, { BnkInstallCinematic, 0, 0, 0, 0, 0 } },
	{ 0x09, 2, { NullTypePairFunc, 0, 0, 0, 0, 0 } },
	{ 0xFFFFFFFF, 0xFFFFFFFF, { NULL, 0, 0, 0, 0, 0 } },
};

void WillLoadFileFromBank(struct GlobalSound_00451698* param_1, edCBankBufferEntry* pBankBuffer)
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
	LevelScheduleManager* pLVar1;

	pLVar1 = LevelScheduleManager::gThis;
	if (param_1 != false) {
		WillLoadFileFromBank
		(CScene::ptable.g_GlobalSoundPtr_00451698,
			(LevelScheduleManager::gThis->levelIOPBank).pBankFileAccessObject);
		pLVar1->loadStage_0x5b48 = 1;
	}
	return;
}

const char* sz_bankSlash = "/";
const char* sz_LevelBank_00433bd8 = "Level.bnk";
const char* sz_LevelIOPBankName = "LevelIOP.bnk";

void LevelScheduleManager::LevelLoading_Begin()
{
	edCBankBufferEntry* pBankBuffer;
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
	pBankBuffer = levelIOPBank.get_free_entry();
	(levelIOPBank).pBankFileAccessObject = pBankBuffer;
	loadStage_0x5b48 = 0;
	(levelIOPBank).pBankFileAccessObject->load(&bankContainer);
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

bool LevelScheduleManager::LevelLoading_Manage()
{
	edCBankBufferEntry* pBVar1;
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
			bVar1 = (this->levelBank).pBankFileAccessObject->is_loaded();
			if (bVar1 != false) {
				this->loadStage_0x5b48 = 4;
			}
		}
		else {
			if (loadStage == 2) {
				bVar1 = edSoundAreAllSoundDataLoaded();
				if ((bVar1 != false) && (bVar1 = edMusicAreAllMusicDataLoaded(), bVar1 != false)) {
					(this->levelIOPBank).pBankFileAccessObject->close();
					(this->levelIOPBank).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
					this->levelIOPBank.terminate();
					this->loadStage_0x5b48 = 3;
				}
			}
			else {
				if (loadStage == 1) {
					levelToLoadID = this->nextLevelID;
					memset(&bankFilePathContainer, 0, sizeof(edCBankInstall));
					this->levelBank.initialize(this->aLevelInfo[levelToLoadID].bankSizeLevel + 0x1000, 1,
						&bankFilePathContainer);
					this->levelBank.bank_buffer_setcb(TableBankCallback);
					/* / + level.bnk */
					edStrCatMulti(filePath, this->levelPath, this->aLevelInfo[levelToLoadID].levelName, sz_bankSlash, sz_LevelBank_00433bd8, 0);
					bankFilePathContainer.filePath = filePath;
					bankFilePathContainer.fileFlagA = 0xc;
					pBVar1 = this->levelBank.get_free_entry();
					(this->levelBank).pBankFileAccessObject = pBVar1;
					(this->levelBank).pBankFileAccessObject->load(&bankFilePathContainer);
					this->loadStage_0x5b48 = 2;
				}
			}
		}
	}
	return this->loadStage_0x5b48 != 4;
}

void LevelScheduleManager::Level_Install()
{
	edCBankInstall SStack32;

	memset(&SStack32, 0, sizeof(edCBankInstall));
	SStack32.fileFlagA = 0;
	(this->levelBank).pBankFileAccessObject->install(&SStack32);
	return;
}

void LevelScheduleManager::Level_Init()
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

void LevelScheduleManager::LevelLoading_End()
{
	this->currentLevelID = this->nextLevelID;
	this->nextLevelID = 0x10;
	this->level_0x5b3c = -1;
	return;
}

struct ScenarioVariable {
	int field_0x0;
	int field_0x4;
};

ScenarioVariable _gScenVarInfo[30] = { 0 };

int LevelScheduleManager::ScenVar_Get(SCENARIC_VARIABLE param_1)
{
	return _gScenVarInfo[param_1].field_0x0;
}
