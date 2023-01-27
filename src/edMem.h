#ifndef _EDMEM_H
#define _EDMEM_H

#include "Types.h"
#include <stdlib.h>

struct edHeap
{
	undefined2 field_0x0;
	undefined2 field_0x2;
	undefined4 freeBytes;
	undefined2 heapID;
	undefined2 field_0xa;
	ushort flags;
	byte funcTableID;
	byte field_0xf;
	void* pStartAddr;
	int align;
	int offset;
	short field_0x1c;
	short field_0x1e;
};

void InitHeap_002902a0(short headerCount);
void ClearHeaps_0028ff10(void);
void SetupDefaultHeaps_0028ff90(void);
void* Alloc_0028e140(edHeap* pInHeap, int size, int align, int offset);
void edMemFree_0028ecc0(edHeap* pHeap);
void StoreSomeFunctionsInRAM(void);
edHeap* GetHeap(short heapID);
void* HeapAlloc(short heap, size_t size, int align, int offset);
void* edMemAlloc(short heapID, size_t size);
void* edMemAllocAlign(int heapID, size_t size, int align);
void edMemFree(void* heapID);
void* AllocateFunc_001002a0(size_t size);
void* Allocate(long amount);

#endif //_EDMEM_H
