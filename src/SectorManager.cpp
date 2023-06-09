#include "SectorManager.h"
#include <string.h>
#include "edSystem.h"
#include "LevelScheduleManager.h"
#include "TimeController.h"
#include "Rendering/CameraPanMasterHeader.h"
#include "Rendering/DisplayList.h"
#include "edStr.h"

SectorManager::SectorManager()
{
	(this->baseSector).pANHR.pThis = 0;
	sceVu0UnitMatrix((TO_SCE_MTX)&SomeIdentityMatrix);
	(this->baseSector).field_0x0 = -1;
	(this->baseSector).field_0x4 = -1;
	(this->baseSector).bankObject.pBankFileAccessObject = (edCBankBufferEntry*)0x0;
	(this->baseSector).pMeshTransform = (MeshTransformParent*)0x0;
	(this->baseSector).pManager100Obj = (undefined*)0x0;
	(this->baseSector).loadStage_0x8 = 0;
	(this->baseSector).field_0x134 = 0.0;
	(this->baseSector).pMeshTransformParent_0x130 = (MeshTransformParent*)0x0;
	memset(&(this->baseSector).textureInfo, 0, sizeof(ed_g2d_manager));
	memset(&(this->baseSector).meshInfo, 0, sizeof(ed_g3d_manager));
	memset(&(this->baseSector).textureInfoB, 0, sizeof(ed_g2d_manager));
	memset(&(this->baseSector).meshInfoB, 0, sizeof(ed_g3d_manager));
	Setup_001ffec0();
	return;
}

void SectorManager::Setup_001ffec0()
{
	int iVar1;
	SectorManagerSubObj* pSVar2;

	(this->baseSector).sectID = -1;
	pSVar2 = this->subObjArray;
	(this->baseSector).field_0x140 = -1;
	this->field_0x36c = 1;
	iVar1 = 0x1e;
	this->sectorArray[0].bankObject.firstField = 0;
	this->count_0x368 = 0;
	this->sectDataCount = 0;
	this->sectorArray[0].field_0x0 = 0;
	this->sectorArray[0].field_0x4 = 0;
	this->field_0x370 = 1;
	do {
		pSVar2->flags = 0;
		pSVar2->pFileData = (undefined*)0x0;
		iVar1 = iVar1 + -6;
		pSVar2->pWindSectorObj = (WindSectorObj*)0x0;
		pSVar2[1].flags = 0;
		pSVar2[1].pFileData = (undefined*)0x0;
		pSVar2[1].pWindSectorObj = (WindSectorObj*)0x0;
		pSVar2[2].flags = 0;
		pSVar2[2].pFileData = (undefined*)0x0;
		pSVar2[2].pWindSectorObj = (WindSectorObj*)0x0;
		pSVar2[3].flags = 0;
		pSVar2[3].pFileData = (undefined*)0x0;
		pSVar2[3].pWindSectorObj = (WindSectorObj*)0x0;
		pSVar2[4].flags = 0;
		pSVar2[4].pFileData = (undefined*)0x0;
		pSVar2[4].pWindSectorObj = (WindSectorObj*)0x0;
		pSVar2[5].flags = 0;
		pSVar2[5].pFileData = (undefined*)0x0;
		pSVar2[5].pWindSectorObj = (WindSectorObj*)0x0;
		pSVar2 = pSVar2 + 6;
	} while (0 < iVar1);
	this->sectorArray[0].loadStage_0x8 = 0;
	return;
}

struct WindSectorObj {
	undefined8 field_0x0;
	struct WindSectorObj* pNext;
	undefined4 field_0xc;
	char* field_0x10;
	uint flags;
	byte field_0x18;
	byte field_0x19;
};


void SectorManager::Func_001fe620()
{
	WindSectorObj* pWindSectorObj;
	int iVar1;
	int* piVar2;
	int local_110;
	int local_10c[66];
	int* local_4;

	this->sectorArray[0].bankObject.firstField = 300;
	for (pWindSectorObj = (WindSectorObj*)this->sectorArray[0].loadStage_0x8; pWindSectorObj != (WindSectorObj*)0x0;
		pWindSectorObj = pWindSectorObj->pNext) {
		IMPLEMENTATION_GUARD(
		if ((pWindSectorObj->field_0x19 == 0) &&
			(iVar1 = Func_001fe870(this, pWindSectorObj->field_0x0, &local_4), iVar1 != 0)) {
			WindSectorObj::Setup_001fd8c0(pWindSectorObj, iVar1, local_4);
		})
	}
	local_110 = 0;
	//ActorManager::Func_00106410(g_ManagerSingletonArray_00451660.g_ActorManager_004516a4, 0x18, &local_110);
	iVar1 = 0;
	if (0 < local_110) {
		IMPLEMENTATION_GUARD(
		piVar2 = &local_110;
		do {
			FUN_001e4ae0(piVar2[1]);
			iVar1 = iVar1 + 1;
			piVar2 = piVar2 + 1;
		} while (iVar1 < local_110);)
	}
	return;
}

void SectorManager::Game_Term()
{
	SectorManagerSubObj* pLVar1;
	int* piVar2;
	Sector* pSVar3;
	ulong uVar4;
	uint uVar5;
	uint uVar6;
	uint* puVar7;
	uint uVar8;
	int* piVar9;
	int iVar10;
	int iVar11;
	uint uVar12;
	int iVar13;
	SectorManagerSubObj* pcVar14;
	LevelInfo* local_1a0;
	int local_190[32];
	int local_110[32];
	uint auStack144[31];
	int local_14;
	int local_10;
	int local_c;
	uint local_8;
	uint local_4;

	/* LoadSect
		*/
	LevelInfo* pcVar5 = &LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->nextLevelID];
	local_1a0 = pcVar5;
	/* /sect */
	edStrCatMulti(this->field_0x4, LevelScheduleManager::gThis->levelPath, pcVar5->levelName, "/", "SECT", 0);
	uVar12 = 0;
	memset(auStack144, 0, 0x78);
	this->sectDataCount = 0;
	this->count_0x368 = pcVar5->sectorCount_0x14;
	iVar13 = 1;
	if (0 < this->count_0x368) {
		pcVar14 = this->subObjArray;
		pLVar1 = &local_1a0->aSectorSubObj[1];
		do {
			uVar8 = (uint)pLVar1->pWindSectorObj;
			puVar7 = (uint*)pLVar1->flags;
			for (iVar10 = pLVar1->field_0xc; iVar10 != 0; iVar10 = iVar10 + -1) {
				IMPLEMENTATION_GUARD(
				uVar4 = FUN_00117440((int**)(puVar7 + 1));
				if (uVar4 == 0) {
					uVar8 = uVar8 & ~(1 << (*puVar7 & 0x1f));
				}
				puVar7 = puVar7 + 2;)
			}
			pcVar14->flags = uVar8;
			pcVar14->field_0xc = -1;
			if (uVar8 != 0) {
				iVar10 = 0;
				uVar6 = 1;
				if (0 < this->count_0x368) {
					puVar7 = auStack144;
					do {
						puVar7 = puVar7 + 1;
						uVar5 = 1 << (uVar6 & 0x1f);
						if ((uVar5 & uVar8) != 0) {
							uVar12 = uVar12 | uVar5;
							iVar10 = iVar10 + 1;
							*puVar7 = *puVar7 | uVar8;
						}
						uVar6 = uVar6 + 1;
					} while ((int)uVar6 <= this->count_0x368);
				}
				if (this->sectDataCount < iVar10) {
					this->sectDataCount = iVar10;
				}
			}
			iVar13 = iVar13 + 1;
			pcVar14 = pcVar14 + 1;
			pLVar1 = pLVar1 + 1;
		} while (iVar13 <= this->count_0x368);
	}
	uVar8 = this->sectDataCount;
	if (uVar8 == 0) {
		this->sectorArray[0].field_0x0 = 0;
	}
	else {
		IMPLEMENTATION_GUARD(
		piVar2 = (int*)AllocateFunc_001002a0((long)(int)(uVar8 * 0x144 + 0x10));
		pSVar3 = (Sector*)
			RunActorInitFunctions_00217580(piVar2, Sector::Constructor_001ffcf0, Sector::Free_001ff110, 0x144, uVar8);
		this->sectorArray = pSVar3;
		iVar13 = 0;
		if (0 < this->sectDataCount) {
			iVar10 = 0;
			do {
				iVar13 = iVar13 + 1;
				iVar11 = (int)&this->sectorArray->field_0x0 + iVar10;
				*(undefined4*)(iVar11 + 0x13c) = 0xffffffff;
				*(undefined4*)(iVar11 + 0x140) = 0;
				iVar10 = iVar10 + 0x144;
			} while (iVar13 < this->sectDataCount);
		}
		local_110[0] = 0;
		uVar8 = 1;
		if (0 < this->count_0x368) {
			do {
				if ((1 << (uVar8 & 0x1f) & uVar12) != 0) {
					local_4 = uVar8;
					FUN_00200120((int)local_110, uVar8);
				}
				uVar8 = uVar8 + 1;
			} while ((int)uVar8 <= this->count_0x368);
		}
		iVar13 = 0;
		piVar2 = local_190;
		do {
			iVar13 = iVar13 + 6;
			*piVar2 = local_1a0->field_0x238[0].field_0x4;
			piVar2[1] = local_1a0->field_0x238[0].field_0x14;
			piVar2[2] = local_1a0->field_0x238[0].field_0x24;
			piVar2[3] = local_1a0->field_0x238[0].field_0x34;
			piVar2[4] = local_1a0->field_0x238[0].field_0x44;
			piVar2[5] = local_1a0->field_0x238[0].field_0x54;
			piVar2 = piVar2 + 6;
			local_1a0 = (LevelInfo*)&local_1a0->field_0x60;
		} while (iVar13 < 0x1e);
		iVar13 = 0;
		if (0 < local_110[0] + -1) {
			piVar2 = local_110;
			iVar10 = local_110[0];
			do {
				iVar11 = iVar13 + 1;
				if (iVar11 < iVar10) {
					piVar9 = local_110 + iVar11;
					do {
						local_10 = piVar9[1];
						local_14 = piVar2[1];
						if (local_190[piVar2[1]] < local_190[piVar9[1]]) {
							FUN_002000f0((int)local_110, iVar11, iVar13);
						}
						iVar11 = iVar11 + 1;
						piVar9 = piVar9 + 1;
						iVar10 = local_110[0];
					} while (iVar11 < local_110[0]);
				}
				iVar13 = iVar13 + 1;
				piVar2 = piVar2 + 1;
			} while (iVar13 < iVar10 + -1);
		}
		this->field_0x370 = 1;
		iVar13 = 0;
		piVar2 = local_110;
		while ((iVar13 < local_110[0] && (this->field_0x370 != 0))) {
			local_8 = piVar2[1];
			uVar8 = ~(1 << (local_8 & 0x1f)) & auStack144[local_8];
			uVar12 = 0;
			if (uVar8 != 0) {
				uVar6 = 1;
				if (0 < this->count_0x368) {
					pcVar14 = this->field_0x4 + 0xc;
					do {
						if (((1 << (uVar6 & 0x1f) & uVar8) != 0) && (pcVar14[0x50] != -1)) {
							uVar12 = uVar12 | 1 << ((int)pcVar14[0x50] & 0x1fU);
						}
						uVar6 = uVar6 + 1;
						pcVar14 = pcVar14 + 0x10;
					} while ((int)uVar6 <= this->count_0x368);
				}
			}
			for (uVar8 = 0; ((1 << (uVar8 & 0x1f) & uVar12) != 0 && ((int)uVar8 < this->sectDataCount)); uVar8 = uVar8 + 1) {
			}
			if ((int)uVar8 < this->sectDataCount) {
				*(char*)&this->subObjArray[local_8].field_0xc = (char)uVar8;
				if (this->sectorArray[uVar8].field_0x140 < local_190[local_8]) {
					this->sectorArray[uVar8].field_0x140 = local_190[local_8];
				}
			}
			else {
				this->field_0x370 = 0;
			}
			piVar2 = piVar2 + 1;
			iVar13 = iVar13 + 1;
		}
		if (this->field_0x370 == 0) {
			local_c = local_110[1];
			iVar13 = local_190[local_110[1]];
			iVar10 = 0;
			if (0 < this->sectDataCount) {
				iVar11 = 0;
				do {
					iVar10 = iVar10 + 1;
					*(int*)((int)&this->sectorArray->field_0x140 + iVar11) = iVar13;
					iVar11 = iVar11 + 0x144;
				} while (iVar10 < this->sectDataCount);
			}
			iVar13 = 1;
			if (0 < this->count_0x368) {
				pcVar14 = this->field_0x4 + 0xc;
				do {
					pcVar14[0x50] = -1;
					iVar13 = iVar13 + 1;
					pcVar14 = pcVar14 + 0x10;
				} while (iVar13 <= this->count_0x368);
			}
		})
	}
	(this->baseSector).sectID = -1;
	this->field_0x36c = 1;
	return;
}

bool SectorManager::LevelLoading_Manage()
{
	uint uVar1;
	int iVar2;
	edCBankBufferEntry* peVar3;
	int iVar4;
	bool bVar5;
	bool bVar6;
	bool bVar7;
	Sector* pSectData;
	int iVar8;
	int iVar9;
	int iVar10;
	bool bVar11;
	BankFilePathContainer BStack96;
	BankFilePathContainer BStack64;
	BankFilePathContainer BStack32;

	bVar11 = true;
	if (LevelScheduleManager::gThis->loadStage_0x5b48 == 4) {
		if ((this->baseSector).loadStage_0x8 == 1) {
			peVar3 = (this->baseSector).bankObject.pBankFileAccessObject;
			bVar5 = false;
			if ((peVar3 != (edCBankBufferEntry*)0x0) && (bVar6 = edCBankBuffer_CheckAccessFlag(peVar3), bVar6 != false)) {
				bVar5 = true;
			}
			bVar6 = true;
			if (bVar5) {
				iVar10 = 0;
				iVar8 = 0;
				while ((iVar10 < this->sectDataCount && (bVar6))) {
					pSectData = (Sector*)((int)&this->sectorArray->field_0x0 + iVar8);
					iVar9 = pSectData->sectID;
					if (iVar9 != -1) {
						if (pSectData->loadStage_0x8 == 1) {
							peVar3 = (pSectData->bankObject).pBankFileAccessObject;
							bVar5 = false;
							if ((peVar3 != (edCBankBufferEntry*)0x0) && (bVar7 = edCBankBuffer_CheckAccessFlag(peVar3), bVar7 != false))
							{
								bVar5 = true;
							}
							if (!bVar5) {
								bVar6 = false;
							}
						}
						else {
							iVar2 = this->field_0x36c;
							iVar4 = pSectData->field_0x140;
							memset(&BStack96, 0, sizeof(BankFilePathContainer));
							initialize(&pSectData->bankObject, iVar4 + 0x1000, 1, &BStack96);
							if (iVar9 == -1) {
								pSectData->loadStage_0x8 = 0;
							}
							else {
								pSectData->LoadBNK(iVar9, (long)iVar2, false);
							}
							bVar6 = false;
						}
					}
					iVar8 = iVar8 + 0x144;
					iVar10 = iVar10 + 1;
				}
				if (bVar6) {
					bVar11 = false;
				}
			}
		}
		else {
			iVar10 = LevelScheduleManager::gThis->level_0x5b40;
			iVar8 = this->field_0x36c;
			iVar9 = LevelScheduleManager::gThis->aLevelInfo[LevelScheduleManager::gThis->nextLevelID].bankSizeSect;
			memset(&BStack32, 0, sizeof(BankFilePathContainer));
			initialize(&(this->baseSector).bankObject, iVar9 + 0x1000, 1, &BStack32);
			if (iVar10 == -1) {
				(this->baseSector).loadStage_0x8 = 0;
				bVar11 = false;
			}
			else {
				this->baseSector.LoadBNK(iVar10, (long)iVar8, false);
				uVar1 = this->subObjArray[iVar10].flags;
				if (this->sectorArray[0].field_0x4 != uVar1) {
					IMPLEMENTATION_GUARD(Func_001feb10(this, uVar1));
				}
				iVar10 = 0;
				if (0 < this->sectDataCount) {
					iVar8 = 0;
					do {
						Sector* pSector = &this->sectorArray[iVar10];
						if (pSector->sectID == -1) {
							iVar2 = pSector->field_0x140;
							memset(&BStack64, 0, sizeof(BankFilePathContainer));
							initialize(&pSector->bankObject, iVar2 + 0x1000, 1, &BStack64);
							*(undefined4*)(iVar9 + 8) = 0;
						}
						iVar10 = iVar10 + 1;
						iVar8 = iVar8 + 0x144;
					} while (iVar10 < this->sectDataCount);
				}
			}
		}
	}
	return bVar11;
}

struct StaticEdFileBase {
	struct edCFiler* pEdFileBase;
	undefined4 field_0x4;
};

StaticEdFileBase StaticEdFileBase_004497f0 = { 0 };

bool CheckFunc_00401fd0(StaticEdFileBase* param_1)
{
	return param_1->field_0x4 == 0;
}

void Sector::InstallCallback()
{
	edLIST* pCameraPanMasterHeader;
	WindSectorObj* pWindSectorObj;
	ed_3D_Scene* pStaticMeshMaster;
	MeshTransformData* pSubSubObjArray;
	FileManager3D* p3DFileManager;
	SectorManager* pSectorManager;
	bool bVar1;
	uint uVar2;
	undefined* puVar3;
	TextureInfo* pTextureInfo;
	MeshTransformParent* pMVar4;
	uint meshSize;
	char* pcVar6;
	MeshTransformParent* pMVar7;
	int iVar8;
	uint uVar9;
	int** ppiVar10;
	int* piVar11;
	char* pMeshData;
	uint inFileIndex;
	uint unaff_s5_lo;
	char* pFileData;
	char* pAnimHierarchy;
	float fVar14;
	uint local_40;
	uint local_30;
	edBANK_ENTRY_INFO local_20;
	int iStack8;
	short sStack2;

	pAnimHierarchy = (char*)0x0;
	pFileData = (char*)0x0;
	this->pManager100Obj = (undefined*)0x0;
	local_40 = 0;
	pcVar6 = (char*)0x0;
	uVar2 = get_element_count((this->bankObject).pBankFileAccessObject);
	inFileIndex = 0;
	pMeshData = pcVar6;
	meshSize = unaff_s5_lo;
	if (uVar2 != 0) {
		do {
			bVar1 = get_info((this->bankObject).pBankFileAccessObject, inFileIndex, &local_20, (char*)0x0);
			pMeshData = pcVar6;
			meshSize = unaff_s5_lo;
			if (bVar1 == false) break;
			uVar9 = local_20.type << 0x10 | local_20.stype;
			if (uVar9 == 0x170001) {
				MY_LOG("Sector::Init Anim Heirarchy: %s\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->fileBuffer, inFileIndex));
				pAnimHierarchy = local_20.fileBufferStart;
				local_40 = local_20.size;
			}
			else {
				if (uVar9 == 0x50003) {
					MY_LOG("Sector::Init LoadTexture B: %s\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->fileBuffer, inFileIndex));
					ed3DInstallG2D(local_20.fileBufferStart, local_20.size, &iStack8, &this->textureInfoB, 1);
				}
				else {
					if (uVar9 == 0x40002) {
						MY_LOG("Sector::Init UNKNOWN: %s\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->fileBuffer, inFileIndex));
						local_30 = local_20.size;
						pFileData = local_20.fileBufferStart;
					}
					else {
						if (uVar9 == 0x70001) {
							MY_LOG("Sector::Init Init Collision: %s\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->fileBuffer, inFileIndex));
							//IMPLEMENTATION_GUARD(
							//puVar3 = (undefined*)
							//	CollisionManager::InstallColFile
							//	(g_ManagerSingletonArray_00451660.g_CollisionManager_00451690,
							//		(long)(int)local_20.fileBufferStart, (long)(int)local_20.length);
							//this->pManager100Obj = puVar3;)
						}
						else {
							if (uVar9 == 0x50001) {
								MY_LOG("Sector::Init LoadTexture A: %s\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->fileBuffer, inFileIndex));
								ed3DInstallG2D(local_20.fileBufferStart, local_20.size, &iStack8, &this->textureInfo, 1);
							}
							else {
								MY_LOG("Sector::Init Mesh: %s\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->fileBuffer, inFileIndex));
								pMeshData = local_20.fileBufferStart;
								meshSize = local_20.size;
								if (uVar9 != 0x40001) {
									pMeshData = pcVar6;
									meshSize = unaff_s5_lo;
								}
							}
						}
					}
				}
			}
			inFileIndex = inFileIndex + 1;
			pcVar6 = pMeshData;
			unaff_s5_lo = meshSize;
		} while (inFileIndex < uVar2);
	}
	pSectorManager = g_ManagerSingletonArray_00451660.g_SectorManager_00451670;
	p3DFileManager = g_ManagerSingletonArray_00451660.g_FileManager3D_00451664;
	pStaticMeshMaster = Scene::_scene_handleA;
	ed3DInstallG3D(pMeshData, meshSize, 0, &iStack8, (TextureInfo*)&this->textureInfo, 0xc, &this->meshInfo);
	pTextureInfo = p3DFileManager->GetCommonSectorG2D();
	ed3DLinkG2DToG3D(&this->meshInfo, pTextureInfo);
	ed3DScenePushCluster(pStaticMeshMaster, &this->meshInfo);
	if (pFileData == (char*)0x0) {
		this->pMeshTransform = (MeshTransformParent*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(
		3DFileManager::HideCommonBackground(p3DFileManager);
		ed3DInstallG3D(pFileData, local_30, 0, &iStack8, (TextureInfo*)&this->textureInfoB, 0xc, &this->meshInfoB);
		pMVar4 = ed3DHierarchyAddToScene(pStaticMeshMaster, &this->meshInfoB, (char*)0x0);
		this->pMeshTransform = pMVar4;
		if (this->pMeshTransform != (MeshTransformParent*)0x0) {
			IMPLEMENTATION_GUARD(3DFileManager::SetupBackground(p3DFileManager, this->pMeshTransform));
		})
	}
	this->field_0x0 = this->field_0x4;
	this->field_0x4 = -1;
	this->pANHR.Install((MeshData_ANHR*)pAnimHierarchy, local_40, &this->meshInfo, pStaticMeshMaster);
	ed3DHierarchyCopyHashCode(&this->meshInfo);
	pMVar4 = gHierarchyManagerFirstFreeNode;
	pSubSubObjArray = gHierarchyManagerBuffer;
	pMeshData = (this->meshInfo).HALL;
	pCameraPanMasterHeader = pStaticMeshMaster->pHeirListA;
	meshSize = edChunckGetNb(pMeshData + 0x10, pMeshData + *(int*)(pMeshData + 8));
	pcVar6 = edHashcodeGet(0x43494d414e5944, (MeshData_HASH*)((this->meshInfo).HALL + 0x10));
	iVar8 = 0;
	if (pcVar6 != (char*)0x0) {
		iVar8 = *(int*)(pcVar6 + 8);
	}
	if (iVar8 == 0) {
		pMVar7 = (MeshTransformParent*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(
		pMVar7 = edLIST::Func_002ab920
		(pCameraPanMasterHeader, pSubSubObjArray, pMVar4, (undefined8*)(iVar8 + 0x10),
			(MeshTransformData*)0x0);
		edLIST::Func_002ad020
		(pCameraPanMasterHeader, pSubSubObjArray, pMVar4, iVar8, (int)pMVar7, (int)(pMeshData + 0x20),
			(meshSize & 0xffff) - 1);
		MeshTransformParent::Func_002ac980(pMVar7, &sStack2);)
	}
	this->pMeshTransformParent_0x130 = pMVar7;
	pWindSectorObj = pSectorManager->subObjArray[this->field_0x0].pWindSectorObj;
	do {
		if (pWindSectorObj == (WindSectorObj*)0x0) {
			fVar14 = GetFrameTime_00291c40();
			this->field_0x134 = fVar14 - this->field_0x134;
			pSectorManager->Func_001fe620();
			return;
		}
		IMPLEMENTATION_GUARD(
		lVar13 = pWindSectorObj->field_0x0;
		if (lVar13 != 0) {
			iVar8 = this->field_0x138;
			if (iVar8 != 0) {
				ppiVar10 = (int**)(iVar8 + 0x10);
				meshSize = 0;
				if (*(uint*)(iVar8 + 8) != 0) {
					do {
						piVar11 = *ppiVar10;
						if ((piVar11[1] != 0) && (lVar13 == *(long*)(piVar11[1] + 0x80))) goto LAB_001fe180;
						meshSize = meshSize + 1;
						ppiVar10 = ppiVar10 + 1;
					} while (meshSize < *(uint*)(iVar8 + 8));
				}
			}
			piVar11 = (int*)0x0;
		LAB_001fe180:
			if (piVar11 == (int*)0x0) {
				iVar8 = MeshTransformParent::Func_001fff60(this->pMeshTransformParent_0x130, lVar13);
			}
			else {
				iVar8 = *piVar11;
			}
			if (iVar8 != 0) {
				WindSectorObj::Setup_001fd8c0(pWindSectorObj, iVar8, piVar11);
			}
		}
		pWindSectorObj = pWindSectorObj->pWindSectorObj;)
	} while (true);
}

void LoadSect_001fe3c0(bool index, void* pSector)
{
	if (index != false) {
		((Sector*)pSector)->InstallCallback();
	}
	return;
}

void Sector::LoadBNK(int param_2, int param_3, bool bFileFlag)
{
	int iVar1;
	char cVar2;
	bool bVar3;
	edCBankBufferEntry* peVar4;
	int sectStringLength;
	float fVar5;
	BankFilePathContainer local_60;
	char acStack64[64];
	char* sectString;

	/* Loads a sectx.bnk file for a level
	   Example: CDEURO/LEVEL/PREINTRO/SECT1.bnk */
	bVar3 = CheckFunc_00401fd0(&StaticEdFileBase_004497f0);
	if (bVar3 != false) {
		fVar5 = GetFrameTime_00291c40();
		this->field_0x134 = fVar5;
		this->field_0x0 = -1;
		this->field_0x4 = param_2;
		peVar4 = (this->bankObject).pBankFileAccessObject;
		if (peVar4 != (edCBankBufferEntry*)0x0) {
			edCBankBuffer_close(peVar4);
			(this->bankObject).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
		}
		peVar4 = get_free_entry(&this->bankObject);
		(this->bankObject).pBankFileAccessObject = peVar4;
		iVar1 = this->field_0x4;
		sectStringLength = edStrCopy(acStack64, (g_ManagerSingletonArray_00451660.g_SectorManager_00451670)->field_0x4);
		sectString = acStack64 + sectStringLength;
		if (iVar1 < 10) {
			*sectString = (char)iVar1 + '0';
			sectString = acStack64 + 1;
		}
		else {
			cVar2 = (char)(iVar1 / 10);
			*sectString = cVar2 + '0';
			acStack64[sectStringLength + 1] = (char)iVar1 + cVar2 * -10 + '0';
			sectString = acStack64 + 2;
		}
		/* Add the '.bnk' suffix to the file name */
		edStrCopy(sectString + sectStringLength, ".bnk");
		memset(&local_60, 0, sizeof(BankFilePathContainer));
		if (bFileFlag == false) {
			local_60.fileFlagA = 0xc;
		}
		else {
			local_60.fileFlagA = 4;
		}
		local_60.filePath = acStack64;
		local_60.fileFunc = LoadSect_001fe3c0;
		local_60.pObjectReference = this;
		bVar3 = load((this->bankObject).pBankFileAccessObject, &local_60);
		if ((bVar3 != false) && (this->loadStage_0x8 = 1, param_3 == 0)) {
			load((this->bankObject).pBankFileAccessObject);
		}
	}
	return;
}

void SectorManager::Level_Install()
{
	edCBankBufferEntry* peVar1;
	bool bVar2;
	bool bVar3;
	int iVar4;
	Sector* pvVar4;
	int iVar5;
	BankFilePathContainer BStack64;
	BankFilePathContainer BStack32;

	if ((this->baseSector).loadStage_0x8 == 1) {
		peVar1 = (this->baseSector).bankObject.pBankFileAccessObject;
		bVar2 = false;
		if ((peVar1 != (edCBankBufferEntry*)0x0) && (bVar3 = edCBankBuffer_CheckAccessFlag(peVar1), bVar3 != false)) {
			bVar2 = true;
		}
		if (bVar2) {
			memset(&BStack32, 0, sizeof(BankFilePathContainer));
			BStack32.fileFlagA = 0;
			BStack32.fileFunc = LoadSect_001fe3c0;
			BStack32.pObjectReference = &this->baseSector;
			edCBankBuffer_file_access_002450e0((this->baseSector).bankObject.pBankFileAccessObject, &BStack32);
			(this->baseSector).loadStage_0x8 = 2;
		}
	}
	iVar5 = 0;
	if (0 < this->sectDataCount) {
		iVar4 = 0;
		do {
			pvVar4 = &this->sectorArray[iVar5];
			if ((pvVar4->sectID != -1) && (pvVar4->loadStage_0x8 == 1)) {
				peVar1 = (pvVar4->bankObject).pBankFileAccessObject;
				bVar2 = false;
				if ((peVar1 != (edCBankBufferEntry*)0x0) && (bVar3 = edCBankBuffer_CheckAccessFlag(peVar1), bVar3 != false)) {
					bVar2 = true;
				}
				if (bVar2) {
					memset(&BStack64, 0, sizeof(BankFilePathContainer));
					BStack64.fileFlagA = 0;
					BStack64.fileFunc = LoadSect_001fe3c0;
					BStack64.pObjectReference = pvVar4;
					edCBankBuffer_file_access_002450e0((pvVar4->bankObject).pBankFileAccessObject, &BStack64);
					pvVar4->loadStage_0x8 = 2;
				}
			}
			iVar5 = iVar5 + 1;
			iVar4 = iVar4 + 0x144;
		} while (iVar5 < this->sectDataCount);
	}
	return;
}
