#include "CameraPanMasterHeader.h"
#include "edMem.h"
#include <string.h>

const char sz_NodeTitle_00432b68[4] = { 'N', 'O', 'D', 'E' };

bool FUN_002909f0(CameraPanMasterHeader* param_1, int param_2, undefined* param_3)
{
	if (param_2 == 2) {
		param_1->headerUnion.field1 = (char*)param_3;
	}
	else {
		param_1->headerUnion.field1 = (char*)param_2;
	}
	return true;
}

bool Setup_00290bf0
(CameraPanMasterHeader* pCameraPanStatic, MeshTransformParentHeader* param_2, int param_3,
	undefined* param_4)
{
	bool bVar1;

	pCameraPanStatic->pCameraPanStatic_0x8 = (MeshTransformParent*)pCameraPanStatic;
	pCameraPanStatic->pLoadedData = (MeshTransformSpecial*)pCameraPanStatic;
	pCameraPanStatic->pCameraPanHeader_0xc = param_2;
	pCameraPanStatic->field_0x10 = 0;
	pCameraPanStatic->count_0x14 = 0;
	bVar1 = FUN_002909f0(pCameraPanStatic, param_3, param_4);
	return bVar1;
}

void* Setup_00290b70(MeshTransformParentHeader* pAllocatedBuffer, int count)
{
	void* pvVar1;
	MeshTransformSpecial* __s;
	int iVar2;

	__s = (MeshTransformSpecial*)(pAllocatedBuffer + 1);
	*(uint*)pAllocatedBuffer->name = *(uint*)sz_NodeTitle_00432b68;
	pAllocatedBuffer->totalCount = count;
	pAllocatedBuffer->usedCount = 0;
	pAllocatedBuffer->field_0x4 = __s;
	pvVar1 = memset(__s, 0, count << 4);
	for (iVar2 = count + -1; iVar2 != 0; iVar2 = iVar2 + -1) {
		__s->pNext_0x4 = __s + 1;
		__s = __s + 1;
	}
	return pvVar1;
}

short* FUN_002908d0(CameraPanMasterHeader* param_1, short* param_2)
{
	MeshTransformParentHeader* pMVar1;
	CameraPanMasterHeader* pCVar2;

	if ((CameraPanMasterHeader*)param_2 != param_1) {
		pCVar2 = *(CameraPanMasterHeader**)(param_2 + 2);
		pMVar1 = param_1->pCameraPanHeader_0xc;
		if (param_2[1] == 0) {
			pCVar2->pCameraPanStatic_0x8 = *(MeshTransformParent**)(param_2 + 4);
			*(CameraPanMasterHeader**)(*(int*)(param_2 + 4) + 4) = pCVar2;
			*param_2 = 0;
			*(MeshTransformSpecial**)(param_2 + 2) = pMVar1->field_0x4;
			pMVar1->field_0x4 = (MeshTransformSpecial*)param_2;
			pMVar1->usedCount = pMVar1->usedCount + -1;
			param_1->count_0x14 = param_1->count_0x14 + -1;
		}
		else {
			*param_2 = 1;
			*(undefined4*)(param_2 + 6) = 0;
		}
		for (; (param_2 = (short*)pCVar2, pCVar2 != param_1 && (*(short*)&pCVar2->headerUnion == 1));
			pCVar2 = (CameraPanMasterHeader*)pCVar2->pLoadedData) {
		}
	}
	return param_2;
}

void Link_00290c10(CameraPanMasterHeader* param_1)
{
	MeshTransformParentHeader* pMVar1;
	CameraPanMasterHeader* pCVar2;

	if (param_1->field_0x10 == 0) {
		if ((CameraPanMasterHeader*)param_1->pLoadedData != param_1) {
			pMVar1 = param_1->pCameraPanHeader_0xc;
			param_1->pCameraPanStatic_0x8->pSubMeshParent = (MeshTransformParent*)pMVar1->field_0x4;
			pMVar1->field_0x4 = param_1->pLoadedData;
			pMVar1->usedCount = pMVar1->usedCount - param_1->count_0x14;
			param_1->count_0x14 = 0;
			param_1->pLoadedData = (MeshTransformSpecial*)param_1;
			param_1->pCameraPanStatic_0x8 = (MeshTransformParent*)param_1;
		}
	}
	else {
		for (pCVar2 = (CameraPanMasterHeader*)param_1->pLoadedData; pCVar2 != param_1;
			pCVar2 = (CameraPanMasterHeader*)FUN_002908d0(param_1, (short*)pCVar2)) {
		}
	}
	return;
}

CameraPanMasterHeader* AllocateCameraAndMesh_00290a10(int cameraPanCount, int meshHeaderCount, long heapID)
{
	bool bVar1;
	CameraPanMasterHeader* pCVar2;
	CameraPanMasterHeader* pAllocatedBuffer;

	if (meshHeaderCount == 0) {
		pCVar2 = (CameraPanMasterHeader*)edMemAlloc((short)heapID, cameraPanCount << 5);
	}
	else {
		pCVar2 = (CameraPanMasterHeader*)edMemAlloc((short)heapID, meshHeaderCount * 0x10 + cameraPanCount * 0x20 + 0x10);
		pAllocatedBuffer = pCVar2 + cameraPanCount;
		Setup_00290b70((MeshTransformParentHeader*)pAllocatedBuffer, meshHeaderCount);
		while (bVar1 = cameraPanCount != 0, cameraPanCount = cameraPanCount + -1, bVar1) {
			Setup_00290bf0(pCVar2 + cameraPanCount, (MeshTransformParentHeader*)pAllocatedBuffer, 0, 0);
		}
	}
	if (pCVar2 == (CameraPanMasterHeader*)0x0) {
		pCVar2 = (CameraPanMasterHeader*)0x0;
	}
	return pCVar2;
}
