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

struct S_MAIN_MEMORY_HEADER
{
	short nextBlock; // -1 if not yet reached, or set to the next block if allocation runs past this block.
	short field_0x2;
	int freeBytes;
	short heapID;
	short field_0xa;
	ushort flags;
	byte funcTableID;
	byte maxStackLevel;
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


#ifdef PLATFORM_WIN
#pragma pack(push,1)
struct VirtualMemory {
	S_MAIN_MEMORY_HEADER g_heap_004a5780[0x1000];
	//char g_memory[0x1685100];
	char g_memory[0x1F85100];
};
#pragma  pack(pop)
#endif // PLATFORM_WIN

int edMemGetMemoryAvailable(EHeap heapID);
void* edMemGetLocalAddress(void* pAlloc);
void* edMemAlloc(EHeap heapID, size_t size);
void* edMemAllocAlign(EHeap heapID, size_t size, int align);
void* edMemAllocAlignBoundary(EHeap heap, size_t size, int align, int offset);
void edMemFree(void* pAlloc);
void* edMemShrink(void* pAlloc, int size);

bool edmemTestAllocOk(int size, int align, int offset);
void edmemFillBlock(uint* data, int numWords, int isFree);
void edmemSetReadOnly(S_MAIN_MEMORY_HEADER* pHeap);
int edMemGetAvailable(EHeap heapID);
void DebugDump(EHeap heapID, long mode);
S_MAIN_MEMORY_HEADER* edmemGetFreeMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap, int param_2);

S_MAIN_MEMORY_HEADER* edmemGetMainHeader();
S_MAIN_MEMORY_HEADER* edmemGetMainHeader(void* heapID);
S_MAIN_MEMORY_HEADER* edmemGetMasterMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap);
void edmemFreeMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap);
bool edmemMakeMaster(S_MAIN_MEMORY_HEADER* pHeap);

void edMemSetFlags(EHeap heapID, ushort newFlags);
void edMemClearFlags(EHeap heapID, ushort flags);

void memMemoryHeadersBlockInit(void);
void memMastersInit(void);
void memHandlersInit(void);
void memDebugInformationInit(void);
void edMemInit(short headerCount);

int edMemGetNbAlloc(void);
int edMemGetNbAllocLeft(void);
int edMemGetNbMaxAlloc(void);

void edMemStackLevelPush(void);
bool edMemTerm(void);
void* edSystemFastRamGetAddr(void);

// Not in this file.
void* AllocateFunc_001002a0(size_t size);
void* Allocate(long amount);
void _edSystemInitVideoHandlers(void);

extern edHeapParams g_SystemHeap_0042df0;
extern uint g_MemWorkSizeB;

extern S_MAIN_MEMORY_HEADER* g_HeapPtr_0040f370;

#define TO_HEAP(a) (void*)a

#endif //_EDMEM_H
