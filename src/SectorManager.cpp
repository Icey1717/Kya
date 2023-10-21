#include "SectorManager.h"
#include <string.h>
#include "edSystem.h"
#include "LevelScheduleManager.h"
#include "TimeController.h"
#include "Rendering/CameraPanMasterHeader.h"
#include "Rendering/DisplayList.h"
#include "edStr.h"
#include "MathOps.h"
#include "ScenaricCondition.h"
#include "MemoryStream.h"

CSectorManager::CSectorManager()
{
	(this->baseSector).pANHR.pThis = 0;
	edF32Matrix4SetIdentityHard(&CHierarchyAnm::_gscale_mat);
	(this->baseSector).desiredSectorID = -1;
	(this->baseSector).sectorIndex = -1;
	(this->baseSector).bankObject.pBankFileAccessObject = (edCBankBufferEntry*)0x0;
	(this->baseSector).pMeshTransform = (edNODE*)0x0;
	(this->baseSector).pManager100Obj = (undefined*)0x0;
	(this->baseSector).loadStage_0x8 = 0;
	(this->baseSector).field_0x134 = 0.0;
	(this->baseSector).pMeshTransformParent_0x130 = (edNODE*)0x0;
	memset(&(this->baseSector).textureInfo, 0, sizeof(ed_g2d_manager));
	memset(&(this->baseSector).meshInfo, 0, sizeof(ed_g3d_manager));
	memset(&(this->baseSector).textureInfoB, 0, sizeof(ed_g2d_manager));
	memset(&(this->baseSector).meshInfoB, 0, sizeof(ed_g3d_manager));
	Level_ClearInternalData();
	return;
}

void CSectorManager::Level_ClearInternalData()
{
	int iVar1;
	SectorManagerSubObj* pSVar2;

	(this->baseSector).sectID = -1;
	pSVar2 = this->subObjArray;
	(this->baseSector).currentSectorID = -1;
	this->field_0x36c = 1;
	iVar1 = 0x1e;
	this->field_0x384 = 0;
	this->count_0x368 = 0;
	this->sectDataCount = 0;
	this->sectorArray = (CSector*)0x0;
	this->field_0x37c = 0;
	this->field_0x370 = 1;
	do {
		pSVar2->flags = 0;
		pSVar2->pFileData = (undefined*)0x0;
		iVar1 = iVar1 + -6;
		pSVar2->pWindSectorObj = (CSectorHierarchy*)0x0;
		pSVar2[1].flags = 0;
		pSVar2[1].pFileData = (undefined*)0x0;
		pSVar2[1].pWindSectorObj = (CSectorHierarchy*)0x0;
		pSVar2[2].flags = 0;
		pSVar2[2].pFileData = (undefined*)0x0;
		pSVar2[2].pWindSectorObj = (CSectorHierarchy*)0x0;
		pSVar2[3].flags = 0;
		pSVar2[3].pFileData = (undefined*)0x0;
		pSVar2[3].pWindSectorObj = (CSectorHierarchy*)0x0;
		pSVar2[4].flags = 0;
		pSVar2[4].pFileData = (undefined*)0x0;
		pSVar2[4].pWindSectorObj = (CSectorHierarchy*)0x0;
		pSVar2[5].flags = 0;
		pSVar2[5].pFileData = (undefined*)0x0;
		pSVar2[5].pWindSectorObj = (CSectorHierarchy*)0x0;
		pSVar2 = pSVar2 + 6;
	} while (0 < iVar1);
	this->pSectorHierarchy = (CSectorHierarchy*)0;
	return;
}

struct S_HIERANM_HIER;

struct CSectorHierarchy {
	undefined8 field_0x0;
	CSectorHierarchy* pNext;
	edNODE* pNode;
	S_HIERANM_HIER* pHier;
	uint flags;
	byte aplha;
	byte field_0x19;
};


void CSectorManager::Func_001fe620()
{
	CSectorHierarchy* pSectorHierarchy;
	int iVar1;
	int* piVar2;
	int local_110;
	int local_10c[66];
	int* local_4;

	this->field_0x384 = 300;
	for (pSectorHierarchy = this->pSectorHierarchy; pSectorHierarchy != (CSectorHierarchy*)0x0;
		pSectorHierarchy = pSectorHierarchy->pNext) {
		IMPLEMENTATION_GUARD(
		if ((pSectorHierarchy->field_0x19 == 0) &&
			(iVar1 = Func_001fe870(this, pSectorHierarchy->field_0x0, &local_4), iVar1 != 0)) {
			CSectorHierarchy::Setup_001fd8c0(pSectorHierarchy, iVar1, local_4);
		})
	}
	local_110 = 0;
	//ActorManager::Func_00106410(Scene::ptable.g_ActorManager_004516a4, 0x18, &local_110);
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

void CSectorManager::LevelLoading_Begin()
{
	SectorManagerSubObj* pLVar1;
	int* piVar2;
	CSector* pSVar3;
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
	edStrCatMulti(this->szSectorFileRoot, LevelScheduleManager::gThis->levelPath, pcVar5->levelName, "/", "SECT", 0);
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
		this->sectorArray = (CSector*)0x0;
	}
	else {
		this->sectorArray = new CSector[uVar8];
		iVar13 = 0;
		if (0 < this->sectDataCount) {
			iVar10 = 0;
			do {
				iVar13 = iVar13 + 1;
				this->sectorArray[iVar13].sectID = -1;
				this->sectorArray[iVar13].currentSectorID = 0;
			} while (iVar13 < this->sectDataCount);
		}
		local_110[0] = 0;
		uVar8 = 1;
		if (0 < this->count_0x368) {
			do {
				if ((1 << (uVar8 & 0x1f) & uVar12) != 0) {
					IMPLEMENTATION_GUARD(
					local_4 = uVar8;
					FUN_00200120((int)local_110, uVar8);)
				}
				uVar8 = uVar8 + 1;
			} while ((int)uVar8 <= this->count_0x368);
		}
		iVar13 = 0;
		piVar2 = local_190;
		do {
			iVar13 = iVar13 + 6;
			IMPLEMENTATION_GUARD(
			*piVar2 = local_1a0->field_0x238[0].field_0x4;
			piVar2[1] = local_1a0->field_0x238[0].field_0x14;
			piVar2[2] = local_1a0->field_0x238[0].field_0x24;
			piVar2[3] = local_1a0->field_0x238[0].field_0x34;
			piVar2[4] = local_1a0->field_0x238[0].field_0x44;
			piVar2[5] = local_1a0->field_0x238[0].field_0x54;)
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
							IMPLEMENTATION_GUARD(
							FUN_002000f0((int)local_110, iVar11, iVar13);)
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
					IMPLEMENTATION_GUARD(
					pcVar14 = this->szSectorFileRoot + 0xc;
					do {
						if (((1 << (uVar6 & 0x1f) & uVar8) != 0) && (pcVar14[0x50] != -1)) {
							uVar12 = uVar12 | 1 << ((int)pcVar14[0x50] & 0x1fU);
						}
						uVar6 = uVar6 + 1;
						pcVar14 = pcVar14 + 0x10;
					} while ((int)uVar6 <= this->count_0x368);)
				}
			}
			for (uVar8 = 0; ((1 << (uVar8 & 0x1f) & uVar12) != 0 && ((int)uVar8 < this->sectDataCount)); uVar8 = uVar8 + 1) {
			}
			if ((int)uVar8 < this->sectDataCount) {
				*(char*)&this->subObjArray[local_8].field_0xc = (char)uVar8;
				if (this->sectorArray[uVar8].currentSectorID < local_190[local_8]) {
					this->sectorArray[uVar8].currentSectorID = local_190[local_8];
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
					*(int*)((int)&this->sectorArray->currentSectorID + iVar11) = iVar13;
					iVar11 = iVar11 + 0x144;
				} while (iVar10 < this->sectDataCount);
			}
			iVar13 = 1;
			if (0 < this->count_0x368) {
				IMPLEMENTATION_GUARD(
				pcVar14 = this->szSectorFileRoot + 0xc;
				do {
					pcVar14[0x50] = -1;
					iVar13 = iVar13 + 1;
					pcVar14 = pcVar14 + 0x10;
				} while (iVar13 <= this->count_0x368);)
			}
		}
	}
	(this->baseSector).sectID = -1;
	this->field_0x36c = 1;
	return;
}

bool CSectorManager::LevelLoading_Manage()
{
	uint uVar1;
	int iVar2;
	edCBankBufferEntry* peVar3;
	int iVar4;
	bool bVar5;
	bool bVar6;
	bool bVar7;
	CSector* pSectData;
	int iVar8;
	int iVar9;
	int iVar10;
	bool bVar11;
	edCBankInstall BStack96;
	edCBankInstall BStack64;
	edCBankInstall BStack32;

	bVar11 = true;
	if (LevelScheduleManager::gThis->loadStage_0x5b48 == 4) {
		if ((this->baseSector).loadStage_0x8 == 1) {
			peVar3 = (this->baseSector).bankObject.pBankFileAccessObject;
			bVar5 = false;
			if ((peVar3 != (edCBankBufferEntry*)0x0) && (bVar6 = peVar3->is_loaded(), bVar6 != false)) {
				bVar5 = true;
			}
			bVar6 = true;
			if (bVar5) {
				iVar10 = 0;
				iVar8 = 0;
				while ((iVar10 < this->sectDataCount && (bVar6))) {
					pSectData = (CSector*)((int)&this->sectorArray->desiredSectorID + iVar8);
					iVar9 = pSectData->sectID;
					if (iVar9 != -1) {
						if (pSectData->loadStage_0x8 == 1) {
							peVar3 = (pSectData->bankObject).pBankFileAccessObject;
							bVar5 = false;
							if ((peVar3 != (edCBankBufferEntry*)0x0) && (bVar7 = peVar3->is_loaded(), bVar7 != false))
							{
								bVar5 = true;
							}
							if (!bVar5) {
								bVar6 = false;
							}
						}
						else {
							iVar2 = this->field_0x36c;
							iVar4 = pSectData->currentSectorID;
							memset(&BStack96, 0, sizeof(edCBankInstall));
							pSectData->bankObject.initialize(iVar4 + 0x1000, 1, &BStack96);
							if (iVar9 == -1) {
								pSectData->loadStage_0x8 = 0;
							}
							else {
								pSectData->Load(iVar9, (long)iVar2, false);
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
			memset(&BStack32, 0, sizeof(edCBankInstall));
			(this->baseSector).bankObject.initialize(iVar9 + 0x1000, 1, &BStack32);
			if (iVar10 == -1) {
				(this->baseSector).loadStage_0x8 = 0;
				bVar11 = false;
			}
			else {
				this->baseSector.Load(iVar10, (long)iVar8, false);
				uVar1 = this->subObjArray[iVar10].flags;
				if (this->field_0x37c != uVar1) {
					IMPLEMENTATION_GUARD(Func_001feb10(this, uVar1));
				}
				iVar10 = 0;
				if (0 < this->sectDataCount) {
					iVar8 = 0;
					do {
						CSector* pSector = &this->sectorArray[iVar10];
						if (pSector->sectID == -1) {
							iVar2 = pSector->currentSectorID;
							memset(&BStack64, 0, sizeof(edCBankInstall));
							pSector->bankObject.initialize(iVar2 + 0x1000, 1, &BStack64);
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

StaticEdFileBase StaticEdFileBase_004497f0 = { 0 };

bool CheckFunc_00401fd0(StaticEdFileBase* param_1)
{
	return param_1->field_0x4 == 0;
}

void CSector::InstallCallback()
{
	edLIST* pCameraPanMasterHeader;
	CSectorHierarchy* pWindSectorObj;
	ed_3D_Scene* pStaticMeshMaster;
	ed_3d_hierarchy_node* pSubSubObjArray;
	FileManager3D* p3DFileManager;
	CSectorManager* pSectorManager;
	bool bVar1;
	uint uVar2;
	undefined* puVar3;
	TextureInfo* pTextureInfo;
	edNODE* pMVar4;
	uint meshSize;
	char* pcVar6;
	edNODE* pMVar7;
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
	uVar2 = (this->bankObject).pBankFileAccessObject->get_element_count();
	inFileIndex = 0;
	pMeshData = pcVar6;
	meshSize = unaff_s5_lo;
	if (uVar2 != 0) {
		do {
			bVar1 = (this->bankObject).pBankFileAccessObject->get_info(inFileIndex, &local_20, (char*)0x0);
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
							//	(Scene::ptable.g_CollisionManager_00451690,
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
	pSectorManager = Scene::ptable.g_SectorManager_00451670;
	p3DFileManager = Scene::ptable.g_FileManager3D_00451664;
	pStaticMeshMaster = Scene::_scene_handleA;
	ed3DInstallG3D(pMeshData, meshSize, 0, &iStack8, &this->textureInfo, 0xc, &this->meshInfo);
	pTextureInfo = p3DFileManager->GetCommonSectorG2D();
	ed3DLinkG2DToG3D(&this->meshInfo, (ed_g2d_manager*)pTextureInfo);
	ed3DScenePushCluster(pStaticMeshMaster, &this->meshInfo);
	if (pFileData == (char*)0x0) {
		this->pMeshTransform = (edNODE*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(
		3DFileManager::HideCommonBackground(p3DFileManager);
		ed3DInstallG3D(pFileData, local_30, 0, &iStack8, (TextureInfo*)&this->textureInfoB, 0xc, &this->meshInfoB);
		pMVar4 = ed3DHierarchyAddToScene(pStaticMeshMaster, &this->meshInfoB, (char*)0x0);
		this->pMeshTransform = pMVar4;
		if (this->pMeshTransform != (edNODE*)0x0) {
			IMPLEMENTATION_GUARD(3DFileManager::SetupBackground(p3DFileManager, this->pMeshTransform));
		})
	}
	this->desiredSectorID = this->sectorIndex;
	this->sectorIndex = -1;
	this->pANHR.Install((MeshData_ANHR*)pAnimHierarchy, local_40, &this->meshInfo, pStaticMeshMaster);
	ed3DHierarchyCopyHashCode(&this->meshInfo);
	pMVar4 = gHierarchyManagerFirstFreeNode;
	pSubSubObjArray = gHierarchyManagerBuffer;
	pMeshData = (char*)(this->meshInfo).HALL;
	pCameraPanMasterHeader = pStaticMeshMaster->pHeirListA;
	meshSize = edChunckGetNb(pMeshData + 0x10, pMeshData + *(int*)(pMeshData + 8));
	pcVar6 = edHashcodeGet(0x43494d414e5944, (ed_Chunck*)((this->meshInfo).HALL + 1));
	iVar8 = 0;
	if (pcVar6 != (char*)0x0) {
		iVar8 = *(int*)(pcVar6 + 8);
	}
	if (iVar8 == 0) {
		pMVar7 = (edNODE*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(
		pMVar7 = edLIST::Func_002ab920
		(pCameraPanMasterHeader, pSubSubObjArray, pMVar4, (undefined8*)(iVar8 + 0x10),
			(ed_3d_hierarchy_node*)0x0);
		edLIST::Func_002ad020
		(pCameraPanMasterHeader, pSubSubObjArray, pMVar4, iVar8, (int)pMVar7, (int)(pMeshData + 0x20),
			(meshSize & 0xffff) - 1);
		edNODE::Func_002ac980(pMVar7, &sStack2);)
	}
	this->pMeshTransformParent_0x130 = pMVar7;
	pWindSectorObj = pSectorManager->subObjArray[this->desiredSectorID].pWindSectorObj;
	do {
		if (pWindSectorObj == (CSectorHierarchy*)0x0) {
			fVar14 = edTimerTimeGet();
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
				iVar8 = edNODE::Func_001fff60(this->pMeshTransformParent_0x130, lVar13);
			}
			else {
				iVar8 = *piVar11;
			}
			if (iVar8 != 0) {
				CSectorHierarchy::Setup_001fd8c0(pWindSectorObj, iVar8, piVar11);
			}
		}
		pWindSectorObj = pWindSectorObj->pWindSectorObj;)
	} while (true);
}

void _gSectorInstallCallback(bool index, void* pSector)
{
	if (index != false) {
		((CSector*)pSector)->InstallCallback();
	}
	return;
}

void CSector::Load(int sectorIndex, int param_3, bool bFileFlag)
{
	int iVar1;
	char cVar2;
	bool bVar3;
	edCBankBufferEntry* peVar4;
	int sectStringLength;
	float fVar5;
	edCBankInstall local_60;
	char acStack64[64];
	char* sectString;

	/* Loads a sectx.bnk file for a level
	   Example: CDEURO/LEVEL/PREINTRO/SECT1.bnk */
	bVar3 = CheckFunc_00401fd0(&StaticEdFileBase_004497f0);
	if (bVar3 != false) {
		fVar5 = edTimerTimeGet();
		this->field_0x134 = fVar5;
		this->desiredSectorID = -1;
		this->sectorIndex = sectorIndex;
		peVar4 = (this->bankObject).pBankFileAccessObject;
		if (peVar4 != (edCBankBufferEntry*)0x0) {
			peVar4->close();
			(this->bankObject).pBankFileAccessObject = (edCBankBufferEntry*)0x0;
		}
		peVar4 = this->bankObject.get_free_entry();
		(this->bankObject).pBankFileAccessObject = peVar4;
		iVar1 = this->sectorIndex;
		sectStringLength = edStrCopy(acStack64, (Scene::ptable.g_SectorManager_00451670)->szSectorFileRoot);
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
		memset(&local_60, 0, sizeof(edCBankInstall));
		if (bFileFlag == false) {
			local_60.fileFlagA = 0xc;
		}
		else {
			local_60.fileFlagA = 4;
		}
		local_60.filePath = acStack64;
		local_60.fileFunc = _gSectorInstallCallback;
		local_60.pObjectReference = this;
		bVar3 = (this->bankObject).pBankFileAccessObject->load(&local_60);
		if ((bVar3 != false) && (this->loadStage_0x8 = 1, param_3 == 0)) {
			(this->bankObject).pBankFileAccessObject->wait();
		}
	}
	return;
}

void CSectorManager::Level_Install()
{
	edCBankBufferEntry* peVar1;
	bool bVar2;
	bool bVar3;
	int iVar4;
	CSector* pvVar4;
	int iVar5;
	edCBankInstall BStack64;
	edCBankInstall BStack32;

	if ((this->baseSector).loadStage_0x8 == 1) {
		peVar1 = (this->baseSector).bankObject.pBankFileAccessObject;
		bVar2 = false;
		if ((peVar1 != (edCBankBufferEntry*)0x0) && (bVar3 = peVar1->is_loaded(), bVar3 != false)) {
			bVar2 = true;
		}
		if (bVar2) {
			memset(&BStack32, 0, sizeof(edCBankInstall));
			BStack32.fileFlagA = 0;
			BStack32.fileFunc = _gSectorInstallCallback;
			BStack32.pObjectReference = &this->baseSector;
			(this->baseSector).bankObject.pBankFileAccessObject->install(&BStack32);
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
				if ((peVar1 != (edCBankBufferEntry*)0x0) && (bVar3 = peVar1->is_loaded(), bVar3 != false)) {
					bVar2 = true;
				}
				if (bVar2) {
					memset(&BStack64, 0, sizeof(edCBankInstall));
					BStack64.fileFlagA = 0;
					BStack64.fileFunc = _gSectorInstallCallback;
					BStack64.pObjectReference = pvVar4;
					(pvVar4->bankObject).pBankFileAccessObject->load(&BStack64);
					pvVar4->loadStage_0x8 = 2;
				}
			}
			iVar5 = iVar5 + 1;
			iVar4 = iVar4 + sizeof(CSector);
		} while (iVar5 < this->sectDataCount);
	}
	return;
}

void CSectorManager::Level_Init()
{
	int iVar1;
	int iVar2;
	int iVar3;
	undefined* puVar4;
	CSectorManager* pSVar5;

	pSVar5 = Scene::ptable.g_SectorManager_00451670;
	iVar1 = this->field_0x36c;
	iVar2 = (this->baseSector).desiredSectorID;
	iVar3 = (this->baseSector).sectID;
	if ((iVar3 != iVar2) || (iVar1 == 0)) {
		if (iVar3 != iVar2) {
			if ((iVar3 != -1) &&
				(puVar4 = (Scene::ptable.g_SectorManager_00451670)->subObjArray[iVar3].pFileData, puVar4 != (undefined*)0x0))
			{
				IMPLEMENTATION_GUARD(
				Scene::_pinstance->PopFogAndClippingSettings((S_STREAM_FOG_DEF*)(puVar4 + 0xc));)
			}
			if ((iVar2 != -1) && (puVar4 = pSVar5->subObjArray[iVar2].pFileData, puVar4 != (undefined*)0x0)) {
				Scene::_pinstance->PushFogAndClippingSettings(*(float*)(puVar4 + 0x1c), (S_STREAM_FOG_DEF*)(puVar4 + 0xc));
			}
		}
		(this->baseSector).sectID = iVar2;
		this->field_0x36c = iVar1;
	}
	return;
}

void CSectorManager::Level_AddAll(ByteCode* pMemoryStream)
{
	char* pcVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	ScenaricCondition local_4;

	pMemoryStream->GetChunk();
	iVar2 = pMemoryStream->GetS32();
	iVar5 = 0;
	if (0 < iVar2) {
		do {
			pcVar1 = pMemoryStream->currentSeekPos;
			pMemoryStream->currentSeekPos = pcVar1 + 0x24;
			iVar3 = *(int*)(pcVar1 + 4);
			if ((iVar3 < 0) || (0x1d < iVar3)) {
				iVar3 = 0;
			}
			this->subObjArray[iVar3].pFileData = (undefined*)pcVar1;
			iVar3 = pMemoryStream->GetS32();
			iVar4 = 0;
			if (0 < iVar3) {
				do {
					pMemoryStream->GetS32();
					//local_4 = {};
					local_4.Create(pMemoryStream);
					local_4.IsVerified();
					iVar4 = iVar4 + 1;
				} while (iVar4 < iVar3);
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < iVar2);
	}
	return;
}

void CSectorManager::Level_Manage()
{
	CSector* pSector;
	int iVar1;
	int iVar2;

	this->baseSector.Level_Manage((this->baseSector).sectID, this->field_0x36c);
	iVar2 = (this->baseSector).desiredSectorID;
	iVar1 = (this->baseSector).currentSectorID;
	if (iVar1 != iVar2) {
		Scene::_pinstance->Level_SectorChange(iVar1, iVar2);
		(this->baseSector).currentSectorID = (this->baseSector).desiredSectorID;
	}
	iVar2 = 0;
	if (0 < this->sectDataCount) {
		IMPLEMENTATION_GUARD(
		iVar1 = 0;
		do {
			pSector = (CSector*)((int)&this->sectorArray->desiredSectorID + iVar1);
			::Level_Manage(pSector, pSector->sectID, (long)this->field_0x36c);
			iVar2 = iVar2 + 1;
			iVar1 = iVar1 + 0x144;
		} while (iVar2 < this->sectDataCount);)
	}
	return;
}

void CSectorManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	uint flags;

	if ((newSectorId != -1) && (flags = this->subObjArray[newSectorId].flags, this->field_0x37c != flags)) {
		IMPLEMENTATION_GUARD(
		SetupCompanionSectors(this, flags);)
	}
	return;
}

void CSectorManager::Level_Create(ByteCode* pMemoryStream)
{
	pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + 4;
	return;
}

void CSector::Level_Manage(int sectID, int param_3)
{
	int iVar1;
	uint* puVar2;
	bool bVar3;
	bool bVar4;
	Timer* pTVar5;
	ed_3D_Scene* peVar6;
	edCBankBufferEntry* pBank;

	iVar1 = this->loadStage_0x8;
	if (iVar1 == 2) {
		puVar2 = (uint*)(Scene::ptable.g_SectorManager_00451670)->subObjArray[this->desiredSectorID].pFileData;
		pTVar5 = Timer::GetTimer();
		peVar6 = GetStaticMeshMasterA_001031b0();
		this->pANHR.Manage(pTVar5->cutsceneDeltaTime, (float)puVar2[8], peVar6, 1);
		if (this->pMeshTransform != (edNODE*)0x0) {
			IMPLEMENTATION_GUARD(
			3DFileManager::ManageBackground(Scene::ptable.g_FileManager3D_00451664, this->pMeshTransform, *puVar2);)
		}
		if (this->desiredSectorID != sectID) {
			IMPLEMENTATION_GUARD(
			Flush(this);
			this->loadStage_0x8 = 0;)
		}
	}
	else {
		if (iVar1 == 1) {
			if (param_3 == 0) {
				(this->bankObject).pBankFileAccessObject->wait();
			}
			pBank = (this->bankObject).pBankFileAccessObject;
			bVar3 = false;
			if ((pBank != (edCBankBufferEntry*)0x0) && (bVar4 = pBank->is_loaded(), bVar4 != false)) {
				bVar3 = true;
			}
			if (bVar3) {
				this->loadStage_0x8 = 2;
			}
		}
		else {
			if ((iVar1 == 0) && (sectID != -1)) {
				this->Load(sectID, param_3, true);
			}
		}
	}
	return;
}
