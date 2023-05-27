#include "DisplayList.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif
#include "../edDlist.h"
#include <assert.h>
#include "../LargeObject.h"
#include "../FrontendManager.h"

GlobalDList* g_GameDisplayListPtr_0044971c = NULL;
GlobalDList* g_FrontendDisplayListPtr_00449720 = NULL;
GlobalDList* pGuiDList = NULL;

void GuiDList_EndCurrent(void)
{
	if (pGuiDList->bEnabled != 0) {
		pGuiDList->field_0x18 = 0;
	}
	return;
}

char* g_NewLine = "\n";
char* sz_DisplayListSpacer_00433970 = "---------------------------------------- - \n";

void SetStaticMeshMaster_002cb380(DisplayListInternal* pDisplayListInternalArray, ed_3D_Scene* pStaticMeshMaster)
{
	uint displayListIndex;
	uint uVar1;

	displayListIndex = 0;
	do {
		uVar1 = displayListIndex + 1;
		pDisplayListInternalArray[displayListIndex].pStaticMeshMaster_0x20 = pStaticMeshMaster;
		displayListIndex = uVar1;
	} while (uVar1 < 2);
	return;
}

uint CalcDisplayListMemory_002cb020(uint param_1, int param_2, int param_3, int param_4, uint* param_5, uint* param_6)
{
	int iVar1;
	uint uVar2;
	int iVar3;
	int iVar4;
	uint uVar5;

	iVar4 = param_4 + param_3 * 4;
	if (param_2 == 0) {
		//trap(7);
	}
	if (false) {
		//trap(7);
	}
	iVar1 = ((iVar4 / param_2) / 0x46 + 1) * param_2;
	if ((param_1 & 2) == 0) {
		if ((param_1 & 1) == 0) {
			return 0;
		}
		iVar3 = param_2 * 0xc0;
		iVar4 = iVar4 * 0x20 + param_2 * 0x90 + 0x640 + iVar1 * 0xc0;
		if ((param_1 & 0x100) != 0) {
			iVar4 = iVar4 + iVar1 * 0x10;
		}
		if ((param_1 & 0x100) != 0) {
			iVar4 = iVar4 + iVar1 * 0x10;
		}
		uVar5 = (iVar4 + 0x40) * 2;
	}
	else {
		uVar5 = iVar4 * 0x40;
		iVar3 = param_2 << 5;
	}
	if (false) {
		iVar3 = iVar3 + 0x10;
	}
	if (false) {
		uVar5 = uVar5 + 0x10;
	}
	uVar2 = uVar5 + iVar3 + 0x60;
	if (false) {
		uVar2 = uVar5 + iVar3 + 0x70;
	}
	if ((param_1 & 0x100) != 0) {
		uVar2 = uVar2 + param_2 * 8;
	}
	if (param_5 != (uint*)0x0) {
		*param_5 = iVar3;
	}
	if (param_6 != (uint*)0x0) {
		*param_6 = uVar5;
	}
	return uVar2;
}

void Setup_002cad90
(DisplayListInternal* pDisplayListInternal, char* param_2, uint param_3, ushort inFlags,
	char* pEndInternalData, uint param_6, undefined4 param_7,
	ed_3D_Scene* pStaticMeshMaster)

{
	int iVar1;
	DisplayListInternal* pDVar2;
	uint uVar3;
	uint uVar4;

	uVar3 = 0;
	do {
		pDVar2 = pDisplayListInternal + uVar3;
		pDVar2->flags_0x0 = inFlags;
		pDVar2->subCommandBufferCount = 0;
		pDVar2->field_0x8 = param_7;
		pDVar2->pStaticMeshMaster_0x20 = pStaticMeshMaster;
		pDVar2->pDisplayListInternalSubObj =
			(DisplayListInternalSubObj_60*)(pEndInternalData + (param_6 >> 1) * uVar3);
		uVar4 = (uint)pDVar2->pDisplayListInternalSubObj & 0xf;
		if (uVar4 != 0) {
			pDVar2->pDisplayListInternalSubObj =
				(DisplayListInternalSubObj_60*)
				((int)pDVar2->pDisplayListInternalSubObj + (0x10 - uVar4));
		}
		if ((inFlags & 2) == 0) {
			if ((inFlags & 1) != 0) {
				iVar1 = (param_3 - (int)(param_2 + 0x640 + -(int)param_2) >> 1) * uVar3;
				pDVar2->field_0x14 = (edpkt_data*)(param_2 + uVar3 * 800);
				pDVar2->field_0xc = param_2 + 0x640 + iVar1;
				pDVar2->field_0x10 = (MeshDrawRenderCommand*)pDVar2->field_0xc;
				pDVar2->field_0x28 = pDVar2->field_0xc + iVar1;
				pDVar2->field_0x14 = (edpkt_data*)pDVar2->field_0xc;
				uVar4 = (uint)pDVar2->field_0xc & 0xf;
				if (uVar4 != 0) {
					pDVar2->field_0xc = pDVar2->field_0xc + (0x10 - uVar4);
				}
				uVar4 = (uint)pDVar2->field_0x10 & 0xf;
				if (uVar4 != 0) {
					pDVar2->field_0x10 = pDVar2->field_0x10 + (0x10 - uVar4);
				}
			}
		}
		else {
			pDVar2->field_0x14 = (edpkt_data*)(param_2 + (param_3 >> 1) * uVar3);
		}
		uVar4 = (uint)pDVar2->field_0x14 & 0xf;
		if (uVar4 != 0) {
			pDVar2->field_0x14 = (edpkt_data*)((int)pDVar2->field_0x14 + (0x10 - uVar4));
		}
		uVar3 = uVar3 + 1;
		pDVar2->pRenderCommands = pDVar2->field_0x14;
	} while (uVar3 < 2);
	return;
}

DisplayListInternal* AllocateDisplayListMemory_002caee0(EHeap heapID, uint inFlags, int param_3, int param_4, int param_5, undefined8 param_6, DisplayListInternal* pInBuffer)
{
	int iVar1;
	uint displayListMemSize;
	uint uVar2;
	DisplayListInternal* pDVar3;
	uint uVar4;
	char* pcVar5;
	uint local_8;
	uint local_4;

	displayListMemSize = CalcDisplayListMemory_002cb020(inFlags, param_3, param_4, param_5, &local_8, &local_4);
	if (pInBuffer == (DisplayListInternal*)0x0) {
		pInBuffer = (DisplayListInternal*)edMemAlloc(heapID, displayListMemSize);
	}
	memset(pInBuffer, 0, displayListMemSize);
	pcVar5 = (char*)((char*)&pInBuffer[2].flags_0x0 + local_8);
	Setup_002cad90(pInBuffer, pcVar5, local_4, (ushort)inFlags, (char*)(pInBuffer + 2), local_8, param_3, (ed_3D_Scene*)1);
	uVar4 = 0;
	if ((inFlags & 0x100) == 0) {
		do {
			uVar2 = uVar4 + 1;
			pInBuffer[uVar4].field_0x2c = (char*)0x0;
			pInBuffer[uVar4].field_0x6 = 0;
			uVar4 = uVar2;
		} while (uVar2 < 2);
	}
	else {
		uVar4 = 0;
		do {
			pDVar3 = pInBuffer + uVar4;
			iVar1 = uVar4 * param_3 * 4;
			uVar4 = uVar4 + 1;
			pDVar3->field_0x2c = pcVar5 + iVar1 + local_4;
			pDVar3->field_0x6 = 0;
		} while (uVar4 < 2);
	}
	return pInBuffer;
}

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
	if (2.147484e+09 < fVar7 * fVar6) {
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

void GlobalDList_Init(void)
{
	ed_3D_Scene* pSVar1;
	int freeMemCalcA;
	GlobalDList* pDVar2;
	DisplayListInternal* pDVar3;
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
	pDVar2 = new GlobalDList(0x200, 0x1000, 0, 0x11, Scene::_scene_handleA);
	g_GameDisplayListPtr_0044971c = pDVar2;
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));

	/* - Game list          : %07d\n */
	edDebugPrintf("- Game list          : %07d\n", freeMemCalcA - iVar5);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	pDVar2 = new GlobalDList(0x20, 0x80, 0, 0x11, Frontend::_scene_handle);
	g_FrontendDisplayListPtr_00449720 = pDVar2;
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));

	/* - Frontend 3D list   : %07d\n */
	edDebugPrintf("- Frontend 3D list   : %07d\n", iVar4 - iVar5);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	pDVar2 = new GlobalDList(0x180, 0xe74, 0, 0x12, pSVar1);
	pGuiDList = pDVar2;
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
	pcVar7 = WorkOutMo(fVar8 / 1048576.0, 3, g_DebugTextBuffer_00468ef0, 0, (char*)0x0);
	/* --- total used : \t%d (%s Mo) ----
	   ----------------------------------------- */
	edDebugPrintf("--- total used : \t%d (%s Mo) ----\n", freeMemCalcA - iVar4, pcVar7);
	edDebugPrintf(sz_DisplayListSpacer_00433970);
	edDebugPrintf(g_NewLine);
	g_GameDisplayListPtr_0044971c->Init();
	g_FrontendDisplayListPtr_00449720->Init();
	if (pGuiDList != (GlobalDList*)0x0) {
		pGuiDList->Init();
	}
	edDlist::edDListSetActiveViewPort(Scene::_pinstance->pCameraObj28_0x4);
	return;
}

DisplayListInternal* gCurDListHandle = NULL;
MeshDrawRenderCommand* gCurDListBuf = NULL;

DisplayListInternal*
edDListSetCurrent(DisplayListInternal* pNewDisplayList)
{
	DisplayListInternal* pDVar1;
	DisplayListInternal* pPreviousDList;

	pPreviousDList = gCurDListHandle;
	//DAT_00448a80 = 0xffffffff;
	//DAT_004496c4 = 0;
	//DAT_004496cc = 0;
	//g_CharacterIndex_00449614 = 0;
	pDVar1 = pNewDisplayList + gCurRenderState;
	//memset(BYTE_ARRAY_0048d3a0, 0, 0x100);
	//DAT_00448a84 = 0xffffffff;
	if ((pDVar1->flags_0x0 & 4) != 0) {
		pDVar1 = pNewDisplayList;
	}
	gCurDListHandle = pNewDisplayList;
	if (gCurDList != (DisplayListInternal*)0x0) {
		gCurDList->field_0x10 = gCurDListBuf;
	}
	if ((pDVar1->flags_0x0 & 1) != 0) {
		gCurDListBuf = pDVar1->field_0x10;
		if (((uint)gCurDListBuf & 0xf) != 0) {
			gCurDListBuf =
				(MeshDrawRenderCommand*)
				((int)gCurDListBuf + (0x10 - ((uint)gCurDListBuf & 0xf)));
		}
	}
	gNbStateAdded = 0;
	gCurDList = pDVar1;
	//PTR_DAT_00449600 =
	//	(undefined*)
	//	((int)pDVar1->field_0x14 +
	//		(int)((int)pNewDisplayList[1].field_0x14 - (int)pNewDisplayList->field_0x14));
	gCurMaterial = (edDList_material*)0x0;
	gCurStatePKT = (edpkt_data*)0x0;
	gCurStatePKTSize = 0;
	//DAT_0044969c = 0;
	return pPreviousDList;
}

bool GuiDList_BeginCurrent(void)
{
	bool bVar1;
	GlobalDList* pDVar2;

	pDVar2 = pGuiDList;
	bVar1 = pGuiDList->bEnabled != 0;
	if (bVar1) {
		pGuiDList->field_0x18 = 1;
		edDListSetCurrent(pDVar2->pDisplayListInternal);
	}
	return bVar1;
}

void GlobalDList_AddToView(void)
{
	if (g_GameDisplayListPtr_0044971c->bEnabled != 0) {
		edDlist::edDlistAddtoView(g_GameDisplayListPtr_0044971c->pDisplayListInternal);
	}
	if (g_FrontendDisplayListPtr_00449720->bEnabled != 0) {
		edDlist::edDlistAddtoView(g_FrontendDisplayListPtr_00449720->pDisplayListInternal);
	}
	if ((pGuiDList != (GlobalDList*)0x0) && (pGuiDList->bEnabled != 0)) {
		edDlist::edDlistAddtoView(pGuiDList->pDisplayListInternal);
	}
	return;
}

GlobalDList::GlobalDList()
{
	field_0x8 = -1;
	field_0xc = -1;
	field_0x10 = -1;
	pDisplayListInternal = (DisplayListInternal*)0x0;
	bEnabled = 0;
}

GlobalDList::GlobalDList(int inField_0x8, int inField_0xc, int inField_0x10, int inField_0x1c, ed_3D_Scene* pInStaticMeshMaster)
{
    DisplayListInternal* pDVar3;

	field_0x8 = inField_0x8;
	field_0xc = inField_0xc;
	field_0x10 = inField_0x10;
	field_0x1c = inField_0x1c;
	pDVar3 = AllocateDisplayListMemory_002caee0(TO_HEAP(H_MAIN), field_0x1c, field_0x8, 0, field_0xc, 0, (DisplayListInternal*)0x0);
	pDisplayListInternal = pDVar3;
	SetStaticMeshMaster_002cb380(pDisplayListInternal, pInStaticMeshMaster);
}

void GlobalDList::Init()
{
	bEnabled = 1;
	field_0x18 = 0;
}

void ed_3D_Scene::RemoveFlag_002a53e0(uint flag)
{
	this->flags_0x4 = this->flags_0x4 & ~flag;
	return;
}

void ed_3D_Scene::ed3DSceneSetFlag(uint flag)
{
	this->flags_0x4 = this->flags_0x4 | flag;
}

void ed_3D_Scene::ed3DSceneSetFogProperty(bool bValue)
{
	if (bValue == false) {
		this->flags_0x4 = this->flags_0x4 & 0xffffffbf;
	}
	else {
		this->flags_0x4 = this->flags_0x4 | 0x40;
	}
	return;
}

void ed_3D_Scene::SetFlag_002a5440(bool bValue)
{
	if (bValue == false) {
		this->flags_0x4 = this->flags_0x4 & 0xffffff7f;
	}
	else {
		this->flags_0x4 = this->flags_0x4 | 0x80;
	}
	return;
}
