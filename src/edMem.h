#ifndef _EDMEM_H
#define _EDMEM_H

#include "Types.h"
#include <stdlib.h>

enum EHeapType
{
	H_INVALID,
	H_MAIN,
	H_MEMKIT,
	H_SCRATCHPAD,
	H_VIDEO
};

typedef void* EHeap;

struct edHeapEntry
{
	short nextBlock; // -1 if not yet reached, or set to the next block if allocation runs past this block.
	short field_0x2;
	int freeBytes;
	short heapID;
	short field_0xa;
	ushort flags;
	byte funcTableID;
	byte field_0xf;
	char* pStartAddr;
	int align;
	int offset;
	short field_0x1c;
	short nextFreeBlock;
#ifdef PLATFORM_WIN
	char padding[12];
#endif
};

struct edHeapParams {
	char* startAddress;
	int size;
	int align;
	int offset;
};

int edMemGetMemoryAvailable(EHeap heapID);
void* edMemGetBlockAddress(void* pAlloc);
void* edMemAllocAlignBoundary(EHeap heapID, size_t size);
void* edMemAllocAlign(EHeap heapID, size_t size, int align);
void* edMemAllocAlignBoundary(EHeap heap, size_t size, int align, int offset);
void edMemFree(void* pAlloc);
void* edMemShrink(void* pAlloc, int size);

bool IsAllocationValid_0028f3d0(int size, int align, int offset);
void SetDataBlock_0028f440(uint* data, int numWords, int isFree);
void SetHeapFlag_0028f4e0(edHeapEntry* pHeap);
int edMemGetAvailable(EHeap heapID);
void DebugDump(EHeap heapID, long mode);
edHeapEntry* AllocateHeapMemory_0028fac0(edHeapEntry* pHeap, int param_2);
edHeapEntry* edmemGetMainHeader(void* heapID);
edHeapEntry* GetBlock_0028fc40(edHeapEntry* pHeap);
void FreeDataBlock_0028fc90(edHeapEntry* pHeap);
bool AllocateNewBlock_0028fd60(edHeapEntry* pHeap);

void edMemSetFlags(EHeap heapID, ushort newFlags);
void edMemClearFlags(EHeap heapID, ushort flags);

void ClearHeaps_0028ff10(void);
void SetupDefaultHeaps_0028ff90(void);
void SetHeapFunctionsA(void);
void EmptyFunction_00290290(void);
void edMemInit(short headerCount);

int GetHeadersUsed_00290320(void);
int GetHeadersLeft_00290340(void);
int GetMaxHeadersUsed_00290350(void);

void IncrementCounter_002903f0(void);
bool Cleanup_00290410(void);
void* edSystemFastRamGetAddr(void);

// Not in this file.
void* AllocateFunc_001002a0(size_t size);
void* Allocate(long amount);
void SetHeapFunctionsB(void);

extern edHeapParams g_SystemHeap_0042df0;
extern uint g_MemWorkSizeB;

#define TO_HEAP(a) (void*)a

#endif //_EDMEM_H
