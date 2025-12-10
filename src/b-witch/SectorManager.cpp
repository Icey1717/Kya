#include "SectorManager.h"
#include <string.h>
#include "EdenLib/edSys/sources/EdSystem.h"
#include "LevelScheduler.h"
#include "TimeController.h"
#include "Rendering/CameraPanMasterHeader.h"
#include "DlistManager.h"
#include "edStr.h"
#include "MathOps.h"
#include "ScenaricCondition.h"
#include "MemoryStream.h"

#include "edBank/edBankFile.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "port/pointer_conv.h"
#include "CollisionManager.h"
#include "ActorManager.h"
#include "ActorClusteriser.h"
#include "EdFileBase.h"

#define SECTOR_LOG(level, format, ...) MY_LOG_CATEGORY("Sector", level, format, ##__VA_ARGS__)

CSectorManager::CSectorManager()
{
	(this->baseSector).pANHR.pThis = 0;
	edF32Matrix4SetIdentityHard(&CHierarchyAnm::_gscale_mat);
	(this->baseSector).desiredSectorID = -1;
	(this->baseSector).sectorIndex = -1;
	(this->baseSector).pBankBufferEntry = (edCBankBufferEntry*)0x0;
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
	this->nbSectors = 0;
	this->aSectors = (CSector*)0x0;
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
	edNODE* pNode;
	CActorClusteriser* piVar2;
	CActorsTable local_110;
	S_HIERANM_HIER* local_4;

	this->field_0x384 = 300;
	for (pSectorHierarchy = this->pSectorHierarchy; pSectorHierarchy != (CSectorHierarchy*)0x0;
		pSectorHierarchy = pSectorHierarchy->pNext) {
		if ((pSectorHierarchy->field_0x19 == 0) &&
			(pNode = RegisterDynamicHierarchy(pSectorHierarchy->hash, &local_4), pNode != 0)) {
			pSectorHierarchy->Sector_Enter(pNode, local_4);
		}
	}

	local_110.nbEntries = 0;
	CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(CLUSTERISER, &local_110);
	iVar1 = 0;
	if (0 < local_110.nbEntries) {
		do {
			piVar2 = reinterpret_cast<CActorClusteriser*>(local_110.aEntries[iVar1]);
			piVar2->Sector_Enter();
			iVar1 = iVar1 + 1;
		} while (iVar1 < local_110.nbEntries);
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
	S_COMPANION_INFO* pCompanionInfo;
	uint uVar8;
	int* piVar9;
	int iVar10;
	int iVar11;
	uint uVar12;
	int iVar13;
	SectorManagerSubObj* pcVar14;
	S_LEVEL_INFO* local_1a0;
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
	S_LEVEL_INFO* pcVar5 = &CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->nextLevelID];
	local_1a0 = pcVar5;
	/* /sect */
	edStrCatMulti(this->szSectorFileRoot, CLevelScheduler::gThis->levelPath, pcVar5->levelName, "/", "SECT", 0);

	SECTOR_LOG(LogLevel::Info, "CSectorManager::LevelLoading_Begin SectorFileRoot: {}", this->szSectorFileRoot);

	uVar12 = 0;
	memset(auStack144, 0, 0x78);
	this->nbSectors = 0;
	this->count_0x368 = pcVar5->maxSectorId;

	SECTOR_LOG(LogLevel::Info, "CSectorManager::LevelLoading_Begin Count: {}", this->count_0x368);

	iVar13 = 1;
	if (0 < this->count_0x368) {
		pcVar14 = this->subObjArray + 1;
		pLVar1 = local_1a0->aSectorSubObj + 1;

		do {
			uVar8 = pLVar1->flags;
			pCompanionInfo = pLVar1->aCompanionInfo;

			for (iVar10 = pLVar1->nbSectorConditions; iVar10 != 0; iVar10 = iVar10 + -1) {
				ScenaricCondition* pCond = &pCompanionInfo->cond;

				uVar4 = pCond->IsVerified();
				if (uVar4 == 0) {
					uVar8 = uVar8 & ~(1 << (pCompanionInfo->conditionIdentifier & 0x1f));
				}

				pCompanionInfo = pCompanionInfo + 1;
			}

			pcVar14->flags = uVar8;
			pcVar14->field_0xc = -1;

			if (uVar8 != 0) {
				iVar10 = 0;
				uVar6 = 1;
				if (0 < this->count_0x368) {
					uint* pCur = auStack144;
					do {
						pCur = pCur + 1;
						uVar5 = 1 << (uVar6 & 0x1f);
						if ((uVar5 & uVar8) != 0) {
							uVar12 = uVar12 | uVar5;
							iVar10 = iVar10 + 1;
							*pCur = *pCur | uVar8;
						}
						uVar6 = uVar6 + 1;
					} while ((int)uVar6 <= this->count_0x368);
				}

				if (this->nbSectors < iVar10) {
					this->nbSectors = iVar10;
				}
			}

			iVar13 = iVar13 + 1;
			pcVar14 = pcVar14 + 1;
			pLVar1 = pLVar1 + 1;
		} while (iVar13 <= this->count_0x368);
	}

	uVar8 = this->nbSectors;
	if (uVar8 == 0) {
		this->aSectors = (CSector*)0x0;
	}
	else {
		this->aSectors = new CSector[uVar8];
		iVar13 = 0;
		if (0 < this->nbSectors) {
			iVar10 = 0;
			do {
				this->aSectors[iVar13].sectID = -1;
				this->aSectors[iVar13].currentSectorID = 0;
				iVar13 = iVar13 + 1;
			} while (iVar13 < this->nbSectors);
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
			piVar2[i] = local_1a0->aSectorSubObj[i].bankSize;
		}

		iVar13 = 0;
		if (0 < table.nbEntries + -1) {
			piVar2 = table.aEntries;
			iVar10 = table.nbEntries;
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
						iVar10 = table.nbEntries;
					} while (iVar11 < table.nbEntries);
				}
				iVar13 = iVar13 + 1;
				piVar2 = piVar2 + 1;
			} while (iVar13 < iVar10 + -1);
		}

		this->field_0x370 = 1;
		iVar13 = 0;
		piVar2 = table.aEntries;
		while ((iVar13 < table.nbEntries && (this->field_0x370 != 0))) {
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

			for (uVar8 = 0; ((1 << (uVar8 & 0x1f) & uVar12) != 0 && ((int)uVar8 < this->nbSectors)); uVar8 = uVar8 + 1) {
			}

			if ((int)uVar8 < this->nbSectors) {
				this->subObjArray[local_8].field_0xc = (char)uVar8;
				if (this->aSectors[uVar8].currentSectorID < local_190[local_8]) {
					this->aSectors[uVar8].currentSectorID = local_190[local_8];
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
			if (0 < this->nbSectors) {
				iVar11 = 0;
				do {
					this->aSectors[iVar10].currentSectorID = iVar13;
					iVar10 = iVar10 + 1;
					iVar11 = iVar11 + 0x144;
				} while (iVar10 < this->nbSectors);
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
	int iVar4;
	bool bVar5;
	bool bVar6;
	bool bVar7;
	CSector* pSectData;
	int iVar8;
	int iVar9;
	int curSectorIndex;
	bool bStillLoading;

	bStillLoading = true;

	if (CLevelScheduler::gThis->loadStage_0x5b48 == 4) {
		if ((this->baseSector).loadStage_0x8 == 1) {
			edCBankBufferEntry* pBaseSectorBankFileAccess = (this->baseSector).pBankBufferEntry;
			bVar5 = false;
			if ((pBaseSectorBankFileAccess != (edCBankBufferEntry*)0x0) && (bVar6 = pBaseSectorBankFileAccess->is_loaded(), bVar6 != false)) {
				bVar5 = true;
			}

			bVar6 = true;

			if (bVar5) {
				curSectorIndex = 0;

				while ((curSectorIndex < this->nbSectors && (bVar6))) {
					pSectData = this->aSectors + curSectorIndex;
					iVar9 = pSectData->sectID;
					if (iVar9 != -1) {
						if (pSectData->loadStage_0x8 == 1) {
							edCBankBufferEntry* pCurSectorBankFileAccess = pSectData->pBankBufferEntry;
							bVar5 = false;

							if ((pCurSectorBankFileAccess != (edCBankBufferEntry*)0x0) && (bVar7 = pCurSectorBankFileAccess->is_loaded(), bVar7 != false))
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
							edCBankInstall sectorBankInstall;
							memset(&sectorBankInstall, 0, sizeof(edCBankInstall));
							pSectData->bankObject.initialize(iVar4 + 0x1000, 1, &sectorBankInstall);

							if (iVar9 == -1) {
								pSectData->loadStage_0x8 = 0;
							}
							else {
								pSectData->Load(iVar9, (long)iVar2, false);
							}

							bVar6 = false;
						}
					}

					curSectorIndex = curSectorIndex + 1;
				}

				if (bVar6) {
					bStillLoading = false;
				}
			}
		}
		else {
			// Load the base sector.
			int baseSectorIndex = CLevelScheduler::gThis->baseSectorIndex;

			const int baseBankSize = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->nextLevelID].bankSizeSect;

			edCBankInstall baseSectorBankInstall;
			memset(&baseSectorBankInstall, 0, sizeof(edCBankInstall));
			(this->baseSector).bankObject.initialize(baseBankSize + 0x1000, 1, &baseSectorBankInstall);

			if (baseSectorIndex == -1) {
				(this->baseSector).loadStage_0x8 = 0;
				bStillLoading = false;
			}
			else {
				this->baseSector.Load(baseSectorIndex, this->field_0x36c, false);
				const uint baseSectorFlags = this->subObjArray[baseSectorIndex].flags;

				if (this->field_0x37c != baseSectorFlags) {
					SetupCompanionSectors(baseSectorFlags);
				}

				curSectorIndex = 0;
				if (0 < this->nbSectors) {
					do {
						CSector* pSector = &this->aSectors[curSectorIndex];
						if (pSector->sectID == -1) {
							edCBankInstall sectorBankInstall;
							memset(&sectorBankInstall, 0, sizeof(edCBankInstall));
							pSector->bankObject.initialize(pSector->currentSectorID + 0x1000, 1, &sectorBankInstall);
							pSector->loadStage_0x8 = 0;
						}

						curSectorIndex = curSectorIndex + 1;
					} while (curSectorIndex < this->nbSectors);
				}
			}
		}
	}

	return bStillLoading;
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

	if (0 < this->nbSectors) {
		do {
			pSector = this->aSectors + iVar6;
			uVar1 = 1 << (pSector->sectID & 0x1fU);

			if ((pSector->sectID != 0xffffffff) && ((uVar1 & flags) == 0)) {
				this->field_0x37c = this->field_0x37c & ~uVar1;
				pSector->sectID = -1;
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < this->nbSectors);
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
					for (iVar6 = 0; (lVar7 < this->nbSectors && this->aSectors[lVar7].sectID != -1); iVar6 = iVar6 + 0x144) {
						lVar7 = (long)((int)lVar7 + 1);
					}
				}

				if (lVar7 < this->nbSectors) {
					this->aSectors[(int)lVar7].sectID = uVar1;
					this->field_0x37c = this->field_0x37c | uVar3;
				}
			}

			uVar1 = uVar1 + 1;
		} while ((int)uVar1 <= this->count_0x368);
	}

	return;
}

CSector::CSector()
{
	(this->pANHR).pThis = (MeshData_ANHR*)0x0;

	edF32Matrix4SetIdentityHard(&CHierarchyAnm::_gscale_mat);

	this->desiredSectorID = -1;
	this->sectorIndex = -1;

	this->pBankBufferEntry = (edCBankBufferEntry*)0x0;

	this->pBackgroundNode = (edNODE*)0x0;
	this->pObbTree = (edObbTREE_DYN*)0x0;

	this->loadStage_0x8 = 0;
	this->field_0x134 = 0.0f;

	this->pMeshTransformParent_0x130 = (edNODE*)0x0;
	memset(&this->sectorTexture, 0, sizeof(ed_g2d_manager));
	memset(&this->sectorMesh, 0, sizeof(ed_g3d_manager));
	memset(&this->backgroundTexture, 0, sizeof(ed_g2d_manager));
	memset(&this->backgroundMesh, 0, sizeof(ed_g3d_manager));
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
	char* pBackgroundFileBuffer;
	char* pAnimHierarchy;
	float fVar14;
	uint local_40;
	uint backgroundFileSize;
	edBANK_ENTRY_INFO bankEntry;
	int iStack8;
	short sStack2;
	S_HIERANM_HIER* pHier;

#ifdef PLATFORM_WIN
	std::string szBackgroundFileName;
#endif

	pAnimHierarchy = (char*)0x0;
	pBackgroundFileBuffer = (char*)0x0;
	this->pObbTree = (edObbTREE_DYN*)0x0;
	local_40 = 0;
	pcVar6 = (char*)0x0;
	uVar2 = this->pBankBufferEntry->get_element_count();
	inFileIndex = 0;
	pMeshData = pcVar6;
	meshSize = unaff_s5_lo;
	if (uVar2 != 0) {
		do {
			bVar1 = this->pBankBufferEntry->get_info(inFileIndex, &bankEntry, (char*)0x0);
			pMeshData = pcVar6;
			meshSize = unaff_s5_lo;
			if (bVar1 == false) break;
			uVar9 = bankEntry.type << 0x10 | bankEntry.stype;
			if (uVar9 == 0x170001) {
				SECTOR_LOG(LogLevel::Info, "Sector::Init Anim Heirarchy: {}\n", DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
				pAnimHierarchy = bankEntry.fileBufferStart;
				local_40 = bankEntry.size;
			}
			else {
				if (uVar9 == 0x50003) {
					SECTOR_LOG(LogLevel::Info, "Sector::Init Background Texture: {}\n", DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
					NAME_NEXT_OBJECT(DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
					ed3DInstallG2D(bankEntry.fileBufferStart, bankEntry.size, &iStack8, &this->backgroundTexture, 1);
				}
				else {
					if (uVar9 == 0x40002) {
#ifdef PLATFORM_WIN
						char* pBackgroundFileName = DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex);
						szBackgroundFileName = pBackgroundFileName;
#endif
						SECTOR_LOG(LogLevel::Info, "Sector::Init Background Mesh: {}\n", pBackgroundFileName);
						backgroundFileSize = bankEntry.size;
						pBackgroundFileBuffer = bankEntry.fileBufferStart;
					}
					else {
						if (uVar9 == 0x70001) {
							SECTOR_LOG(LogLevel::Info, "Sector::Init Init Collision: {}\n", DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
							this->pObbTree = CScene::ptable.g_CollisionManager_00451690->InstallColFile(bankEntry.fileBufferStart, bankEntry.size);
						}
						else {
							if (uVar9 == 0x50001) {
								SECTOR_LOG(LogLevel::Info, "Sector::Init Sector Texture: {}\n", DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
								NAME_NEXT_OBJECT(DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
								ed3DInstallG2D(bankEntry.fileBufferStart, bankEntry.size, &iStack8, &this->sectorTexture, 1);
							}
							else {
								SECTOR_LOG(LogLevel::Info, "Sector::Init Sector Mesh: {}\n", DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
								NAME_NEXT_OBJECT(DebugFindFilePath(this->pBankBufferEntry->pFileHeader, inFileIndex));
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

	if (pBackgroundFileBuffer == (char*)0x0) {
		this->pBackgroundNode = (edNODE*)0x0;
	}
	else {
		NAME_NEXT_OBJECT("%s", szBackgroundFileName.c_str());
		p3DFileManager->HideCommonBackground();

		ed3DInstallG3D(pBackgroundFileBuffer, backgroundFileSize, 0, &iStack8, &this->backgroundTexture, 0xc, &this->backgroundMesh);

		this->pBackgroundNode = ed3DHierarchyAddToScene(pStaticMeshMaster, &this->backgroundMesh, (char*)0x0);
		if (this->pBackgroundNode != (edNODE*)0x0) {
			p3DFileManager->SetupBackground(this->pBackgroundNode);
		}

		SECTOR_LOG(LogLevel::Info, "Sector::Init Background complete");
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
		pChunck = (ed_Chunck*)LOAD_POINTER(pHashCode->pData);
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

		edNODE* pNode;
		ulong hash = pSectorHierarchy->hash;
		if (hash != 0) {
			MeshData_ANHR* pMVar1 = (this->pANHR).pThis;
			if (pMVar1 != (MeshData_ANHR*)0x0) {
				int* pCurHier = reinterpret_cast<int*>(pMVar1 + 1);
				int curHierarchy = 0;
				if (pMVar1->nb3dHierarchies != 0) {
					do {
						pHier = LOAD_POINTER_CAST(S_HIERANM_HIER*, *pCurHier);
						ed_3d_hierarchy* pHierarchy3d = LOAD_POINTER_CAST(ed_3d_hierarchy*, pHier->p3dHierarchy);
						if ((pHierarchy3d != (ed_3d_hierarchy*)0x0) && (hash == pHierarchy3d->hash.number)) goto LAB_001fe180;
						curHierarchy = curHierarchy + 1;
						pCurHier = pCurHier + 1;
					} while (curHierarchy < pMVar1->nb3dHierarchies);
				}
			}

			pHier = (S_HIERANM_HIER*)0x0;
		LAB_001fe180:
			if (pHier == (S_HIERANM_HIER*)0x0) {
				pNode = ed3DHierarchyNodeGetByHashcodeFromList(this->pMeshTransformParent_0x130, hash);
			}
			else {
				pNode = LOAD_POINTER_CAST(edNODE*, pHier->pNode);
			}

			if (pNode != 0) {
				pSectorHierarchy->Sector_Enter(pNode, pHier);
			}
		}
		pSectorHierarchy = pSectorHierarchy->pNext;

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
	edCBankBufferEntry* pBankBufferEntry;
	int sectStringLength;
	float fVar5;
	edCBankInstall bankInstall;
	char bankFilePath[64];
	char* sectString;

	/* Loads a sectx.bnk file for a level
	   Example: CDEURO/LEVEL/PREINTRO/SECT1.bnk */
	bVar3 = StaticEdFileBase_004497f0.Check();
	if (bVar3 != false) {
		fVar5 = edTimerTimeGet();
		this->field_0x134 = fVar5;
		this->desiredSectorID = -1;
		this->sectorIndex = sectorIndex;
		pBankBufferEntry = this->pBankBufferEntry;
		if (pBankBufferEntry != (edCBankBufferEntry*)0x0) {
			pBankBufferEntry->close();
			this->pBankBufferEntry = (edCBankBufferEntry*)0x0;
		}
		pBankBufferEntry = this->bankObject.get_free_entry();
		this->pBankBufferEntry = pBankBufferEntry;
		iVar1 = this->sectorIndex;
		sectStringLength = edStrCopy(bankFilePath, (CScene::ptable.g_SectorManager_00451670)->szSectorFileRoot);
		sectString = bankFilePath + sectStringLength;
		if (iVar1 < 10) {
			*sectString = (char)iVar1 + '0';
			sectString = bankFilePath + 1;
		}
		else {
			cVar2 = (char)(iVar1 / 10);
			*sectString = cVar2 + '0';
			bankFilePath[sectStringLength + 1] = (char)iVar1 + cVar2 * -10 + '0';
			sectString = bankFilePath + 2;
		}
		/* Add the '.bnk' suffix to the file name */
		edStrCopy(sectString + sectStringLength, ".bnk");
		memset(&bankInstall, 0, sizeof(edCBankInstall));
		if (bFileFlag == false) {
			bankInstall.fileFlagA = 0xc;
		}
		else {
			bankInstall.fileFlagA = 4;
		}

		bankInstall.filePath = bankFilePath;
		bankInstall.fileFunc = _gSectorInstallCallback;
		bankInstall.pObjectReference = this;

		const bool bSuccess = this->pBankBufferEntry->load(&bankInstall);
		if ((bSuccess != false) && (this->loadStage_0x8 = 1, param_3 == 0)) {
			this->pBankBufferEntry->wait();
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
		peVar1 = (this->baseSector).pBankBufferEntry;
		bVar2 = false;

		if ((peVar1 != (edCBankBufferEntry*)0x0) && (bVar3 = peVar1->is_loaded(), bVar3 != false)) {
			bVar2 = true;
		}

		if (bVar2) {
			memset(&BStack32, 0, sizeof(edCBankInstall));
			BStack32.fileFlagA = 0;
			BStack32.fileFunc = _gSectorInstallCallback;
			BStack32.pObjectReference = &this->baseSector;
			(this->baseSector).pBankBufferEntry->install(&BStack32);
			(this->baseSector).loadStage_0x8 = 2;
		}
	}

	iVar5 = 0;
	if (0 < this->nbSectors) {
		iVar4 = 0;
		do {
			pvVar4 = &this->aSectors[iVar5];
			if ((pvVar4->sectID != -1) && (pvVar4->loadStage_0x8 == 1)) {
				peVar1 = pvVar4->pBankBufferEntry;
				bVar2 = false;
				if ((peVar1 != (edCBankBufferEntry*)0x0) && (bVar3 = peVar1->is_loaded(), bVar3 != false)) {
					bVar2 = true;
				}

				if (bVar2) {
					memset(&BStack64, 0, sizeof(edCBankInstall));
					BStack64.fileFlagA = 0;
					BStack64.fileFunc = _gSectorInstallCallback;
					BStack64.pObjectReference = pvVar4;
					pvVar4->pBankBufferEntry->install(&BStack64);
					pvVar4->loadStage_0x8 = 2;
				}
			}

			iVar5 = iVar5 + 1;
			iVar4 = iVar4 + sizeof(CSector);
		} while (iVar5 < this->nbSectors);
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

void CSectorManager::Level_Term()
{
	undefined* puVar1;
	edCBankBufferEntry* peVar2;
	bool bBankIsLoaded;
	bool bVar4;
	int iVar5;
	int iVar6;
	CSector* pSector;

	iVar6 = this->field_0x36c;
	iVar5 = (this->baseSector).sectID;
	if ((iVar5 != -1) || (iVar6 == 0)) {
		if ((iVar5 != -1) && ((iVar5 != -1 && (puVar1 = (CScene::ptable.g_SectorManager_00451670)->subObjArray[iVar5].pFileData, puVar1 != (undefined*)0x0)))) {
			CScene::_pinstance->PopFogAndClippingSettings((S_STREAM_FOG_DEF*)(puVar1 + 0xc));
		}

		(this->baseSector).sectID = -1;
		this->field_0x36c = iVar6;
	}

	if ((this->baseSector).loadStage_0x8 != 0) {
		peVar2 = (this->baseSector).pBankBufferEntry;
		bBankIsLoaded = false;
		if ((peVar2 != (edCBankBufferEntry*)0x0) && (bVar4 = peVar2->is_loaded(), bVar4 != false)) {
			bBankIsLoaded = true;
		}

		if (!bBankIsLoaded) {
			(this->baseSector).pBankBufferEntry->wait();
		}

		this->baseSector.Flush();
		(this->baseSector).loadStage_0x8 = 0;
	}

	iVar6 = 0;
	if (0 < this->nbSectors) {
		do {
			pSector = this->aSectors + iVar6;
			if (pSector->loadStage_0x8 != 0) {
				peVar2 = pSector->pBankBufferEntry;
				bBankIsLoaded = false;
				if ((peVar2 != (edCBankBufferEntry*)0x0) && (bVar4 = peVar2->is_loaded(), bVar4 != false)) {
					bBankIsLoaded = true;
				}

				if (!bBankIsLoaded) {
					pSector->pBankBufferEntry->wait();
				}

				pSector->Flush();
				pSector->loadStage_0x8 = 0;
			}

			iVar6 = iVar6 + 1;
		} while (iVar6 < this->nbSectors);
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

void CSectorManager::Level_ClearAll()
{
	int curSectorIndex;
	edCBankBufferEntry* pBank;

	pBank = (this->baseSector).pBankBufferEntry;
	if (pBank != (edCBankBufferEntry*)0x0) {
		pBank->close();
		(this->baseSector).pBankBufferEntry = (edCBankBufferEntry*)0x0;
	}

	(this->baseSector).bankObject.terminate();

	if (this->aSectors != (CSector*)0x0) {
		curSectorIndex = 0;
		if (0 < this->nbSectors) {
			do {
				CSector* pSector = this->aSectors + curSectorIndex;
				if (pSector->pBankBufferEntry != (edCBankBufferEntry*)0x0) {
					pSector->pBankBufferEntry->close();
					pSector->pBankBufferEntry = (edCBankBufferEntry*)0x0;
				}

				pSector->bankObject.terminate();
				curSectorIndex = curSectorIndex + 1;
			} while (curSectorIndex < this->nbSectors);
		}
		
		delete[] this->aSectors;
		this->aSectors = (CSector*)0x0;
	}

	Level_ClearInternalData();

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
	if (0 < this->nbSectors) {
		do {
			pSector = this->aSectors + iVar2;
			pSector->Level_Manage(pSector->sectID, (long)this->field_0x36c);
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbSectors);
	}
	return;
}

void CSectorManager::Level_ManagePaused()
{
	if ((GameFlags & 0x200) == 0) {
		Level_Manage();
	}

	return;
}

void CSectorManager::Level_Draw()
{
	return;
}

void CSectorManager::Level_Reset()
{
	int iVar1;
	int iVar2;
	undefined* puVar3;
	CSectorManager* pCVar4;

	pCVar4 = CScene::ptable.g_SectorManager_00451670;

	iVar1 = (this->baseSector).sectID;
	iVar2 = CLevelScheduler::gThis->aLevelInfo[CLevelScheduler::gThis->currentLevelID].sectorStartIndex;
	if (iVar1 != iVar2) {
		if ((iVar1 != -1) &&
			(puVar3 = (CScene::ptable.g_SectorManager_00451670)->subObjArray[iVar1].pFileData, puVar3 != (undefined*)0x0)) {
			CScene::_pinstance->PopFogAndClippingSettings((S_STREAM_FOG_DEF*)(puVar3 + 0xc));
		}

		if ((iVar2 != -1) && (puVar3 = pCVar4->subObjArray[iVar2].pFileData, puVar3 != (undefined*)0x0)) {
			CScene::_pinstance->PushFogAndClippingSettings(*(float*)(puVar3 + 0x1c), (S_STREAM_FOG_DEF*)(puVar3 + 0xc));
		}
	}

	(this->baseSector).sectID = iVar2;
	this->field_0x36c = 0;

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

char* CSectorManager::ProfileGetName()
{
	return "Sector";
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
	S_HIERANM_HIER* piStack4;

	local_110.nbEntries = 0;
	CScene::ptable.g_ActorManager_004516a4->GetActorsByClassID(CLUSTERISER, &local_110);

	iVar3 = 0;
	if (0 < local_110.nbEntries) {
		do {
			//::EmptyFunction();
			iVar3 = iVar3 + 1;
		} while (iVar3 < local_110.nbEntries);
	}

	for (pCVar1 = this->pSectorHierarchy; pCVar1 != (CSectorHierarchy*)0x0; pCVar1 = pCVar1->pNext) {
		if ((pCVar1->field_0x19 != 0) && ((peVar2 = RegisterDynamicHierarchy(pCVar1->hash, &piStack4), peVar2 == (edNODE*)0x0 ||
				(peVar2 != pCVar1->pNode)))) {
			pCVar1->field_0x19 = 0;
			pCVar1->pNode = (edNODE*)0x0;
			pCVar1->pHier = (S_HIERANM_HIER*)0x0;
		}
	}
	return;
}

edNODE* CSectorManager::RegisterDynamicHierarchy(ulong hash, S_HIERANM_HIER** pOutHier)
{
	MeshData_ANHR* pMVar1;
	int iVar2;
	edNODE* peVar3;
	int* pMVar4;
	int* piVar4;
	uint uVar5;
	S_HIERANM_HIER** ppSVar6;
	int iVar7;
	int iVar8;
	S_HIERANM_HIER* pSVar9;

	if (hash == 0) {
		*pOutHier = (S_HIERANM_HIER*)0x0;
		peVar3 = (edNODE*)0x0;
	}
	else {
		pMVar1 = (this->baseSector).pANHR.pThis;

		if (pMVar1 != (MeshData_ANHR*)0x0) {
			pMVar4 = (int*)(pMVar1 + 1);
			uVar5 = 0;
			if (pMVar1->nb3dHierarchies != 0) {
				do {
					pSVar9 = LOAD_POINTER_CAST(S_HIERANM_HIER*, *pMVar4);

					if ((pSVar9->p3dHierarchy != 0x0) && (hash == LOAD_POINTER_CAST(ed_3d_hierarchy*, pSVar9->p3dHierarchy)->hash.number))
						goto LAB_001fe910;

					uVar5 = uVar5 + 1;
					pMVar4 = pMVar4 + 1;
				} while (uVar5 < pMVar1->nb3dHierarchies);
			}
		}
		pSVar9 = (S_HIERANM_HIER*)0x0;

	LAB_001fe910:
		iVar8 = 0;
		iVar7 = 0;
		while ((iVar8 < this->nbSectors && (pSVar9 == (S_HIERANM_HIER*)0x0))) {
			CSector* pSector = this->aSectors + iVar8;

			if (pSector->desiredSectorID != -1) {
				pMVar1 = (this->baseSector).pANHR.pThis;

				if (pMVar1 != (MeshData_ANHR*)0x0) {
					pMVar4 = (int*)(pMVar1 + 1);
					uVar5 = 0;
					if (pMVar1->nb3dHierarchies != 0) {
						do {
							pSVar9 = LOAD_POINTER_CAST(S_HIERANM_HIER*, *pMVar4);

							if ((pSVar9->p3dHierarchy != 0x0) && (hash == LOAD_POINTER_CAST(ed_3d_hierarchy*, pSVar9->p3dHierarchy)->hash.number))
								goto LAB_001fe9a0;

							uVar5 = uVar5 + 1;
							ppSVar6 = ppSVar6 + 1;
						} while (uVar5 < pMVar1->nb3dHierarchies);
					}
				}
				pSVar9 = (S_HIERANM_HIER*)0x0;
			}
		LAB_001fe9a0:
			iVar8 = iVar8 + 1;
		}

		if (pSVar9 == (S_HIERANM_HIER*)0x0) {
			peVar3 = ed3DHierarchyNodeGetByHashcodeFromList(CScene::ptable.g_C3DFileManager_00451664->pLastMeshTransformParent, hash);
			if (peVar3 == (edNODE*)0x0) {
				peVar3 = ed3DHierarchyNodeGetByHashcodeFromList(this->baseSector.pMeshTransformParent_0x130, hash);
			}

			iVar8 = 0;
			while ((iVar8 < this->nbSectors && (peVar3 == (edNODE*)0x0))) {
				CSector* pSector = this->aSectors + iVar8;
				if (pSector->desiredSectorID != -1) {
					peVar3 = ed3DHierarchyNodeGetByHashcodeFromList(pSector->pMeshTransformParent_0x130, hash);
				}
				iVar8 = iVar8 + 1;
			}
		}
		else {
			peVar3 = LOAD_POINTER_CAST(edNODE*, pSVar9->pNode);
		}

		*pOutHier = pSVar9;
	}

	return peVar3;
}

void CSectorManager::UnregisterDynamicHierarchy(CSectorHierarchy* pSector, int index)
{
	CSectorHierarchy* pCVar1;
	CSectorHierarchy* pCVar2;
	CSectorHierarchy* pCVar3;

	if (index == -1) {
		pCVar1 = this->pSectorHierarchy;
	}
	else {
		pCVar1 = this->subObjArray[index].aSectorHierarchies;
	}

	pCVar2 = (CSectorHierarchy*)0x0;
	while ((pCVar3 = pCVar1, pCVar3 != (CSectorHierarchy*)0x0 && (pCVar3 != pSector))) {
		pCVar2 = pCVar3;
		pCVar1 = pCVar3->pNext;
	}

	if (pCVar3 != (CSectorHierarchy*)0x0) {
		if (pCVar2 == (CSectorHierarchy*)0x0) {
			if (index == -1) {
				this->pSectorHierarchy = pCVar3->pNext;
			}
			else {
				this->subObjArray[index].aSectorHierarchies = pCVar3->pNext;
			}
		}
		else {
			pCVar2->pNext = pCVar3->pNext;
		}
		pCVar3->pNext = (CSectorHierarchy*)0x0;
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
				this->pBankBufferEntry->wait();
			}
			pBank = this->pBankBufferEntry;
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
		uVar2 = pMVar4->nb3dHierarchies;

		int* pNext = (int*)(pMVar4 + 1);

		uVar3 = 0;
		if (uVar2 != 0) {
			do {
				ANHR_Internal* peVar3 = LOAD_POINTER_CAST(ANHR_Internal*, pNext[uVar3]);

				if (peVar3->pHierNode != 0x0) {
					ed3DHierarchyRemoveFromScene(pScene, LOAD_POINTER_CAST(edNODE*, peVar3->pHierNode));
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
	this->hash = pByteCode->GetU64();
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
	ulong lVar5;
	uint uVar6;
	int iVar7;
	CSector* pCVar8;
	int* pMVar9;
	S_HIERANM_HIER* pHier;
	S_HIERANM_HIER* local_4;
	CSectorManager* pSectorManager;

	pSectorManager = CScene::ptable.g_SectorManager_00451670;
	if (this->hash != 0) {
		if (param_2 == -1) {
			this->pNext = (CScene::ptable.g_SectorManager_00451670)->pSectorHierarchy;
			pSectorManager->pSectorHierarchy = this;
			peVar3 = pSectorManager->RegisterDynamicHierarchy(this->hash, &local_4);
			if (peVar3 != (edNODE*)0x0) {
				Sector_Enter(peVar3, local_4);
			}
		}
		else {
			pcVar2 = (CScene::ptable.g_SectorManager_00451670)->szSectorFileRoot;
			this->pNext = (CScene::ptable.g_SectorManager_00451670)->subObjArray[param_2].aSectorHierarchies;
			(CScene::ptable.g_SectorManager_00451670)->subObjArray[param_2].aSectorHierarchies = this;

			pCVar8 = &pSectorManager->baseSector;
			if (param_2 != (pSectorManager->baseSector).desiredSectorID) {
				iVar7 = 0;
				if (0 < pSectorManager->nbSectors) {
					pCVar8 = pSectorManager->aSectors;
					do {
						if (param_2 == pCVar8->desiredSectorID) goto LAB_001fdad8;
						iVar7 = iVar7 + 1;
						pCVar8 = pCVar8 + 1;
					} while (iVar7 < pSectorManager->nbSectors);
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
					lVar5 = this->hash;
					if (lVar5 != 0) {
						pMVar1 = (pCVar8->pANHR).pThis;
						if (pMVar1 != (MeshData_ANHR*)0x0) {
							pMVar9 = reinterpret_cast<int*>(pMVar1 + 1);
							uVar6 = 0;
							if (pMVar1->nb3dHierarchies != 0) {
								do {
									pHier = LOAD_POINTER_CAST(S_HIERANM_HIER*, *pMVar9);
									if ((pHier->p3dHierarchy != 0x0) && (lVar5 == LOAD_POINTER_CAST(ed_3d_hierarchy*, pHier->p3dHierarchy)->hash.number))
										goto LAB_001fdb90;
									
									uVar6 = uVar6 + 1;
									pMVar9 = pMVar9 + 1;
								} while (uVar6 < pMVar1->nb3dHierarchies);
							}
						}
						pHier = (S_HIERANM_HIER*)0x0;
					LAB_001fdb90:
						if (pHier == (S_HIERANM_HIER*)0x0) {
							peVar3 = ed3DHierarchyNodeGetByHashcodeFromList(pCVar8->pMeshTransformParent_0x130, lVar5);
						}
						else {
							peVar3 = LOAD_POINTER_CAST(edNODE*, pHier->pNode);
						}

						if (peVar3 != (edNODE*)0x0) {
							Sector_Enter(peVar3, pHier);
						}
					}
				}
			}
		}
	}
	return;
}

void CSectorHierarchy::Term(int index)
{
	if (this->hash != 0) {
		this->pNode = (edNODE*)0x0;
		this->pHier = (S_HIERANM_HIER*)0x0;
		CScene::ptable.g_SectorManager_00451670->UnregisterDynamicHierarchy(this, index);
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

void CSectorHierarchy::SetAlpha(byte alpha)
{
	if (alpha != this->alpha) {
		this->alpha = alpha;

		if (this->pNode != (edNODE*)0x0) {
			ed3DHierarchyNodeSetAlpha(this->pNode, this->alpha);
		}
	}

	return;
}

void CSectorHierarchy::Sector_Enter(edNODE* pNode, S_HIERANM_HIER* pHier)
{
	S_HIERANM_HIER* pSVar1;

	this->pNode = pNode;
	this->pHier = pHier;
	pSVar1 = this->pHier;
	if (pSVar1 == (S_HIERANM_HIER*)0x0) {
		if ((this->pNode != (edNODE*)0x0) && ((this->flags & 0x40000000) != 0)) {
			ed3DHierarchyNodeSetFlag(this->pNode, 0x40);
		}
	}
	else {
		if ((this->flags & 0x40000000) == 0) {
			pSVar1->flags_0x20 = pSVar1->flags_0x20 & 0xbfffffff;
		}
		else {
			pSVar1->flags_0x20 = pSVar1->flags_0x20 | 0x40000000;
		}
		if ((this->flags & 0x20000000) == 0) {
			this->pHier->flags_0x20 = this->pHier->flags_0x20 & 0xdfffffff;
		}
		else {
			this->pHier->flags_0x20 = this->pHier->flags_0x20 | 0x20000000;
		}
	}

	if (this->pNode != (edNODE*)0x0) {
		ed3DHierarchyNodeSetAlpha(this->pNode, this->alpha);
	}

	this->field_0x19 = 1;

	return;
}
