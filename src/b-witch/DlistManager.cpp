#include "DlistManager.h"
#include "SectorManager.h"
#include "DlistManager.h"
#include "ActorManager.h"
#include "edDlist.h"
#include "Actor.h"
#include "Frontend.h"
#include "PoolAllocators.h"
#include "edDList/edDList.inl"

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

CGlobalDList* pGameDList = NULL;
CGlobalDList* pFrontend2DDList = NULL;
CGlobalDList* pGuiDList = NULL;

char* g_NewLine = "\n";
char* sz_DisplayListSpacer_00433970 = "---------------------------------------- - \n";

float FLOAT_00448510 = 0.1f;

void FUN_00290620(float param_1, int param_2, char* param_3, long param_4, char* param_5)
{
	int iVar1;
	char* pcVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	char acStack32[32];

	iVar1 = 0;
	if (param_4 != 0) {
		if (param_1 == 0.0) {
			param_4 = 0;
		}
		else {
			if (param_1 < FLOAT_00448510) {
				for (; param_1 < 1.0; param_1 = param_1 * 10.0) {
					iVar1 = iVar1 + -1;
				}
			}
			else {
				if (10.0 <= param_1) {
					for (; 10.0 <= param_1; param_1 = param_1 * FLOAT_00448510) {
						iVar1 = iVar1 + 1;
					}
				}
				else {
					param_4 = 0;
				}
			}
		}
	}
	fVar7 = param_1;
	if (param_1 < 0.0) {
		fVar7 = -param_1;
	}
	fVar6 = 1.0;
	for (iVar3 = 0; iVar3 < param_2; iVar3 = iVar3 + 1) {
		fVar6 = fVar6 * 10.0;
	}
	if (2.147484e+09f < fVar7 * fVar6) {
		sprintf(param_3, "%s", "ERROR");
	}
	else {
		iVar4 = 0x13;
		iVar3 = (int)(fVar7 * fVar6 + 0.5);
		for (iVar5 = 0; (0 < iVar4 && ((iVar3 != 0 || (iVar5 <= param_2)))); iVar5 = iVar5 + 1) {
			param_5 = acStack32 + iVar4;
			iVar4 = iVar4 + -1;
			*param_5 = (char)iVar3 + (char)(iVar3 / 10) * -10 + '0';
			iVar3 = iVar3 / 10;
		}
		if (param_1 < 0.0) {
			*param_3 = '-';
			param_3 = param_3 + 1;
		}
		for (iVar3 = 0; iVar3 < iVar5 - param_2; iVar3 = iVar3 + 1) {
			*param_3 = *param_5;
			param_5 = param_5 + 1;
			param_3 = param_3 + 1;
		}
		*param_3 = '.';
		for (iVar3 = 0; pcVar2 = param_3 + 1, iVar3 < param_2; iVar3 = iVar3 + 1) {
			*pcVar2 = *param_5;
			param_5 = param_5 + 1;
			param_3 = pcVar2;
		}
		if (param_4 == 0) {
			*pcVar2 = '\0';
		}
		else {
			*pcVar2 = 'e';
			sprintf(param_3 + 2, "%d", iVar1);
		}
	}
	return;
}

char* WorkOutMo(float param_1, int param_2, char* param_3, long param_4, char* param_5)
{
	FUN_00290620(param_1, param_2, param_3, param_4, param_5);
	return param_3;
}

char g_DebugTextBuffer_00468ef0[16] = { 0 };

void CGlobalDListManager::Level_Init()
{
	GlobalDlistEntry* pCurEntry;
	CGlobalDListPatch* pCurPatchA;
	CGlobalDListPatch* pCurPatchB;
	ed_3D_Scene* pScene;
	int iVar6;
	int iVar7;
	uint uVar8;
	int iVar9;
	int curIndex;
	uint size;
	int unaff_s6_lo;
	int unaff_s7_lo;
	int unaff_s8_lo;

	this->bBeganLevelInit = 1;

	if (this->dlistCount != 0) {
		pCurEntry = this->ppGlobalDlist;

		if ((pCurEntry->nbMatrices != 0) || (pCurEntry->nbInstances != 0)) {
			if (pCurEntry->nbRegisteredPatches < 2) {
				pCurEntry->nbRegisteredPatches = 2;
			}

			pScene = CScene::_scene_handleA;
			pCurEntry = this->ppGlobalDlist;
			int nbRegisteredPatches = pCurEntry->nbRegisteredPatches;
			int nbMatrices = pCurEntry->nbMatrices;
			iVar6 = pCurEntry->nbInstances;
			pCurPatchA = pCurEntry->pDlistPatch;
			pCurPatchA->nbCommands = nbRegisteredPatches;
			pCurPatchA->nbMatrices = nbMatrices;
			pCurPatchA->field_0x10 = iVar6;
			pCurPatchA->flags = 0x101;
			pCurPatchA->pDisplayListInternal = edDListNew(TO_HEAP(H_MAIN), 0x101, nbRegisteredPatches, iVar6, nbMatrices, 0, (DisplayList*)0x0);
			edDListSetSceneUsed(pCurPatchA->pDisplayListInternal, pScene);
			this->ppGlobalDlist->pDlistPatch->Init();
			this->field_0x1c = 1;
		}

		if (1 < this->dlistCount) {
			size = 0;
			curIndex = 1;
			if (1 < this->dlistCount) {
				do {
					iVar9 = curIndex;
					if (curIndex == -1) {
						iVar9 = 0;
					}

					pCurEntry = this->ppGlobalDlist;
					if (pCurEntry[iVar9].nbMatrices == 0) {
						iVar9 = curIndex;
						if (curIndex == -1) {
							iVar9 = 0;
						}

						if (pCurEntry[iVar9].nbInstances != 0) goto LAB_002d82e8;
					}
					else {
					LAB_002d82e8:
						iVar9 = curIndex;
						if (curIndex == -1) {
							iVar9 = 0;
						}

						if ((int)pCurEntry[iVar9].nbRegisteredPatches < 2) {
							iVar9 = curIndex;
							if (curIndex == -1) {
								iVar9 = 0;
							}

							pCurEntry[iVar9].nbRegisteredPatches = 2;
						}

						iVar9 = curIndex;
						if (curIndex == -1) {
							iVar9 = 0;
						}

						pCurEntry = this->ppGlobalDlist;
						iVar9 = pCurEntry[iVar9].nbMatrices;
						iVar6 = curIndex;
						if (curIndex == -1) {
							iVar6 = 0;
						}

						iVar6 = pCurEntry[iVar6].nbInstances;
						iVar7 = curIndex;
						if (curIndex == -1) {
							iVar7 = 0;
						}

						iVar7 = pCurEntry[iVar7].nbRegisteredPatches;
						uVar8 = edDListGetBufSizeNeeded(0x101, iVar7, iVar6, iVar9, (uint*)0x0, (uint*)0x0);
						if ((int)size < (int)uVar8) {
							size = uVar8;
							unaff_s6_lo = iVar9;
							unaff_s8_lo = iVar6;
							unaff_s7_lo = iVar7;
						}
					}

					curIndex = curIndex + 1;
				} while (curIndex < this->dlistCount);
			}

			if (size != 0) {
				curIndex = edMemGetAvailable(TO_HEAP(H_MAIN));
				this->pDisplayList = (DisplayList*)edMemAlloc(TO_HEAP(H_MAIN), size);
				pScene = CScene::_scene_handleA;
				pCurPatchA = this->ppGlobalDlist[1].pDlistPatch;
				pCurPatchA->nbCommands = unaff_s7_lo;
				pCurPatchA->nbMatrices = unaff_s6_lo;
				pCurPatchA->field_0x10 = unaff_s8_lo;
				pCurPatchA->flags = 0x101;
				pCurPatchA->pDisplayListInternal = edDListNew(TO_HEAP(H_MAIN), 0x101, unaff_s7_lo, unaff_s8_lo, unaff_s6_lo, 0, this->pDisplayList);
				edDListSetSceneUsed(pCurPatchA->pDisplayListInternal, pScene);
				this->ppGlobalDlist[1].pDlistPatch->Init();
				iVar9 = edMemGetAvailable(TO_HEAP(H_MAIN));
				/* - Sector patchable \t\t: %d\n\n */
				edDebugPrintf("- Sector patchable \t\t: %d\n\n", curIndex - iVar9);
				curIndex = 2;
				if (2 < this->dlistCount) {
					do {
						iVar9 = curIndex;
						if (curIndex == -1) {
							iVar9 = 0;
						}
						pCurEntry = this->ppGlobalDlist;
						if (pCurEntry[iVar9].pDlistPatch != (CGlobalDListPatch*)0x0) {
							pCurPatchA = pCurEntry[1].pDlistPatch;
							iVar9 = curIndex;
							if (curIndex == -1) {
								iVar9 = 0;
							}

							pCurPatchB = pCurEntry[iVar9].pDlistPatch;
							pCurPatchB->field_0x4d9 = 1;
							pCurPatchB->pDisplayListInternal = pCurPatchA->pDisplayListInternal;
							pCurPatchB->nbCommands = pCurPatchA->nbCommands;
							pCurPatchB->nbMatrices = pCurPatchA->nbMatrices;
							pCurPatchB->field_0x10 = pCurPatchA->field_0x10;
							pCurPatchB->flags = pCurPatchA->flags;
							iVar9 = curIndex;
							if (curIndex == -1) {
								iVar9 = 0;
							}
							this->ppGlobalDlist[iVar9].pDlistPatch->Init();
						}
						curIndex = curIndex + 1;
					} while (curIndex < this->dlistCount);
				}
			}
		}

		edDebugPrintf("-----------------------------");
	}

	this->bCompletedLevelInit = 1;

	return;
}

void CGlobalDListManager::Level_Term()
{
	return;
}

void CGlobalDListManager::Level_ClearAll()
{
	int iVar2;
	CGlobalDListPatch* piVar1;

	if (this->ppGlobalDlist != (GlobalDlistEntry*)0x0) {
		iVar2 = 0;
		if (0 < this->dlistCount) {
			do {
				piVar1 = this->ppGlobalDlist[iVar2].pDlistPatch;
				if (piVar1 != (CGlobalDListPatch*)0x0) {
					piVar1->Term();
					delete piVar1;
				}
				iVar2 = iVar2 + 1;
			} while (iVar2 < this->dlistCount);
		}

		delete[] this->ppGlobalDlist;

		// HACK
		this->ppGlobalDlist = 0x0;
		this->dlistCount = 0;
	}

	return;
}

void CGlobalDListManager::Level_Manage()
{
	CGlobalDListPatch* pCurList;
	uint uVar4;
	uint uVar5;
	int sectorPatchIndex;
	int nbTotalPatches;
	S_GLOBAL_DLIST_PATCH** pCurEntry;
	int curPatchIndex;
	//ParticleData_b8* piVar2;

	if (this->dlistCount != 0) {
		if (this->field_0x1c == 1) {
			pCurList = this->ppGlobalDlist->pDlistPatch;

			if ((pCurList != (CGlobalDListPatch*)0x0) && (nbTotalPatches = pCurList->nbTotalPatches, nbTotalPatches != 0)) {
				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 1;
					edDListSetCurrent(pCurList->pDisplayListInternal);
				}

				curPatchIndex = 0;
				pCurEntry = pCurList->aPatches;
				if (0 < nbTotalPatches) {
					do {
						if (*pCurEntry != (S_GLOBAL_DLIST_PATCH*)0x0) {
							(*pCurEntry)->pOwner->InitDlistPatchable(curPatchIndex);
						}

						pCurEntry = pCurEntry + 1;
						curPatchIndex = curPatchIndex + 1;
					} while (curPatchIndex < nbTotalPatches);
				}

				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 0;
				}
			}

			this->field_0x1c = 0;
		}

		if (((1 < this->dlistCount) && (this->field_0x18 == 1)) && (sectorPatchIndex = this->activeSectorPatchId, sectorPatchIndex != -1)) {
			pCurList = this->ppGlobalDlist[sectorPatchIndex].pDlistPatch;

			if ((pCurList != (CGlobalDListPatch*)0x0) && (nbTotalPatches = pCurList->nbTotalPatches, nbTotalPatches != 0)) {
				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 1;
					edDListSetCurrent(pCurList->pDisplayListInternal);
				}

				curPatchIndex = 0;
				if (0 < nbTotalPatches) {
					sectorPatchIndex = sectorPatchIndex << 0x10;
					pCurEntry = pCurList->aPatches;
					do {
						if (*pCurEntry != (S_GLOBAL_DLIST_PATCH*)0x0) {
							(*pCurEntry)->pOwner->InitDlistPatchable(sectorPatchIndex);
						}

						curPatchIndex = curPatchIndex + 1;
						pCurEntry = pCurEntry + 1;
						sectorPatchIndex = sectorPatchIndex + 1;
					} while (curPatchIndex < nbTotalPatches);
				}

				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 0;
				}
			}

			this->field_0x18 = 0;
		}

		_ExecuteCallFunc();
	}

	uVar5 = this->field_0x24 + 1;
	uVar4 = uVar5 & 1;
	if (((int)uVar5 < 0) && (uVar4 != 0)) {
		uVar4 = uVar4 - 2;
	}
	this->field_0x24 = uVar4;

	return;
}

void CGlobalDListManager::Level_ManagePaused()
{
	this->Level_Manage();
}

void CGlobalDListManager::Level_Draw()
{
	int bEnabled;
	CGlobalDListPatch* pDlistPatch;

	if (this->dlistCount != 0) {
		pDlistPatch = this->ppGlobalDlist->pDlistPatch;
		bEnabled = pDlistPatch->bEnabled;
		if ((bEnabled != 0) && (bEnabled != 0)) {
			edDlistAddtoView(pDlistPatch->pDisplayListInternal);
		}

		if (1 < this->dlistCount) {
			pDlistPatch = (CGlobalDListPatch*)0x0;
			if (this->activeSectorPatchId != -1) {
				pDlistPatch = this->ppGlobalDlist[this->activeSectorPatchId].pDlistPatch;
			}

			if ((((pDlistPatch != (CGlobalDListPatch*)0x0) && (bEnabled = pDlistPatch->bEnabled, bEnabled != 0)) &&
				(this->field_0x18 != 2)) && (bEnabled != 0)) {
				edDlistAddtoView(pDlistPatch->pDisplayListInternal);
			}
		}
	}
	return;
}

void CGlobalDListManager::Level_SectorChange(int oldSectorId, int newSectorId)
{
	if (1 < this->dlistCount) {
		if (newSectorId == -1) {
			this->field_0x18 = 2;
		}
		else {
			this->field_0x18 = 1;
		}

		SectorChange(newSectorId);
	}

	return;
}

void CGlobalDListManager::Level_Create()
{
	GlobalDlistEntry* pGVar1;
	CGlobalDListPatch* pvVar2;
	CGlobalDListPatch* iVar8;
	int iVar2;
	char** ppcVar3;
	undefined4* puVar4;
	CGlobalDListManager* pCVar5;
	CGlobalDListPatch* pCVar6;
	int iVar7;
	int iVar9;
	int iVar10;
	int* pSectorActors;
	uint sectorIndex;
	int aNbSectorActors[32];

	iVar7 = 0;
	this->dlistCount = -1;
	this->bBeganLevelInit = 0;
	this->pDisplayList = (DisplayList*)0x0;
	this->ppGlobalDlist = (GlobalDlistEntry*)0x0;
	this->activeSectorPatchId = -1;
	this->field_0x18 = 0;
	this->field_0x1c = 0;
	//this->field_0x20 = -1;
	this->bCompletedLevelInit = 0;
	this->nbActiveCallFuncElements = 0;
	//this->count_0x25ac = 0;
	this->field_0x24 = 0;
	this->nbRegData = 0;
	pCVar5 = this;
	//do {
	//	*(undefined4*)&pCVar5->field_0x25b4 = 0;
	//	*(undefined4*)&pCVar5->field_0x25bc = 0;
	//	iVar7 = iVar7 + 8;
	//	*(undefined4*)&pCVar5->field_0x25c4 = 0;
	//	*(undefined4*)&pCVar5->field_0x25cc = 0;
	//	*(undefined4*)&pCVar5->field_0x25d4 = 0;
	//	*(undefined4*)&pCVar5->field_0x25dc = 0;
	//	*(undefined4*)&pCVar5->field_0x25e4 = 0;
	//	*(undefined4*)&pCVar5->field_0x25ec = 0;
	//	pCVar5 = (CGlobalDListManager*)&pCVar5->field_0x40;
	//} while (iVar7 < 0x80);

	iVar7 = 0x1d;
	pSectorActors = aNbSectorActors + iVar7;
	this->dlistCount = (CScene::ptable.g_SectorManager_00451670)->count_0x368 + 1;
	do {
		pSectorActors[0] = 0;
		pSectorActors[-1] = 0;
		pSectorActors[-2] = 0;
		pSectorActors[-3] = 0;
		pSectorActors[-4] = 0;
		pSectorActors[-5] = 0;
		iVar7 = iVar7 + -6;
		pSectorActors = pSectorActors + -6;
	} while (-1 < iVar7);

	iVar7 = (CScene::ptable.g_ActorManager_004516a4)->nbActors;
	iVar10 = 0;
	if (0 < iVar7) {
		iVar9 = 0;
		do {
			CActor* pActor = (CActor*)0x0;
			if (iVar10 != -1) {
				pActor = (CScene::ptable.g_ActorManager_004516a4)->aActors[iVar10];
			}

			iVar2 = pActor->sectorId;
			if (iVar2 < 0) {
				iVar2 = 0;
			}

			iVar10 = iVar10 + 1;
			iVar9 = iVar9 + 4;
			aNbSectorActors[iVar2] = aNbSectorActors[iVar2] + 1;
		} while (iVar10 < iVar7);
	}

	if (this->dlistCount != 0) {
		this->ppGlobalDlist = new GlobalDlistEntry[this->dlistCount];
		sectorIndex = 0;
		if (0 < this->dlistCount) {
			pSectorActors = aNbSectorActors;
			do {
				// Always create a patch for sector 0 and 1, otherwise only if there are actors in the sector.
				if ((sectorIndex < 2) || (0 < *pSectorActors)) {
					ppGlobalDlist[sectorIndex].pDlistPatch = new CGlobalDListPatch(sectorIndex);
				}
				else {
					ppGlobalDlist[sectorIndex].pDlistPatch = NULL;
				}

				ppGlobalDlist[sectorIndex].nbMatrices = 0;
				ppGlobalDlist[sectorIndex].nbInstances = 0;
				ppGlobalDlist[sectorIndex].nbRegisteredPatches = 0;

				sectorIndex = sectorIndex + 1;
				pSectorActors = pSectorActors + 1;
			} while (sectorIndex < this->dlistCount);
		}
	}

	return;
}

void CGlobalDListManager::SectorChange(int newSectorId)
{
	GlobalDlistEntry* pActiveGlobalDlist;
	int iVar2;
	CGlobalDListPatch* pActiveDlistPatch;
	uint uVar4;
	uint uVar5;
	CGlobalDListPatch* pCVar6;

	if (newSectorId == -1) {
		if (this->activeSectorPatchId == -1) {
			pCVar6 = (CGlobalDListPatch*)0x0;
		}
		else {
			pCVar6 = this->ppGlobalDlist[this->activeSectorPatchId].pDlistPatch;
		}

		if (pCVar6 != (CGlobalDListPatch*)0x0) {
			pCVar6->bEnabled = 0;
		}

		this->activeSectorPatchId = -1;
	}
	else {
		this->activeSectorPatchId = newSectorId;

		pCVar6 = (CGlobalDListPatch*)0x0;
		if (this->activeSectorPatchId != -1) {
			pCVar6 = this->ppGlobalDlist[this->activeSectorPatchId].pDlistPatch;
		}

		if (pCVar6 != (CGlobalDListPatch*)0x0) {
			pActiveGlobalDlist = &this->ppGlobalDlist[newSectorId];

			if ((pActiveGlobalDlist->nbRegisteredPatches == 0) ||
				((pActiveGlobalDlist->nbMatrices == 0 && (pActiveGlobalDlist->nbInstances == 0)))) {
				pCVar6->bEnabled = 0;
			}
			else {
				pActiveDlistPatch = this->ppGlobalDlist[this->activeSectorPatchId].pDlistPatch;

				if (pActiveDlistPatch != (CGlobalDListPatch*)0x0) {
					uVar4 = pActiveGlobalDlist->nbRegisteredPatches;
					uVar5 = pActiveGlobalDlist->nbMatrices;
					if ((uVar4 != 0) && ((pActiveGlobalDlist->nbInstances != 0 || (uVar5 != 0)))) {
						edDListPatchableReset(pActiveDlistPatch->pDisplayListInternal, uVar4, pActiveGlobalDlist[this->activeSectorPatchId].nbInstances, uVar5);
					}
				}

				pCVar6->bEnabled = 1;
			}
		}
	}

	return;
}

int gpCurPatchRGBABuf;

void CGlobalDListManager::_ExecuteCallFunc()
{
	byte callType;
	ushort uVar2;
	GlobalDlistEntry* ppDlist;
	S_GLOBAL_DLIST_PATCH* pcVar4;
	bool bVar5;
	DisplayListCommand* pDVar6;
	CGlobalDListPatch* pGlobalPatch;
	uint curPatchId;
	CallFuncElement* ppMVar9;
	S_GLOBAL_DLIST_PATCH* pPatch;
	undefined4* puVar11;
	undefined4* puVar12;
	CGlobalDListPatch* pCVar13;
	int iVar14;
	uint* puVar15;
	CallFuncElement* pCurCallFuncElement;
	CallFuncElement* pCallFuncElement;
	int curCallFuncElementIndex;
	undefined4 uVar19;
	undefined4 uVar20;

	curCallFuncElementIndex = 0;
	pCurCallFuncElement = this->aActiveCallFuncElements;
	if (0 < this->nbActiveCallFuncElements) {
		do {
			ppDlist = this->ppGlobalDlist;
			pCallFuncElement = pCurCallFuncElement;
			curPatchId = pCurCallFuncElement->patchId >> 0x10;
			pGlobalPatch = ppDlist[curPatchId].pDlistPatch;
			bVar5 = false;
			if (curPatchId != 0) {
				if (this->activeSectorPatchId == curPatchId) {
					const int id = this->activeSectorPatchId;
					pCVar13 = (CGlobalDListPatch*)0x0;
					if (id != -1) {
						pCVar13 = ppDlist[id].pDlistPatch;
					}
					if (pCVar13 == (CGlobalDListPatch*)0x0) {
						pCurCallFuncElement->nbElements = 0;
						bVar5 = true;
					}
					else {
						bVar5 = true;
						if (this->field_0x18 != 1) {
							if (this->activeSectorPatchId != 0xffffffff) {
								if (id == -1) {
									pCVar13 = (CGlobalDListPatch*)0x0;
								}
								else {
									pCVar13 = ppDlist[id].pDlistPatch;
								}
								bVar5 = false;
								if (curPatchId == pCVar13->field_0x4dc) goto LAB_002d73f0;
							}
							pCurCallFuncElement->nbElements = 0;
							bVar5 = true;
						}
					}
				}
				else {
					pCurCallFuncElement->nbElements = 0;
					bVar5 = true;
				}
			}
		LAB_002d73f0:
			if (!bVar5) {
				callType = pCurCallFuncElement->type;
				if (callType == CALL_ELEMENT_SET_ACTIVE) {
					uVar2 = *reinterpret_cast<ushort*>(pCallFuncElement);
					if (pGlobalPatch->bEnabled != 0) {
						pGlobalPatch->field_0x18 = 1;
						edDListSetCurrent(pGlobalPatch->pDisplayListInternal);
						pGlobalPatch->pCurrentPatch = pGlobalPatch->aPatches[uVar2];
						pPatch = pGlobalPatch->pCurrentPatch;
						pcVar4 = pGlobalPatch->aPatches[uVar2];
						pDVar6 = edDListPatchableInfo(&pPatch->pVertex, &pPatch->pRgba, &pPatch->pSt, &pPatch->pUv, pcVar4->nbMatrices + pcVar4->nbInstances, (uint)uVar2);
						pPatch->field_0x20 = pDVar6;
						pGlobalPatch->field_0x4d8 = 1;
					}

					curPatchId = *reinterpret_cast<ushort*>(pCallFuncElement);
					if (pPatch == 0) {
						pGlobalPatch->aPatches[curPatchId]->field_0x0 = pGlobalPatch->aPatches[curPatchId]->field_0x0 & 0xfffffffe;
					}
					else {
						pGlobalPatch->aPatches[curPatchId]->field_0x0 = pGlobalPatch->aPatches[curPatchId]->field_0x0 | 1;
					}

					edDListPatchableShowProp(curPatchId, pCurCallFuncElement->bActive);
					edDListPatcheEnd(-1, 0);

					if (pGlobalPatch->bEnabled != 0) {
						pGlobalPatch->field_0x18 = 0;
					}

					pGlobalPatch->field_0x4d8 = 0;
					pGlobalPatch->pCurrentPatch = 0;
				}
				else {
					if (callType == CALL_ELEMENT_HIDE_SPRITE) {
						uVar2 = *(ushort*)pCallFuncElement;
						if (pGlobalPatch->bEnabled != 0) {
							pGlobalPatch->field_0x18 = 1;
							edDListSetCurrent(pGlobalPatch->pDisplayListInternal);

							pGlobalPatch->pCurrentPatch = pGlobalPatch->aPatches[uVar2];
							pPatch = pGlobalPatch->pCurrentPatch;
							pcVar4 = pGlobalPatch->aPatches[uVar2];
							pDVar6 = edDListPatchableInfo(&pPatch->pVertex, &pPatch->pRgba, &pPatch->pSt, &pPatch->pUv, pcVar4->nbMatrices + pcVar4->nbInstances, (uint)uVar2);
							pPatch->field_0x20 = pDVar6;
							pGlobalPatch->field_0x4d8 = 1;
						}

						const int bActive = this->aActiveCallFuncElements[curCallFuncElementIndex].bActive;
						_rgba* p_Var7 = pGlobalPatch->pCurrentPatch->pRgba;
						_rgba* p_Var16 = p_Var7 + bActive * 4;
						gpCurPatchRGBABuf = 0;

						// Mask out the alpha?
						uint local_c = (*(uint*)(p_Var7 + bActive * 4)) & 0xffffff;

						// Since there is four vtx in the sprite.
						p_Var16[0].rgba = local_c;
						p_Var16[1].rgba = local_c;
						p_Var16[2].rgba = local_c;
						p_Var16[3].rgba = local_c;

						edDListPatcheEnd(-1, 0);

						if (pGlobalPatch->bEnabled != 0) {
							pGlobalPatch->field_0x18 = 0;
						}

						pGlobalPatch->field_0x4d8 = 0;
						pGlobalPatch->pCurrentPatch = 0;
					}
					else {
						if (callType == CALL_ELEMENT_HIDE_VERTEX) {
							IMPLEMENTATION_GUARD(
							uVar2 = *(ushort*)pCallFuncElement;
							if (pGlobalPatch->bEnabled != 0) {
								pGlobalPatch->field_0x18 = 1;
								edDListSetCurrent(pGlobalPatch->pDisplayListInternal);
								pGlobalPatch->pCurrentPatch = (pGlobalPatch->aPatches + (uVar2 - 8))[8];
								pPatch = pGlobalPatch->pCurrentPatch;
								pcVar4 = (pGlobalPatch->aPatches + (uVar2 - 8))[8];
								pDVar6 = edDListPatchableInfo
								((long)(pPatch + 0x10), (long)(pPatch + 0x14), (long)(pPatch + 0x18),
									(long)(pPatch + 0x1c), *(int*)(pcVar4 + 8) + *(int*)(pcVar4 + 0xc), (ulong)uVar2);
								*(DisplayListCommand**)(pPatch + 0x20) = pDVar6;
								pGlobalPatch->field_0x4d8 = 1;
							}
							curPatchId = *(uint*)&pCurCallFuncElement->field_0x2c;
							iVar14 = curPatchId * 0x10;
							puVar11 = (undefined4*)(*(int*)(pGlobalPatch->pCurrentPatch + 0x10) + iVar14);
							uVar19 = puVar11[1];
							uVar20 = puVar11[2];
							pPatch = *(int*)(pGlobalPatch->pCurrentPatch + 0x10);
							if (gCurStripPatchable == 0) {
								puVar12 = (undefined4*)(pPatch + iVar14);
								*puVar12 = *puVar11;
								puVar12[1] = uVar19;
								puVar12[2] = uVar20;
								puVar12[3] = 0xc000;
							}
							else {
								puVar12 = (undefined4*)(pPatch + iVar14);
								*puVar12 = *puVar11;
								puVar12[1] = uVar19;
								puVar12[2] = uVar20;
								if (gCurDListInfo3DPatchable->field_0x4c != 3) {
									puVar12[3] = 0xc000;
								}
								curPatchId = curPatchId / 0x46;
								BYTE_ARRAY_0048d3a0[curPatchId] = BYTE_ARRAY_0048d3a0[curPatchId] + 1;
								if ((int)DAT_00448a84 < (int)curPatchId) {
									DAT_00448a84 = curPatchId;
								}
							}
							DAT_0044970c = 0;
							edDListPatcheEnd(-1, 0);

							if (pGlobalPatch->bEnabled != 0) {
								pGlobalPatch->field_0x18 = 0;
							}

							pGlobalPatch->field_0x4d8 = 0;
							pGlobalPatch->pCurrentPatch = 0;)
						}
						else {
							if (callType == 0) {
								IMPLEMENTATION_GUARD(
								if (pGlobalPatch->bEnabled != 0) {
									pGlobalPatch->field_0x18 = 1;
									edDListSetCurrent(pGlobalPatch->pDisplayListInternal);
								}
								(**(code**)(**(int**)&pCurCallFuncElement->field_0x2c + 0x10))(*(int**)&pCurCallFuncElement->field_0x2c, *pCallFuncElement);
								if (pGlobalPatch->bEnabled != 0) {
									pGlobalPatch->field_0x18 = 0;
								})
							}
						}
					}
				}

				pCurCallFuncElement->nbElements = pCurCallFuncElement->nbElements + -1;
			}

			if (pCurCallFuncElement->nbElements == 0) {
				this->nbActiveCallFuncElements = this->nbActiveCallFuncElements + -1;

				if (this->nbActiveCallFuncElements != 0) {
					ppMVar9 = this->aActiveCallFuncElements + this->nbActiveCallFuncElements;
					*pCallFuncElement = *ppMVar9;
				}
			}
			else {
				pCurCallFuncElement = pCurCallFuncElement + 1;
				curCallFuncElementIndex = curCallFuncElementIndex + 1;
			}
		} while (curCallFuncElementIndex < this->nbActiveCallFuncElements);
	}

	curCallFuncElementIndex = 0;
	_reg_data* pCurRegData = this->aRegData;
	if (0 < this->nbRegData) {
		do {
			int patchId = pCurRegData->patchId >> 0x10;
			bVar5 = true;
			if ((patchId == this->activeSectorPatchId) || (patchId == 0)) {
				pGlobalPatch = BeginCurrent(pCurRegData->patchId);
				if (pGlobalPatch != (CGlobalDListPatch*)0x0) {
					_ExecuteCallFunc_BeginDList(pCurRegData);
					pGlobalPatch = this->ppGlobalDlist[this->lastPatchId >> 0x10].pDlistPatch;
					edDListPatcheEnd(-1, 0);

					if (pGlobalPatch->bEnabled != 0) {
						pGlobalPatch->field_0x18 = 0;
					}

					pGlobalPatch->field_0x4d8 = 0;
					pGlobalPatch->pCurrentPatch = 0;
					this->lastPatchId = -1;
				}
			}
			else {
				bVar5 = false;
			}

			if ((pCurRegData->field_0x4 < 1) || (!bVar5)) {
				this->nbRegData = this->nbRegData + -1;
				pCurRegData->patchId = this->aRegData[this->nbRegData].patchId;
				pCurRegData->field_0x4 = this->aRegData[this->nbRegData].field_0x4;
			}
			else {
				pCurRegData = pCurRegData + 1;
				curCallFuncElementIndex = curCallFuncElementIndex + 1;
			}
		} while (curCallFuncElementIndex < this->nbRegData);
	}

	return;
}

bool CGlobalDListManager::SetActive(int patchId, int index)
{
	bool uVar1;
	int iVar1;

	if ((this->bCompletedLevelInit == 0) || (this->activeSectorPatchId == -1)) {
		iVar1 = -1;
	}
	else {
		iVar1 = 1;
	}

	uVar1 = false;
	if (iVar1 == 1) {
		if ((this->activeSectorPatchId == patchId >> 0x10) || (patchId >> 0x10 == 0)) {
			uVar1 = _AddCallFuncElement(patchId, CALL_ELEMENT_SET_ACTIVE, index);
		}
		else {
			uVar1 = false;
		}
	}

	return uVar1;
}

CGlobalDListPatch* CGlobalDListManager::BeginCurrent(int patchId)
{
	bool bEnabled;
	S_GLOBAL_DLIST_PATCH* pCurrentPatch;
	S_GLOBAL_DLIST_PATCH* pPatchInfo;
	DisplayListCommand* pDListCmd;
	CGlobalDListPatch* pPatch;
	_reg_data* pIntIt;
	int index;
	_reg_data* pNextFree;

	if (patchId >> 0x10 == 0) {
		if (this->field_0x1c == 1) {
			return (CGlobalDListPatch*)0x0;
		}
	}
	else {
		pPatch = (CGlobalDListPatch*)0x0;

		if (this->activeSectorPatchId != -1) {
			pPatch = this->ppGlobalDlist[this->activeSectorPatchId].pDlistPatch;
		}
		if ((pPatch == (CGlobalDListPatch*)0x0) || (this->field_0x18 == 1)) {
			return (CGlobalDListPatch*)0x0;
		}
	}

	index = 0;
	pIntIt = this->aRegData;
	do {
		index = index + 1;
		pNextFree = pIntIt;
		pIntIt = pIntIt + 1;
		if (this->nbRegData <= index) break;
	} while (patchId != pNextFree->patchId);

	pPatch = this->ppGlobalDlist[patchId >> 0x10].pDlistPatch;
	bEnabled = pPatch->bEnabled != 0;

	if (bEnabled) {
		pPatch->field_0x18 = 1;
		edDListSetCurrent(pPatch->pDisplayListInternal);
	}

	if (bEnabled) {
		pPatch->pCurrentPatch = pPatch->aPatches[patchId & 0xffffU];
		pCurrentPatch = pPatch->pCurrentPatch;
		pPatchInfo = pPatch->aPatches[patchId & 0xffffU];
		pDListCmd = edDListPatchableInfo(&pCurrentPatch->pVertex, &pCurrentPatch->pRgba, &pCurrentPatch->pSt, &pCurrentPatch->pUv, pPatchInfo->nbMatrices + pPatchInfo->nbInstances, patchId & 0xffffU);
		pCurrentPatch->field_0x20 = pDListCmd;
		pPatch->field_0x4d8 = 1;
		this->lastPatchId = patchId;
		if (patchId == pNextFree->patchId) {
			_ExecuteCallFunc_BeginDList(pNextFree);
		}
	}
	else {
		pPatch = (CGlobalDListPatch*)0x0;
	}

	return pPatch;
}

void CGlobalDListManager::EndCurrent(int nbVertex, int param_2)
{
	CGlobalDListPatch* pPatch;
	CGlobalDListManager* pManager;

	pManager = CScene::ptable.g_GlobalDListManager_004516bc;
	pPatch = (CScene::ptable.g_GlobalDListManager_004516bc)->ppGlobalDlist[(CScene::ptable.g_GlobalDListManager_004516bc)->lastPatchId >> 0x10].pDlistPatch;

	edDListPatcheEnd(nbVertex, param_2 & 0xff);

	if (pPatch->bEnabled != 0) {
		pPatch->field_0x18 = 0;
	}

	pPatch->field_0x4d8 = 0;
	pPatch->pCurrentPatch = (S_GLOBAL_DLIST_PATCH*)0x0;
	pManager->lastPatchId = -1;

	return;
}

bool CGlobalDListManager::_AddCallFuncElement(int patchId, DLIST_FUNCTION type, int bActive)
{
	int iVar1;
	bool bSuccess;
	CallFuncElement* pCVar3;
	int iVar4;

	iVar1 = this->nbActiveCallFuncElements;
	bSuccess = false;
	if (iVar1 < 400) {
		iVar4 = 0;
		pCVar3 = this->aActiveCallFuncElements;
		if (0 < iVar1) {
			do {
				if (((pCVar3->patchId == patchId) && (pCVar3->type == CALL_ELEMENT_SET_ACTIVE)) && (type == CALL_ELEMENT_SET_ACTIVE)) {
					bSuccess = true;
					pCVar3->type = type;
					pCVar3->bActive = bActive;
					pCVar3->nbElements = 2;
					break;
				}

				iVar4 = iVar4 + 1;
				pCVar3 = pCVar3 + 1;
			} while (iVar4 < iVar1);
		}

		if (!bSuccess) {
			this->aActiveCallFuncElements[this->nbActiveCallFuncElements].patchId = patchId;
			this->aActiveCallFuncElements[this->nbActiveCallFuncElements].type = type;
			this->aActiveCallFuncElements[this->nbActiveCallFuncElements].bActive = bActive;
			this->aActiveCallFuncElements[this->nbActiveCallFuncElements].nbElements = 2;
			this->nbActiveCallFuncElements = this->nbActiveCallFuncElements + 1;
		}

		bSuccess = true;
	}
	else {
		bSuccess = false;
	}

	return bSuccess;
}

bool CGlobalDListManager::_AddCallFuncElement2(int patchId, DLIST_FUNCTION type, short instanceId, int param_5)
{
	bool bSuccess;
	_reg_data* pRegData;
	int nextFreeRegIndex;
	int curRegDataIndex;

	bSuccess = false;
	if (this->nbActiveCallFuncElements2 < 400) {
		nextFreeRegIndex = 0;

		this->aActiveCallFuncElements2[this->nbActiveCallFuncElements2].patchId = patchId;
		this->aActiveCallFuncElements2[this->nbActiveCallFuncElements2].type = type;
		this->aActiveCallFuncElements2[this->nbActiveCallFuncElements2].instanceId = instanceId;
		this->aActiveCallFuncElements2[this->nbActiveCallFuncElements2].field_0x4 = param_5;
		this->aActiveCallFuncElements2[this->nbActiveCallFuncElements2].field_0xb = (byte)this->field_0x24;
		this->nbActiveCallFuncElements2 = this->nbActiveCallFuncElements2 + 1;

		for (pRegData = this->aRegData; (nextFreeRegIndex < this->nbRegData && (pRegData->patchId != patchId)); pRegData = pRegData + 1) {
			nextFreeRegIndex = nextFreeRegIndex + 1;
		}

		if (this->nbRegData <= nextFreeRegIndex) {
			curRegDataIndex = this->nbRegData;
			this->nbRegData = curRegDataIndex + 1;
			this->aRegData[curRegDataIndex].patchId = patchId;
		}

		bSuccess = true;
		this->aRegData[nextFreeRegIndex].field_0x4 = this->aRegData[nextFreeRegIndex].field_0x4 + 1;
	}

	return bSuccess;
}

void CGlobalDListManager::_ExecuteCallFunc_BeginDList(_reg_data* pRegData)
{
	byte bVar1;
	short sVar2;
	CGlobalDListPatch* pCVar3;
	bool bVar4;
	int iVar5;
	CGlobalDListPatch* pCVar6;
	edVertex* peVar7;
	int iVar9;
	int iVar10;
	ulong uVar11;
	CallFuncElement2* pElement2;
	edVertex* peVar13;
	ulong uVar14;
	uint uVar15;
	ulong uVar16;
	ulong uVar17;
	_rgba* p_Var18;
	undefined4* puVar19;
	float* pfVar20;
	_rgba* p_Var21;
	float fVar22;
	float fVar23;
	_rgba local_c;

	if (this->nbActiveCallFuncElements2 != 0) {
		pCVar3 = this->ppGlobalDlist[this->lastPatchId >> 0x10].pDlistPatch;
		iVar5 = 0;
		pElement2 = this->aActiveCallFuncElements2;
		if (0 < this->nbActiveCallFuncElements2) {
			do {
				iVar10 = pElement2->patchId;
				if ((iVar10 == this->lastPatchId) && ((uint)pElement2->field_0xb != this->field_0x24))
				{
					uVar15 = iVar10 >> 0x10;
					bVar4 = false;
					if (uVar15 != 0) {
						iVar10 = this->activeSectorPatchId;
						pCVar6 = (CGlobalDListPatch*)0x0;

						if (iVar10 != -1) {
							pCVar6 = this->ppGlobalDlist[iVar10].pDlistPatch;
						}

						if (pCVar6 != (CGlobalDListPatch*)0x0) {
							if ((this->field_0x18 != 1) && (this->activeSectorPatchId != -1)) {
								pCVar6 = (CGlobalDListPatch*)0x0;

								if (iVar10 != -1) {
									pCVar6 = this->ppGlobalDlist[iVar10].pDlistPatch;
								}

								if (uVar15 == pCVar6->field_0x4dc) goto LAB_002d6c48;
							}

							bVar4 = true;
						}
					}
				LAB_002d6c48:
					if (!bVar4) {
						bVar1 = pElement2->type;
						if (bVar1 == 9) {
							p_Var21 = pCVar3->pCurrentPatch->pRgba;
							iVar10 = (int)pElement2->instanceId;
							gpCurPatchRGBABuf = 0;
							uint temp = (uint)p_Var21[iVar10 * 4].rgba & 0xffffff;
							local_c.rgba = temp;
							edDListPatchRGBASprite_Inline(p_Var21, &local_c, iVar10);
						}
						else {
							if (bVar1 == 8) {
								IMPLEMENTATION_GUARD(
								sVar2 = pElement2->instanceId;
								uVar15 = SEXT24(sVar2);
								peVar13 = pCVar3->pCurrentPatch->pVertex + uVar15;
								fVar22 = peVar13->y;
								fVar23 = peVar13->z;
								peVar7 = pCVar3->pCurrentPatch->pVertex;
								if (gCurStripPatchable == (ed_3d_strip*)0x0) {
									peVar7 = peVar7 + uVar15;
									peVar7->x = peVar13->x;
									peVar7->y = fVar22;
									peVar7->z = fVar23;
									peVar7->skip = 0xc000;
								}
								else {
									peVar7 = peVar7 + uVar15;
									peVar7->x = peVar13->x;
									peVar7->y = fVar22;
									peVar7->z = fVar23;
									if (gCurDListInfo3DPatchable->primType != 3) {
										peVar7->skip = 0xc000;
									}
									iVar10 = (int)((ulong)uVar15 * 0xd41d41d5 >> 0x20);
									uVar15 = ((uint)(sVar2 - iVar10) >> 1) + iVar10 >> 6;
									gCurPacketPatched[uVar15] = gCurPacketPatched[uVar15] + 1;
									if (gLastPacketPatched < (int)uVar15) {
										gLastPacketPatched = uVar15;
									}
								}
								DAT_0044970c = 0;)
							}
							else {
								if (bVar1 == 0xc) {
									IMPLEMENTATION_GUARD(
									sVar2 = pElement2->instanceId;
									uVar11 = SEXT28(sVar2);
									uVar14 = uVar11;
									if (0x47 < uVar11) {
										uVar14 = SEXT48((int)((int)sVar2 + (((int)sVar2 - 0x48U) / 0x46 + 1) * 2));
									}
									iVar10 = pCVar3->pCurrentPatch->nbMatrices;
									uVar17 = SEXT48(iVar10);
									iVar9 = (int)sVar2;
									uVar16 = uVar11;
									if (0x47 < uVar11) {
										uVar16 = SEXT48((int)(iVar9 + ((iVar9 - 0x48U) / 0x46 + 1) * 2));
									}
									if (0x47 < uVar17) {
										uVar17 = SEXT48((int)(iVar10 + ((iVar10 - 0x48U) / 0x46 + 1) * 2));
									}
									puVar19 = (undefined4*)(pCVar3->pCurrentPatch->pSt + (int)uVar16 * 4);
									*puVar19 = *(undefined4*)(pElement2->field_0x4 + (int)uVar14 * 4);
									if (((1 < uVar11) && ((uint)(iVar9 % 0x46) < 2)) && (uVar16 < uVar17)) {
										puVar19[2] = *puVar19;
									}
									DAT_00449710 = 0;)
								}
								else {
									if (bVar1 == 0xb) {
										IMPLEMENTATION_GUARD(
										sVar2 = pElement2->instanceId;
										uVar11 = SEXT28(sVar2);
										uVar14 = uVar11;
										if (0x47 < uVar11) {
											uVar14 = SEXT48((int)((int)sVar2 + (((int)sVar2 - 0x48U) / 0x46 + 1) * 2));
										}
										p_Var21 = (_rgba*)(pElement2->field_0x4 + (int)uVar14 * 4);
										iVar10 = pCVar3->pCurrentPatch->nbMatrices;
										uVar16 = SEXT48(iVar10);
										iVar9 = (int)sVar2;
										uVar14 = uVar11;
										if (0x47 < uVar11) {
											uVar14 = SEXT48((int)(iVar9 + ((iVar9 - 0x48U) / 0x46 + 1) * 2));
										}
										if (0x47 < uVar16) {
											uVar16 = SEXT48((int)(iVar10 + ((iVar10 - 0x48U) / 0x46 + 1) * 2));
										}
										p_Var18 = pCVar3->pCurrentPatch->pRgba + (int)uVar14;
										*p_Var18 = *p_Var21;
										if (((1 < uVar11) && ((uint)(iVar9 % 0x46) < 2)) && (uVar14 < uVar16)) {
											p_Var18[2] = *p_Var21;
										}
										gpCurPatchRGBABuf = 0;)
									}
									else {
										if (bVar1 == 10) {
											IMPLEMENTATION_GUARD(
											sVar2 = pElement2->instanceId;
											peVar7 = pCVar3->pCurrentPatch->pVertex;
											uVar15 = SEXT24(sVar2);
											pfVar20 = (float*)(pElement2->field_0x4 + uVar15 * 0x10);
											if (gCurStripPatchable == (ed_3d_strip*)0x0) {
												peVar7 = peVar7 + uVar15;
												peVar7->x = *pfVar20;
												peVar7->y = pfVar20[1];
												peVar7->z = pfVar20[2];
												peVar7->skip = (uint)pfVar20[3];
											}
											else {
												peVar7 = peVar7 + uVar15;
												peVar7->x = *pfVar20;
												peVar7->y = pfVar20[1];
												peVar7->z = pfVar20[2];
												if (gCurDListInfo3DPatchable->primType != 3) {
													peVar7->skip = (uint)pfVar20[3];
												}
												iVar10 = (int)((ulong)uVar15 * 0xd41d41d5 >> 0x20);
												uVar15 = ((uint)(sVar2 - iVar10) >> 1) + iVar10 >> 6;
												gCurPacketPatched[uVar15] = gCurPacketPatched[uVar15] + 1;
												if (gLastPacketPatched < (int)uVar15) {
													gLastPacketPatched = uVar15;
												}
											}
											DAT_0044970c = 0;)
										}
									}
								}
							}
						}

						this->nbActiveCallFuncElements2 = this->nbActiveCallFuncElements2 + -1;

						if (this->nbActiveCallFuncElements2 != 0) {
							CallFuncElement2* pOther = &this->aActiveCallFuncElements2[this->nbActiveCallFuncElements2];
							*pElement2 = *pOther;
						}

						pRegData->field_0x4 = pRegData->field_0x4 + -1;
					}
				}
				else {
					pElement2 = pElement2 + 1;
					iVar5 = iVar5 + 1;
				}
			} while (iVar5 < this->nbActiveCallFuncElements2);
		}
	}

	return;
}

CGlobalDListPatch::~CGlobalDListPatch()
{
	if (this->field_0x4d9 != 0) {
		this->pDisplayListInternal = (DisplayList*)0x0;
	}

	return;
}

void GlobalDList_Init(void)
{
	ed_3D_Scene* pSVar1;
	int freeMemCalcA;
	CGlobalDList* pDVar2;
	DisplayList* pDVar3;
	int iVar4;
	int iVar5;
	uint uVar6;
	char* pcVar7;
	float fVar8;

	/* -----------------------------------------
	   ----- Memory used by display list ------- */
	edDebugPrintf(g_NewLine);
	edDebugPrintf(sz_DisplayListSpacer_00433970);
	edDebugPrintf("----- Memory used by display list -------\n");
	edDebugPrintf(g_NewLine);
	freeMemCalcA = edMemGetAvailable(TO_HEAP(H_MAIN));
	pGameDList = new CGlobalDList(0x200, 0x1000, 0, 0x11, CScene::_scene_handleA);
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));

	/* - Game list          : %07d\n */
	edDebugPrintf("- Game list          : %07d\n", freeMemCalcA - iVar5);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	pFrontend2DDList = new CGlobalDList(0x20, 0x80, 0, 0x11, CFrontend::_scene_handle);
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));

	/* - Frontend 3D list   : %07d\n */
	edDebugPrintf("- Frontend 3D list   : %07d\n", iVar4 - iVar5);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	pGuiDList = new CGlobalDList(0x180, 0xe74, 0, 0x12, pSVar1);
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));
	/* - Gui list           : %07d\n */
	edDebugPrintf("- Gui list           : %07d\n", iVar4 - iVar5);
	edDebugPrintf(g_NewLine);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));
	uVar6 = freeMemCalcA - iVar5;
	if ((int)uVar6 < 0) {
		fVar8 = (float)(uVar6 >> 1 | uVar6 & 1);
		fVar8 = fVar8 + fVar8;
	}
	else {
		fVar8 = (float)uVar6;
	}
	pcVar7 = WorkOutMo(fVar8 / 1048576.0f, 3, g_DebugTextBuffer_00468ef0, 0, (char*)0x0);
	/* --- total used : \t%d (%s Mo) ----
	   ----------------------------------------- */
	edDebugPrintf("--- total used : \t%d (%s Mo) ----\n", freeMemCalcA - iVar4, pcVar7);
	edDebugPrintf(sz_DisplayListSpacer_00433970);
	edDebugPrintf(g_NewLine);
	pGameDList->Init();
	pFrontend2DDList->Init();
	if (pGuiDList != (CGlobalDList*)0x0) {
		pGuiDList->Init();
	}
	edDListSetActiveViewPort(CScene::_pinstance->pViewportA);
	return;
}

void GlobalDList_AddToView(void)
{
	if (pGameDList->bEnabled != 0) {
		edDlistAddtoView(pGameDList->pDisplayListInternal);
	}

	if (pFrontend2DDList->bEnabled != 0) {
		edDlistAddtoView(pFrontend2DDList->pDisplayListInternal);
	}

	if ((pGuiDList != (CGlobalDList*)0x0) && (pGuiDList->bEnabled != 0)) {
		edDlistAddtoView(pGuiDList->pDisplayListInternal);
	}

	return;
}

bool GameDList_BeginCurrent(void)
{
	bool bVar1;
	CGlobalDList* pCVar2;

	pCVar2 = pGameDList;
	bVar1 = pGameDList->bEnabled != 0;
	if (bVar1) {
		pGameDList->field_0x18 = 1;
		edDListSetCurrent(pCVar2->pDisplayListInternal);
	}
	return bVar1;
}

void GameDList_EndCurrent(void)
{
	if (pGameDList->bEnabled != 0) {
		pGameDList->field_0x18 = 0;
	}
	return;
}

uint GameDListPatch_Register(CObject* pObject, int nbMatrices, int nbInstances)
{
	CGlobalDListPatch* pSectorPatch;
	int nbPatches;
	CGlobalDListManager* pCVar3;
	S_GLOBAL_DLIST_PATCH* pNewPatch;
	int sectorId;

	pCVar3 = CScene::ptable.g_GlobalDListManager_004516bc;
	sectorId = pObject->sectorId;
	if (sectorId == -1) {
		sectorId = 0;
	}

	pSectorPatch = (CScene::ptable.g_GlobalDListManager_004516bc)->ppGlobalDlist[sectorId].pDlistPatch;

	pNewPatch = NewPool_S_EYES_BRIGHT_SHADOW(1);
	pSectorPatch->aPatches[pSectorPatch->nbTotalPatches] = pNewPatch;
	pNewPatch->field_0x0 = 0;
	pNewPatch->pOwner = pObject;
	pNewPatch->nbMatrices = nbMatrices;
	pNewPatch->nbInstances = nbInstances;

	nbPatches = pSectorPatch->nbTotalPatches;
	pSectorPatch->nbTotalPatches = nbPatches + 1;
	pCVar3->ppGlobalDlist[sectorId].nbMatrices = pCVar3->ppGlobalDlist[sectorId].nbMatrices + nbMatrices;
	pCVar3->ppGlobalDlist[sectorId].nbInstances = pCVar3->ppGlobalDlist[sectorId].nbInstances + nbInstances;
	pCVar3->ppGlobalDlist[sectorId].nbRegisteredPatches = pCVar3->ppGlobalDlist[sectorId].nbRegisteredPatches + 1;

	// Store patch ID and index in one unsigned integer to return.
	return sectorId * 0x10000 + nbPatches;
}

// Should be in: D:/Projects/b-witch/DlistManager.cpp
CGlobalDListPatch* GameDListPatch_BeginCurrent(int patchId)
{
	return CScene::ptable.g_GlobalDListManager_004516bc->BeginCurrent(patchId);
}

void GameDListPatch_EndCurrent(int nbVertex, int param_2)
{
	return CScene::ptable.g_GlobalDListManager_004516bc->EndCurrent(nbVertex, param_2);
}

void GameDListPatch_HideSprite(int patchId, uint index)
{
	CScene::ptable.g_GlobalDListManager_004516bc->_AddCallFuncElement(patchId, CALL_ELEMENT_HIDE_SPRITE, index);

	return;
}

void GameDListPatch_HideVertex(int patchId, int index)
{
	CScene::ptable.g_GlobalDListManager_004516bc->_AddCallFuncElement(patchId, CALL_ELEMENT_HIDE_VERTEX, index);

	return;
}

bool GuiDList_BeginCurrent(void)
{
	bool bVar1;

	bVar1 = pGuiDList->bEnabled != 0;
	if (bVar1) {
		pGuiDList->field_0x18 = 1;
		edDListSetCurrent(pGuiDList->pDisplayListInternal);
	}

	return bVar1;
}

void GuiDList_EndCurrent(void)
{
	if (pGuiDList->bEnabled != 0) {
		pGuiDList->field_0x18 = 0;
	}
	return;
}

bool Frontend2DDList_BeginCurrent(void)
{
	bool bVar1;
	CGlobalDList* pCVar2;

	pCVar2 = pFrontend2DDList;
	bVar1 = pFrontend2DDList->bEnabled != 0;
	if (bVar1) {
		pFrontend2DDList->field_0x18 = 1;
		edDListSetCurrent(pCVar2->pDisplayListInternal);
	}
	return bVar1;
}

void FrontendDList_EndCurrent(void)
{
	if (pFrontend2DDList->bEnabled != 0) {
		pFrontend2DDList->field_0x18 = 0;
	}
	return;
}

CGlobalDList::CGlobalDList()
{
	this->nbCommands = -1;
	this->nbMatrices = -1;
	this->field_0x10 = -1;
	this->pDisplayListInternal = (DisplayList*)0x0;
	this->bEnabled = 0;
}

CGlobalDList::CGlobalDList(int nbCommands, int nbMatrices, int inField_0x10, int flags, ed_3D_Scene* pInStaticMeshMaster)
{
	this->nbCommands = nbCommands;
	this->nbMatrices = nbMatrices;
	this->field_0x10 = inField_0x10;
	this->flags = flags;

	this->pDisplayListInternal = edDListNew(TO_HEAP(H_MAIN), flags, nbCommands, inField_0x10, nbMatrices, 0, (DisplayList*)0x0);
	edDListSetSceneUsed(this->pDisplayListInternal, pInStaticMeshMaster);
}

CGlobalDList::~CGlobalDList()
{
	if (this->pDisplayListInternal != (DisplayList*)0x0) {
		edDListDelete(this->pDisplayListInternal);
	}

	return;
}

void CGlobalDList::Init()
{
	this->bEnabled = 1;
	this->field_0x18 = 0;

	return;
}