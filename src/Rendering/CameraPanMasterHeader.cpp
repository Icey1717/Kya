#include "CameraPanMasterHeader.h"
#include <string.h>

const char sz_NodeTitle_00432b68[4] = { 'N', 'O', 'D', 'E' };

bool edListSetMode(edLIST* param_1, int param_2, CameraPanFunc* param_3)
{
	if (param_2 == 2) {
		param_1->headerUnion.field1 = param_3;
	}
	else {
		param_1->headerUnion.field0 = param_2;
	}
	return true;
}

bool edListLink
(edLIST* pList, edNODE_MANAGER* param_2, int param_3,
	CameraPanFunc* param_4)
{
	bool bVar1;

	pList->pCameraPanStatic_0x8 = (MeshTransformParent*)pList;
	pList->pLoadedData = (MeshTransformSpecial*)pList;
	pList->pCameraPanHeader_0xc = param_2;
	pList->field_0x10 = 0;
	pList->count_0x14 = 0;
	bVar1 = edListSetMode(pList, param_3, param_4);
	return bVar1;
}

void* Setup_00290b70(edNODE_MANAGER* pAllocatedBuffer, int count)
{
	void* pvVar1;
	MeshTransformSpecial* __s;
	int iVar2;

	__s = (MeshTransformSpecial*)(pAllocatedBuffer + 1);
	*(uint*)pAllocatedBuffer->name = *(uint*)sz_NodeTitle_00432b68;
	pAllocatedBuffer->totalCount = count;
	pAllocatedBuffer->usedCount = 0;
	pAllocatedBuffer->pNextFree = __s;
	pvVar1 = memset(__s, 0, count << 4);
	for (iVar2 = count + -1; iVar2 != 0; iVar2 = iVar2 + -1) {
		__s->pNext_0x4 = __s + 1;
		__s = __s + 1;
	}
	return pvVar1;
}

short* FUN_002908d0(edLIST* param_1, short* param_2)
{
	edNODE_MANAGER* pMVar1;
	edLIST* pCVar2;

	if ((edLIST*)param_2 != param_1) {
		pCVar2 = *(edLIST**)(param_2 + 2);
		pMVar1 = param_1->pCameraPanHeader_0xc;
		if (param_2[1] == 0) {
			pCVar2->pCameraPanStatic_0x8 = *(MeshTransformParent**)(param_2 + 4);
			*(edLIST**)(*(int*)(param_2 + 4) + 4) = pCVar2;
			*param_2 = 0;
			*(MeshTransformSpecial**)(param_2 + 2) = pMVar1->pNextFree;
			pMVar1->pNextFree = (MeshTransformSpecial*)param_2;
			pMVar1->usedCount = pMVar1->usedCount + -1;
			param_1->count_0x14 = param_1->count_0x14 + -1;
		}
		else {
			*param_2 = 1;
			*(undefined4*)(param_2 + 6) = 0;
		}
		for (; (param_2 = (short*)pCVar2, pCVar2 != param_1 && (*(short*)&pCVar2->headerUnion == 1));
			pCVar2 = (edLIST*)pCVar2->pLoadedData) {
		}
	}
	return param_2;
}

void edListClear(edLIST* pCameraPanMasterHeader)
{
	edNODE_MANAGER* pMVar1;
	edLIST* pCVar2;

	if (pCameraPanMasterHeader->field_0x10 == 0) {
		if ((edLIST*)pCameraPanMasterHeader->pLoadedData != pCameraPanMasterHeader) {
			pMVar1 = pCameraPanMasterHeader->pCameraPanHeader_0xc;
			pCameraPanMasterHeader->pCameraPanStatic_0x8->pNext = (MeshTransformParent*)pMVar1->pNextFree;
			pMVar1->pNextFree = pCameraPanMasterHeader->pLoadedData;
			pMVar1->usedCount = pMVar1->usedCount - pCameraPanMasterHeader->count_0x14;
			pCameraPanMasterHeader->count_0x14 = 0;
			pCameraPanMasterHeader->pLoadedData = (MeshTransformSpecial*)pCameraPanMasterHeader;
			pCameraPanMasterHeader->pCameraPanStatic_0x8 = (MeshTransformParent*)pCameraPanMasterHeader;
		}
	}
	else {
		for (pCVar2 = (edLIST*)pCameraPanMasterHeader->pLoadedData; pCVar2 != pCameraPanMasterHeader;
			pCVar2 = (edLIST*)FUN_002908d0(pCameraPanMasterHeader, (short*)pCVar2)) {
		}
	}
	return;
}

edLIST* edListNew(int cameraPanCount, int meshHeaderCount, void* heapID)
{
	bool bVar1;
	edLIST* pCVar2;
	edLIST* pAllocatedBuffer;

	if (meshHeaderCount == 0) {
		pCVar2 = (edLIST*)edMemAlloc(heapID, cameraPanCount << 5);
	}
	else {
		pCVar2 = (edLIST*)edMemAlloc(heapID, meshHeaderCount * 0x10 + cameraPanCount * 0x20 + 0x10);
		pAllocatedBuffer = pCVar2 + cameraPanCount;
		Setup_00290b70((edNODE_MANAGER*)pAllocatedBuffer, meshHeaderCount);
		while (bVar1 = cameraPanCount != 0, cameraPanCount = cameraPanCount + -1, bVar1) {
			edListLink(pCVar2 + cameraPanCount, (edNODE_MANAGER*)pAllocatedBuffer, 0, NULL);
		}
	}
	if (pCVar2 == (edLIST*)0x0) {
		pCVar2 = (edLIST*)0x0;
	}
	return pCVar2;
}
