#include "DlistManager.h"
#include "SectorManager.h"
#include "Rendering/DisplayList.h"
#include "ActorManager.h"
#include "edDlist.h"
#include "Actor.h"

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

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
	int iVar6;
	S_GLOBAL_DLIST_PATCH** pCurEntry;
	int iVar8;
	int iVar9;
	//ParticleData_b8* piVar2;

	if (this->dlistCount != 0) {
		if (this->field_0x1c == 1) {
			pCurList = this->ppGlobalDlist->pDlistPatch;

			if ((pCurList != (CGlobalDListPatch*)0x0) && (iVar6 = pCurList->nbTotalPatches, iVar6 != 0)) {
				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 1;
					edDListSetCurrent(pCurList->pDisplayListInternal);
				}

				iVar9 = 0;
				pCurEntry = pCurList->aPatches;
				if (0 < iVar6) {
					do {
						if (*pCurEntry != (S_GLOBAL_DLIST_PATCH*)0x0) {
							(*pCurEntry)->pOwner->InitDlistPatchable(iVar9);
						}

						pCurEntry = pCurEntry + 1;
						iVar9 = iVar9 + 1;
					} while (iVar9 < iVar6);
				}

				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 0;
				}
			}

			this->field_0x1c = 0;
		}

		if (((1 < this->dlistCount) && (this->field_0x18 == 1)) && (iVar6 = this->field_0x14, iVar6 != -1)) {
			pCurList = this->ppGlobalDlist[iVar6].pDlistPatch;

			if ((pCurList != (CGlobalDListPatch*)0x0) && (iVar9 = pCurList->nbTotalPatches, iVar9 != 0)) {
				if (pCurList->bEnabled != 0) {
					pCurList->field_0x18 = 1;
					edDListSetCurrent(pCurList->pDisplayListInternal);
				}

				iVar8 = 0;
				if (0 < iVar9) {
					iVar6 = iVar6 << 0x10;
					pCurEntry = pCurList->aPatches;
					do {
						if (*pCurEntry != (S_GLOBAL_DLIST_PATCH*)0x0) {
							(*pCurEntry)->pOwner->InitDlistPatchable(iVar6);
						}

						iVar8 = iVar8 + 1;
						pCurEntry = pCurEntry + 1;
						iVar6 = iVar6 + 1;
					} while (iVar8 < iVar9);
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
			if (this->field_0x14 != -1) {
				pDlistPatch = this->ppGlobalDlist[this->field_0x14].pDlistPatch;
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
	this->field_0x14 = -1;
	this->field_0x18 = 0;
	this->field_0x1c = 0;
	//this->field_0x20 = -1;
	this->bCompletedLevelInit = 0;
	this->nbActiveCallFuncElements = 0;
	//this->count_0x25ac = 0;
	this->field_0x24 = 0;
	this->field_0x29b0 = 0;
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
	GlobalDlistEntry* pGVar1;
	int iVar2;
	CGlobalDListPatch* pCVar3;
	uint uVar4;
	uint uVar5;
	CGlobalDListPatch* pCVar6;

	if (newSectorId == -1) {
		if (this->field_0x14 == -1) {
			pCVar6 = (CGlobalDListPatch*)0x0;
		}
		else {
			pCVar6 = this->ppGlobalDlist[this->field_0x14].pDlistPatch;
		}

		if (pCVar6 != (CGlobalDListPatch*)0x0) {
			pCVar6->bEnabled = 0;
		}

		this->field_0x14 = -1;
	}
	else {
		this->field_0x14 = newSectorId;
		pCVar6 = (CGlobalDListPatch*)0x0;
		if (this->field_0x14 != -1) {
			pCVar6 = this->ppGlobalDlist[this->field_0x14].pDlistPatch;
		}

		if (pCVar6 != (CGlobalDListPatch*)0x0) {
			pGVar1 = &this->ppGlobalDlist[newSectorId];
			if ((pGVar1->nbRegisteredPatches == 0) ||
				((pGVar1->nbMatrices == 0 && (pGVar1[newSectorId].nbInstances == 0)))) {
				pCVar6->bEnabled = 0;
			}
			else {
				iVar2 = this->field_0x14;
				pCVar3 = this->ppGlobalDlist[iVar2].pDlistPatch;

				if (pCVar3 != (CGlobalDListPatch*)0x0) {
					uVar4 = pGVar1->nbRegisteredPatches;
					uVar5 = pGVar1->nbMatrices;
					if ((uVar4 != 0) && ((pGVar1->nbInstances != 0 || (uVar5 != 0)))) {
						edDListPatchableReset(pCVar3->pDisplayListInternal, uVar4, pGVar1[iVar2].nbInstances, uVar5);
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
				if (this->field_0x14 == curPatchId) {
					const int id = this->field_0x14;
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
							if (this->field_0x14 != 0xffffffff) {
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
	pCurCallFuncElement = this->aActiveCallFuncElements;
	if (0 < this->field_0x29b0) {
		IMPLEMENTATION_GUARD(
		do {
			pPatch = *(int*)&pCurCallFuncElement->field_0x25b0 >> 0x10;
			bVar5 = true;
			if ((pPatch == this->field_0x14) || (pPatch == 0)) {
				pGlobalPatch = BeginCurrent(this, (long)*(int*)&pCurCallFuncElement->field_0x25b0);
				if (pGlobalPatch != (CGlobalDListPatch*)0x0) {
					_ExecuteCallFunc_BeginDList(this, (int)(ManagerFunctionData**)&pCurCallFuncElement->field_0x25b0);
					pGlobalPatch = this->ppGlobalDlist[this->field_0x20 >> 0x10].pDlistPatch;
					edDListPatcheEnd(-1, 0);
					if (pGlobalPatch->bEnabled != 0) {
						pGlobalPatch->field_0x18 = 0;
					}
					pGlobalPatch->field_0x4d8 = 0;
					pGlobalPatch->pCurrentPatch = 0;
					this->field_0x20 = -1;
				}
			}
			else {
				bVar5 = false;
			}
			if ((*(int*)&pCurCallFuncElement->field_0x25b4 < 1) || (!bVar5)) {
				this->field_0x29b0 = this->field_0x29b0 + -1;
				pPatch = this->field_0x29b0;
				*(ManagerFunctionData**)&pCurCallFuncElement->field_0x25b0 = (&this->pManagerFunctionData + pPatch * 2)[0x96c];
				*(ManagerFunctionData**)&pCurCallFuncElement->field_0x25b4 = (&this->pManagerFunctionData + pPatch * 2)[0x96d];
			}
			else {
				pCurCallFuncElement = (CGlobalDListManager*)&pCurCallFuncElement->dlistCount;
				curCallFuncElementIndex = curCallFuncElementIndex + 1;
			}
		} while (curCallFuncElementIndex < this->field_0x29b0);)
	}

	return;
}

bool CGlobalDListManager::SetActive(int patchId, int index)
{
	bool uVar1;
	int iVar1;

	if ((this->bCompletedLevelInit == 0) || (this->field_0x14 == -1)) {
		iVar1 = -1;
	}
	else {
		iVar1 = 1;
	}

	uVar1 = false;
	if (iVar1 == 1) {
		if ((this->field_0x14 == patchId >> 0x10) || (patchId >> 0x10 == 0)) {
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
	return nullptr;
}

void CGlobalDListManager::EndCurrent(int nbVertex, int param_2)
{
}


bool CGlobalDListManager::_AddCallFuncElement(int patchId, int type, int bActive)
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

CGlobalDListPatch::~CGlobalDListPatch()
{
	if (this->field_0x4d9 != 0) {
		this->pDisplayListInternal = (DisplayList*)0x0;
	}

	return;
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
