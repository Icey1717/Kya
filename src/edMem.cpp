#include "edMem.h"
#include "edSystem.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <assert.h>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif

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
	struct S_MAIN_MEMORY_HEADER* pHeapMainHeaders;
	short headerCount;
	short freeHeaders;
	short maxHeaders;
	short heapID;
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
	if ((pHeap->flags & 8) == 0) {
		if ((pHeap->flags & 0x200) == 0) {
			freeBytes = pHeap->freeBytes;
		}
		else {
			freeBytes = (int)(pHeap->pStartAddr +
				(pHeap->freeBytes - *(int*)MemoryMasterBlock.pHeapMainHeaders[pHeap->nextFreeBlock].pStartAddr));
		}
	}
	else {
		freeBytes = 0;
		for (nextBlockID = pHeap->nextFreeBlock; nextBlockID != -1;
			nextBlockID = MemoryMasterBlock.pHeapMainHeaders[nextBlockID].nextBlock) {
			iVar2 = MemoryMasterBlock.pHeapMainHeaders[nextBlockID].freeBytes;
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

	if (heapID == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, "edMemAlloc");
		pNewAlloc = (void*)0x0;
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		pNewAlloc = edMemAllocAlignBoundary(heapID, size, pHeap->align, pHeap->offset);
	}
	return pNewAlloc;
}

void* edMemAllocAlign(EHeap heapID, size_t size, int align)
{
	EFileLoadMode fileLoadMode;
	S_MAIN_MEMORY_HEADER* pHeap;
	void* pNewAllocation;

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

	MY_LOG("edMemAlloc id: {}, size: {}, align: {}, offset: {}\n", (int)heap, size, align, offset);

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
			freeMemory = edMemGetAvailable(heap);
			edDebugPrintf(g_szCannotAllocate, size, freeMemory);
		}
	}
	return pNewAllocation;
}

void edMemFree(void* pAlloc)
{
	S_MAIN_MEMORY_HEADER* peVar1;

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
	int iVar2;
	bool bVar3;
	uint pcVar8;
	char* pcVar4;
	short sVar5;
	char* pcVar6;
	char* pcVar7;
	char* pcVar9;
	S_MAIN_MEMORY_HEADER* pHeap;
	char* pReturn;
	short uVar11;
	ushort uVar10;
	ushort uVar13;
	char* local_10;
	short uVar12;
	S_MAIN_MEMORY_HEADER* heapArray;
	S_MAIN_MEMORY_HEADER* pNewHeap;

	heapArray = MemoryMasterBlock.pHeapMainHeaders;
	if (((pMainMemHeader->flags & 2) == 0) || ((pMainMemHeader->flags & 4) == 0)) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 3, pMainMemHeader->pStartAddr);
	}
	if ((pMainMemHeader->flags & 0x80) == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 4, pMainMemHeader->pStartAddr);
		pReturn = (char*)0x0;
	}
	else {
		if ((size & 0xfU) != 0) {
			size = size + (0x10U - size & 0xf);
		}
		if ((pMainMemHeader->align == 0) || (align % pMainMemHeader->align == 0)) {
			if ((pMainMemHeader->offset == 0) || (offset <= (int)pMainMemHeader->offset)) {
				bVar3 = edmemTestAllocOk(size, align, offset);
				if (bVar3 == false) {
					pReturn = (char*)0x0;
				}
				else {
					if (((pMainMemHeader->flags & 8) == 0) && (bVar3 = edmemMakeMaster(pMainMemHeader), bVar3 == false)) {
						pReturn = (char*)0x0;
					}
					else {
						uVar12 = pMainMemHeader->nextFreeBlock;
						uVar13 = pMainMemHeader->flags;
						pcVar8 = 0x7fffffff;
						pHeap = heapArray + uVar12;
						uVar11 = -1;
						pNewHeap = pHeap;
						if ((uVar13 & 0x100) == 0) {
							while (uVar12 != -1) {
								pNewHeap = heapArray + uVar12;
								pcVar7 = (char*)pNewHeap->freeBytes;
								uint freeBytes = pNewHeap->freeBytes;
								// ???? This comparison fails on release?
								bool a = true; //freeBytes < pcVar8
								bool b = size <= freeBytes;

								MY_LOG("{} {}", a, b);

								if (a && b) {
									pcVar4 = pNewHeap->pStartAddr;
									pcVar9 = pcVar4;
									if ((uVar13 & 0x10) != 0) {
										pcVar9 = pcVar4 + 0x10;
									}
									if ((align != 0) && ((ulong)pcVar9 % align != 0)) {
										pcVar9 = pcVar9 + (align - (ulong)pcVar9 % align);
									}
									pcVar6 = pcVar9 + size;
									if (offset != 0) {
										if (((ulong)pcVar9 / offset != (ulong)pcVar6 / offset) &&
											(pcVar9 = pcVar9 + (offset - (ulong)pcVar9 % offset), align != 0)) {
											pcVar6 = pcVar9 + size;
											if ((ulong)pcVar9 % align == 0) goto LAB_0028e378;
											pcVar9 = pcVar9 + (align - (ulong)pcVar9 % align);
										}
										pcVar6 = pcVar9 + size;
									}
								LAB_0028e378:
									if (((ulong)pcVar6 + -((ulong)pcVar4)) <= freeBytes) {
										pHeap = pNewHeap;
										uVar11 = uVar12;
										pcVar8 = freeBytes;
										pReturn = pcVar9;
									}
								}
								uVar12 = pNewHeap->nextBlock;
							}
						}
						else {
							while (uVar10 = pNewHeap->nextBlock, uVar10 != 0xffff) {
								uVar12 = uVar10;
								pNewHeap = heapArray + (short)uVar10;
							}
							while ((pNewHeap != pMainMemHeader && (uVar11 == -1))) {
								if (size < pNewHeap->freeBytes) {
									pcVar7 = pNewHeap->pStartAddr + (pNewHeap->freeBytes - size);
									if ((align != 0) && ((ulong)pcVar7 % align != 0)) {
										pcVar7 = pcVar7 + -((ulong)pcVar7 % align);
									}
									if ((((offset != 0) && ((ulong)pcVar7 / offset != (ulong)(pcVar7 + size) / offset)) &&
										(pcVar7 = pcVar7 + -((ulong)pcVar7 % offset), align != 0)) && ((ulong)pcVar7 % align != 0)) {
										pcVar7 = pcVar7 + -((ulong)pcVar7 % align);
									}
									pcVar4 = pcVar7;
									if ((uVar13 & 0x10) != 0) {
										pcVar4 = pcVar7 + -0x10;
									}
									if (pNewHeap->pStartAddr <= pcVar4) {
										pHeap = pNewHeap;
										uVar11 = uVar12;
										pReturn = pcVar7;
									}
								}
								uVar12 = pNewHeap->field_0x2;
								pNewHeap = heapArray + uVar12;
							}
						}
						if (uVar11 == -1) {
							edDebugPrintf(sz_CannotAllocate_004325c0, size);
							CallHandlerFunction(&edSysHandlerMemory_004890c0, 0, pMainMemHeader->pStartAddr);
							pReturn = (char*)0x0;
						}
						else {
							pcVar7 = pHeap->pStartAddr;
							sVar1 = pHeap->field_0x2;
							iVar2 = pHeap->freeBytes;
							uVar13 = pHeap->nextBlock;
							ulong testValue = (ulong)pcVar7 + (iVar2 - (ulong)(pReturn + size));
							MY_LOG("edmemWorkAlloc test 0x{:x} {} 0x{:x} {:x} {}", (uintptr_t)pcVar7, iVar2, (uintptr_t)pReturn, size, testValue);
							if (testValue == 0) {
								if (sVar1 == -1) {
									MY_LOG("edmemWorkAlloc nextFreeBlock {}", uVar13);
									pMainMemHeader->nextFreeBlock = uVar13;
								}
								else {
									heapArray[sVar1].nextBlock = uVar13;
								}
								uVar10 = uVar11;
								if (uVar13 != -1) {
									heapArray[uVar13].field_0x2 = sVar1;
								}
							}
							else {
								pNewHeap = edmemGetFreeMemoryHeader(pHeap, 0);
								if (pNewHeap == (S_MAIN_MEMORY_HEADER*)0x0) {
									return (char*)0x0;
								}
								pHeap->freeBytes = (int)(pcVar7 + (iVar2 - (ulong)(pReturn + size)));
								pHeap->pStartAddr = pReturn + size;
								if ((pMainMemHeader->flags & 0x10) != 0) {
									edmemFillBlock((uint*)pHeap->pStartAddr, pHeap->freeBytes, 0);
								}
								if ((MemoryMasterBlock.flags & 0x20) != 0) {
									edmemSetReadOnly(pHeap);
								}
								pHeap = pNewHeap;
								uVar10 = (ushort)((uint)((ulong)pNewHeap - (ulong)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
								uVar13 = uVar11;
							}
							pHeap->flags = pMainMemHeader->flags & 0x10 | 0x86;
							pHeap->funcTableID = pMainMemHeader->funcTableID;
							pHeap->maxStackLevel = MemoryMasterBlock.stackLevel;
							pHeap->pStartAddr = pReturn;
							pHeap->freeBytes = size;
							pHeap->align = align;
							pHeap->offset = offset;
							pHeap->field_0x1c = -1;
							pHeap->field_0x2 = -1;
							pHeap->nextBlock = -1;
							local_10 = pReturn;
							if ((pMainMemHeader->flags & 0x10) != 0) {
								*(undefined4*)(pReturn + -0x10) = g_EdenName_004325d8;
								*(undefined4*)(pReturn + -4) = g_EdenName_004325d8;
								*(ushort*)(pReturn + -0xc) = uVar10;
								*(undefined2*)(pReturn + -10) = 0;
								*(undefined4*)(pReturn + -8) = 0;
								edmemFillBlock((uint*)pHeap->pStartAddr, pHeap->freeBytes, 1);
								local_10 = pReturn + -0x10;
							}
							if (((ulong)(short)pMainMemHeader->flags & 0x8000U) != 0) {
								pReturn = (char*)((uint)pReturn & 0xdfffffff);
							}
							if (local_10 + -(ulong)pcVar7 != (char*)0x0) {
								pNewHeap = edmemGetFreeMemoryHeader(pHeap, 0);
								if (pNewHeap == (S_MAIN_MEMORY_HEADER*)0x0) {
									return (char*)0x0;
								}
								pNewHeap->flags = 2;
								pNewHeap->pStartAddr = pcVar7;
								pNewHeap->freeBytes = (ulong)(local_10 + -(ulong)pcVar7);
								sVar5 = (short)((uint)((ulong)pNewHeap - (ulong)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
								if (sVar1 == -1) {
									MY_LOG("Marking B {}", sVar5);
									pMainMemHeader->nextFreeBlock = sVar5;
								}
								else {
									heapArray[sVar1].nextBlock = sVar5;
								}
								if (uVar13 != -1) {
									heapArray[uVar13].field_0x2 = sVar5;
								}
								pNewHeap->field_0x2 = sVar1;
								pNewHeap->nextBlock = uVar13;
								if ((pMainMemHeader->flags & 0x10) != 0) {
									edmemFillBlock((uint*)pNewHeap->pStartAddr, pNewHeap->freeBytes, 0);
								}
								if ((MemoryMasterBlock.flags & 0x20) != 0) {
									edmemSetReadOnly(pNewHeap);
								}
							}
							if ((pMainMemHeader->flags & 0x10) == 0) {
								pReturn = (char*)(int)(short)((uint)((ulong)pHeap - (ulong)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
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
	return pReturn;
}

void* edmemWorkAllocForce(S_MAIN_MEMORY_HEADER* param_1, void* addr)
{
	IMPLEMENTATION_GUARD();
	// Unused.
	return NULL;
}

void edmemWorkFree(S_MAIN_MEMORY_HEADER* pHeap)
{
	bool bVar1;
	short sVar2;
	S_MAIN_MEMORY_HEADER* peVar3;
	ushort uVar4;
	S_MAIN_MEMORY_HEADER* peVar5;
	S_MAIN_MEMORY_HEADER* peVar6;
	char* pcVar7;
	short unaff_s0_lo;
	short unaff_s1_lo;
	short sVar8;
	long lVar9;
	char cVar10;

	peVar3 = MemoryMasterBlock.pHeapMainHeaders;
	if ((pHeap->flags & 4) != 0) {
		pcVar7 = pHeap->pStartAddr;
		if ((pHeap->flags & 0x10) != 0) {
			*(undefined4*)(pcVar7 + -0x10) = g_FreeName_004325e0;
			*(undefined4*)(pcVar7 + -4) = g_FreeName_004325e0;
		}
		if ((pHeap->flags & 8) != 0) {
			sVar8 = pHeap->field_0x1c;
			while (sVar8 != -1) {
				peVar5 = peVar3 + sVar8;
				if ((peVar5->flags & 4) == 0) {
					edmemFreeMemoryHeader(peVar5);
				}
				else {
					edmemWorkFree(peVar5);
				}
				sVar8 = pHeap->field_0x1c;
			}
		}
		lVar9 = (long)pHeap->field_0x1c;
		if (lVar9 != -1) {
			while (lVar9 != -1) {
				sVar8 = peVar3[lVar9].heapID;
				edmemFreeMemoryHeader(peVar3 + lVar9);
				lVar9 = (long)sVar8;
			}
		}
		if ((pHeap->flags & 0x200) != 0) {
			edmemWorkFree(peVar3 + pHeap->nextFreeBlock);
		}
		if ((pHeap->flags & 0x10) != 0) {
			edmemFillBlock((uint*)(pHeap->pStartAddr + -0x10), pHeap->freeBytes + 0x10, 0);
		}
		pHeap->flags = 2;
		peVar5 = peVar3 + pHeap->field_0xa;
		sVar8 = (short)((uint)((ulong)pHeap - (ulong)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
		bVar1 = (peVar5->flags & 4) != 0;
		if (bVar1) {
			sVar2 = pHeap->heapID;
		}
		else {
			pHeap->freeBytes = (int)(pHeap->pStartAddr + (pHeap->freeBytes - (ulong)peVar5->pStartAddr));
			pHeap->pStartAddr = peVar5->pStartAddr;
			unaff_s1_lo = peVar5->field_0x2;
			unaff_s0_lo = peVar5->nextBlock;
			edmemFreeMemoryHeader(peVar5);
			sVar2 = pHeap->heapID;
		}
		cVar10 = !bVar1;
		if (sVar2 != -1) {
			peVar5 = peVar3 + sVar2;
			if ((peVar5->flags & 4) == 0) {
				pHeap->freeBytes = (int)(peVar5->pStartAddr + (peVar5->freeBytes - (ulong)pHeap->pStartAddr));
				if (!(bool)cVar10) {
					unaff_s1_lo = peVar5->field_0x2;
				}
				unaff_s0_lo = peVar5->nextBlock;
				cVar10 = cVar10 + '\x01';
				edmemFreeMemoryHeader(peVar5);
			}
			else {
				if (!(bool)cVar10) {
					uVar4 = peVar5->flags;
					while (((uVar4 & 4) != 0 && (peVar5->heapID != -1))) {
						peVar5 = peVar3 + peVar5->heapID;
						uVar4 = peVar5->flags;
					}
					if ((uVar4 & 4) == 0) {
						unaff_s1_lo = peVar5->field_0x2;
						cVar10 = '\x01';
						unaff_s0_lo = (ushort)((uint)((ulong)peVar5 - (ulong)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
					}
				}
			}
		}
		if (cVar10 == '\0') {
			sVar2 = pHeap->field_0xa;
			peVar5 = pHeap;
			while (true) {
				peVar6 = peVar3 + sVar2;
				uVar4 = peVar6->flags & 4;
				if ((uVar4 == 0) || (peVar3 + peVar6->field_0x1c == peVar5)) break;
				sVar2 = peVar6->field_0xa;
				peVar5 = peVar6;
			}
			unaff_s1_lo = 0xffff;
			unaff_s0_lo = unaff_s1_lo;
			if (uVar4 == 0) {
				unaff_s1_lo = (ushort)((uint)((ulong)peVar6 - (ulong)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
				unaff_s0_lo = peVar6->nextBlock;
			}
		}
		if (unaff_s1_lo == -1) {
			peVar5 = edmemGetMasterMemoryHeader(pHeap);
			MY_LOG("Marking C {}", sVar8);
			peVar5->nextFreeBlock = sVar8;
		}
		else {
			peVar3[unaff_s1_lo].nextBlock = sVar8;
		}
		if (unaff_s1_lo != -1) {
			peVar3[unaff_s1_lo].field_0x2 = sVar8;
		}
		pHeap->field_0x2 = unaff_s1_lo;
		pHeap->nextBlock = unaff_s0_lo;
		peVar5 = peVar3 + pHeap->field_0xa;
		if (peVar3 + peVar5->field_0x1c == pHeap) {
			if (pHeap->heapID == -1) {
				edmemFreeMemoryHeader(pHeap);
				if ((peVar5->flags & 0x10) != 0) {
					edmemFillBlock((uint*)peVar5->pStartAddr, peVar5->freeBytes, 1);
				}
			}
			else {
				pHeap->flags = 2;
				pHeap->freeBytes = (int)(pHeap->pStartAddr + (pHeap->freeBytes - (ulong)peVar5->pStartAddr));
				pHeap->pStartAddr = peVar5->pStartAddr;
				if ((MemoryMasterBlock.flags & 0x20) != 0) {
					edmemSetReadOnly(pHeap);
				}
			}
		}
		else {
			pHeap->flags = 2;
			pcVar7 = pHeap->pStartAddr + -(ulong)(peVar5->pStartAddr + peVar5->freeBytes);
			if (pcVar7 != (char*)0x0) {
				pHeap->freeBytes = (int)(pcVar7 + pHeap->freeBytes);
				pHeap->pStartAddr = pHeap->pStartAddr + -(ulong)pcVar7;
			}
			if ((MemoryMasterBlock.flags & 0x20) != 0) {
				edmemSetReadOnly(pHeap);
			}
		}
	}
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
	bool bVar1;
	int iVar2;

	if (offset == 0) {
		bVar1 = true;
	}
	else {
		iVar2 = offset;
		if (align != 0) {
			iVar2 = offset + (align - offset % align);
		}
		bVar1 = true;
		if (offset << 1 < iVar2 + size) {
			/* Allocation impossible : size + align > boundary\n */
			edDebugPrintf(g_szAllocationImpossible_004325f0);
			bVar1 = false;
		}
	}
	return bVar1;
}

void edmemFillBlock(uint* data, int numWords, int isFree)
{
	int iVar1;
	uint uVar2;

	if ((isFree == 0) || ((MemoryMasterBlock.flags & 0x10) != 0)) {
		if ((isFree == 0) && ((MemoryMasterBlock.flags & 8) == 0)) {
			*data = g_FreeName_00432628;
		}
		else {
			uVar2 = g_FreeName_00432628;
			if (isFree != 0) {
				uVar2 = g_UsedName_00432630;
			}
			iVar1 = numWords >> 2;
			if (numWords < 0) {
				iVar1 = numWords + 3 >> 2;
			}
			for (; iVar1 != 0; iVar1 = iVar1 + -1) {
				*data = uVar2;
				data = data + 1;
			}
		}
	}
	return;
}

void edmemSetReadOnly(S_MAIN_MEMORY_HEADER* pHeap)
{
	if ((pHeap->flags & 0x10) != 0) {
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
	if (((uVar3 == 0) || (uVar3 == 0)) || (peVar2->field_0x1c == -1)) {
		iVar4 = peVar2->freeBytes;
	}
	else {
		for (sVar1 = peVar2->nextFreeBlock; sVar1 != -1;
			sVar1 = MemoryMasterBlock.pHeapMainHeaders[sVar1].nextBlock) {
			iVar4 = iVar4 + MemoryMasterBlock.pHeapMainHeaders[sVar1].freeBytes;
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
	short sVar1;
	S_MAIN_MEMORY_HEADER* peVar2;
	S_MAIN_MEMORY_HEADER* peVar3;

	sVar1 = MemoryMasterBlock.heapID;
	peVar3 = MemoryMasterBlock.pHeapMainHeaders;
	if (MemoryMasterBlock.freeHeaders == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 2, (char*)0x0);
		peVar2 = (S_MAIN_MEMORY_HEADER*)0x0;
	}
	else {
		MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + -1;
		peVar2 = MemoryMasterBlock.pHeapMainHeaders + MemoryMasterBlock.heapID;
		MemoryMasterBlock.heapID = peVar2->heapID;
		if ((long)MemoryMasterBlock.maxHeaders < (long)((int)MemoryMasterBlock.headerCount - (int)MemoryMasterBlock.freeHeaders)) {
			MemoryMasterBlock.maxHeaders = (short)((int)MemoryMasterBlock.headerCount - (int)MemoryMasterBlock.freeHeaders);
		}
		peVar2->flags = 2;
		peVar2->field_0x2 = 0xffff;
		peVar2->nextBlock = 0xffff;

		MY_LOG("edmemGetFreeMemoryHeader nextFreeBlock {}", -1);
		peVar2->nextFreeBlock = -1;
		if (param_2 == 0) {
			if (pHeap == (S_MAIN_MEMORY_HEADER*)0x0) {
				peVar2->heapID = 0xffff;
				peVar2->field_0xa = 0xffff;
			}
			else {
				peVar2->heapID = (short)((uint)((char*)pHeap - (char*)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
				peVar2->field_0xa = pHeap->field_0xa;
				peVar3 = peVar3 + (short)pHeap->field_0xa;
				if (peVar2->heapID == peVar3->heapID) {
					peVar3->heapID = sVar1;
				}
				else {
					peVar3->field_0x1c = sVar1;
				}
				pHeap->field_0xa = sVar1;
			}
		}
		else {
			peVar2->heapID = 0xffff;
			peVar2->field_0xa = (short)((uint)((char*)pHeap - (char*)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
			pHeap->field_0x1c = sVar1;

			MY_LOG("edmemGetFreeMemoryHeader nextFreeBlock {}", sVar1);
			pHeap->nextFreeBlock = sVar1;
		}
	}
	return peVar2;
}

S_MAIN_MEMORY_HEADER* edmemGetMainHeader(void* heapID)
{
	short sVar1;

	sVar1 = (short)heapID;
	if ((ulong)MemoryMasterBlock.headerCount <= (ulong)heapID) {
		sVar1 = *(short*)((ulong)heapID + -0xc);
	}
	return MemoryMasterBlock.pHeapMainHeaders + sVar1;
}

S_MAIN_MEMORY_HEADER* edmemGetMasterMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap)
{
	short sVar1;
	S_MAIN_MEMORY_HEADER* peVar2;

	sVar1 = pHeap->field_0xa;
	while (peVar2 = MemoryMasterBlock.pHeapMainHeaders + sVar1, MemoryMasterBlock.pHeapMainHeaders + peVar2->heapID == pHeap) {
		sVar1 = peVar2->field_0xa;
		pHeap = peVar2;
	}
	return peVar2;
}

void edmemFreeMemoryHeader(S_MAIN_MEMORY_HEADER* pHeap)
{
	short sVar1;
	S_MAIN_MEMORY_HEADER* peVar2;
	short sVar3;
	S_MAIN_MEMORY_HEADER* peVar4;

	peVar2 = MemoryMasterBlock.pHeapMainHeaders;
	sVar3 = (short)((uint)((int)pHeap - (int)MemoryMasterBlock.pHeapMainHeaders) / sizeof(S_MAIN_MEMORY_HEADER));
	if (pHeap->field_0xa != -1) {
		sVar1 = pHeap->heapID;
		peVar4 = MemoryMasterBlock.pHeapMainHeaders + pHeap->field_0xa;
		if (peVar4->field_0x1c == sVar3) {
			peVar4->field_0x1c = sVar1;
			if (pHeap->heapID == -1) {
				peVar4->flags = peVar4->flags & 0xfff7;
			}
		}
		else {
			peVar4->heapID = sVar1;
		}
		if (pHeap->heapID == -1) {
			pHeap->field_0xa = 0;
		}
		else {
			peVar2[sVar1].field_0xa = pHeap->field_0xa;
		}
	}
	pHeap->flags = 0;
	pHeap->heapID = MemoryMasterBlock.heapID;
	pHeap->field_0x2 = -1;
	pHeap->nextBlock = -1;
	MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + 1;
	MemoryMasterBlock.heapID = sVar3;
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
				pHeap->flags = pHeap->flags | 8;
				pFreeHeader->pStartAddr = pHeap->pStartAddr;
				pFreeHeader->freeBytes = pHeap->freeBytes;
				if ((pHeap->flags & 0x10) != 0) {
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
	short sVar1;
	S_MAIN_MEMORY_HEADER* peVar2;

	peVar2 = MemoryMasterBlock.pHeapMainHeaders;
	memset((char*)MemoryMasterBlock.pHeapMainHeaders, 0, MemoryMasterBlock.headerCount * sizeof(S_MAIN_MEMORY_HEADER));
	for (sVar1 = 0; sVar1 < MemoryMasterBlock.headerCount; sVar1 = sVar1 + 1) {
		peVar2->heapID = sVar1 + 1;
		peVar2 = peVar2 + 1;
	}

#ifdef PLATFORM_WIN
	// Poison the end of the heap so we can see it in memory
	//memset(&MemoryMasterBlock.pHeapMainHeaders[0xfff], 0x5, sizeof(S_MAIN_MEMORY_HEADER));
#endif

	return;
}

void memMastersInit(void)
{
	S_MAIN_MEMORY_HEADER* pHeader;

	pHeader = MemoryMasterBlock.pHeapMainHeaders;

	pHeader->flags = 2;
	pHeader->funcTableID = 0xff;
	pHeader->maxStackLevel = 0xff;
	pHeader->pStartAddr = (char*)0xffffffff;
	pHeader->freeBytes = -1;
	pHeader->field_0xa = 0xffff;
	pHeader->heapID = 0xffff;
	pHeader->align = -1;
	pHeader->offset = 0xffffffff;
	pHeader->field_0x1c = -1;
	pHeader->nextFreeBlock = -1;
	pHeader->field_0x2 = 0xffff;
	pHeader->nextBlock = 0xffff;
	MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + -1;
	MemoryMasterBlock.maxHeaders = MemoryMasterBlock.maxHeaders + 1;

	pHeader[1].flags = 0x96;
	pHeader[1].funcTableID = 0;
	pHeader[1].maxStackLevel = 0;
	pHeader[1].pStartAddr = (char*)(MemoryMasterBlock.pHeapMainHeaders + MemoryMasterBlock.headerCount);
	int initialAlloc = (int)((char*)pHeader[1].pStartAddr - (char*)MemoryMasterBlock.pHeapMainHeaders);
	pHeader[1].freeBytes = g_MemWorkSizeB - initialAlloc;
	pHeader[1].field_0xa = 0xffff;
	pHeader[1].heapID = 2;
	pHeader[1].align = g_SystemHeap_0042df0.align;
	pHeader[1].offset = g_SystemHeap_0042df0.offset;
	pHeader[1].field_0x1c = -1;
	pHeader[1].nextFreeBlock = -1;
	pHeader[1].field_0x2 = 0xffff;
	pHeader[1].nextBlock = 0xffff;
	MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + -1;
	MemoryMasterBlock.maxHeaders = MemoryMasterBlock.maxHeaders + 1;

	pHeader[2].flags = 0x96;
	pHeader[2].funcTableID = 0;
	pHeader[2].maxStackLevel = 0;
	pHeader[2].pStartAddr = g_Heap3_00424e00.startAddress;
	pHeader[2].freeBytes = g_Heap3_00424e00.size;
	pHeader[2].field_0xa = 1;
	pHeader[2].heapID = 3;
	pHeader[2].align = g_Heap3_00424e00.align;
	pHeader[2].offset = g_Heap3_00424e00.offset;
	pHeader[2].field_0x1c = -1;
	pHeader[2].nextFreeBlock = -1;
	pHeader[2].field_0x2 = 0xffff;
	pHeader[2].nextBlock = 0xffff;
	MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + -1;
	MemoryMasterBlock.maxHeaders = MemoryMasterBlock.maxHeaders + 1;

	pHeader[3].flags = 0x96;
	pHeader[3].funcTableID = 0;
	pHeader[3].maxStackLevel = 0;
	pHeader[3].pStartAddr = g_ScratchpadHeap_00424e10.startAddress;
	pHeader[3].freeBytes = g_ScratchpadHeap_00424e10.size;
	pHeader[3].field_0xa = 2;
	pHeader[3].heapID = 4;
	pHeader[3].align = g_ScratchpadHeap_00424e10.align;
	pHeader[3].offset = g_ScratchpadHeap_00424e10.offset;
	pHeader[3].field_0x1c = -1;
	pHeader[3].nextFreeBlock = -1;
	pHeader[3].field_0x2 = 0xffff;
	pHeader[3].nextBlock = 0xffff;
	MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + -1;
	MemoryMasterBlock.maxHeaders = MemoryMasterBlock.maxHeaders + 1;

	pHeader[4].flags = 0x86;
	pHeader[4].funcTableID = 1;
	pHeader[4].maxStackLevel = 0;
	pHeader[4].pStartAddr = g_Heap4_00424e20.startAddress;
	pHeader[4].freeBytes = g_Heap4_00424e20.size;
	pHeader[4].field_0xa = 3;
	pHeader[4].heapID = 0xffff;
	pHeader[4].align = g_Heap4_00424e20.align;
	pHeader[4].offset = g_Heap4_00424e20.offset;
	pHeader[4].field_0x1c = -1;
	pHeader[4].nextFreeBlock = -1;
	pHeader[4].field_0x2 = 0xffff;
	pHeader[4].nextBlock = 0xffff;
	MemoryMasterBlock.maxHeaders = MemoryMasterBlock.maxHeaders + 1;
	MemoryMasterBlock.freeHeaders = MemoryMasterBlock.freeHeaders + -1;

	MemoryMasterBlock.heapID = MemoryMasterBlock.maxHeaders;
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

void edMemInit(short headerCount)
{
	MemoryMasterBlock.maxHeaders = 0;
	MemoryMasterBlock.heapID = 0;
	MemoryMasterBlock.stackLevel = 0;
	MemoryMasterBlock.field_0xd = 0;
	MemoryMasterBlock.pHeapMainHeaders = (S_MAIN_MEMORY_HEADER*)g_SystemHeap_0042df0.startAddress;
	MemoryMasterBlock.headerCount = headerCount;
	MemoryMasterBlock.freeHeaders = headerCount;

	edMemStackLevelPush();
	memMemoryHeadersBlockInit();
	memMastersInit();
	memHandlersInit();
	memDebugInformationInit();
	return;
}

int edMemGetNbAlloc(void)
{
	return (int)MemoryMasterBlock.headerCount - (int)MemoryMasterBlock.freeHeaders;
}

int edMemGetNbAllocLeft(void)
{
	return (int)MemoryMasterBlock.freeHeaders;
}

int edMemGetNbMaxAlloc(void)
{
	return (int)MemoryMasterBlock.maxHeaders;
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
	pHeader = MemoryMasterBlock.pHeapMainHeaders;
	for (heapID = 0; heapID < MemoryMasterBlock.headerCount; heapID = heapID + 1) {
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