#include "edMem.h"
#include "edSystem.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <assert.h>

short g_MaxHeadersUsed_00424d68 = 0;
short g_TotalHeaders_00424d64 = 0;
short g_FreeHeaders_00424d66 = 0;
edHeap* g_HeapArray_00424d60 = NULL;

void InitHeap_002902a0(short headerCount)
{
	g_MaxHeadersUsed_00424d68 = 0;
	//DAT_00424d6a = 0;
	//BYTE_00424d6c = 0;
	//DAT_00424d6d = 0;
	g_HeapArray_00424d60 = edSystem::edSystemDataPtr_0042df0;
	g_TotalHeaders_00424d64 = headerCount;
	g_FreeHeaders_00424d66 = headerCount;
	//FUN_002903f0();
	ClearHeaps_0028ff10();
	SetupDefaultHeaps_0028ff90();
	StoreSomeFunctionsInRAM();
	//EmptyFunction();
	return;
}

void ClearHeaps_0028ff10(void)
{
	short sVar1;
	edHeap* peVar2;

	peVar2 = g_HeapArray_00424d60;
	memset((char*)g_HeapArray_00424d60, 0, (int)g_TotalHeaders_00424d64 << 5);
	for (sVar1 = 0; sVar1 < g_TotalHeaders_00424d64; sVar1 = sVar1 + 1) {
		peVar2->heapID = sVar1 + 1;
		peVar2 = peVar2 + 1;
	}
	return;
}

int g_DefaultAlign_00424df8 = 0x10;
int g_DefaultOffset_00424dfc = 0;

void SetupDefaultHeaps_0028ff90(void)
{
	edHeap* peVar1;
	edHeap* puVar1;

	peVar1 = g_HeapArray_00424d60;
	g_HeapArray_00424d60->flags = 2;
	peVar1->funcTableID = 0xff;
	peVar1->field_0xf = 0xff;
	peVar1->pStartAddr = (undefined*)0xffffffff;
	peVar1->freeBytes = 0xffffffff;
	peVar1->field_0xa = 0xffff;
	peVar1->heapID = 0xffff;
	peVar1->align = 0xffffffff;
	peVar1->offset = 0xffffffff;
	peVar1->field_0x1c = -1;
	peVar1->field_0x1e = -1;
	peVar1->field_0x2 = 0xffff;
	peVar1->field_0x0 = 0xffff;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
	peVar1[1].flags = 0x96;
	peVar1[1].funcTableID = 0;
	peVar1[1].field_0xf = 0;
	peVar1[1].pStartAddr = (undefined*)(g_HeapArray_00424d60 + g_TotalHeaders_00424d64);
	//peVar1[1].freeBytes = g_MemWorkSizeB - (int)(peVar1[1].pStartAddr + -(int)g_HeapArray_00424d60);
	peVar1[1].field_0xa = 0xffff;
	peVar1[1].heapID = 2;
	peVar1[1].align = g_DefaultAlign_00424df8;
	peVar1[1].offset = g_DefaultOffset_00424dfc;
	peVar1[1].field_0x1c = -1;
	peVar1[1].field_0x1e = -1;
	peVar1[1].field_0x2 = 0xffff;
	peVar1[1].field_0x0 = 0xffff;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
}

void* Alloc_0028e140(edHeap* pInHeap, int size, int align, int offset)
{
#if defined(PLATFORM_PS2)
	return memalign(align, size);
#else
	return malloc(size);
#endif
}

void edMemFree_0028ecc0(edHeap* pHeap)
{
#if defined(PLATFORM_PS2)
	free(pHeap);
#else
	free(pHeap);
#endif
}

struct HeapFuncTable
{
	void* (*alloc)(edHeap*, int, int, int);
	void* field_0x4;
	void (*free)(edHeap*);
	void* field_0xc;
};

HeapFuncTable g_HeapFunctionTableArray_00489030[2];

void StoreSomeFunctionsInRAM(void)
{
	g_HeapFunctionTableArray_00489030[0].alloc = Alloc_0028e140;
	//g_HeapFunctionTableArray_00489030[0].field_0x4 = FUN_0028e820;
	g_HeapFunctionTableArray_00489030[0].free = edMemFree_0028ecc0;
	//g_HeapFunctionTableArray_00489030[0].field_0xc = FUN_0028f120;
	g_HeapFunctionTableArray_00489030[1].alloc = Alloc_0028e140;
	//g_HeapFunctionTableArray_00489030[1].field_0x4 = FUN_0028e820;
	g_HeapFunctionTableArray_00489030[1].free = edMemFree_0028ecc0;
	//g_HeapFunctionTableArray_00489030[1].field_0xc = FUN_0028f120;
	return;
}

edHeap* GetHeap(void* heapID)
{
	short sVar1;

	sVar1 = (short)heapID;
	if ((long)g_TotalHeaders_00424d64 <= (long)(int)heapID) {
		sVar1 = *(short*)((int)heapID + -0xc);
	}
	return g_HeapArray_00424d60 + (int)(((long)sVar1 << 0x30) >> 0x30);
}

void* HeapAlloc(short heap, size_t size, int align, int offset)
{
	EFileLoadMode EVar1;
	edHeap* peVar2;
	void* pvVar3;
	int iVar4;

	if (heap == 0) {
		/* edMemAlloc */
		//CallSystemFunction(&edMemObj_004890c0, 1, g_szEdMemAlloc_00432508);
		pvVar3 = (void*)0x0;
	}
	else {
		if ((heap == 2) && (EVar1 = GetFileLoadMode_00424d9c(), EVar1 == FLM_CD_DVD)) {
			/* write in MemKit, you'll pay your kinder! */
			//PrintString(g_T'es_pas_fou_d');
			//CallSystemFunction(&edMemObj_004890c0, 4, (char*)0x0);
		}
		peVar2 = GetHeap((void*)heap);
		pvVar3 = (*g_HeapFunctionTableArray_00489030[(char)peVar2->funcTableID].alloc)(peVar2, (int)size, align, offset);
		if (pvVar3 == (void*)0x0) {
			//iVar4 = GetCurrentFreeMemory(heap);
			/* Can't allocate %d (Mem Free is: %d)\n */
			//PrintString(s__Can't_allocate_%d_(Mem_Free_is:_00432570,size,iVar4);
		}
	}
	return pvVar3;
}

void* WillAllocate_0028ddd0(void* param_1)
{
	edHeap* peVar1;
	void* puVar2;

	if (param_1 == (void*)0x0) {
		assert(false);
		//CallSystemFunction(&edMemObj_004890c0, 1, s_edMemAlloc_004324f8);
		puVar2 = (void*)0x0;
	}
	else {
		peVar1 = GetHeap(param_1);
		puVar2 = peVar1->pStartAddr;
	}
	return puVar2;
}

void* edMemAlloc(short heapID, size_t size)
{
	edHeap* peVar1;
	void* pvVar2;

	if (heapID == 0) {
		//CallSystemFunction(&edMemObj_004890c0, 1, g_szEdMemAlloc_00432508);
		pvVar2 = (void*)0x0;
	}
	else {
		peVar1 = GetHeap((void*)heapID);
		pvVar2 = HeapAlloc(heapID, size, peVar1->align, peVar1->offset);
	}
	return pvVar2;
}

void* edMemAllocAlign(int heapID, size_t size, int align)
{
	EFileLoadMode EVar1;
	edHeap* peVar2;
	void* pvVar3;

	if (heapID == 0) {
		/* edMemAllocAlign */
		//CallSystemFunction(&edMemObj_004890c0, 1, s_edMemAllocAlign_00432520);
		pvVar3 = (void*)0x0;
	}
	else {
		if ((heapID == 2) && (EVar1 = GetFileLoadMode_00424d9c(), EVar1 == FLM_CD_DVD)) {
			///* T'es pas fau d' - You are not mistaken */
			//PrintString(g_T'es_pas_fou_d');
			//CallSystemFunction(&edMemObj_004890c0, 4, (char*)0x0);
		}
		peVar2 = GetHeap((void*)heapID);
		pvVar3 = HeapAlloc((short)heapID, size, align, peVar2->offset);
	}
	return pvVar3;
}

void edMemFree(void* heapID)
{
	edHeap* peVar1;

	if (heapID == 0) {
		/* edMemFree */
		//CallSystemFunction(&edMemObj_004890c0, 1, s_edMemFree_00432598);
	}
	else {
//#if defined(PLATFORM_PS2)
//		peVar1 = GetHeap((short)heapID);
//#else
		peVar1 = GetHeap((void*)1);
//#endif
		(*g_HeapFunctionTableArray_00489030[(char)peVar1->funcTableID].free)((edHeap*)heapID); //peVar1); HACK
	}
	return;
}

void* AllocateFunc_001002a0(size_t size)
{
	void* pvVar1;

	pvVar1 = edMemAlloc(1, size);
	return pvVar1;
}

void* Allocate(long amount)
{
	void* pvVar1;

	pvVar1 = edMemAlloc(1, (int)amount);
	return pvVar1;
}
