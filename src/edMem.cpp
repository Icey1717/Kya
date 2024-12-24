#include "edMem.h"
#include "edSystem.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <assert.h>

#include <unordered_set>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

#define ED_MEM_LOG(level, format, ...) MY_LOG_CATEGORY("edMem", level, format, ##__VA_ARGS__)

#define MEM_FLAG_LINKED_DEPENDENT_BLOCKS 0x8
#define MEM_FLAG_REQUIRES_HEADER 0x10
#define MEM_FLAG_REVERSE_TRAVERSAL 0x100

struct HeapFuncTable
{
	char* (*alloc)(S_MAIN_MEMORY_HEADER*, int, int, int);
	void* (*force)(S_MAIN_MEMORY_HEADER*, void*);
	void (*free)(S_MAIN_MEMORY_HEADER*);
	char* (*shrink)(S_MAIN_MEMORY_HEADER*, int);
};

HeapFuncTable MemoryHandlers[2];

struct edSysHandlerMemory {
	edSysHandlerMemory(struct edSysHandlersNodeTable* inNodeParent, int inMaxEventID, int inMainIdentifier)
		: nodeParent(inNodeParent)
		, maxEventID(inMaxEventID)
		, mainIdentifier(inMainIdentifier)
	{

	}

	struct edSysHandlersNodeTable* nodeParent;
	struct edSysHandlersPoolEntry* entries[8];
	int maxEventID;
	int mainIdentifier;
};

uint g_MemWorkSizeB = 0x1B4A880;

// Contiguous

struct MasterMemoryBlk {
	struct S_MAIN_MEMORY_HEADER* aBlocks;
	short nbTotalBlocks;
	short nbFreeBlocks;
	short nbUsedBlocks;
	short nextFreeBlock;
	byte stackLevel;
	byte field_0xd;
	byte field_0xe;
	undefined field_0xf;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	ushort flags;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

MasterMemoryBlk MemoryMasterBlock;

static void PrintBlockInfo(const char* pIdentifier, S_MAIN_MEMORY_HEADER* pBlock)
{
	ED_MEM_LOG(LogLevel::Info, "PrintBlockInfo ----------------------------------------------------");

	ulong thisBlock = ((ulong)pBlock - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER);
	ED_MEM_LOG(LogLevel::Info, "PrintBlockInfo Identifier: {} | 0x{:x}", pIdentifier, thisBlock);

	ED_MEM_LOG(LogLevel::Info, "PrintBlockInfo Block: 0x{:x} FreeBytes: 0x{:x} Align: 0x{:x} Offset: 0x{:x} Flags: 0x{:x} StackLevel: 0x{:x} BlockIndex: 0x{:x} FreeListHead: 0x{:x}",
		(uintptr_t)pBlock, pBlock->freeBytes, pBlock->align, pBlock->offset, pBlock->flags, pBlock->maxStackLevel, pBlock->blockIndex, pBlock->freeListHead);

	ED_MEM_LOG(LogLevel::Info, "PrintBlockInfo NextFreeBlock: 0x{:x} PrevFreeBlock: 0x{:x} FreeListHeadAllocatedTo: 0x{:x} ParentBlockIndex: 0x{:x}",
		pBlock->nextFreeBlock, pBlock->prevFreeBlock, pBlock->freeListHeadAllocatedTo, pBlock->parentBlockIndex);

	ED_MEM_LOG(LogLevel::Info, "PrintBlockInfo ----------------------------------------------------");

	return;
}

void sanityCheckFreeList(S_MAIN_MEMORY_HEADER* freeListHead) {
	
}

// End Contiguous

edSysHandlerMemory edSysHandlerMemory_004890c0 = edSysHandlerMemory(&g_SysHandlersNodeTable_00489170, 8, 3);

// Contiguous

char* g_szEdMemGetMemoryAvailable = "edMemGetMemoryAvailable";
char* g_szEdMemAlloc_004324f8 = "edMemAlloc";
char* g_szEdMemAlloc_00432508 = "edMemAlloc";
char* g_szMemkitWarning = "T'es pas fou d'crire en MemKit, tu vas payer tes kinder !\n";
char* g_szCannotAllocate = " Can't allocate %d (Mem Free is: %d)\n";
char* g_szEdMemFree_00432598 = "edMemFree";
char* g_szEdMemShrink_004325a8 = "edMemShrink";
char* sz_CannotAllocate_004325c0 = " Can't allocate %d \n";

const uint g_EdenName_004325d8 = 0x4e454445;
const uint g_FreeName_004325e0 = 0x65657266;

char* g_szAllocationImpossible_004325f0 = "Allocation impossible : size + align > boundary\n";

const uint g_FreeName_00432628 = 0x65657266;
const uint g_UsedName_00432630 = 0x64657375;

// Debug stuff here

char* g_szEdMemSetFlags_00432b28 = "edMemSetFlags";
char* g_szEdMemClearFlags_00432b40 = "edMemClearFlags";

// End Contiguous

edHeapParams g_SystemHeap_0042df0 = { (char*)0x0, 0x0, 0x10, 0x0 };
edHeapParams g_Heap3_00424e00 = { (char*)0x02000000, 0x6000000, 0x10, 0x0 };
edHeapParams g_ScratchpadHeap_00424e10 = { (char*)0x70000000, 0x4000, 0x10, 0x0 };
edHeapParams g_Heap4_00424e20 = { (char*)0x0, 0x400000, 0x4, 0x0 };

void CallHandlerFunction(edSysHandlerMemory* pHandler, int id, char* pData)
{
	edSysHandlersCall(pHandler->mainIdentifier, pHandler->entries, pHandler->maxEventID, id, pData);
	return;
}

int edMemGetMemoryAvailable(EHeap heapID)
{
	short nextBlockID;
	int iVar2;
	S_MAIN_MEMORY_HEADER* pHeap;
	int freeBytes;

	if (heapID == TO_HEAP(H_INVALID)) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemGetMemoryAvailable);
	}

	pHeap = edmemGetMainHeader((void*)heapID);
	if ((pHeap->flags & MEM_FLAG_LINKED_DEPENDENT_BLOCKS) == 0) {
		if ((pHeap->flags & 0x200) == 0) {
			freeBytes = pHeap->freeBytes;
		}
		else {
			freeBytes = (int)(pHeap->pStartAddr +
				(pHeap->freeBytes - *(int*)MemoryMasterBlock.aBlocks[pHeap->freeListHead].pStartAddr));
		}
	}
	else {
		freeBytes = 0;
		for (nextBlockID = pHeap->freeListHead; nextBlockID != -1;
			nextBlockID = MemoryMasterBlock.aBlocks[nextBlockID].nextFreeBlock) {
			iVar2 = MemoryMasterBlock.aBlocks[nextBlockID].freeBytes;
			if (freeBytes < iVar2) {
				freeBytes = iVar2;
			}
		}
	}
	MY_LOG("edMemGetMemoryAvailable 0x{:x} bytes\n", freeBytes);
	return freeBytes;
}

void* edMemGetLocalAddress(void* pAlloc)
{
	S_MAIN_MEMORY_HEADER* pHeap;
	void* pBlockAddress;

	if (pAlloc == (void*)0x0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemAlloc_004324f8);
		pBlockAddress = (void*)0x0;
	}
	else {
		pHeap = edmemGetMainHeader(pAlloc);
		pBlockAddress = pHeap->pStartAddr;
	}
	return pBlockAddress;
}

void* edMemAlloc(EHeap heapID, size_t size)
{
	S_MAIN_MEMORY_HEADER* pHeap;
	void* pNewAlloc;

	ED_MEM_LOG(LogLevel::Info, "\nedMemAllocAlign Heap: {}, size: 0x{:x}", (int)heapID, size);

	if (heapID == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, "edMemAlloc");
		pNewAlloc = (void*)0x0;
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		pNewAlloc = edMemAllocAlignBoundary(heapID, size, pHeap->align, pHeap->offset);
	}

	assert(pNewAlloc);

	return pNewAlloc;
}

void* edMemAllocAlign(EHeap heapID, size_t size, int align)
{
	EFileLoadMode fileLoadMode;
	S_MAIN_MEMORY_HEADER* pHeap;
	void* pNewAllocation;

	ED_MEM_LOG(LogLevel::Info, "\nedMemAllocAlign Heap: 0x{:x}, size: 0x{:x}, align: 0x{:x}", (uintptr_t)heapID, size, align);

	if (heapID == TO_HEAP(H_INVALID)) {
		/* edMemAllocAlign */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, "edMemAllocAlign");
		pNewAllocation = (void*)0x0;
	}
	else {
		if ((heapID == TO_HEAP(H_MEMKIT)) && (fileLoadMode = GetFileLoadMode_00424d9c(), fileLoadMode == FLM_CD_DVD)) {
			edDebugPrintf(g_szMemkitWarning);
			CallHandlerFunction(&edSysHandlerMemory_004890c0, 4, (char*)0x0);
		}

		pHeap = edmemGetMainHeader((void*)heapID);
		pNewAllocation = edMemAllocAlignBoundary(heapID, size, align, pHeap->offset);
	}
	return pNewAllocation;
}

void* edMemAllocAlignBoundary(EHeap heap, size_t size, int align, int offset)
{
	EFileLoadMode fileLoadMode;
	S_MAIN_MEMORY_HEADER* pHeap;
	void* pNewAllocation;
	int freeMemory;

	ED_MEM_LOG(LogLevel::Info, "\nedMemAllocAlignBoundary Heap: 0x{:x}, size: 0x{:x}, align: 0x{:x}, offset: 0x{:x}", (uintptr_t)heap, size, align, offset);

	if (((uintptr_t)heap) > H_VIDEO) {
		ED_MEM_LOG(LogLevel::Info, "Allocating to existing specific block");
	}

	if (heap == TO_HEAP(H_INVALID)) {
		/* edMemAlloc */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemAlloc_00432508);
		pNewAllocation = (void*)0x0;
	}
	else {
		if ((heap == TO_HEAP(H_MEMKIT)) && (fileLoadMode = GetFileLoadMode_00424d9c(), fileLoadMode == FLM_CD_DVD)) {
			edDebugPrintf(g_szMemkitWarning);
			CallHandlerFunction(&edSysHandlerMemory_004890c0, 4, (char*)0x0);
		}

		pHeap = edmemGetMainHeader((void*)heap);
		pNewAllocation = (*MemoryHandlers[(char)pHeap->funcTableID].alloc)(pHeap, (int)size, align, offset);
		if (pNewAllocation == (void*)0x0) {
			FLUSH_LOG();
			freeMemory = edMemGetAvailable(heap);
			edDebugPrintf(g_szCannotAllocate, size, freeMemory);
		}
	}
	return pNewAllocation;
}

void edMemFree(void* pAlloc)
{
	S_MAIN_MEMORY_HEADER* peVar1;

	ED_MEM_LOG(LogLevel::Info, "\nedMemFree pAlloc: 0x{:x}", (uintptr_t)pAlloc);

	if (pAlloc == (void*)0x0) {
		/* edMemFree */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemFree_00432598);
	}
	else {
		peVar1 = edmemGetMainHeader(pAlloc);
		(*MemoryHandlers[(char)peVar1->funcTableID].free)(peVar1);
	}
	return;
}

void* edMemShrink(void* pAlloc, int size)
{
	S_MAIN_MEMORY_HEADER* pHeap;
	void* pNewAllocation;

	if (pAlloc == (void*)0x0) {
		/* EdMemShrink */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemShrink_004325a8);
		pNewAllocation = (void*)0x0;
	}
	else {
		pHeap = edmemGetMainHeader(pAlloc);
		pNewAllocation = (*MemoryHandlers[(char)pHeap->funcTableID].shrink)(pHeap, size);
	}
	return pNewAllocation;
}

char* edmemWorkAlloc(S_MAIN_MEMORY_HEADER* pMainMemHeader, int size, int align, int offset)
{
	short sVar1;
	bool bVar3;
	uint lastBlockFreeBytes;
	char* pcVar4;
	short sVar5;
	char* pcVar6;
	char* pcVar9;
	S_MAIN_MEMORY_HEADER* pNextFreeBlock;
	char* pReturn;
	short destBlock;
	ushort uVar10;
	ushort headerFlags;
	char* pAdjustedReturn;
	short currentBlockIndex;
	S_MAIN_MEMORY_HEADER* pBlocks;
	S_MAIN_MEMORY_HEADER* pCurrentBlock;

	ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc size: 0x{:x}, align: 0x{:x}, offset: 0x{:x}", size, align, offset);

	PrintBlockInfo("Main", pMainMemHeader);
		
	pBlocks = MemoryMasterBlock.aBlocks;
	if (((pMainMemHeader->flags & 2) == 0) || ((pMainMemHeader->flags & 4) == 0)) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 3, pMainMemHeader->pStartAddr);
	}

	if ((pMainMemHeader->flags & 0x80) == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 4, pMainMemHeader->pStartAddr);
		pReturn = (char*)0x0;
	}
	else {
		if ((size & 0xfU) != 0) {
			// Adjust upwards to the next 16 byte boundary
			size = size + (0x10U - size & 0xf);
			ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Adjusted size to 0x{:x}", size);
		}

		ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Adjusted Header info pMainMemHeader->align: 0x{:x}, pMainMemHeader->offset: 0x{:x}", 
			pMainMemHeader->align, pMainMemHeader->offset);

		if ((pMainMemHeader->align == 0) || (align % pMainMemHeader->align == 0)) {
			if ((pMainMemHeader->offset == 0) || (offset <= pMainMemHeader->offset)) {
				if (edmemTestAllocOk(size, align, offset) == false) {
					pReturn = (char*)0x0;
				}
				else {
					if (((pMainMemHeader->flags & MEM_FLAG_LINKED_DEPENDENT_BLOCKS) == 0) && (bVar3 = edmemMakeMaster(pMainMemHeader), bVar3 == false)) {
						pReturn = (char*)0x0;
					}
					else {
						currentBlockIndex = pMainMemHeader->freeListHead;
						headerFlags = pMainMemHeader->flags;

						ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc freeListHead: 0x{:x}, flags: 0x{:x}", pMainMemHeader->freeListHead, headerFlags);

						lastBlockFreeBytes = 0x7fffffff;
						pNextFreeBlock = pBlocks + currentBlockIndex;
						destBlock = -1;
						pCurrentBlock = pNextFreeBlock;

						if ((headerFlags & MEM_FLAG_REVERSE_TRAVERSAL) == 0) {
							ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Forward traversal");

							int sanityCheckBlockIndex = pMainMemHeader->freeListHead;

							while (sanityCheckBlockIndex != -1) {
								pCurrentBlock = pBlocks + sanityCheckBlockIndex;

								S_MAIN_MEMORY_HEADER* pNext = pBlocks + pCurrentBlock->nextFreeBlock;

								if (pCurrentBlock->nextFreeBlock != -1) {
									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc block: 0x{:x} <- 0x{:x}", pNext->prevFreeBlock, sanityCheckBlockIndex);
								}

								//FLUSH_LOG();

								assert((pCurrentBlock->nextFreeBlock == -1) || pNext->prevFreeBlock == sanityCheckBlockIndex);

								sanityCheckBlockIndex = pCurrentBlock->nextFreeBlock;
							}

							while (currentBlockIndex != -1) {
								pCurrentBlock = pBlocks + currentBlockIndex;

								PrintBlockInfo("Forward", pCurrentBlock);

								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc block: 0x{:x} free bytes: 0x{:x} (required: 0x{:x})", currentBlockIndex, pCurrentBlock->freeBytes, size);
	
								bool newBlockFreeBytesLess = pCurrentBlock->freeBytes < lastBlockFreeBytes;
								bool freeBytesGreaterThanSize = size <= pCurrentBlock->freeBytes;

								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc newBlockFreeBytesLess {} freeBytesGreaterThanSize {}", newBlockFreeBytesLess, freeBytesGreaterThanSize);

								if (newBlockFreeBytesLess && freeBytesGreaterThanSize) {
									char* pBlockStart = pCurrentBlock->pStartAddr;
									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc pBlockStart 0x{:x}", (uintptr_t)pBlockStart);

									if ((headerFlags & MEM_FLAG_REQUIRES_HEADER) != 0) {
										pBlockStart = pCurrentBlock->pStartAddr + 0x10;
										ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Adjusted + 0x10 pBlockStart 0x{:x}", (uintptr_t)pBlockStart);
									}

									if ((align != 0) && ((ulong)pBlockStart % align != 0)) {
										pBlockStart = pBlockStart + (align - (ulong)pBlockStart % align);
										ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Adjusted align pBlockStart 0x{:x}", (uintptr_t)pBlockStart);
									}

									char* pBlockAllocEnd = pBlockStart + size;

									if (offset != 0) {
										IMPLEMENTATION_GUARD(); // Could use some checking?
										if (((ulong)pBlockStart / offset != (ulong)pBlockAllocEnd / offset) &&
											(pBlockStart = pBlockStart + (offset - (ulong)pBlockStart % offset), align != 0)) {
											pBlockAllocEnd = pBlockStart + size;

											if ((ulong)pBlockStart % align == 0) goto LAB_0028e378;

											pBlockStart = pBlockStart + (align - (ulong)pBlockStart % align);
										}

										pBlockAllocEnd = pBlockStart + size;
									}

								LAB_0028e378:
									const ulong blockAllocSizeTotal = (ulong)pBlockAllocEnd + -((ulong)pCurrentBlock->pStartAddr);

									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc blockAllocSizeTotal 0x{:x} of 0x{:x} free", blockAllocSizeTotal, pCurrentBlock->freeBytes);

									if (blockAllocSizeTotal <= pCurrentBlock->freeBytes) {
										pNextFreeBlock = pCurrentBlock;
										destBlock = currentBlockIndex;
										lastBlockFreeBytes = pCurrentBlock->freeBytes;
										pReturn = pBlockStart;
									}
								}

								currentBlockIndex = pCurrentBlock->nextFreeBlock;
							}
						}
						else {
							ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Reverse traversal");

							PrintBlockInfo("Reverse Start", pCurrentBlock);

							short lastBlock;
							while (lastBlock = pCurrentBlock->nextFreeBlock, lastBlock != -1) {
								currentBlockIndex = lastBlock;
								pCurrentBlock = pBlocks + lastBlock;
								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc currentBlockIndex: 0x{:x}", currentBlockIndex);
								PrintBlockInfo("Reverse", pCurrentBlock);
							}

							PrintBlockInfo("Reverse Last", pCurrentBlock);

							ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc last block: 0x{:x}", currentBlockIndex);

							while ((pCurrentBlock != pMainMemHeader && (destBlock == -1))) {
								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc block: 0x{:x} free bytes: 0x{:x} (required: 0x{:x})", currentBlockIndex, pCurrentBlock->freeBytes, size);

								if (size < pCurrentBlock->freeBytes) {
									char* pAllocDest = pCurrentBlock->pStartAddr + (pCurrentBlock->freeBytes - size);
									if ((align != 0) && ((ulong)pAllocDest % align != 0)) {
										pAllocDest = pAllocDest + -((ulong)pAllocDest % align);
									}

									if ((((offset != 0) && ((ulong)pAllocDest / offset != (ulong)(pAllocDest + size) / offset)) &&
										(pAllocDest = pAllocDest + -((ulong)pAllocDest % offset), align != 0)) && ((ulong)pAllocDest % align != 0)) {
										pAllocDest = pAllocDest + -((ulong)pAllocDest % align);
									}

									pcVar4 = pAllocDest;
									if ((headerFlags & MEM_FLAG_REQUIRES_HEADER) != 0) {
										pcVar4 = pAllocDest + -0x10;
									}

									if (pCurrentBlock->pStartAddr <= pcVar4) {
										pNextFreeBlock = pCurrentBlock;
										destBlock = currentBlockIndex;
										pReturn = pAllocDest;
									}
								}

								currentBlockIndex = pCurrentBlock->prevFreeBlock;
								pCurrentBlock = pBlocks + currentBlockIndex;
							}
						}

						ED_MEM_LOG(LogLevel::Info, "\nedmemWorkAlloc destBlock: 0x{:x}", destBlock);

						if (destBlock == -1) {
							edDebugPrintf(sz_CannotAllocate_004325c0, size);
							CallHandlerFunction(&edSysHandlerMemory_004890c0, 0, pMainMemHeader->pStartAddr);
							pReturn = (char*)0x0;
						}
						else {
							PrintBlockInfo("Destination", pNextFreeBlock);

							char* pBlockStart = pNextFreeBlock->pStartAddr;
							short prevBlock = pNextFreeBlock->prevFreeBlock;
							int freeBytes = pNextFreeBlock->freeBytes;
							short nextBlock = pNextFreeBlock->nextFreeBlock;

							ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc pBlockStart 0x{:x} freeBytes 0x{:x} prevBlock 0x{:x} nextBlock 0x{:x}", (uintptr_t)pBlockStart, freeBytes, prevBlock, nextBlock);

							ulong remainingBlockSpace = (ulong)pBlockStart + (freeBytes - (ulong)(pReturn + size));
							ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc freeBytes: 0x{:x} size: {:x} remainingBlockSpace 0x{:x}", freeBytes, size, remainingBlockSpace);
							if (remainingBlockSpace == 0) {
								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Completely filled block");

								if (prevBlock == -1) {
									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Completely filled free list head");

									pMainMemHeader->freeListHead = nextBlock;
									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Updating free list head to 0x{:x}", nextBlock);
								}
								else {
									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Updating linked list to point towards next free block, not this one");

									pBlocks[prevBlock].nextFreeBlock = nextBlock;
								}

								uVar10 = destBlock;
								if (nextBlock != -1) {
									pBlocks[nextBlock].prevFreeBlock = prevBlock;
								}
							}
							else {
								pCurrentBlock = edmemGetFreeMemoryHeader(pNextFreeBlock, 0);
								if (pCurrentBlock == (S_MAIN_MEMORY_HEADER*)0x0) {
									return (char*)0x0;
								}

								pNextFreeBlock->freeBytes = (int)(pBlockStart + (freeBytes - (ulong)(pReturn + size)));
								pNextFreeBlock->pStartAddr = pReturn + size;
								if ((pMainMemHeader->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
									edmemFillBlock((uint*)pNextFreeBlock->pStartAddr, pNextFreeBlock->freeBytes, 0);
								}

								if ((MemoryMasterBlock.flags & 0x20) != 0) {
									edmemSetReadOnly(pNextFreeBlock);
								}

								pNextFreeBlock = pCurrentBlock;
								uVar10 = (ushort)((uint)((ulong)pCurrentBlock - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));
								nextBlock = destBlock;
							}

							pNextFreeBlock->flags = pMainMemHeader->flags & MEM_FLAG_REQUIRES_HEADER | 0x86;
							pNextFreeBlock->funcTableID = pMainMemHeader->funcTableID;
							pNextFreeBlock->maxStackLevel = MemoryMasterBlock.stackLevel;
							pNextFreeBlock->pStartAddr = pReturn;
							pNextFreeBlock->freeBytes = size;
							pNextFreeBlock->align = align;
							pNextFreeBlock->offset = offset;
							pNextFreeBlock->blockIndex = -1;
							pNextFreeBlock->prevFreeBlock = -1;
							pNextFreeBlock->nextFreeBlock = -1;

							pAdjustedReturn = pReturn;
							if ((pMainMemHeader->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Marking block EDEN");
								*(undefined4*)(pReturn + -0x10) = g_EdenName_004325d8;
								*(undefined4*)(pReturn + -4) = g_EdenName_004325d8;
								*(ushort*)(pReturn + -0xc) = uVar10;
								*(undefined2*)(pReturn + -10) = 0;
								*(undefined4*)(pReturn + -8) = 0;
								edmemFillBlock((uint*)pNextFreeBlock->pStartAddr, pNextFreeBlock->freeBytes, 1);
								pAdjustedReturn = pReturn + -0x10;
							}

							if ((pMainMemHeader->flags & 0x8000U) != 0) {
								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc adjusting pReturn");
								pReturn = (char*)((uint)pReturn & 0xdfffffff);
							}

							const ulong allocatedSize = (ulong)pAdjustedReturn + -(ulong)pBlockStart;
							ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc AllocatedSize: 0x{:x}", allocatedSize);

							if (allocatedSize != 0x0) {
								ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Has remaining space! Creating new block with remaining space!");

								pCurrentBlock = edmemGetFreeMemoryHeader(pNextFreeBlock, 0);
								if (pCurrentBlock == (S_MAIN_MEMORY_HEADER*)0x0) {
									return (char*)0x0;
								}

								pCurrentBlock->flags = 2;
								pCurrentBlock->pStartAddr = pBlockStart;
								pCurrentBlock->freeBytes = (ulong)(pAdjustedReturn + -(ulong)pBlockStart);
								short blockIndex = (short)((uint)((ulong)pCurrentBlock - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));
								if (prevBlock == -1) {
									pMainMemHeader->freeListHead = blockIndex;
									ED_MEM_LOG(LogLevel::Info, "edmemWorkAlloc Updating free list head to 0x{:x}", blockIndex);
								}
								else {
									pBlocks[prevBlock].nextFreeBlock = blockIndex;
								}

								if (nextBlock != -1) {
									pBlocks[nextBlock].prevFreeBlock = blockIndex;
								}

								pCurrentBlock->prevFreeBlock = prevBlock;
								pCurrentBlock->nextFreeBlock = nextBlock;

								if ((pMainMemHeader->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
									edmemFillBlock((uint*)pCurrentBlock->pStartAddr, pCurrentBlock->freeBytes, 0);
								}

								if ((MemoryMasterBlock.flags & 0x20) != 0) {
									edmemSetReadOnly(pCurrentBlock);
								}

								PrintBlockInfo("Remaining", pCurrentBlock);
							}

							if ((pMainMemHeader->flags & MEM_FLAG_REQUIRES_HEADER) == 0) {
								pReturn = (char*)(int)(short)((uint)((ulong)pNextFreeBlock - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));
							}
						}
					}
				}
			}
			else {
				pReturn = (char*)0x0;
			}
		}
		else {
			pReturn = (char*)0x0;
		}
	}

	sanityCheckFreeList(MemoryMasterBlock.aBlocks + pMainMemHeader->freeListHead);

	return pReturn;
}

void* edmemWorkAllocForce(S_MAIN_MEMORY_HEADER* param_1, void* addr)
{
	IMPLEMENTATION_GUARD();
	// Unused.
	return NULL;
}

void edmemWorkFree(S_MAIN_MEMORY_HEADER* pBlockToFree)
{
	bool bHasFlag_0x4;
	short freeListHeadAllocatedTo;
	S_MAIN_MEMORY_HEADER* pBlocks;
	ushort uVar4;
	S_MAIN_MEMORY_HEADER* pParentBlock;
	S_MAIN_MEMORY_HEADER* peVar6;
	char* pBlockStartAddr;
	short nextFreeBlock;
	short prevFreeBlock;
	short freeBlockIndex;
	bool bHasProcessedParentBlock;

	ED_MEM_LOG(LogLevel::Info, "edmemWorkFree pHeap: 0x{:x}", (uintptr_t)pBlockToFree);
	PrintBlockInfo("BeginFree", pBlockToFree);

	pBlocks = MemoryMasterBlock.aBlocks;

	if ((pBlockToFree->flags & 4) != 0) {
		pBlockStartAddr = pBlockToFree->pStartAddr;
		if ((pBlockToFree->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
			*(undefined4*)(pBlockStartAddr + -0x10) = g_FreeName_004325e0;
			*(undefined4*)(pBlockStartAddr + -4) = g_FreeName_004325e0;
		}

		if ((pBlockToFree->flags & MEM_FLAG_LINKED_DEPENDENT_BLOCKS) != 0) {
			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Freeing linked blocks");

			short blockIndex = pBlockToFree->blockIndex;
			while (blockIndex != -1) {
				S_MAIN_MEMORY_HEADER* pCurrentBlock = pBlocks + blockIndex;

				PrintBlockInfo("Freeing", pCurrentBlock);

				if ((pCurrentBlock->flags & 4) == 0) {
					edmemFreeMemoryHeader(pCurrentBlock);
				}
				else {
					edmemWorkFree(pCurrentBlock);
				}

				blockIndex = pBlockToFree->blockIndex;
			}
		}

		short blockIndex = pBlockToFree->blockIndex;
		if (blockIndex != -1) {
			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Freeing local blocks");

			while (blockIndex != -1) {
				short freeListHeadAllocatedTo = pBlocks[blockIndex].freeListHeadAllocatedTo;

				PrintBlockInfo("Freeing", pBlocks + blockIndex);
				edmemFreeMemoryHeader(pBlocks + blockIndex);
				blockIndex = freeListHeadAllocatedTo;
			}
		}

		if ((pBlockToFree->flags & 0x200) != 0) {
			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Freeing child blocks");
			PrintBlockInfo("Freeing", pBlocks + pBlockToFree->freeListHead);
			edmemWorkFree(pBlocks + pBlockToFree->freeListHead);
		}

		if ((pBlockToFree->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
			edmemFillBlock((uint*)(pBlockToFree->pStartAddr + -0x10), pBlockToFree->freeBytes + 0x10, 0);
		}

		pBlockToFree->flags = 2;
		pParentBlock = pBlocks + pBlockToFree->parentBlockIndex;
		freeBlockIndex = (short)((uint)((ulong)pBlockToFree - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));

		ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Freeing block 0x{:x}", freeBlockIndex);

		bHasFlag_0x4 = (pParentBlock->flags & 4) != 0;
		if (bHasFlag_0x4) {
			freeListHeadAllocatedTo = pBlockToFree->freeListHeadAllocatedTo;
		}
		else {
			pBlockToFree->freeBytes = (int)(pBlockToFree->pStartAddr + (pBlockToFree->freeBytes - (ulong)pParentBlock->pStartAddr));

			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree New freeBytes 0x{:x}", pBlockToFree->freeBytes);

			pBlockToFree->pStartAddr = pParentBlock->pStartAddr;
			prevFreeBlock = pParentBlock->prevFreeBlock;
			nextFreeBlock = pParentBlock->nextFreeBlock;
			edmemFreeMemoryHeader(pParentBlock);
			freeListHeadAllocatedTo = pBlockToFree->freeListHeadAllocatedTo;
		}

		bHasProcessedParentBlock = !bHasFlag_0x4;
		if (freeListHeadAllocatedTo != -1) {
			pParentBlock = pBlocks + freeListHeadAllocatedTo;

			PrintBlockInfo("Parent from Local", pParentBlock);

			if ((pParentBlock->flags & 4) == 0) {
				pBlockToFree->freeBytes = (int)(pParentBlock->pStartAddr + (pParentBlock->freeBytes - (ulong)pBlockToFree->pStartAddr));
				ED_MEM_LOG(LogLevel::Info, "edmemWorkFree New freeBytes 0x{:x}", pBlockToFree->freeBytes);

				if (!bHasProcessedParentBlock) {
					prevFreeBlock = pParentBlock->prevFreeBlock;
				}

				nextFreeBlock = pParentBlock->nextFreeBlock;
				bHasProcessedParentBlock = true;

				edmemFreeMemoryHeader(pParentBlock);
			}
			else {
				if (!bHasProcessedParentBlock) {
					uVar4 = pParentBlock->flags;
					while (((uVar4 & 4) != 0 && (pParentBlock->freeListHeadAllocatedTo != -1))) {
						pParentBlock = pBlocks + pParentBlock->freeListHeadAllocatedTo;
						uVar4 = pParentBlock->flags;
					}

					if ((uVar4 & 4) == 0) {
						prevFreeBlock = pParentBlock->prevFreeBlock;
						bHasProcessedParentBlock = true;
						nextFreeBlock = (ushort)((uint)((ulong)pParentBlock - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));
					}
				}
			}
		}

		if (bHasProcessedParentBlock == false) {
			short parentBlockIndex = pBlockToFree->parentBlockIndex;
			pParentBlock = pBlockToFree;
			while (true) {
				peVar6 = pBlocks + parentBlockIndex;
				uVar4 = peVar6->flags & 4;

				if ((uVar4 == 0) || (pBlocks + peVar6->blockIndex == pParentBlock)) break;

				parentBlockIndex = peVar6->parentBlockIndex;
				pParentBlock = peVar6;
			}

			prevFreeBlock = -1;
			nextFreeBlock = prevFreeBlock;

			if (uVar4 == 0) {
				prevFreeBlock = (ushort)((uint)((ulong)peVar6 - (ulong)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));
				nextFreeBlock = peVar6->nextFreeBlock;
			}
		}

		if (prevFreeBlock == -1) {
			S_MAIN_MEMORY_HEADER* pMainMemHeader = edmemGetMasterMemoryHeader(pBlockToFree);
			pMainMemHeader->freeListHead = freeBlockIndex;
			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Updating free list head to 0x{:x}", freeBlockIndex);
		}
		else {
			pBlocks[prevFreeBlock].nextFreeBlock = freeBlockIndex;
			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Updating prevFreeBlock (0x{:x}) -> nextFreeBlock to 0x{:x}", prevFreeBlock, freeBlockIndex);
		}

		if (nextFreeBlock != -1) {
			pBlocks[nextFreeBlock].prevFreeBlock = freeBlockIndex;
			ED_MEM_LOG(LogLevel::Info, "edmemWorkFree Updating prevFreeBlock (0x{:x}) -> prevFreeBlock to 0x{:x}", prevFreeBlock, freeBlockIndex);
		}

		pBlockToFree->prevFreeBlock = prevFreeBlock;
		pBlockToFree->nextFreeBlock = nextFreeBlock;
		pParentBlock = pBlocks + pBlockToFree->parentBlockIndex;

		if (pBlocks + pParentBlock->blockIndex == pBlockToFree) {
			if (pBlockToFree->freeListHeadAllocatedTo == -1) {
				edmemFreeMemoryHeader(pBlockToFree);
				if ((pParentBlock->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
					edmemFillBlock((uint*)pParentBlock->pStartAddr, pParentBlock->freeBytes, 1);
				}
			}
			else {
				pBlockToFree->flags = 2;
				pBlockToFree->freeBytes = (int)(pBlockToFree->pStartAddr + (pBlockToFree->freeBytes - (ulong)pParentBlock->pStartAddr));
				pBlockToFree->pStartAddr = pParentBlock->pStartAddr;
				if ((MemoryMasterBlock.flags & 0x20) != 0) {
					edmemSetReadOnly(pBlockToFree);
				}
			}
		}
		else {
			pBlockToFree->flags = 2;
			pBlockStartAddr = pBlockToFree->pStartAddr + -(ulong)(pParentBlock->pStartAddr + pParentBlock->freeBytes);
			if (pBlockStartAddr != (char*)0x0) {
				pBlockToFree->freeBytes = (int)(pBlockStartAddr + pBlockToFree->freeBytes);
				pBlockToFree->pStartAddr = pBlockToFree->pStartAddr + -(ulong)pBlockStartAddr;
			}

			if ((MemoryMasterBlock.flags & 0x20) != 0) {
				edmemSetReadOnly(pBlockToFree);
			}
		}
	}

	PrintBlockInfo("EndFree", pBlockToFree);

	return;
}

char* edmemWorkShrink(S_MAIN_MEMORY_HEADER* pHeap, int newSize)
{
	// Not yet implemented.
	IMPLEMENTATION_GUARD();
	return NULL;
}

bool edmemTestAllocOk(int size, int align, int offset)
{
	bool bSuccess;
	int adjustedOffset;

	ED_MEM_LOG(LogLevel::Info, "edmemTestAllocOk size: 0x{:x}, align: 0x{:x}, offset: 0x{:x}", size, align, offset);

	if (offset == 0) {
		ED_MEM_LOG(LogLevel::Info, "edmemTestAllocOk offset is 0, no need to check");
		bSuccess = true;
	}
	else {
		adjustedOffset = offset;

		if (align != 0) {
			adjustedOffset = offset + (align - offset % align);
			ED_MEM_LOG(LogLevel::Info, "edmemTestAllocOk Adjusted offset to 0x{:x}", adjustedOffset);
		}

		bSuccess = true;
		if (offset << 1 < adjustedOffset + size) {
			/* Allocation impossible : size + align > boundary\n */
			edDebugPrintf(g_szAllocationImpossible_004325f0);
			ED_MEM_LOG(LogLevel::Error, "edmemTestAllocOk Allocation impossible : size + align > boundary");
			bSuccess = false;
		}
	}

	return bSuccess;
}

void edmemFillBlock(uint* pData, int numWords, int isFree)
{
	int nbWordsToWrite;
	uint fillValue;

	ED_MEM_LOG(LogLevel::Info, "edmemFillBlock numWords: 0x{:x}, isFree: {}", numWords, isFree);

	if ((isFree == 0) || ((MemoryMasterBlock.flags & MEM_FLAG_REQUIRES_HEADER) != 0)) {
		if ((isFree == 0) && ((MemoryMasterBlock.flags & MEM_FLAG_LINKED_DEPENDENT_BLOCKS) == 0)) {
			*pData = g_FreeName_00432628;
		}
		else {
			fillValue = g_FreeName_00432628;
			if (isFree != 0) {
				fillValue = g_UsedName_00432630;
			}

			nbWordsToWrite = numWords >> 2;
			if (numWords < 0) {
				nbWordsToWrite = numWords + 3 >> 2;
			}

			for (; nbWordsToWrite != 0; nbWordsToWrite = nbWordsToWrite + -1) {
				*pData = fillValue;
				pData = pData + 1;
			}
		}
	}

	return;
}

void edmemSetReadOnly(S_MAIN_MEMORY_HEADER* pHeap)
{
	if ((pHeap->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
		pHeap->flags = pHeap->flags | 0x20;
	}
	return;
}

int edMemGetAvailable(EHeap heapID)
{
	short sVar1;
	S_MAIN_MEMORY_HEADER* peVar2;
	ushort uVar3;
	int iVar4;

	iVar4 = 0;
	peVar2 = edmemGetMainHeader((void*)heapID);
	uVar3 = peVar2->flags & 4;
	if (((uVar3 == 0) || (uVar3 == 0)) || (peVar2->blockIndex == -1)) {
		iVar4 = peVar2->freeBytes;
	}
	else {
		for (sVar1 = peVar2->freeListHead; sVar1 != -1;
			sVar1 = MemoryMasterBlock.aBlocks[sVar1].nextFreeBlock) {
			iVar4 = iVar4 + MemoryMasterBlock.aBlocks[sVar1].freeBytes;
		}
	}
	return iVar4;
}

void DebugDump(EHeap heapID, long mode)
{
	// Unimplemented.
}

S_MAIN_MEMORY_HEADER* edmemGetFreeMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap, int param_2)
{
	short nextFreeBlock;
	S_MAIN_MEMORY_HEADER* pNewBlock;
	S_MAIN_MEMORY_HEADER* pBlocks;

	nextFreeBlock = MemoryMasterBlock.nextFreeBlock;
	pBlocks = MemoryMasterBlock.aBlocks;

	if (MemoryMasterBlock.nbFreeBlocks == 0) {
		ED_MEM_LOG(LogLevel::Error, "edmemGetFreeMemoryHeader No free blocks available");
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 2, (char*)0x0);
		pNewBlock = (S_MAIN_MEMORY_HEADER*)0x0;
	}
	else {
		MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + -1;

		ED_MEM_LOG(LogLevel::Info, "edmemGetFreeMemoryHeader nextFreeBlock 0x{:x} (nbFreeBlocks: 0x{:x})", nextFreeBlock, MemoryMasterBlock.nbFreeBlocks);

		pNewBlock = MemoryMasterBlock.aBlocks + MemoryMasterBlock.nextFreeBlock;
		MemoryMasterBlock.nextFreeBlock = pNewBlock->freeListHeadAllocatedTo;

		if (MemoryMasterBlock.nbUsedBlocks < (MemoryMasterBlock.nbTotalBlocks - MemoryMasterBlock.nbFreeBlocks)) {
			MemoryMasterBlock.nbUsedBlocks = (MemoryMasterBlock.nbTotalBlocks - MemoryMasterBlock.nbFreeBlocks);
		}

		ED_MEM_LOG(LogLevel::Info, "edmemGetFreeMemoryHeader nbUsedBlocks 0x{:x}", MemoryMasterBlock.nbUsedBlocks);

		pNewBlock->flags = 2;
		pNewBlock->prevFreeBlock = -1;
		pNewBlock->nextFreeBlock = -1;
		pNewBlock->freeListHead = -1;

		if (param_2 == 0) {
			if (pHeap == (S_MAIN_MEMORY_HEADER*)0x0) {
				pNewBlock->freeListHeadAllocatedTo = -1;
				pNewBlock->parentBlockIndex = -1;
			}
			else {
				pNewBlock->freeListHeadAllocatedTo = (short)((uint)((char*)pHeap - (char*)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));
				pNewBlock->parentBlockIndex = pHeap->parentBlockIndex;

				S_MAIN_MEMORY_HEADER* pMasterHeader = pBlocks + pHeap->parentBlockIndex;
				if (pNewBlock->freeListHeadAllocatedTo == pMasterHeader->freeListHeadAllocatedTo) {
					pMasterHeader->freeListHeadAllocatedTo = nextFreeBlock;
				}
				else {
					pMasterHeader->blockIndex = nextFreeBlock;
				}

				pHeap->parentBlockIndex = nextFreeBlock;
			}
		}
		else {
			pNewBlock->freeListHeadAllocatedTo = -1;
			pNewBlock->parentBlockIndex = (short)((uint)((char*)pHeap - (char*)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));

			pHeap->blockIndex = nextFreeBlock;
			pHeap->freeListHead = nextFreeBlock;

			ED_MEM_LOG(LogLevel::Info, "edmemGetFreeMemoryHeader Updating free list head to 0x{:x}", nextFreeBlock);
		}
	}

	PrintBlockInfo("New", pNewBlock);

	return pNewBlock;
}

S_MAIN_MEMORY_HEADER* edmemGetMainHeader(void* heapID)
{
	short sVar1;

	sVar1 = (short)heapID;
	if ((ulong)MemoryMasterBlock.nbTotalBlocks <= (ulong)heapID) {
		sVar1 = *(short*)((ulong)heapID + -0xc);
		ED_MEM_LOG(LogLevel::Info, "edmemGetMainHeader block index: 0x{:x}", sVar1);
	}

	return MemoryMasterBlock.aBlocks + sVar1;
}

S_MAIN_MEMORY_HEADER* edmemGetMasterMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap)
{
	short sVar1;
	S_MAIN_MEMORY_HEADER* peVar2;

	sVar1 = pHeap->parentBlockIndex;
	while (peVar2 = MemoryMasterBlock.aBlocks + sVar1, MemoryMasterBlock.aBlocks + peVar2->freeListHeadAllocatedTo == pHeap) {
		sVar1 = peVar2->parentBlockIndex;
		pHeap = peVar2;
	}
	return peVar2;
}

void edmemFreeMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap)
{
	short sVar1;
	S_MAIN_MEMORY_HEADER* pBlocks;
	short blockIndex;
	S_MAIN_MEMORY_HEADER* peVar4;

	ED_MEM_LOG(LogLevel::Info, "edmemFreeMemoryHeader pHeap: 0x{:x}", (uintptr_t)pHeap);

	pBlocks = MemoryMasterBlock.aBlocks;

	blockIndex = (short)((uint)((int)pHeap - (int)MemoryMasterBlock.aBlocks) / sizeof(S_MAIN_MEMORY_HEADER));

	ED_MEM_LOG(LogLevel::Info, "edmemFreeMemoryHeader blockIndex: 0x{:x}", blockIndex);

	if (pHeap->parentBlockIndex != -1) {
		sVar1 = pHeap->freeListHeadAllocatedTo;
		peVar4 = MemoryMasterBlock.aBlocks + pHeap->parentBlockIndex;
		if (peVar4->blockIndex == blockIndex) {
			peVar4->blockIndex = sVar1;
			if (pHeap->freeListHeadAllocatedTo == -1) {
				peVar4->flags = peVar4->flags & 0xfff7;
			}
		}
		else {
			peVar4->freeListHeadAllocatedTo = sVar1;
		}
		if (pHeap->freeListHeadAllocatedTo == -1) {
			pHeap->parentBlockIndex = 0;
		}
		else {
			pBlocks[sVar1].parentBlockIndex = pHeap->parentBlockIndex;
		}
	}

	pHeap->flags = 0;
	pHeap->freeListHeadAllocatedTo = MemoryMasterBlock.nextFreeBlock;
	pHeap->prevFreeBlock = -1;
	pHeap->nextFreeBlock = -1;
	MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + 1;
	MemoryMasterBlock.nextFreeBlock = blockIndex;

	return;
}

bool edmemMakeMaster(S_MAIN_MEMORY_HEADER* pHeap)
{
	bool bSuccess;
	S_MAIN_MEMORY_HEADER* pFreeHeader;

	if ((pHeap->flags & 0x200) == 0) {
		if ((pHeap->flags & 0x800) == 0) {
			pFreeHeader = edmemGetFreeMemoryHeader(pHeap, 1);

			if (pFreeHeader == (S_MAIN_MEMORY_HEADER*)0x0) {
				bSuccess = false;
			}
			else {
				pHeap->flags = pHeap->flags | MEM_FLAG_LINKED_DEPENDENT_BLOCKS;
				pFreeHeader->pStartAddr = pHeap->pStartAddr;
				pFreeHeader->freeBytes = pHeap->freeBytes;

				if ((pHeap->flags & MEM_FLAG_REQUIRES_HEADER) != 0) {
					edmemFillBlock((uint*)pFreeHeader->pStartAddr, pFreeHeader->freeBytes, 0);
				}

				bSuccess = true;
			}
		}
		else {
			bSuccess = false;
		}
	}
	else {
		bSuccess = false;
	}

	return bSuccess;
}

void edMemSetFlags(EHeap heapID, ushort newFlags)
{
	S_MAIN_MEMORY_HEADER* pHeap;
	ushort flags;

	if (heapID == TO_HEAP(H_INVALID)) {
		/* emSetFlags */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemSetFlags_00432b28);
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		flags = newFlags & 0x85e0;
		if ((newFlags & 0x20) != 0) {
			flags = flags ^ 0x20;
			edmemSetReadOnly(pHeap);
		}
		pHeap->flags = pHeap->flags | flags;
	}
	return;
}

void edMemClearFlags(EHeap heapID, ushort flags)
{
	S_MAIN_MEMORY_HEADER* pHeap;
	ushort newFlags;

	if (heapID == TO_HEAP(H_INVALID)) {
		/* edMemClearFlags */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemClearFlags_00432b40);
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		newFlags = flags & 0x91e0;
		if ((flags & 0x20) != 0) {
			newFlags = 0x1020;
		}
		pHeap->flags = pHeap->flags & ~newFlags;
	}
	return;
}

void memMemoryHeadersBlockInit(void)
{
	short blockIndex;
	S_MAIN_MEMORY_HEADER* pCurBlock;

	pCurBlock = MemoryMasterBlock.aBlocks;
	memset(MemoryMasterBlock.aBlocks, 0, MemoryMasterBlock.nbTotalBlocks * sizeof(S_MAIN_MEMORY_HEADER));

	// Set block index for all blocks.
	for (blockIndex = 0; blockIndex < MemoryMasterBlock.nbTotalBlocks; blockIndex = blockIndex + 1) {
		pCurBlock->freeListHeadAllocatedTo = blockIndex + 1;
		pCurBlock = pCurBlock + 1;
	}

	return;
}

void memMastersInit(void)
{
	// Use the first 4 blocks as the first block for each heap.

	S_MAIN_MEMORY_HEADER* pBlocks = MemoryMasterBlock.aBlocks;

	pBlocks[0].flags = 2;
	pBlocks[0].funcTableID = 0xff;
	pBlocks[0].maxStackLevel = 0xff;
	pBlocks[0].pStartAddr = (char*)0xffffffff;
	pBlocks[0].freeBytes = -1;
	pBlocks[0].parentBlockIndex = -1;
	pBlocks[0].freeListHeadAllocatedTo = -1;
	pBlocks[0].align = -1;
	pBlocks[0].offset = -1;
	pBlocks[0].blockIndex = -1;
	pBlocks[0].freeListHead = -1;
	pBlocks[0].prevFreeBlock = -1;
	pBlocks[0].nextFreeBlock = -1;
	MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + -1;
	MemoryMasterBlock.nbUsedBlocks = MemoryMasterBlock.nbUsedBlocks + 1;

	pBlocks[1].flags = 0x96;
	pBlocks[1].funcTableID = 0;
	pBlocks[1].maxStackLevel = 0;
	pBlocks[1].pStartAddr = (char*)(MemoryMasterBlock.aBlocks + MemoryMasterBlock.nbTotalBlocks);
	int initialAlloc = (int)((char*)pBlocks[1].pStartAddr - (char*)MemoryMasterBlock.aBlocks);
	pBlocks[1].freeBytes = g_MemWorkSizeB - initialAlloc;
	pBlocks[1].parentBlockIndex = -1;
	pBlocks[1].freeListHeadAllocatedTo = 2;
	pBlocks[1].align = g_SystemHeap_0042df0.align;
	pBlocks[1].offset = g_SystemHeap_0042df0.offset;
	pBlocks[1].blockIndex = -1;
	pBlocks[1].freeListHead = -1;
	pBlocks[1].prevFreeBlock = -1;
	pBlocks[1].nextFreeBlock = -1;
	MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + -1;
	MemoryMasterBlock.nbUsedBlocks = MemoryMasterBlock.nbUsedBlocks + 1;

	pBlocks[2].flags = 0x96;
	pBlocks[2].funcTableID = 0;
	pBlocks[2].maxStackLevel = 0;
	pBlocks[2].pStartAddr = g_Heap3_00424e00.startAddress;
	pBlocks[2].freeBytes = g_Heap3_00424e00.size;
	pBlocks[2].parentBlockIndex = 1;
	pBlocks[2].freeListHeadAllocatedTo = 3;
	pBlocks[2].align = g_Heap3_00424e00.align;
	pBlocks[2].offset = g_Heap3_00424e00.offset;
	pBlocks[2].blockIndex = -1;
	pBlocks[2].freeListHead = -1;
	pBlocks[2].prevFreeBlock = -1;
	pBlocks[2].nextFreeBlock = -1;
	MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + -1;
	MemoryMasterBlock.nbUsedBlocks = MemoryMasterBlock.nbUsedBlocks + 1;

	pBlocks[3].flags = 0x96;
	pBlocks[3].funcTableID = 0;
	pBlocks[3].maxStackLevel = 0;
	pBlocks[3].pStartAddr = g_ScratchpadHeap_00424e10.startAddress;
	pBlocks[3].freeBytes = g_ScratchpadHeap_00424e10.size;
	pBlocks[3].parentBlockIndex = 2;
	pBlocks[3].freeListHeadAllocatedTo = 4;
	pBlocks[3].align = g_ScratchpadHeap_00424e10.align;
	pBlocks[3].offset = g_ScratchpadHeap_00424e10.offset;
	pBlocks[3].blockIndex = -1;
	pBlocks[3].freeListHead = -1;
	pBlocks[3].prevFreeBlock = -1;
	pBlocks[3].nextFreeBlock = -1;
	MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + -1;
	MemoryMasterBlock.nbUsedBlocks = MemoryMasterBlock.nbUsedBlocks + 1;

	pBlocks[4].flags = 0x86;
	pBlocks[4].funcTableID = 1;
	pBlocks[4].maxStackLevel = 0;
	pBlocks[4].pStartAddr = g_Heap4_00424e20.startAddress;
	pBlocks[4].freeBytes = g_Heap4_00424e20.size;
	pBlocks[4].parentBlockIndex = 3;
	pBlocks[4].freeListHeadAllocatedTo = -1;
	pBlocks[4].align = g_Heap4_00424e20.align;
	pBlocks[4].offset = g_Heap4_00424e20.offset;
	pBlocks[4].blockIndex = -1;
	pBlocks[4].freeListHead = -1;
	pBlocks[4].prevFreeBlock = -1;
	pBlocks[4].nextFreeBlock = -1;
	MemoryMasterBlock.nbUsedBlocks = MemoryMasterBlock.nbUsedBlocks + 1;
	MemoryMasterBlock.nbFreeBlocks = MemoryMasterBlock.nbFreeBlocks + -1;

	MemoryMasterBlock.nextFreeBlock = MemoryMasterBlock.nbUsedBlocks;
	return;
}

void memHandlersInit(void)
{
	MemoryHandlers[0].alloc = edmemWorkAlloc;
	MemoryHandlers[0].force = edmemWorkAllocForce;
	MemoryHandlers[0].free = edmemWorkFree;
	MemoryHandlers[0].shrink = edmemWorkShrink;
	_edSystemInitVideoHandlers();
	return;
}

void memDebugInformationInit(void)
{
	return;
}


#ifdef PLATFORM_WIN
S_MAIN_MEMORY_HEADER* g_HeapPtr_0040f370 = NULL;
#else
S_MAIN_MEMORY_HEADER* g_HeapPtr_0040f370 = (S_MAIN_MEMORY_HEADER*)0x004a5780;
#endif

S_MAIN_MEMORY_HEADER* edmemGetMainHeader()
{
	return g_HeapPtr_0040f370;
}

void edMemInit(short nbTotalBlocks)
{
	MemoryMasterBlock.nbUsedBlocks = 0;
	MemoryMasterBlock.nextFreeBlock = 0;
	MemoryMasterBlock.stackLevel = 0;
	MemoryMasterBlock.field_0xd = 0;
	MemoryMasterBlock.aBlocks = reinterpret_cast<S_MAIN_MEMORY_HEADER*>(g_SystemHeap_0042df0.startAddress);
	MemoryMasterBlock.nbTotalBlocks = nbTotalBlocks;
	MemoryMasterBlock.nbFreeBlocks = nbTotalBlocks;

	edMemStackLevelPush();
	memMemoryHeadersBlockInit();
	memMastersInit();
	memHandlersInit();
	memDebugInformationInit();
	return;
}

int edMemGetNbAlloc(void)
{
	return (int)MemoryMasterBlock.nbTotalBlocks - (int)MemoryMasterBlock.nbFreeBlocks;
}

int edMemGetNbAllocLeft(void)
{
	return (int)MemoryMasterBlock.nbFreeBlocks;
}

int edMemGetNbMaxAlloc(void)
{
	return (int)MemoryMasterBlock.nbUsedBlocks;
}

void edMemStackLevelPush(void)
{
	MemoryMasterBlock.stackLevel = MemoryMasterBlock.stackLevel + 1;
	return;
}

bool edMemTerm(void)
{
	ushort flags;
	bool bSuccess;
	S_MAIN_MEMORY_HEADER* pHeader;
	short heapID;

	bSuccess = true;
	MemoryMasterBlock.stackLevel = MemoryMasterBlock.stackLevel - 1;
	pHeader = MemoryMasterBlock.aBlocks;
	for (heapID = 0; heapID < MemoryMasterBlock.nbTotalBlocks; heapID = heapID + 1) {
		flags = pHeader->flags;
		if ((((flags & 2) != 0) && ((flags & 4) != 0)) && (MemoryMasterBlock.stackLevel < pHeader->maxStackLevel)) {
			bSuccess = false;
			pHeader->flags = flags | 0x4000;
		}
		pHeader = pHeader + 1;
	}
	return bSuccess;
}

void* edSystemFastRamGetAddr(void)
{
#ifndef PLATFORM_PS2
	static void* g_fakeScratch = malloc(0x10000);
	return g_fakeScratch;
#endif
	return (void*)g_ScratchpadHeap_00424e10.startAddress;
}

// Not in this file.

void* AllocateFunc_001002a0(size_t size)
{
	void* pvVar1;

	pvVar1 = edMemAlloc(TO_HEAP(H_MAIN), size);
	return pvVar1;
}

void* Allocate(long amount)
{
	void* pvVar1;

	pvVar1 = edMemAlloc(TO_HEAP(H_MAIN), (int)amount);
	return pvVar1;
}

void _edSystemInitVideoHandlers(void)
{
	MemoryHandlers[1].alloc = edmemWorkAlloc;
	MemoryHandlers[1].force = edmemWorkAllocForce;
	MemoryHandlers[1].free = edmemWorkFree;
	MemoryHandlers[1].shrink = edmemWorkShrink;
	return;
}