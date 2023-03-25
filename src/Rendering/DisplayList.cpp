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

DisplayList* g_GuiDisplayListPtr_00449724;

void DisplayListFunc_002d6340(void)
{
	if (g_GuiDisplayListPtr_00449724->bEnabled != 0) {
		g_GuiDisplayListPtr_00449724->field_0x18 = 0;
	}
	return;
}

char* g_NewLine = "\n";
char* sz_DisplayListSpacer_00433970 = "---------------------------------------- - \n";

void SetStaticMeshMaster_002cb380(DisplayListInternal* pDisplayListInternalArray, StaticMeshMaster* pStaticMeshMaster)
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
	StaticMeshMaster* pStaticMeshMaster)

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
				pDVar2->field_0x14 = (RenderCommand*)(param_2 + uVar3 * 800);
				pDVar2->field_0xc = param_2 + 0x640 + iVar1;
				pDVar2->field_0x10 = (MeshDrawRenderCommand*)pDVar2->field_0xc;
				pDVar2->field_0x28 = pDVar2->field_0xc + iVar1;
				pDVar2->field_0x14 = (RenderCommand*)pDVar2->field_0xc;
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
			pDVar2->field_0x14 = (RenderCommand*)(param_2 + (param_3 >> 1) * uVar3);
		}
		uVar4 = (uint)pDVar2->field_0x14 & 0xf;
		if (uVar4 != 0) {
			pDVar2->field_0x14 = (RenderCommand*)((int)pDVar2->field_0x14 + (0x10 - uVar4));
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
	Setup_002cad90(pInBuffer, pcVar5, local_4, (ushort)inFlags, (char*)(pInBuffer + 2), local_8, param_3, (StaticMeshMaster*)1);
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

void SetupDisplayLists(void)
{
	StaticMeshMaster* pSVar1;
	int freeMemCalcA;
	DisplayList* pDVar2;
	DisplayListInternal* pDVar3;
	int iVar4;
	int iVar5;
	uint uVar6;
	char* pcVar7;
	float fVar8;

	/* -----------------------------------------
	   ----- Memory used by display list ------- */
	PrintString(g_NewLine);
	PrintString(sz_DisplayListSpacer_00433970);
	PrintString("----- Memory used by display list -------\n");
	PrintString(g_NewLine);
	//freeMemCalcA = GetCurrentFreeMemory(1);
	//pDVar2 = (DisplayList*)Allocate(0x20);
	pSVar1 = NULL; // g_StaticMeshMasterA_00448808;
	//if (pDVar2 != (DisplayList*)0x0) {
	//    pDVar2->pVTable = &DisplayListHeader::VTable_004406d0;
	//    pDVar2->field_0x8 = -1;
	//    pDVar2->field_0xc = -1;
	//    pDVar2->field_0x10 = -1;
	//    pDVar2->pDisplayListInternal = (DisplayListInternal*)0x0;
	//    pDVar2->bEnabled = 0;
	//    pDVar2->field_0x8 = 0x200;
	//    pDVar2->field_0xc = 0x1000;
	//    pDVar2->field_0x10 = 0;
	//    pDVar2->field_0x1c = 0x11;
	//    pDVar3 = (DisplayListInternal*)AllocateDisplayListMemory_002caee0(TO_HEAP(H_MAIN), 0x11, 0x200, 0, 0x1000, 0, (char*)0x0);
	//    pDVar2->pDisplayListInternal = pDVar3;
	//    DisplayListInternal::SetStaticMeshMaster_002cb380(pDVar2->pDisplayListInternal, pSVar1);
	//}
	//g_GameDisplayListPtr_0044971c = pDVar2;
	//GetCurrentFreeMemory(1);
	///* - Game list          : %07d\n */
	//PrintString(s_ - _Game_list_:_ % 07d_004339d0);
	//GetCurrentFreeMemory(1);
	//pDVar2 = (DisplayList*)Allocate(0x20);
	//pSVar1 = PTR_StaticMeshMaster_ARRAY_004d7d60[12].field_0x0_00448818;
	//if (pDVar2 != (DisplayList*)0x0) {
	//    pDVar2->pVTable = &DisplayListHeader::VTable_004406d0;
	//    pDVar2->field_0x8 = -1;
	//    pDVar2->field_0xc = -1;
	//    pDVar2->field_0x10 = -1;
	//    pDVar2->pDisplayListInternal = (DisplayListInternal*)0x0;
	//    pDVar2->bEnabled = 0;
	//    pDVar2->field_0x8 = 0x20;
	//    pDVar2->field_0xc = 0x80;
	//    pDVar2->field_0x10 = 0;
	//    pDVar2->field_0x1c = 0x11;
	//    pDVar3 = (DisplayListInternal*)AllocateDisplayListMemory_002caee0(TO_HEAP(H_MAIN), 0x11, 0x20, 0, 0x80, 0, (char*)0x0);
	//    pDVar2->pDisplayListInternal = pDVar3;
	//    DisplayListInternal::SetStaticMeshMaster_002cb380(pDVar2->pDisplayListInternal, pSVar1);
	//}
	//g_FrontendDisplayListPtr_00449720 = pDVar2;
	//GetCurrentFreeMemory(1);
	///* - Frontend 3D list   : %07d\n */
	//PrintString(s_ - _Frontend_3D_list_:_ % 07d_004339f0);
	//iVar4 = GetCurrentFreeMemory(1);
	pDVar2 = new DisplayList(0x180, 0xe74, 0, 0x12, pSVar1);
	g_GuiDisplayListPtr_00449724 = pDVar2;
	//iVar5 = GetCurrentFreeMemory(1);
	///* - Gui list           : %07d\n */
	//PrintString(s_ - _Gui_list_:_ % 07d_00433a10, iVar4 - iVar5);
	//PrintString(g_NewLine);
	//iVar4 = GetCurrentFreeMemory(1);
	//iVar5 = GetCurrentFreeMemory(1);
	//uVar6 = freeMemCalcA - iVar5;
	//if ((int)uVar6 < 0) {
	//    fVar8 = (float)(uVar6 >> 1 | uVar6 & 1);
	//    fVar8 = fVar8 + fVar8;
	//}
	//else {
	//    fVar8 = (float)uVar6;
	//}
	//pcVar7 = WorkOutMo(fVar8 / 1048576.0, 3, g_DebugTextBuffer_00468ef0, 0);
	///* --- total used : \t%d (%s Mo) ----
	//   ----------------------------------------- */
	//PrintString(s_-- - _total_used_:_ % d_(% s_Mo)_----_00433a30, freeMemCalcA - iVar4, pcVar7);
	//PrintString(s_--------------------------------_00433970);
	//PrintString(g_NewLine);
	//(*(code*)g_GameDisplayListPtr_0044971c->pVTable->init)();
	//(*(code*)g_FrontendDisplayListPtr_00449720->pVTable->init)();
	if (g_GuiDisplayListPtr_00449724 != (DisplayList*)0x0) {
		g_GuiDisplayListPtr_00449724->Init();
	}
	edDlist::RenderCommand_002ca930(g_LargeObject_006db450->pCameraObj28_0x4);
	return;
}

DisplayListInternal* g_CurrentDisplayListBase_004495dc = NULL;
MeshDrawRenderCommand* PTR_MeshDrawRenderCommand_004495fc = NULL;

DisplayListInternal*
SetActiveDisplayList_002cab60(DisplayListInternal* pNewDisplayList)
{
	DisplayListInternal* pDVar1;
	DisplayListInternal* pPreviousDList;

	pPreviousDList = g_CurrentDisplayListBase_004495dc;
	//DAT_00448a80 = 0xffffffff;
	//DAT_004496c4 = 0;
	//DAT_004496cc = 0;
	//g_CharacterIndex_00449614 = 0;
	pDVar1 = pNewDisplayList + edDlist::g_DisplayListGlobal_00449664;
	//memset(BYTE_ARRAY_0048d3a0, 0, 0x100);
	//DAT_00448a84 = 0xffffffff;
	if ((pDVar1->flags_0x0 & 4) != 0) {
		pDVar1 = pNewDisplayList;
	}
	g_CurrentDisplayListBase_004495dc = pNewDisplayList;
	if (edDlist::g_DisplayListEnd_004495d8 != (DisplayListInternal*)0x0) {
		edDlist::g_DisplayListEnd_004495d8->field_0x10 = PTR_MeshDrawRenderCommand_004495fc;
	}
	if ((pDVar1->flags_0x0 & 1) != 0) {
		PTR_MeshDrawRenderCommand_004495fc = pDVar1->field_0x10;
		if (((uint)PTR_MeshDrawRenderCommand_004495fc & 0xf) != 0) {
			PTR_MeshDrawRenderCommand_004495fc =
				(MeshDrawRenderCommand*)
				((int)PTR_MeshDrawRenderCommand_004495fc + (0x10 - ((uint)PTR_MeshDrawRenderCommand_004495fc & 0xf)));
		}
	}
	edDlist::g_DisplayListCommandCount_004495d4 = 0;
	edDlist::g_DisplayListEnd_004495d8 = pDVar1;
	//PTR_DAT_00449600 =
	//	(undefined*)
	//	((int)pDVar1->field_0x14 +
	//		(int)((int)pNewDisplayList[1].field_0x14 - (int)pNewDisplayList->field_0x14));
	edDlist::g_pMaterialInfo_00449644 = (MaterialInfo*)0x0;
	edDlist::g_CachedRenderCommandPtr_00449650 = (RenderCommand*)0x0;
	edDlist::INT_00449654 = 0;
	//DAT_0044969c = 0;
	return pPreviousDList;
}

bool GuiDisplayListFunc_002d6360(void)
{
	bool bVar1;
	DisplayList* pDVar2;

	pDVar2 = g_GuiDisplayListPtr_00449724;
	bVar1 = g_GuiDisplayListPtr_00449724->bEnabled != 0;
	if (bVar1) {
		g_GuiDisplayListPtr_00449724->field_0x18 = 1;
		SetActiveDisplayList_002cab60(pDVar2->pDisplayListInternal);
	}
	return bVar1;
}

void ActivateDisplayLists_002d6490(void)
{
	//if (g_GameDisplayListPtr_0044971c->bEnabled != 0) {
	//	WillSetActiveDisplayList_002cac70(g_GameDisplayListPtr_0044971c->pDisplayListInternal);
	//}
	//if (g_FrontendDisplayListPtr_00449720->bEnabled != 0) {
	//	WillSetActiveDisplayList_002cac70(g_FrontendDisplayListPtr_00449720->pDisplayListInternal);
	//}
	if ((g_GuiDisplayListPtr_00449724 != (DisplayList*)0x0) && (g_GuiDisplayListPtr_00449724->bEnabled != 0)) {
		edDlist::WillSetActiveDisplayList_002cac70(g_GuiDisplayListPtr_00449724->pDisplayListInternal);
	}
	return;
}

DisplayList::DisplayList()
{
	field_0x8 = -1;
	field_0xc = -1;
	field_0x10 = -1;
	pDisplayListInternal = (DisplayListInternal*)0x0;
	bEnabled = 0;
}

DisplayList::DisplayList(int inField_0x8, int inField_0xc, int inField_0x10, int inField_0x1c, StaticMeshMaster* pInStaticMeshMaster)
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

void DisplayList::Init()
{
	bEnabled = 1;
	field_0x18 = 0;
}

void StaticMeshMaster::SetFlag_002a5400(uint flag)
{
	this->flags_0x4 = this->flags_0x4 | flag;
}

void StaticMeshMaster::SetFlag_002a5410(bool bValue)
{
	if (bValue == false) {
		this->flags_0x4 = this->flags_0x4 & 0xffffffbf;
	}
	else {
		this->flags_0x4 = this->flags_0x4 | 0x40;
	}
	return;
}

void StaticMeshMaster::SetFlag_002a5440(bool bValue)
{
	if (bValue == false) {
		this->flags_0x4 = this->flags_0x4 & 0xffffff7f;
	}
	else {
		this->flags_0x4 = this->flags_0x4 | 0x80;
	}
	return;
}
