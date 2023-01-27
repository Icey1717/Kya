#include "LevelScheduleManager.h"
#include "IniFile.h"
#include "edMem.h"
#include "edC/edCBank.h"

#include <string.h>
#include <stdio.h>

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif

#include "edSystem.h"

LevelScheduleManager* g_LevelScheduleManager_00449728 = NULL;

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
	pcVar3 = (LevelInfo*)(levelPath + levelID * 0x418 + -8);
	level_0x5b40 = *(int*)&pcVar3->field_0xac;
	currentElevatorID = -1;
	level_0x5b50 = 0x10;
	level_0x5b54 = -1;
	if (field_0x78 == 0) {
		iVar2 = nextElevatorID;
		if (((iVar2 != -1) && (-1 < iVar2)) && (iVar2 < pcVar3->maxElevatorID_0xa8)) {
			iVar2 = *(int*)(&pcVar3->field_0xf4 + iVar2 * 0x28);
			if (iVar2 != -1) {
				level_0x5b40 = iVar2;
			}
			level_0x5b50 = currentLevelID;
			level_0x5b54 = param_4;
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

void SetupLevelInfo_002d97c0(LevelInfo* pLevelInfo, bool param_2)
{
	LoadLoopObject_418_18* pLVar1;
	int iVar2;
	undefined8* puVar3;

	/* This will clear our current level name */
	if (param_2 != false) {
		pLevelInfo->field_0x0 = 0;
		pLevelInfo->field_0x8 = 0;
		pLevelInfo->field_0xc = 0;
		pLevelInfo->field_0x14 = 0;
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

ulong FUN_00189a30(int param_1, int param_2)
{
	return (long)param_1 & 0xffffffffU | (long)param_2 << 0x20;
}


void LevelScheduleManager::StoreLevelFolders(char* fileData, LevelInfo* pLevelInfo)
{
	ulong uVar1;
	int iVar2;
	LoadLoopObject_418_18* pLVar3;

	uVar1 = FUN_00189a30(*(int*)(fileData + 0x18), *(int*)(fileData + 0x1c));
	pLevelInfo->field_0x0 = uVar1;
	pLVar3 = pLevelInfo->field_0x238;
	iVar2 = 0;
	pLevelInfo->field_0x20 = *(undefined4*)(fileData + 0x28);
	pLevelInfo->field_0x8 = *(undefined4*)(fileData + 0xc);
	pLevelInfo->field_0xc = *(undefined4*)(fileData + 0x10);
	pLevelInfo->bankSize = *(int*)(fileData + 0x14);
	pLevelInfo->field_0x14 = *(undefined4*)(fileData + 0x24);
	*(undefined4*)(pLevelInfo->levelName + 0xc) = *(undefined4*)(fileData + 0x30);
	pLevelInfo->field_0x1c = *(undefined4*)(fileData + 0x20);
	do {
		pLVar3->field_0x4 = 0;
		pLVar3->field_0x0 = 0;
		iVar2 = iVar2 + 6;
		pLVar3->field_0x8 = 0;
		pLVar3->field_0xc = 0;
		pLVar3->field_0x14 = 0;
		pLVar3->field_0x10 = 0;
		pLVar3->field_0x18 = 0;
		pLVar3->field_0x1c = 0;
		pLVar3->field_0x24 = 0;
		pLVar3->field_0x20 = 0;
		pLVar3->field_0x28 = 0;
		pLVar3->field_0x2c = 0;
		pLVar3->field_0x34 = 0;
		pLVar3->field_0x30 = 0;
		pLVar3->field_0x38 = 0;
		pLVar3->field_0x3c = 0;
		pLVar3->field_0x44 = 0;
		pLVar3->field_0x40 = 0;
		pLVar3->field_0x48 = 0;
		pLVar3->field_0x4c = 0;
		pLVar3->field_0x54 = 0;
		pLVar3->field_0x50 = 0;
		pLVar3->field_0x58 = 0;
		pLVar3->field_0x5c = 0;
		pLVar3 = pLVar3 + 1;
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
	if (true) {
		do {
			/* Make sure the 12 characters after the level name are clear */
			pLevelInfo->levelName[iVar2] = '\0';
			iVar2 = iVar2 + 1;
		} while (iVar2 < 0xc);
	}
	return;
}

void LevelScheduleManager::LevelLoadSetup(char* pFileData, int count, LevelInfo* pLevelInfo)
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

void LevelScheduleManager::LoadLevelInfoBnk()
{
	MY_LOG("LevelScheduleManager::LoadLevelInfoBnk\n");

	char cVar1;
	int iVar2;
	undefined4* puVar3;
	bool bVar4;
	edCBankBuffer* infoLevelsFileBuffer;
	int inFileIndex;
	uint* puVar5;
	LevelInfo* pLevelInfo;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	int* fileData;
	BankFileData outHeader;
	char levelInfoFilePath[512];
	edCBank bank;
	BankFilePathContainer infoLevelsPathPtr;

	memset(&infoLevelsPathPtr, 0, 0x18);
	edCBank_Setup(&bank, 0x10000, 1, &infoLevelsPathPtr);
	edCBank_SetDeserializeData(&bank, &g_LevelInfoTypePairData_004256e0);
	/* CDEURO/LEVEL/ + Info/levels.bnk */
	levelInfoFilePath[0] = '\0';
	FormatFilePath(levelInfoFilePath, levelPath, "Info/levels.bnk", 0);
	infoLevelsFileBuffer = edCBank_GetBankBuffer(&bank);
	infoLevelsPathPtr.filePath = levelInfoFilePath;
	bVar4 = edCBankBuffer_file_access(infoLevelsFileBuffer, &infoLevelsPathPtr);
	if (bVar4 != false) {
		inFileIndex = CheckFileLoadAndGetParam(infoLevelsFileBuffer);
		while (inFileIndex != 0) {
			inFileIndex = inFileIndex + -1;
			bVar4 = GetFileDataForIndex(infoLevelsFileBuffer, inFileIndex, &outHeader, (char*)0x0);
			puVar3 = (undefined4*)outHeader.fileBufferStart;
			if (bVar4 != false) {
				fileData = (int*)(outHeader.fileBufferStart + 4);
				if (true) {
					switch (*(undefined4*)outHeader.fileBufferStart) {
					case 9:
						iVar2 = *fileData;
						if ((-1 < iVar2) && (iVar2 < 0x10)) {
							pLevelInfo = &aLevelInfo[iVar2];
							StoreLevelFolders((char*)fileData, pLevelInfo);
							LevelLoadSetup((char*)(puVar3 + 0xf), puVar3[0xc], pLevelInfo);
							//puVar5 = LevelLoadSetup(inLevelLoadMaster, puVar3 + 0xf + puVar3[0xc] * 7, puVar3[10], levelNameHolderObj);
							//LevelLoadSetup(inLevelLoadMaster, puVar5, puVar3[0xe], *fileData);
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
							//inLevelLoadMaster->field_0x4210 = inLevelLoadMaster->field_0x4210 + *(int*)(inLevelLoadMaster->levelPath + iVar2 * 0x418 + -8 + 0xb0);
						}
					}
				}
			}
		}
		edCBankBuffer_close(infoLevelsFileBuffer);
	}
	edCBank_Free_00244e10(&bank);
	return;
}

int INT_ARRAY_0048ed60[16] = { 0 };
int INT_ARRAY_0048eda0[16] = { 0 };

void LevelScheduleManager::OnBeginGame()
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

	lVar2 = (SaveBigAlloc*)edMemAlloc(1, 0x10000);
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
	field_0x5b58 = 0;
	field_0x5b5c = 0;
	field_0x74 = 0;
	field_0x78 = 0;
	field_0x7c = 0;
	field_0x80 = 0;
	(pLevelBank).pBankFileAccessObject = (edCBankBuffer*)0x0;
	(levelIOPBank).pBankFileAccessObject = (edCBankBuffer*)0x0;
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
		PrintString(g_szNoPathError_00433c20);
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
	LoadLevelInfoBnk();
	//LoadEpisodesFromINI();
	pLVar12 = aLevelInfo;
	iVar11 = 0;
	do {
		iVar3 = strcmp(local_80, pLVar12->levelName);
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

namespace LoadLevelBank
{
	TypePairData TypePairFunctionData_0040e780[24];
}

const char* sz_bankSlash = "/";
const char* sz_LevelIOPBankName = "LevelIOP.bnk";

void LevelScheduleManager::LoadStageOne()
{
	edCBankBuffer* pBankBuffer;
	int cachedNextLevelID;
	char filePath[128];
	BankFilePathContainer bankContainer;

	/* This is part of the level load process
	   It loads in the level IOP bank for the specific level found in the level load
	   master.
	   Example: CDEURO/LEVEL/PREINTRO/LevelIOP.bnk */
	//MemSetFlags(1, 0x100);
	cachedNextLevelID = nextLevelID;
	memset(&bankContainer, 0, 0x18);
	edCBank_Setup(&levelIOPBank,
		aLevelInfo[cachedNextLevelID].bankSize + 0x1000, 1,
		&bankContainer);
	edCBank_SetDeserializeData
	(&levelIOPBank, LoadLevelBank::TypePairFunctionData_0040e780);
	/* / + LevelIOP.bnk */
	FormatFilePath(filePath, levelPath,
		aLevelInfo[cachedNextLevelID].levelName, sz_bankSlash, sz_LevelIOPBankName, 0);
	bankContainer.filePath = filePath;
	bankContainer.pObjectReference = 0;
	bankContainer.fileFlagA = 4;
	//local_20.fileFunc = (int)WillLoadFilefromBank;
	pBankBuffer = edCBank_GetBankBuffer(&levelIOPBank);
	(levelIOPBank).pBankFileAccessObject = pBankBuffer;
	loadStage_0x5b48 = 0;
	edCBankBuffer_file_access((levelIOPBank).pBankFileAccessObject, &bankContainer);
	//iVar2 = MemClearFlags(1, 0x100);
}
