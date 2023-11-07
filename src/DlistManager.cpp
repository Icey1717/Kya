#include "DlistManager.h"
#include "SectorManager.h"
#include "Rendering/DisplayList.h"
#include "ActorManager.h"
#include "edDlist.h"
#include "Actor.h"

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

void CGlobalDList::Init()
{
	this->bEnabled = 1;
	this->field_0x18 = 0;
	return;
}

void CGlobalDListManager::Level_Init()
{
	GlobalDlistEntry* pGVar1;
	CGlobalDListPatch* pCVar2;
	CGlobalDListPatch* pCVar3;
	ed_3D_Scene* peVar4;
	DisplayListInternal* pDVar5;
	int iVar6;
	int iVar7;
	uint uVar8;
	int iVar9;
	int iVar10;
	uint size;
	int unaff_s6_lo;
	int unaff_s7_lo;
	int unaff_s8_lo;

	this->bBeganLevelInit = 1;
	if (this->dlistCount != 0) {
		pGVar1 = this->ppGlobalDlist;
		if ((pGVar1->field_0x4 != 0) || (pGVar1->field_0xc != 0)) {
			if (pGVar1->field_0x8 < 2) {
				pGVar1->field_0x8 = 2;
			}
			peVar4 = CScene::_scene_handleA;
			pGVar1 = this->ppGlobalDlist;
			iVar10 = pGVar1->field_0x8;
			iVar9 = pGVar1->field_0x4;
			iVar6 = pGVar1->field_0xc;
			pCVar2 = pGVar1->pDlistPatch;
			pCVar2->field_0x8 = iVar10;
			pCVar2->field_0xc = iVar9;
			pCVar2->field_0x10 = iVar6;
			pCVar2->field_0x1c = 0x101;
			pDVar5 = edDListNew(TO_HEAP(H_MAIN), 0x101, iVar10, iVar6, iVar9, 0, (DisplayListInternal*)0x0);
			pCVar2->pDisplayListInternal = pDVar5;
			edDListSetSceneUsed(pCVar2->pDisplayListInternal, peVar4);
			this->ppGlobalDlist->pDlistPatch->Init();
			this->field_0x1c = 1;
		}
		if (1 < this->dlistCount) {
			size = 0;
			iVar10 = 1;
			if (1 < this->dlistCount) {
				do {
					iVar9 = iVar10;
					if (iVar10 == -1) {
						iVar9 = 0;
					}
					pGVar1 = this->ppGlobalDlist;
					if (pGVar1[iVar9].field_0x4 == 0) {
						iVar9 = iVar10;
						if (iVar10 == -1) {
							iVar9 = 0;
						}
						if (pGVar1[iVar9].field_0xc != 0) goto LAB_002d82e8;
					}
					else {
					LAB_002d82e8:
						iVar9 = iVar10;
						if (iVar10 == -1) {
							iVar9 = 0;
						}
						if ((int)pGVar1[iVar9].field_0x8 < 2) {
							iVar9 = iVar10;
							if (iVar10 == -1) {
								iVar9 = 0;
							}
							pGVar1[iVar9].field_0x8 = 2;
						}
						iVar9 = iVar10;
						if (iVar10 == -1) {
							iVar9 = 0;
						}
						pGVar1 = this->ppGlobalDlist;
						iVar9 = pGVar1[iVar9].field_0x4;
						iVar6 = iVar10;
						if (iVar10 == -1) {
							iVar6 = 0;
						}
						iVar6 = pGVar1[iVar6].field_0xc;
						iVar7 = iVar10;
						if (iVar10 == -1) {
							iVar7 = 0;
						}
						iVar7 = pGVar1[iVar7].field_0x8;
						uVar8 = edDListGetBufSizeNeeded(0x101, iVar7, iVar6, iVar9, (uint*)0x0, (uint*)0x0);
						if ((int)size < (int)uVar8) {
							size = uVar8;
							unaff_s6_lo = iVar9;
							unaff_s8_lo = iVar6;
							unaff_s7_lo = iVar7;
						}
					}
					iVar10 = iVar10 + 1;
				} while (iVar10 < this->dlistCount);
			}
			if (size != 0) {
				iVar10 = edMemGetAvailable(TO_HEAP(H_MAIN));
				pDVar5 = (DisplayListInternal*)edMemAlloc(TO_HEAP(H_MAIN), size);
				this->pDisplayList = pDVar5;
				peVar4 = CScene::_scene_handleA;
				pDVar5 = this->pDisplayList;
				pCVar2 = this->ppGlobalDlist[1].pDlistPatch;
				pCVar2->field_0x8 = unaff_s7_lo;
				pCVar2->field_0xc = unaff_s6_lo;
				pCVar2->field_0x10 = unaff_s8_lo;
				pCVar2->field_0x1c = 0x101;
				pDVar5 = edDListNew(TO_HEAP(H_MAIN), 0x101, unaff_s7_lo, unaff_s8_lo, unaff_s6_lo, 0, pDVar5);
				pCVar2->pDisplayListInternal = pDVar5;
				edDListSetSceneUsed(pCVar2->pDisplayListInternal, peVar4);
				this->ppGlobalDlist[1].pDlistPatch->Init();
				iVar9 = edMemGetAvailable(TO_HEAP(H_MAIN));
				/* - Sector patchable \t\t: %d\n\n */
				edDebugPrintf("- Sector patchable \t\t: %d\n\n", iVar10 - iVar9);
				iVar10 = 2;
				if (2 < this->dlistCount) {
					do {
						iVar9 = iVar10;
						if (iVar10 == -1) {
							iVar9 = 0;
						}
						pGVar1 = this->ppGlobalDlist;
						if (pGVar1[iVar9].pDlistPatch != (CGlobalDListPatch*)0x0) {
							pCVar2 = pGVar1[1].pDlistPatch;
							iVar9 = iVar10;
							if (iVar10 == -1) {
								iVar9 = 0;
							}
							pCVar3 = pGVar1[iVar9].pDlistPatch;
							pCVar3->field_0x4d9 = 1;
							pCVar3->pDisplayListInternal = pCVar2->pDisplayListInternal;
							pCVar3->field_0x8 = pCVar2->field_0x8;
							pCVar3->field_0xc = pCVar2->field_0xc;
							pCVar3->field_0x10 = pCVar2->field_0x10;
							pCVar3->field_0x1c = pCVar2->field_0x1c;
							iVar9 = iVar10;
							if (iVar10 == -1) {
								iVar9 = 0;
							}
							this->ppGlobalDlist[iVar9].pDlistPatch->Init();
						}
						iVar10 = iVar10 + 1;
					} while (iVar10 < this->dlistCount);
				}
			}
		}
		/* ----------------------------- */
		edDebugPrintf("-----------------------------");
	}
	this->bCompletedLevelInit = 1;
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
	}
	return;
}

void CGlobalDListManager::Level_Manage()
{
	CGlobalDListPatch* pCVar1;
	int* piVar3;
	uint uVar4;
	uint uVar5;
	int iVar6;
	CGlobalDListPatch* pCVar7;
	int iVar8;
	int iVar9;
	//ParticleData_b8* piVar2;

	if (this->dlistCount != 0) {
		if (this->field_0x1c == 1) {
			pCVar1 = this->ppGlobalDlist->pDlistPatch;
			if ((pCVar1 != (CGlobalDListPatch*)0x0) && (iVar6 = pCVar1->brightEyeCount, iVar6 != 0)) {
				if (pCVar1->bEnabled != 0) {
					pCVar1->field_0x18 = 1;
					edDListSetCurrent(pCVar1->pDisplayListInternal);
				}
				iVar9 = 0;
				pCVar7 = pCVar1;
				if (0 < iVar6) {
					do {
						if (pCVar7->pBrightEye[iVar9] != (S_EYES_BRIGHT_SHADOW*)0x0) {
							pCVar7->pBrightEye[iVar9]->pObject->InitDlistPatchable();
						}
						iVar9 = iVar9 + 1;
					} while (iVar9 < iVar6);
				}
				if (pCVar1->bEnabled != 0) {
					pCVar1->field_0x18 = 0;
				}
			}
			this->field_0x1c = 0;
		}
		if (((1 < this->dlistCount) && (this->field_0x18 == 1)) && (iVar6 = this->field_0x14, iVar6 != -1)) {
			pCVar1 = this->ppGlobalDlist[iVar6].pDlistPatch;
			if ((pCVar1 != (CGlobalDListPatch*)0x0) && (iVar9 = pCVar1->brightEyeCount, iVar9 != 0)) {
				if (pCVar1->bEnabled != 0) {
					pCVar1->field_0x18 = 1;
					edDListSetCurrent(pCVar1->pDisplayListInternal);
				}
				iVar8 = 0;
				if (0 < iVar9) {
					iVar6 = iVar6 << 0x10;
					pCVar7 = pCVar1;
					do {
						IMPLEMENTATION_GUARD(
						if (pCVar7->pBrightEye[0] != (char*)0x0) {
							piVar3 = *(int**)(pCVar7->pBrightEye[0] + 4);
							(**(code**)(*piVar3 + 0x10))(piVar3, iVar6);
						}
						iVar8 = iVar8 + 1;
						pCVar7 = (CGlobalDListPatch*)&(pCVar7->base).pDisplayListInternal;
						iVar6 = iVar6 + 1;)
					} while (iVar8 < iVar9);
				}
				if (pCVar1->bEnabled != 0) {
					pCVar1->field_0x18 = 0;
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
	int iVar1;
	CGlobalDListPatch* pCVar2;

	if (this->dlistCount != 0) {
		pCVar2 = this->ppGlobalDlist->pDlistPatch;
		iVar1 = pCVar2->bEnabled;
		if ((iVar1 != 0) && (iVar1 != 0)) {
			edDlistAddtoView(pCVar2->pDisplayListInternal);
		}
		if (1 < this->dlistCount) {
			pCVar2 = (CGlobalDListPatch*)0x0;
			if (this->field_0x14 != -1) {
				pCVar2 = this->ppGlobalDlist[this->field_0x14].pDlistPatch;
			}
			if ((((pCVar2 != (CGlobalDListPatch*)0x0) && (iVar1 = pCVar2->bEnabled, iVar1 != 0)) &&
				(this->field_0x18 != 2)) && (iVar1 != 0)) {
				edDlistAddtoView(pCVar2->pDisplayListInternal);
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
	int* piVar11;
	uint uVar12;
	int local_80[29];
	undefined4 local_c[3];

	iVar7 = 0;
	this->dlistCount = -1;
	this->bBeganLevelInit = 0;
	this->pDisplayList = (DisplayListInternal*)0x0;
	this->ppGlobalDlist = (GlobalDlistEntry*)0x0;
	this->field_0x14 = -1;
	this->field_0x18 = 0;
	this->field_0x1c = 0;
	//this->field_0x20 = -1;
	this->bCompletedLevelInit = 0;
	this->field_0x12e8 = 0;
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
	puVar4 = local_c;
	this->dlistCount = (CScene::ptable.g_SectorManager_00451670)->count_0x368 + 1;
	//do {
	//	*puVar4 = 0;
	//	puVar4[-1] = 0;
	//	iVar7 = iVar7 + -6;
	//	puVar4[-2] = 0;
	//	puVar4[-3] = 0;
	//	puVar4[-4] = 0;
	//	puVar4[-5] = 0;
	//	puVar4 = puVar4 + -6;
	//} while (-1 < iVar7);
	iVar7 = (CScene::ptable.g_ActorManager_004516a4)->actorCount_0x58;
	iVar10 = 0;
	if (0 < iVar7) {
		IMPLEMENTATION_GUARD(
		iVar9 = 0;
		do {
			iVar2 = 0;
			if (iVar10 != -1) {
				iVar2 = *(int*)((int)(CScene::ptable.g_ActorManager_004516a4)->aActors + iVar9);
			}
			iVar2 = *(int*)(iVar2 + 4);
			if (iVar2 < 0) {
				iVar2 = 0;
			}
			iVar10 = iVar10 + 1;
			iVar9 = iVar9 + 4;
			local_80[iVar2] = local_80[iVar2] + 1;
		} while (iVar10 < iVar7);)
	}
	if (this->dlistCount != 0) {
		pGVar1 = new GlobalDlistEntry[this->dlistCount];
		this->ppGlobalDlist = pGVar1;
		uVar12 = 0;
		if (0 < this->dlistCount) {
			piVar11 = local_80;
			do {
				if ((uVar12 < 2) || (0 < *piVar11)) {
					ppGlobalDlist[uVar12].pDlistPatch = new CGlobalDListPatch(uVar12);
				}
				else {
					ppGlobalDlist[uVar12].pDlistPatch = NULL;
				}

				ppGlobalDlist[uVar12].field_0x4 = 0;
				ppGlobalDlist[uVar12].field_0xc = 0;
				ppGlobalDlist[uVar12].field_0x8 = 0;

				uVar12 = uVar12 + 1;
				piVar11 = piVar11 + 1;
			} while ((int)uVar12 < this->dlistCount);
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
			if ((pGVar1->field_0x8 == 0) ||
				((pGVar1->field_0x4 == 0 && (pGVar1[newSectorId].field_0xc == 0)))) {
				pCVar6->bEnabled = 0;
			}
			else {
				iVar2 = this->field_0x14;
				pCVar3 = this->ppGlobalDlist[iVar2].pDlistPatch;
				if (pCVar3 != (CGlobalDListPatch*)0x0) {
					uVar4 = pGVar1->field_0x8;
					uVar5 = pGVar1->field_0x4;
					if ((uVar4 != 0) && ((pGVar1->field_0xc != 0 || (uVar5 != 0)))) {
						IMPLEMENTATION_GUARD(
						edDListPatchableReset(pCVar3->pDisplayListInternal, uVar4, pGVar1[iVar2].field_0xc, uVar5);)
					}
				}
				pCVar6->bEnabled = 1;
			}
		}
	}
	return;
}

void CGlobalDListManager::_ExecuteCallFunc()
{
	char cVar1;
	ushort uVar2;
	GlobalDlistEntry* pGVar3;
	char* pcVar4;
	bool bVar5;
	DisplayListInternalSubObj_60* pDVar6;
	CGlobalDListPatch* pCVar7;
	uint uVar8;
	//ManagerFunctionData** ppMVar9;
	int iVar10;
	undefined4* puVar11;
	undefined4* puVar12;
	CGlobalDListPatch* pCVar13;
	int iVar14;
	uint* puVar15;
	CGlobalDListManager* pCVar16;
	//ManagerFunctionData** ppMVar17;
	int iVar18;
	undefined4 uVar19;
	undefined4 uVar20;
	uint local_c;

	iVar18 = 0;
	pCVar16 = this;
	if (0 < this->field_0x12e8) {
		IMPLEMENTATION_GUARD(
		do {
			pGVar3 = this->ppGlobalDlist;
			ppMVar17 = (ManagerFunctionData**)&pCVar16->field_0x28;
			uVar8 = *(int*)&pCVar16->field_0x28 >> 0x10;
			pCVar7 = pGVar3[uVar8].pDlistPatch;
			bVar5 = false;
			if (uVar8 != 0) {
				if (this->field_0x14 == uVar8) {
					iVar10 = this->field_0x14;
					pCVar13 = (CGlobalDListPatch*)0x0;
					if (iVar10 != -1) {
						pCVar13 = pGVar3[iVar10].pDlistPatch;
					}
					if (pCVar13 == (CGlobalDListPatch*)0x0) {
						*(undefined*)&pCVar16->field_0x31 = 0;
						bVar5 = true;
					}
					else {
						bVar5 = true;
						if (this->field_0x18 != 1) {
							if (this->field_0x14 != 0xffffffff) {
								if (iVar10 == -1) {
									pCVar13 = (CGlobalDListPatch*)0x0;
								}
								else {
									pCVar13 = pGVar3[iVar10].pDlistPatch;
								}
								bVar5 = false;
								if (uVar8 == pCVar13->field_0x4dc) goto LAB_002d73f0;
							}
							*(undefined*)&pCVar16->field_0x31 = 0;
							bVar5 = true;
						}
					}
				}
				else {
					*(undefined*)&pCVar16->field_0x31 = 0;
					bVar5 = true;
				}
			}
		LAB_002d73f0:
			if (!bVar5) {
				cVar1 = *(char*)&pCVar16->field_0x30;
				if (cVar1 == '\x01') {
					uVar2 = *(ushort*)ppMVar17;
					if ((pCVar7->base).bEnabled != 0) {
						(pCVar7->base).field_0x18 = 1;
						edDListSetCurrent((pCVar7->base).pDisplayListInternal);
						pCVar7->field_0x4d0 = (pCVar7->pBrightEye + (uVar2 - 8))[8];
						iVar10 = pCVar7->field_0x4d0;
						pcVar4 = (pCVar7->pBrightEye + (uVar2 - 8))[8];
						pDVar6 = edDListPatchableInfo
						((long)(iVar10 + 0x10), (long)(iVar10 + 0x14), (long)(iVar10 + 0x18), (long)(iVar10 + 0x1c),
							*(int*)(pcVar4 + 8) + *(int*)(pcVar4 + 0xc), (ulong)uVar2);
						*(DisplayListInternalSubObj_60**)(iVar10 + 0x20) = pDVar6;
						pCVar7->field_0x4d8 = 1;
					}
					iVar10 = *(int*)&pCVar16->field_0x2c;
					uVar8 = (uint) * (ushort*)ppMVar17;
					if (iVar10 == 0) {
						*(uint*)pCVar7->pBrightEye[uVar8] = *(uint*)pCVar7->pBrightEye[uVar8] & 0xfffffffe;
					}
					else {
						*(uint*)pCVar7->pBrightEye[uVar8] = *(uint*)pCVar7->pBrightEye[uVar8] | 1;
					}
					edDListPatchableShowProp(uVar8, (uchar)iVar10);
					edDListPatcheEnd(-1, 0);
					if ((pCVar7->base).bEnabled != 0) {
						(pCVar7->base).field_0x18 = 0;
					}
					pCVar7->field_0x4d8 = 0;
					pCVar7->field_0x4d0 = 0;
				}
				else {
					if (cVar1 == '\x03') {
						uVar2 = *(ushort*)ppMVar17;
						if ((pCVar7->base).bEnabled != 0) {
							(pCVar7->base).field_0x18 = 1;
							edDListSetCurrent((pCVar7->base).pDisplayListInternal);
							pCVar7->field_0x4d0 = (pCVar7->pBrightEye + (uVar2 - 8))[8];
							iVar10 = pCVar7->field_0x4d0;
							pcVar4 = (pCVar7->pBrightEye + (uVar2 - 8))[8];
							pDVar6 = edDListPatchableInfo
							((long)(iVar10 + 0x10), (long)(iVar10 + 0x14), (long)(iVar10 + 0x18),
								(long)(iVar10 + 0x1c), *(int*)(pcVar4 + 8) + *(int*)(pcVar4 + 0xc), (ulong)uVar2);
							*(DisplayListInternalSubObj_60**)(iVar10 + 0x20) = pDVar6;
							pCVar7->field_0x4d8 = 1;
						}
						iVar10 = *(int*)(pCVar7->field_0x4d0 + 0x14);
						iVar14 = *(int*)&pCVar16->field_0x2c * 0x10;
						puVar15 = (uint*)(iVar10 + iVar14);
						gpCurPatchRGBABuf = 0;
						local_c = *(uint*)(iVar10 + iVar14) & 0xffffff;
						*puVar15 = local_c;
						puVar15[1] = local_c;
						puVar15[2] = local_c;
						puVar15[3] = local_c;
						edDListPatcheEnd(-1, 0);
						if ((pCVar7->base).bEnabled != 0) {
							(pCVar7->base).field_0x18 = 0;
						}
						pCVar7->field_0x4d8 = 0;
						pCVar7->field_0x4d0 = 0;
					}
					else {
						if (cVar1 == '\x02') {
							uVar2 = *(ushort*)ppMVar17;
							if ((pCVar7->base).bEnabled != 0) {
								(pCVar7->base).field_0x18 = 1;
								edDListSetCurrent((pCVar7->base).pDisplayListInternal);
								pCVar7->field_0x4d0 = (pCVar7->pBrightEye + (uVar2 - 8))[8];
								iVar10 = pCVar7->field_0x4d0;
								pcVar4 = (pCVar7->pBrightEye + (uVar2 - 8))[8];
								pDVar6 = edDListPatchableInfo
								((long)(iVar10 + 0x10), (long)(iVar10 + 0x14), (long)(iVar10 + 0x18),
									(long)(iVar10 + 0x1c), *(int*)(pcVar4 + 8) + *(int*)(pcVar4 + 0xc), (ulong)uVar2);
								*(DisplayListInternalSubObj_60**)(iVar10 + 0x20) = pDVar6;
								pCVar7->field_0x4d8 = 1;
							}
							uVar8 = *(uint*)&pCVar16->field_0x2c;
							iVar14 = uVar8 * 0x10;
							puVar11 = (undefined4*)(*(int*)(pCVar7->field_0x4d0 + 0x10) + iVar14);
							uVar19 = puVar11[1];
							uVar20 = puVar11[2];
							iVar10 = *(int*)(pCVar7->field_0x4d0 + 0x10);
							if (gCurStripPatchable == 0) {
								puVar12 = (undefined4*)(iVar10 + iVar14);
								*puVar12 = *puVar11;
								puVar12[1] = uVar19;
								puVar12[2] = uVar20;
								puVar12[3] = 0xc000;
							}
							else {
								puVar12 = (undefined4*)(iVar10 + iVar14);
								*puVar12 = *puVar11;
								puVar12[1] = uVar19;
								puVar12[2] = uVar20;
								if (gCurDListInfo3DPatchable->field_0x4c != 3) {
									puVar12[3] = 0xc000;
								}
								uVar8 = uVar8 / 0x46;
								BYTE_ARRAY_0048d3a0[uVar8] = BYTE_ARRAY_0048d3a0[uVar8] + 1;
								if ((int)DAT_00448a84 < (int)uVar8) {
									DAT_00448a84 = uVar8;
								}
							}
							DAT_0044970c = 0;
							edDListPatcheEnd(-1, 0);
							if ((pCVar7->base).bEnabled != 0) {
								(pCVar7->base).field_0x18 = 0;
							}
							pCVar7->field_0x4d8 = 0;
							pCVar7->field_0x4d0 = 0;
						}
						else {
							if (cVar1 == '\0') {
								if ((pCVar7->base).bEnabled != 0) {
									(pCVar7->base).field_0x18 = 1;
									edDListSetCurrent((pCVar7->base).pDisplayListInternal);
								}
								(**(code**)(**(int**)&pCVar16->field_0x2c + 0x10))(*(int**)&pCVar16->field_0x2c, *ppMVar17);
								if ((pCVar7->base).bEnabled != 0) {
									(pCVar7->base).field_0x18 = 0;
								}
							}
						}
					}
				}
				*(char*)&pCVar16->field_0x31 = *(char*)&pCVar16->field_0x31 + -1;
			}
			if (*(char*)&pCVar16->field_0x31 == '\0') {
				this->field_0x12e8 = this->field_0x12e8 + -1;
				if (this->field_0x12e8 != 0) {
					ppMVar9 = &this->pManagerFunctionData + this->field_0x12e8 * 3;
					*ppMVar17 = ppMVar9[10];
					*(ManagerFunctionData**)&pCVar16->field_0x2c = ppMVar9[0xb];
					*(undefined*)&pCVar16->field_0x30 = *(undefined*)(ppMVar9 + 0xc);
					*(undefined*)&pCVar16->field_0x31 = *(undefined*)((int)ppMVar9 + 0x31);
				}
			}
			else {
				pCVar16 = (CGlobalDListManager*)&pCVar16->pDisplayList;
				iVar18 = iVar18 + 1;
			}
		} while (iVar18 < this->field_0x12e8);
		)
	}
	iVar18 = 0;
	pCVar16 = this;
	if (0 < this->field_0x29b0) {
		IMPLEMENTATION_GUARD(
		do {
			iVar10 = *(int*)&pCVar16->field_0x25b0 >> 0x10;
			bVar5 = true;
			if ((iVar10 == this->field_0x14) || (iVar10 == 0)) {
				pCVar7 = BeginCurrent(this, (long)*(int*)&pCVar16->field_0x25b0);
				if (pCVar7 != (CGlobalDListPatch*)0x0) {
					_ExecuteCallFunc_BeginDList(this, (int)(ManagerFunctionData**)&pCVar16->field_0x25b0);
					pCVar7 = this->ppGlobalDlist[this->field_0x20 >> 0x10].pDlistPatch;
					edDListPatcheEnd(-1, 0);
					if ((pCVar7->base).bEnabled != 0) {
						(pCVar7->base).field_0x18 = 0;
					}
					pCVar7->field_0x4d8 = 0;
					pCVar7->field_0x4d0 = 0;
					this->field_0x20 = -1;
				}
			}
			else {
				bVar5 = false;
			}
			if ((*(int*)&pCVar16->field_0x25b4 < 1) || (!bVar5)) {
				this->field_0x29b0 = this->field_0x29b0 + -1;
				iVar10 = this->field_0x29b0;
				*(ManagerFunctionData**)&pCVar16->field_0x25b0 = (&this->pManagerFunctionData + iVar10 * 2)[0x96c];
				*(ManagerFunctionData**)&pCVar16->field_0x25b4 = (&this->pManagerFunctionData + iVar10 * 2)[0x96d];
			}
			else {
				pCVar16 = (CGlobalDListManager*)&pCVar16->dlistCount;
				iVar18 = iVar18 + 1;
			}
		} while (iVar18 < this->field_0x29b0);)
	}
	return;
}

bool CGlobalDListManager::SetActive(int param_2, int param_3)
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
		if ((this->field_0x14 == param_2 >> 0x10) || (param_2 >> 0x10 == 0)) {
			IMPLEMENTATION_GUARD(
			uVar1 = _AddCallFuncElement(this, param_2, 1, param_3);)
		}
		else {
			uVar1 = false;
		}
	}
	return uVar1;
}
