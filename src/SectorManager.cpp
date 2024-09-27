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

#include "edBankFile.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "port/pointer_conv.h"
#include "CollisionManager.h"
#include "ActorManager.h"
#include "ActorClusteriser.h"

#define SECTOR_LOG(level, format, ...) MY_LOG_CATEGORY("Sector", level, format, ##__VA_ARGS__)

CSectorManager::CSectorManager()
{
	(this->baseSector).pANHR.pThis = 0;
	edF32Matrix4SetIdentityHard(&CHierarchyAnm::_gscale_mat);
	(this->baseSector).desiredSectorID = -1;
	(this->baseSector).sectorIndex = -1;
	(this->baseSector).bankObject.pBankFileAccessObject = (edCBankBufferEntry*)0x0;
	(this->baseSector).pBackgroundNode = (edNODE*)0x0;
	(this->baseSector).pObbTree = (edObbTREE_DYN*)0x0;
	(this->baseSector).loadStage_0x8 = 0;
	(this->baseSector).field_0x134 = 0.0;
	(this->baseSector).pMeshTransformParent_0x130 = (edNODE*)0x0;
	memset(&(this->baseSector).sectorTexture, 0, sizeof(ed_g2d_manager));
	memset(&(this->baseSector).sectorMesh, 0, sizeof(ed_g3d_manager));
	memset(&(this->baseSector).backgroundTexture, 0, sizeof(ed_g2d_manager));
	memset(&(this->baseSector).backgroundMesh, 0, sizeof(ed_g3d_manager));
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
		pSVar2->aSectorHierarchies = 0x0;
		pSVar2[1].flags = 0;
		pSVar2[1].pFileData = (undefined*)0x0;
		pSVar2[1].aSectorHierarchies = 0x0;
		pSVar2[2].flags = 0;
		pSVar2[2].pFileData = (undefined*)0x0;
		pSVar2[2].aSectorHierarchies = 0x0;
		pSVar2[3].flags = 0;
		pSVar2[3].pFileData = (undefined*)0x0;
		pSVar2[3].aSectorHierarchies = 0x0;
		pSVar2[4].flags = 0;
		pSVar2[4].pFileData = (undefined*)0x0;
		pSVar2[4].aSectorHierarchies = 0x0;
		pSVar2[5].flags = 0;
		pSVar2[5].pFileData = (undefined*)0x0;
		pSVar2[5].aSectorHierarchies = 0x0;
		pSVar2 = pSVar2 + 6;
	} while (0 < iVar1);
	this->pSectorHierarchy = (CSectorHierarchy*)0;
	return;
}


void CSectorManager::InstallEnterSector()
{
	CSectorHierarchy* pSectorHierarchy;
	int iVar1;
	CActorClusteriser* piVar2;
	CActorsTable local_110;
	int* local_4;

	this->field_0x384 = 300;
	for (pSectorHierarchy = this->pSectorHierarchy; pSectorHierarchy != (CSectorHierarchy*)0x0;
		pSectorHierarchy = pSectorHierarchy->pNext) {
		IMPLEMENTATION_GUARD(
		if ((pSectorHierarchy->field_0x19 == 0) &&
			(iVar1 = Func_001fe870(this, pSectorHierarchy->field_0x0, &local_4), iVar1 != 0)) {
			CSectorHierarchy::Sector_Enter(pSectorHierarchy, iVar1, local_4);
		})
	}

	local_110.entryCount = 0;
	CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(CLUSTERISER, &local_110);
	iVar1 = 0;
	if (0 < local_110.entryCount) {
		do {
			piVar2 = reinterpret_cast<CActorClusteriser*>(local_110.aEntries[iVar1]);
			piVar2->Sector_Enter();
			iVar1 = iVar1 + 1;
		} while (iVar1 < local_110.entryCount);
	}
	return;
}

void CSectorManager::LevelLoading_Begin()
{
	SectorManagerSubObjOther* pLVar1;
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
	CFixedTable<int, 32> table;
	uint auStack144[31];
	int local_14;
	int local_10;
	int local_c;
	uint local_8;
	uint local_4;

	/* LoadSect
		*/
	LevelInfo* pcVar5 = &CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->nextLevelID];
	local_1a0 = pcVar5;
	/* /sect */
	edStrCatMulti(this->szSectorFileRoot, CLevelScheduler::gThis->levelPath, pcVar5->levelName, "/", "SECT", 0);

	SECTOR_LOG(LogLevel::Info, "CSectorManager::LevelLoading_Begin SectorFileRoot: {}", this->szSectorFileRoot);

	uVar12 = 0;
	memset(auStack144, 0, 0x78);
	this->sectDataCount = 0;
	this->count_0x368 = pcVar5->sectorCount_0x14;

	SECTOR_LOG(LogLevel::Info, "CSectorManager::LevelLoading_Begin Count: {}", this->count_0x368);

	iVar13 = 1;
	if (0 < this->count_0x368) {
		pcVar14 = this->subObjArray + 1;
		pLVar1 = &local_1a0->aSectorSubObj[1];

		do {
			uVar8 = pLVar1->flags;
			puVar7 = pLVar1->field_0xc;

			for (iVar10 = pLVar1->aSectorHierarchies; iVar10 != 0; iVar10 = iVar10 + -1) {
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
				this->sectorArray[iVar13].sectID = -1;
				this->sectorArray[iVar13].currentSectorID = 0;
				iVar13 = iVar13 + 1;
			} while (iVar13 < this->sectDataCount);
		}

		uVar8 = 1;
		if (0 < this->count_0x368) {
			do {
				if ((1 << (uVar8 & 0x1f) & uVar12) != 0) {
					local_4 = uVar8;
					table.Add(uVar8);
				}
				uVar8 = uVar8 + 1;
			} while ((int)uVar8 <= this->count_0x368);
		}
		iVar13 = 0;
		piVar2 = local_190;

		for (int i = 0; i < 0x1e; i++) {
			piVar2[i] = (int)local_1a0->aSectorSubObj[i].pFileData;
		}
		iVar13 = 0;
		if (0 < table.entryCount + -1) {
			piVar2 = table.aEntries;
			iVar10 = table.entryCount;
			do {
				iVar11 = iVar13 + 1;
				if (iVar11 < iVar10) {
					piVar9 = table.aEntries + iVar13;
					do {
						if (local_190[piVar2[0]] < local_190[piVar9[1]]) {
							table.Swap(iVar11, iVar13);
						}
						iVar11 = iVar11 + 1;
						piVar9 = piVar9 + 1;
						iVar10 = table.entryCount;
					} while (iVar11 < table.entryCount);
				}
				iVar13 = iVar13 + 1;
				piVar2 = piVar2 + 1;
			} while (iVar13 < iVar10 + -1);
		}

		this->field_0x370 = 1;
		iVar13 = 0;
		piVar2 = table.aEntries;
		while ((iVar13 < table.entryCount && (this->field_0x370 != 0))) {
			local_8 = table.aEntries[iVar13];
			uVar8 = ~(1 << (local_8 & 0x1f)) & auStack144[local_8];
			uVar12 = 0;
			if (uVar8 != 0) {
				uVar6 = 1;
				if (0 < this->count_0x368) {
					pcVar14 = &this->subObjArray[uVar6];
					do {
						if (((1 << (uVar6 & 0x1f) & uVar8) != 0) && (pcVar14->field_0xc != -1)) {
							uVar12 = uVar12 | 1 << (pcVar14->field_0xc & 0x1fU);
						}

						uVar6 = uVar6 + 1;
						pcVar14 = pcVar14 + 1;
					} while (uVar6 <= this->count_0x368);
				}
			}

			for (uVar8 = 0; ((1 << (uVar8 & 0x1f) & uVar12) != 0 && ((int)uVar8 < this->sectDataCount)); uVar8 = uVar8 + 1) {
			}

			if ((int)uVar8 < this->sectDataCount) {
				this->subObjArray[local_8].field_0xc = (char)uVar8;
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
			local_c = table.aEntries[0];
			iVar13 = local_190[table.aEntries[0]];
			iVar10 = 0;
			if (0 < this->sectDataCount) {
				iVar11 = 0;
				do {
					this->sectorArray[iVar10].currentSectorID = iVar13;
					iVar10 = iVar10 + 1;
					iVar11 = iVar11 + 0x144;
				} while (iVar10 < this->sectDataCount);
			}

			iVar13 = 1;
			if (0 < this->count_0x368) {
				pcVar14 = &this->subObjArray[1];
				do {
					pcVar14->field_0xc = -1;
					iVar13 = iVar13 + 1;
					pcVar14 = pcVar14 + 1;
				} while (iVar13 <= this->count_0x368);
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
	if (CLevelScheduler::gThis->loadStage_0x5b48 == 4) {
		if ((this->baseSector).loadStage_0x8 == 1) {
			peVar3 = (this->baseSector).bankObject.pBankFileAccessObject;
			bVar5 = false;
			if ((peVar3 != (edCBankBufferEntry*)0x0) && (bVar6 = peVar3->is_loaded(), bVar6 != false)) {
				bVar5 = true;
			}
			bVar6 = true;
			if (bVar5) {
				iVar10 = 0;
				while ((iVar10 < this->sectDataCount && (bVar6))) {
					pSectData = this->sectorArray + iVar10;
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
					iVar10 = iVar10 + 1;
				}
				if (bVar6) {
					bVar11 = false;
				}
			}
		}
		else {
			iVar10 = CLevelScheduler::gThis->level_0x5b40;
			iVar8 = this->field_0x36c;
			iVar9 = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->nextLevelID].bankSizeSect;
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
					SetupCompanionSectors(uVar1);
				}

				iVar10 = 0;
				if (0 < this->sectDataCount) {
					do {
						CSector* pSector = &this->sectorArray[iVar10];
						if (pSector->sectID == -1) {
							iVar2 = pSector->currentSectorID;
							memset(&BStack64, 0, sizeof(edCBankInstall));
							pSector->bankObject.initialize(iVar2 + 0x1000, 1, &BStack64);
							pSector->loadStage_0x8 = 0;
						}

						iVar10 = iVar10 + 1;
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

void CSectorManager::SetupCompanionSectors(uint flags)
{
	uint uVar1;
	SectorManagerSubObj* pcVar2;
	uint uVar3;
	int iVar4;
	CSector* pSector;
	int iVar6;
	long lVar7;

	iVar6 = 0;

	if (0 < this->sectDataCount) {
		do {
			pSector = this->sectorArray + iVar6;
			uVar1 = 1 << (pSector->sectID & 0x1fU);

			if ((pSector->sectID != 0xffffffff) && ((uVar1 & flags) == 0)) {
				this->field_0x37c = this->field_0x37c & ~uVar1;
				pSector->sectID = -1;
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < this->sectDataCount);
	}

	uVar1 = 1;
	if (0 < this->count_0x368) {
		do {
			pcVar2 = &this->subObjArray[uVar1];
			uVar3 = 1 << (uVar1 & 0x1f);
			if ((uVar3 & flags) != (uVar3 & this->field_0x37c)) {
				lVar7 = pcVar2->field_0xc;
				if (this->field_0x370 == 0) {
					lVar7 = 0;
					for (iVar6 = 0; (lVar7 < this->sectDataCount && this->sectorArray[lVar7].sectID != -1); iVar6 = iVar6 + 0x144) {
						lVar7 = (long)((int)lVar7 + 1);
					}
				}

				if (lVar7 < this->sectDataCount) {
					this->sectorArray[(int)lVar7].sectID = uVar1;
					this->field_0x37c = this->field_0x37c | uVar3;
				}
			}

			uVar1 = uVar1 + 1;
		} while ((int)uVar1 <= this->count_0x368);
	}

	return;
}

void CSector::InstallCallback()
{
	edLIST* pList;
	CSectorHierarchy* pSectorHierarchy;
	ed_3D_Scene* pStaticMeshMaster;
	ed_3d_hierarchy_node* pHierNode;
	C3DFileManager* p3DFileManager;
	CSectorManager* pSectorManager;
	bool bVar1;
	uint uVar2;
	undefined* puVar3;
	TextureInfo* pTextureInfo;
	edNODE* pMVar4;
	uint meshSize;
	char* pcVar6;
	edNODE* pNewNode;
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
	edBANK_ENTRY_INFO bankEntry;
	int iStack8;
	short sStack2;

#ifdef PLATFORM_WIN
	std::string szBackgroundFileName;
#endif

	pAnimHierarchy = (char*)0x0;
	pFileData = (char*)0x0;
	this->pObbTree = (edObbTREE_DYN*)0x0;
	local_40 = 0;
	pcVar6 = (char*)0x0;
	uVar2 = (this->bankObject).pBankFileAccessObject->get_element_count();
	inFileIndex = 0;
	pMeshData = pcVar6;
	meshSize = unaff_s5_lo;
	if (uVar2 != 0) {
		do {
			bVar1 = (this->bankObject).pBankFileAccessObject->get_info(inFileIndex, &bankEntry, (char*)0x0);
			pMeshData = pcVar6;
			meshSize = unaff_s5_lo;
			if (bVar1 == false) break;
			uVar9 = bankEntry.type << 0x10 | bankEntry.stype;
			if (uVar9 == 0x170001) {
				SECTOR_LOG(LogLevel::Info, "Sector::Init Anim Heirarchy: {}\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
				pAnimHierarchy = bankEntry.fileBufferStart;
				local_40 = bankEntry.size;
			}
			else {
				if (uVar9 == 0x50003) {
					SECTOR_LOG(LogLevel::Info, "Sector::Init Background Texture: {}\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
					NAME_NEXT_OBJECT(DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
					ed3DInstallG2D(bankEntry.fileBufferStart, bankEntry.size, &iStack8, &this->backgroundTexture, 1);
				}
				else {
					if (uVar9 == 0x40002) {
#ifdef PLATFORM_WIN
						char* pBackgroundFileName = DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex);
						szBackgroundFileName = pBackgroundFileName;
#endif
						SECTOR_LOG(LogLevel::Info, "Sector::Init Background Mesh: {}\n", pBackgroundFileName);
						local_30 = bankEntry.size;
						pFileData = bankEntry.fileBufferStart;
					}
					else {
						if (uVar9 == 0x70001) {
							SECTOR_LOG(LogLevel::Info, "Sector::Init Init Collision: {}\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
							this->pObbTree = CScene::ptable.g_CollisionManager_00451690->InstallColFile(bankEntry.fileBufferStart, bankEntry.size);
						}
						else {
							if (uVar9 == 0x50001) {
								SECTOR_LOG(LogLevel::Info, "Sector::Init Sector Texture: {}\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
								NAME_NEXT_OBJECT(DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
								ed3DInstallG2D(bankEntry.fileBufferStart, bankEntry.size, &iStack8, &this->sectorTexture, 1);
							}
							else {
								SECTOR_LOG(LogLevel::Info, "Sector::Init Sector Mesh: {}\n", DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
								NAME_NEXT_OBJECT(DebugFindFilePath((this->bankObject).pBankFileAccessObject->pFileHeader, inFileIndex));
								pMeshData = bankEntry.fileBufferStart;
								meshSize = bankEntry.size;
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
	pSectorManager = CScene::ptable.g_SectorManager_00451670;
	p3DFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pStaticMeshMaster = CScene::_scene_handleA;

	ed3DInstallG3D(pMeshData, meshSize, 0, &iStack8, &this->sectorTexture, 0xc, &this->sectorMesh);

	pTextureInfo = p3DFileManager->GetCommonSectorG2D();
	ed3DLinkG2DToG3D(&this->sectorMesh, &pTextureInfo->manager);

	ed3DScenePushCluster(pStaticMeshMaster, &this->sectorMesh);

	if (pFileData == (char*)0x0) {
		this->pBackgroundNode = (edNODE*)0x0;
	}
	else {
		NAME_NEXT_OBJECT("%s", szBackgroundFileName.c_str());
		p3DFileManager->HideCommonBackground();
		ed3DInstallG3D(pFileData, local_30, 0, &iStack8, &this->backgroundTexture, 0xc, &this->backgroundMesh);
		pMVar4 = ed3DHierarchyAddToScene(pStaticMeshMaster, &this->backgroundMesh, (char*)0x0);
		this->pBackgroundNode = pMVar4;
		if (this->pBackgroundNode != (edNODE*)0x0) {
			p3DFileManager->SetupBackground(this->pBackgroundNode);
		}
	}
	this->desiredSectorID = this->sectorIndex;
	this->sectorIndex = -1;
	this->pANHR.Install((MeshData_ANHR*)pAnimHierarchy, local_40, &this->sectorMesh, pStaticMeshMaster);
	ed3DHierarchyCopyHashCode(&this->sectorMesh);
	pMVar4 = gHierarchyManagerFirstFreeNode;
	pHierNode = gHierarchyManagerBuffer;
	ed_Chunck* pHALL = (this->sectorMesh).HALL;
	pList = pStaticMeshMaster->pHierListA;
	meshSize = edChunckGetNb(pHALL + 1, (char*)pHALL + pHALL->size);
	ed_hash_code* pHashCode = (ed_hash_code*)edHashcodeGet(0x43494d414e5944, (ed_Chunck*)((this->sectorMesh).HALL + 1));
	ed_Chunck* pChunck = 0;
	if (pHashCode != (ed_hash_code*)0x0) {
		pChunck = (ed_Chunck*)LOAD_SECTION(pHashCode->pData);
	}
	if (pChunck == (ed_Chunck*)0x0) {
		pNewNode = (edNODE*)0x0;
	}
	else {
		pNewNode = ed3DHierarchyAddNode(pList, pHierNode, pMVar4, (ed_g3d_hierarchy*)(pChunck + 1), (ed_3d_hierarchy*)0x0);
		ed3DHierarchyAddSonsToList
		(pList, pHierNode, pMVar4, pChunck, pNewNode, (ed_hash_code*)(pHALL + 2), (meshSize & 0xffff) - 1);
		ed3DHierarchyRefreshSonNumbers(pNewNode, &sStack2);
	}
	this->pMeshTransformParent_0x130 = pNewNode;
	pSectorHierarchy = pSectorManager->subObjArray[this->desiredSectorID].aSectorHierarchies;
	do {
		if (pSectorHierarchy == (CSectorHierarchy*)0x0) {
			fVar14 = edTimerTimeGet();
			this->field_0x134 = fVar14 - this->field_0x134;
			pSectorManager->InstallEnterSector();
			return;
		}

		IMPLEMENTATION_GUARD(
		lVar13 = pSectorHierarchy->field_0x0;
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
				CSectorHierarchy::Sector_Enter(pSectorHierarchy, iVar8, piVar11);
			}
		}
		pSectorHierarchy = pSectorHierarchy->pSectorHierarchy;)
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
		sectStringLength = edStrCopy(acStack64, (CScene::ptable.g_SectorManager_00451670)->szSectorFileRoot);
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
					(pvVar4->bankObject).pBankFileAccessObject->install(&BStack64);
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

	pSVar5 = CScene::ptable.g_SectorManager_00451670;
	iVar1 = this->field_0x36c;
	iVar2 = (this->baseSector).desiredSectorID;
	iVar3 = (this->baseSector).sectID;
	if ((iVar3 != iVar2) || (iVar1 == 0)) {
		if (iVar3 != iVar2) {
			if ((iVar3 != -1) &&
				(puVar4 = (CScene::ptable.g_SectorManager_00451670)->subObjArray[iVar3].pFileData, puVar4 != (undefined*)0x0))
			{
				CScene::_pinstance->PopFogAndClippingSettings((S_STREAM_FOG_DEF*)(puVar4 + 0xc));
			}
			if ((iVar2 != -1) && (puVar4 = pSVar5->subObjArray[iVar2].pFileData, puVar4 != (undefined*)0x0)) {
				CScene::_pinstance->PushFogAndClippingSettings(*(float*)(puVar4 + 0x1c), (S_STREAM_FOG_DEF*)(puVar4 + 0xc));
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
		CScene::_pinstance->Level_SectorChange(iVar1, iVar2);
		(this->baseSector).currentSectorID = (this->baseSector).desiredSectorID;
	}

	iVar2 = 0;
	if (0 < this->sectDataCount) {
		do {
			pSector = this->sectorArray + iVar2;
			pSector->Level_Manage(pSector->sectID, (long)this->field_0x36c);
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->sectDataCount);
	}
	return;
}

void CSectorManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	uint flags;

	if ((newSectorId != -1) && (flags = this->subObjArray[newSectorId].flags, this->field_0x37c != flags)) {
		SetupCompanionSectors(flags);
	}
	return;
}

void CSectorManager::Level_Create(ByteCode* pMemoryStream)
{
	pMemoryStream->currentSeekPos = pMemoryStream->currentSeekPos + 4;
	return;
}

void CSectorManager::SwitchToSector(int sectID, bool param_3)
{
	int iVar1;
	undefined* puVar2;
	CSectorManager* pCVar3;

	SECTOR_LOG(LogLevel::Info, "CSectorManager::SwitchToSector Switching to sector: {} - {}", sectID, param_3);

	pCVar3 = CScene::ptable.g_SectorManager_00451670;

	iVar1 = (this->baseSector).sectID;

	SECTOR_LOG(LogLevel::Info, "CSectorManager::SwitchToSector Current sector: {}", iVar1);

	if ((iVar1 != sectID) || (param_3 == false)) {
		if (iVar1 != sectID) {
			if ((iVar1 != -1) && (puVar2 = (CScene::ptable.g_SectorManager_00451670)->subObjArray[iVar1].pFileData, puVar2 != (undefined*)0x0))
			{
				CScene::_pinstance->PopFogAndClippingSettings((S_STREAM_FOG_DEF*)(puVar2 + 0xc));
			}

			if ((sectID != -1) && (puVar2 = pCVar3->subObjArray[sectID].pFileData, puVar2 != (undefined*)0x0)) {
				CScene::_pinstance->PushFogAndClippingSettings(*(float*)(puVar2 + 0x1c), (S_STREAM_FOG_DEF*)(puVar2 + 0xc));
			}
		}

		(this->baseSector).sectID = sectID;

		this->field_0x36c = (int)param_3;
	}

	return;
}

void CSectorManager::Flush()
{
	CSectorHierarchy* pCVar1;
	edNODE* peVar2;
	int iVar3;
	CActorsTable local_110;
	int* piStack4;

	local_110.entryCount = 0;
	CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(CLUSTERISER, &local_110);

	iVar3 = 0;
	if (0 < local_110.entryCount) {
		do {
			//::EmptyFunction();
			iVar3 = iVar3 + 1;
		} while (iVar3 < local_110.entryCount);
	}

	for (pCVar1 = this->pSectorHierarchy; pCVar1 != (CSectorHierarchy*)0x0; pCVar1 = pCVar1->pNext) {
		IMPLEMENTATION_GUARD(
		if ((pCVar1->field_0x19 != 0) &&
			((peVar2 = (edNODE*)Func_001fe870(this, pCVar1->field_0x0, &piStack4), peVar2 == (edNODE*)0x0 ||
				(peVar2 != pCVar1->pNode)))) {
			pCVar1->field_0x19 = 0;
			pCVar1->pNode = (edNODE*)0x0;
			pCVar1->pHier = (S_HIERANM_HIER*)0x0;
		})
	}
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
		puVar2 = (uint*)(CScene::ptable.g_SectorManager_00451670)->subObjArray[this->desiredSectorID].pFileData;
		pTVar5 = Timer::GetTimer();
		peVar6 = GetStaticMeshMasterA_001031b0();
		this->pANHR.Manage(pTVar5->cutsceneDeltaTime, (float)puVar2[8], peVar6, 1);
		if (this->pBackgroundNode != (edNODE*)0x0) {
			CScene::ptable.g_C3DFileManager_00451664->ManageBackground(this->pBackgroundNode, *puVar2);
		}
		if (this->desiredSectorID != sectID) {
			Flush();
			this->loadStage_0x8 = 0;
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

void CSector::Flush()
{
	CSectorHierarchy* pSectorHierarchy;
	uint uVar2;
	edNODE* pNode;
	ed_3D_Scene* pScene;
	C3DFileManager* p3DFileManager;
	CSectorManager* pSectorManager;
	uint uVar3;
	MeshData_ANHR* pMVar4;

	pSectorManager = CScene::ptable.g_SectorManager_00451670;
	p3DFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pScene = CScene::_scene_handleA;

	for (pSectorHierarchy = (CScene::ptable.g_SectorManager_00451670)->subObjArray[this->desiredSectorID].aSectorHierarchies;
		pSectorHierarchy != (CSectorHierarchy*)0x0; pSectorHierarchy = pSectorHierarchy->pNext) {
		pSectorHierarchy->field_0x19 = 0;
		pSectorHierarchy->pNode = (edNODE*)0x0;
		pSectorHierarchy->pHier = (S_HIERANM_HIER*)0x0;
	}

	if (this->pMeshTransformParent_0x130 != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(pScene, this->pMeshTransformParent_0x130);
		this->pMeshTransformParent_0x130 = (edNODE*)0x0;
	}

	pMVar4 = (this->pANHR).pThis;
	if (pMVar4 != (MeshData_ANHR*)0x0) {
		uVar2 = pMVar4->otherEntryCount;

		int* pNext = (int*)(pMVar4 + 1);

		uVar3 = 0;
		if (uVar2 != 0) {
			do {
				ANHR_Internal* peVar3 = LOAD_SECTION_CAST(ANHR_Internal*, pNext[uVar3]);

				if (peVar3->pHierNode != 0x0) {
					ed3DHierarchyRemoveFromScene(pScene, LOAD_SECTION_CAST(edNODE*, peVar3->pHierNode));
				}

				uVar3 = uVar3 + 1;
			} while (uVar3 < uVar2);
		}

		(this->pANHR).pThis = (MeshData_ANHR*)0x0;
	}

	ed3DScenePopCluster(pScene, &this->sectorMesh);
	ed3DUnInstallG3D(&this->sectorMesh);
	ed3DUnInstallG2D(&this->sectorTexture);

	if (this->pBackgroundNode != (edNODE*)0x0) {
		ed3DHierarchyRemoveFromScene(pScene, this->pBackgroundNode);
		ed3DUnInstallG3D(&this->backgroundMesh);
		ed3DUnInstallG2D(&this->backgroundTexture);
		this->pBackgroundNode = (edNODE*)0x0;
		p3DFileManager->ShowCommonBackground();
	}

	if (this->pObbTree != (edObbTREE_DYN*)0x0) {
		CScene::ptable.g_CollisionManager_00451690->UninstallColFile(this->pObbTree);
	}

	pSectorManager->Flush();
	this->desiredSectorID = -1;
	this->sectorIndex = -1;
	return;
}

void CSectorHierarchy::Create(ByteCode* pByteCode)
{
	ulong uVar1;

	uVar1 = pByteCode->GetU64();
	this->field_0x0 = uVar1;
	this->pNext = (CSectorHierarchy*)0x0;
	this->pNode = (edNODE*)0x0;
	this->pHier = (S_HIERANM_HIER*)0x0;
	this->alpha = 0x80;
	this->field_0x19 = 0;
	this->flags = 0x20000000;
	return;
}

void CSectorHierarchy::Init(int param_2)
{
	MeshData_ANHR* pMVar1;
	char* pcVar2;
	edNODE* peVar3;
	ed_g3d_manager* peVar4;
	long lVar5;
	uint uVar6;
	int iVar7;
	CSector* pCVar8;
	MeshData_ANHR* pMVar9;
	S_HIERANM_HIER* pHier;
	S_HIERANM_HIER* local_4;
	CSectorManager* pSectormanager;

	pSectormanager = CScene::ptable.g_SectorManager_00451670;
	if (this->field_0x0 != 0) {
		if (param_2 == -1) {
			this->pNext = (CScene::ptable.g_SectorManager_00451670)->pSectorHierarchy;
			pSectormanager->pSectorHierarchy = this;
			IMPLEMENTATION_GUARD(
			peVar3 = (edNODE*)CSectorManager::Func_001fe870(pSectormanager, this->field_0x0, (int**)&local_4);
			if (peVar3 != (edNODE*)0x0) {
				Sector_Enter(this, peVar3, local_4);
			})
		}
		else {
			IMPLEMENTATION_GUARD(
			pcVar2 = (CScene::ptable.g_SectorManager_00451670)->szSectorFileRoot;
			this->pNext = *(CSectorHierarchy**)(pcVar2 + param_2 * 0x10 + -4 + 0x4c);
			*(CSectorHierarchy**)(pcVar2 + param_2 * 0x10 + -4 + 0x4c) = this;
			pCVar8 = &pSectormanager->baseSector;
			if (param_2 != (pSectormanager->baseSector).desiredSectorID) {
				iVar7 = 0;
				if (0 < pSectormanager->sectDataCount) {
					pCVar8 = pSectormanager->sectorArray;
					do {
						if (param_2 == pCVar8->desiredSectorID) goto LAB_001fdad8;
						iVar7 = iVar7 + 1;
						pCVar8 = pCVar8 + 1;
					} while (iVar7 < pSectormanager->sectDataCount);
				}
				pCVar8 = (CSector*)0x0;
			}
		LAB_001fdad8:
			if (pCVar8 != (CSector*)0x0) {
				if ((pCVar8->loadStage_0x8 == 2) || ((pCVar8->loadStage_0x8 == 1 && (pCVar8->sectorIndex == -1)))) {
					peVar4 = &pCVar8->sectorMesh;
				}
				else {
					peVar4 = (ed_g3d_manager*)0x0;
				}
				if (peVar4 != (ed_g3d_manager*)0x0) {
					lVar5 = this->field_0x0;
					if (lVar5 != 0) {
						pMVar1 = (pCVar8->pANHR).pThis;
						if (pMVar1 != (MeshData_ANHR*)0x0) {
							pMVar9 = pMVar1 + 1;
							uVar6 = 0;
							if (pMVar1->otherEntryCount != 0) {
								do {
									pHier = *(S_HIERANM_HIER**)pMVar9->field_0x0;
									if ((*(int*)&pHier->field_0x4 != 0) && (lVar5 == *(long*)(*(int*)&pHier->field_0x4 + 0x80)))
										goto LAB_001fdb90;
									uVar6 = uVar6 + 1;
									pMVar9 = (MeshData_ANHR*)&pMVar9->field_0x4;
								} while (uVar6 < pMVar1->otherEntryCount);
							}
						}
						pHier = (S_HIERANM_HIER*)0x0;
					LAB_001fdb90:
						if (pHier == (S_HIERANM_HIER*)0x0) {
							peVar3 = (edNODE*)Func_001fff60(pCVar8->pMeshTransformParent_0x130, lVar5);
						}
						else {
							peVar3 = *(edNODE**)pHier;
						}
						if (peVar3 != (edNODE*)0x0) {
							Sector_Enter(this, peVar3, pHier);
						}
					}
				}
			})
		}
	}
	return;
}

void CSectorHierarchy::Term(int index)
{
	if (this->field_0x0 != 0) {
		this->pNode = (edNODE*)0x0;
		this->pHier = (S_HIERANM_HIER*)0x0;
		IMPLEMENTATION_GUARD(
		CScene::ptable.g_SectorManager_00451670->UnregisterDynamicHierarchy(this, index);)
	}

	return;
}

void CSectorHierarchy::SetHiddenOn()
{
	S_HIERANM_HIER* pSVar1;

	if ((this->flags & 0x40000000) == 0) {
		this->flags = this->flags | 0x40000000;
		pSVar1 = this->pHier;
		if (pSVar1 == (S_HIERANM_HIER*)0x0) {
			if (this->pNode != (edNODE*)0x0) {
				ed3DHierarchyNodeSetFlag(this->pNode, 0x40);
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			pSVar1->flags_0x20 = pSVar1->flags_0x20 | 0x40000000;)
		}
	}
	return;
}

void CSectorHierarchy::SetHiddenOff()
{
	S_HIERANM_HIER* pSVar1;

	if ((this->flags & 0x40000000) != 0) {
		this->flags = this->flags & 0xbfffffff;
		pSVar1 = this->pHier;
		if (pSVar1 == (S_HIERANM_HIER*)0x0) {
			if (this->pNode != (edNODE*)0x0) {
				ed3DHierarchyNodeClrFlag(this->pNode, 0x40);
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			pSVar1->flags_0x20 = pSVar1->flags_0x20 & 0xbfffffff;)
		}
	}
	return;
}
