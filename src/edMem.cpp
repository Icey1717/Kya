#include "edMem.h"
#include "edSystem.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <assert.h>

struct HeapFuncTable
{
	char* (*alloc)(edHeapEntry*, int, int, int);
	void* (*field_0x4)(edHeapEntry*, void*);
	void (*free)(edHeapEntry*);
	char* (*shrink)(edHeapEntry*, int);
};

HeapFuncTable g_HeapFunctionTableArray_00489030[2];

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

edHeapEntry* MemoryMasterBlock = NULL;
short g_TotalHeaders_00424d64 = 0;
short g_FreeHeaders_00424d66 = 0;
short g_MaxHeadersUsed_00424d68 = 0;
short g_MaxHeadersUsed_00424d6a = 0;
byte BYTE_00424d6c = 0;
byte BYTE_00424d6d = 0;
byte BYTE_00424d6e = 1;

ushort USHORT_00424d76 = 0;

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
	edHeapEntry* pHeap;
	int freeBytes;

	// Might wanna check this is actually working.
	assert(false);

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
				(pHeap->freeBytes - *(int*)MemoryMasterBlock[pHeap->nextFreeBlock].pStartAddr));
		}
	}
	else {
		freeBytes = 0;
		for (nextBlockID = pHeap->nextFreeBlock; nextBlockID != -1;
			nextBlockID = MemoryMasterBlock[nextBlockID].nextBlock) {
			iVar2 = MemoryMasterBlock[nextBlockID].freeBytes;
			if (freeBytes < iVar2) {
				freeBytes = iVar2;
			}
		}
	}
	return freeBytes;
}

void* edMemGetBlockAddress(void* pAlloc)
{
	edHeapEntry* pHeap;
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
	edHeapEntry* pHeap;
	void* pNewAlloc;

	if (heapID == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, "edMemAlloc");
		pNewAlloc = (void*)0x0;
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		pNewAlloc = edMemAlloc(heapID, size, pHeap->align, pHeap->offset);
	}
	return pNewAlloc;
}

void* edMemAllocAlign(EHeap heapID, size_t size, int align)
{
	EFileLoadMode fileLoadMode;
	edHeapEntry* pHeap;
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
		pNewAllocation = edMemAlloc(heapID, size, align, pHeap->offset);
	}
	return pNewAllocation;
}

void* edMemAlloc(EHeap heap, size_t size, int align, int offset)
{
	EFileLoadMode fileLoadMode;
	edHeapEntry* pHeap;
	void* pNewAllocation;
	int freeMemory;

	MY_LOG("edMemAlloc id: %d, size: %d, align: %d, offset: %d\n", heap, size, align, offset);

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
		pNewAllocation = (*g_HeapFunctionTableArray_00489030[(char)pHeap->funcTableID].alloc)(pHeap, (int)size, align, offset);
		if (pNewAllocation == (void*)0x0) {
			freeMemory = edMemGetAvailable(heap);
			edDebugPrintf(g_szCannotAllocate, size, freeMemory);
		}
	}
	return pNewAllocation;
}

void edMemFree(void* pAlloc)
{
	edHeapEntry* peVar1;

	if (pAlloc == (void*)0x0) {
		/* edMemFree */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemFree_00432598);
	}
	else {
		peVar1 = edmemGetMainHeader(pAlloc);
		(*g_HeapFunctionTableArray_00489030[(char)peVar1->funcTableID].free)(peVar1);
	}
	return;
}

void* edMemShrink(void* pAlloc, int size)
{
	edHeapEntry* pHeap;
	void* pNewAllocation;

	if (pAlloc == (void*)0x0) {
		/* EdMemShrink */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemShrink_004325a8);
		pNewAllocation = (void*)0x0;
	}
	else {
		pHeap = edmemGetMainHeader(pAlloc);
		pNewAllocation = (*g_HeapFunctionTableArray_00489030[(char)pHeap->funcTableID].shrink)(pHeap, size);
	}
	return pNewAllocation;
}

char* Alloc_0028e140(edHeapEntry* pInHeap, int size, int align, int offset)
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
	edHeapEntry* pHeap;
	char* pReturn;
	short uVar11;
	ushort uVar10;
	ushort uVar13;
	char* local_10;
	short uVar12;
	edHeapEntry* heapArray;
	edHeapEntry* pNewHeap;

	heapArray = MemoryMasterBlock;
	if (((pInHeap->flags & 2) == 0) || ((pInHeap->flags & 4) == 0)) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 3, pInHeap->pStartAddr);
	}
	if ((pInHeap->flags & 0x80) == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 4, pInHeap->pStartAddr);
		pReturn = (char*)0x0;
	}
	else {
		if ((size & 0xfU) != 0) {
			size = size + (0x10U - size & 0xf);
		}
		if ((pInHeap->align == 0) || (align % pInHeap->align == 0)) {
			if ((pInHeap->offset == 0) || (offset <= (int)pInHeap->offset)) {
				bVar3 = IsAllocationValid_0028f3d0(size, align, offset);
				if (bVar3 == false) {
					pReturn = (char*)0x0;
				}
				else {
					if (((pInHeap->flags & 8) == 0) && (bVar3 = AllocateNewBlock_0028fd60(pInHeap), bVar3 == false)) {
						pReturn = (char*)0x0;
					}
					else {
						uVar12 = pInHeap->nextFreeBlock;
						uVar13 = pInHeap->flags;
						pcVar8 = 0x7fffffff;
						pHeap = heapArray + uVar12;
						uVar11 = -1;
						pNewHeap = pHeap;
						if ((uVar13 & 0x100) == 0) {
							while (uVar12 != -1) {
								pNewHeap = heapArray + uVar12;
								pcVar7 = (char*)pNewHeap->freeBytes;
								if (((uint)pcVar7 < pcVar8) && ((uint)size <= (uint)pcVar7)) {
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
									if (pcVar6 + -(ulong)pcVar4 <= pcVar7) {
										pHeap = pNewHeap;
										uVar11 = uVar12;
										pcVar8 = (uint)pcVar7;
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
							while ((pNewHeap != pInHeap && (uVar11 == -1))) {
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
							CallHandlerFunction(&edSysHandlerMemory_004890c0, 0, pInHeap->pStartAddr);
							pReturn = (char*)0x0;
						}
						else {
							pcVar7 = pHeap->pStartAddr;
							sVar1 = pHeap->field_0x2;
							iVar2 = pHeap->freeBytes;
							uVar13 = pHeap->nextBlock;
							char* testValue = pcVar7 + (iVar2 - (ulong)(pReturn + size));
							if (testValue == (char*)0x0) {
								if (sVar1 == -1) {
									pInHeap->nextFreeBlock = uVar13;
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
								pNewHeap = AllocateHeapMemory_0028fac0(pHeap, 0);
								if (pNewHeap == (edHeapEntry*)0x0) {
									return (char*)0x0;
								}
								pHeap->freeBytes = (int)(pcVar7 + (iVar2 - (ulong)(pReturn + size)));
								pHeap->pStartAddr = pReturn + size;
								if ((pInHeap->flags & 0x10) != 0) {
									SetDataBlock_0028f440((uint*)pHeap->pStartAddr, pHeap->freeBytes, 0);
								}
								if ((USHORT_00424d76 & 0x20) != 0) {
									SetHeapFlag_0028f4e0(pHeap);
								}
								pHeap = pNewHeap;
								uVar10 = (ushort)((uint)((ulong)pNewHeap - (ulong)MemoryMasterBlock) / sizeof(edHeapEntry));
								uVar13 = uVar11;
							}
							pHeap->flags = pInHeap->flags & 0x10 | 0x86;
							pHeap->funcTableID = pInHeap->funcTableID;
							pHeap->field_0xf = BYTE_00424d6c;
							pHeap->pStartAddr = pReturn;
							pHeap->freeBytes = size;
							pHeap->align = align;
							pHeap->offset = offset;
							pHeap->field_0x1c = -1;
							pHeap->field_0x2 = -1;
							pHeap->nextBlock = -1;
							local_10 = pReturn;
							if ((pInHeap->flags & 0x10) != 0) {
								*(undefined4*)(pReturn + -0x10) = g_EdenName_004325d8;
								*(undefined4*)(pReturn + -4) = g_EdenName_004325d8;
								*(ushort*)(pReturn + -0xc) = uVar10;
								*(undefined2*)(pReturn + -10) = 0;
								*(undefined4*)(pReturn + -8) = 0;
								SetDataBlock_0028f440((uint*)pHeap->pStartAddr, pHeap->freeBytes, 1);
								local_10 = pReturn + -0x10;
							}
							if (((ulong)(short)pInHeap->flags & 0x8000U) != 0) {
								pReturn = (char*)((uint)pReturn & 0xdfffffff);
							}
							if (local_10 + -(ulong)pcVar7 != (char*)0x0) {
								pNewHeap = AllocateHeapMemory_0028fac0(pHeap, 0);
								if (pNewHeap == (edHeapEntry*)0x0) {
									return (char*)0x0;
								}
								pNewHeap->flags = 2;
								pNewHeap->pStartAddr = pcVar7;
								pNewHeap->freeBytes = (ulong)(local_10 + -(ulong)pcVar7);
								sVar5 = (short)((uint)((ulong)pNewHeap - (ulong)MemoryMasterBlock) / sizeof(edHeapEntry));
								if (sVar1 == -1) {
									pInHeap->nextFreeBlock = sVar5;
								}
								else {
									heapArray[sVar1].nextBlock = sVar5;
								}
								if (uVar13 != -1) {
									heapArray[uVar13].field_0x2 = sVar5;
								}
								pNewHeap->field_0x2 = sVar1;
								pNewHeap->nextBlock = uVar13;
								if ((pInHeap->flags & 0x10) != 0) {
									SetDataBlock_0028f440((uint*)pNewHeap->pStartAddr, pNewHeap->freeBytes, 0);
								}
								if ((USHORT_00424d76 & 0x20) != 0) {
									SetHeapFlag_0028f4e0(pNewHeap);
								}
							}
							if ((pInHeap->flags & 0x10) == 0) {
								pReturn = (char*)(int)(short)((uint)((ulong)pHeap - (ulong)MemoryMasterBlock) / sizeof(edHeapEntry));
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

void* FUN_0028e820(edHeapEntry* param_1, void* addr)
{
	// Unused.
	return NULL;
}

void Free_0028ecc0(edHeapEntry* pHeap)
{
	bool bVar1;
	short sVar2;
	edHeapEntry* peVar3;
	ushort uVar4;
	edHeapEntry* peVar5;
	edHeapEntry* peVar6;
	char* pcVar7;
	short unaff_s0_lo;
	short unaff_s1_lo;
	short sVar8;
	long lVar9;
	char cVar10;

	peVar3 = MemoryMasterBlock;
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
					FreeDataBlock_0028fc90(peVar5);
				}
				else {
					Free_0028ecc0(peVar5);
				}
				sVar8 = pHeap->field_0x1c;
			}
		}
		lVar9 = (long)pHeap->field_0x1c;
		if (lVar9 != -1) {
			while (lVar9 != -1) {
				sVar8 = peVar3[lVar9].heapID;
				FreeDataBlock_0028fc90(peVar3 + lVar9);
				lVar9 = (long)sVar8;
			}
		}
		if ((pHeap->flags & 0x200) != 0) {
			Free_0028ecc0(peVar3 + pHeap->nextFreeBlock);
		}
		if ((pHeap->flags & 0x10) != 0) {
			SetDataBlock_0028f440((uint*)(pHeap->pStartAddr + -0x10), pHeap->freeBytes + 0x10, 0);
		}
		pHeap->flags = 2;
		peVar5 = peVar3 + pHeap->field_0xa;
		sVar8 = (short)((uint)((ulong)pHeap - (ulong)MemoryMasterBlock) / sizeof(edHeapEntry));
		bVar1 = (peVar5->flags & 4) != 0;
		if (bVar1) {
			sVar2 = pHeap->heapID;
		}
		else {
			pHeap->freeBytes = (int)(pHeap->pStartAddr + (pHeap->freeBytes - (ulong)peVar5->pStartAddr));
			pHeap->pStartAddr = peVar5->pStartAddr;
			unaff_s1_lo = peVar5->field_0x2;
			unaff_s0_lo = peVar5->nextBlock;
			FreeDataBlock_0028fc90(peVar5);
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
				FreeDataBlock_0028fc90(peVar5);
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
						unaff_s0_lo = (ushort)((uint)((ulong)peVar5 - (ulong)MemoryMasterBlock) / sizeof(edHeapEntry));
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
				unaff_s1_lo = (ushort)((uint)((ulong)peVar6 - (ulong)MemoryMasterBlock) / sizeof(edHeapEntry));
				unaff_s0_lo = peVar6->nextBlock;
			}
		}
		if (unaff_s1_lo == -1) {
			peVar5 = GetBlock_0028fc40(pHeap);
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
				FreeDataBlock_0028fc90(pHeap);
				if ((peVar5->flags & 0x10) != 0) {
					SetDataBlock_0028f440((uint*)peVar5->pStartAddr, peVar5->freeBytes, 1);
				}
			}
			else {
				pHeap->flags = 2;
				pHeap->freeBytes = (int)(pHeap->pStartAddr + (pHeap->freeBytes - (ulong)peVar5->pStartAddr));
				pHeap->pStartAddr = peVar5->pStartAddr;
				if ((USHORT_00424d76 & 0x20) != 0) {
					SetHeapFlag_0028f4e0(pHeap);
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
			if ((USHORT_00424d76 & 0x20) != 0) {
				SetHeapFlag_0028f4e0(pHeap);
			}
		}
	}
	return;
}

char* Shrink_0028f120(edHeapEntry* pHeap, int newSize)
{
	// Not yet implemented.
	assert(false);
	return NULL;
}

bool IsAllocationValid_0028f3d0(int size, int align, int offset)
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

void SetDataBlock_0028f440(uint* data, int numWords, int isFree)
{
	int iVar1;
	uint uVar2;

	if ((isFree == 0) || ((USHORT_00424d76 & 0x10) != 0)) {
		if ((isFree == 0) && ((USHORT_00424d76 & 8) == 0)) {
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

void SetHeapFlag_0028f4e0(edHeapEntry* pHeap)
{
	if ((pHeap->flags & 0x10) != 0) {
		pHeap->flags = pHeap->flags | 0x20;
	}
	return;
}

int edMemGetAvailable(EHeap heapID)
{
	short sVar1;
	edHeapEntry* peVar2;
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
			sVar1 = MemoryMasterBlock[sVar1].nextBlock) {
			iVar4 = iVar4 + MemoryMasterBlock[sVar1].freeBytes;
		}
	}
	return iVar4;
}

void DebugDump(EHeap heapID, long mode)
{
	// Unimplemented.
}

edHeapEntry* AllocateHeapMemory_0028fac0(edHeapEntry* pHeap, int param_2)
{
	short sVar1;
	edHeapEntry* peVar2;
	edHeapEntry* peVar3;

	sVar1 = g_MaxHeadersUsed_00424d6a;
	peVar3 = MemoryMasterBlock;
	if (g_FreeHeaders_00424d66 == 0) {
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 2, (char*)0x0);
		peVar2 = (edHeapEntry*)0x0;
	}
	else {
		g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
		peVar2 = MemoryMasterBlock + g_MaxHeadersUsed_00424d6a;
		g_MaxHeadersUsed_00424d6a = peVar2->heapID;
		if ((long)g_MaxHeadersUsed_00424d68 < (long)((int)g_TotalHeaders_00424d64 - (int)g_FreeHeaders_00424d66)) {
			g_MaxHeadersUsed_00424d68 = (short)((int)g_TotalHeaders_00424d64 - (int)g_FreeHeaders_00424d66);
		}
		peVar2->flags = 2;
		peVar2->field_0x2 = 0xffff;
		peVar2->nextBlock = 0xffff;
		peVar2->nextFreeBlock = -1;
		if (param_2 == 0) {
			if (pHeap == (edHeapEntry*)0x0) {
				peVar2->heapID = 0xffff;
				peVar2->field_0xa = 0xffff;
			}
			else {
				peVar2->heapID = (short)((uint)((char*)pHeap - (char*)MemoryMasterBlock) / sizeof(edHeapEntry));
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
			peVar2->field_0xa = (short)((uint)((char*)pHeap - (char*)MemoryMasterBlock) / sizeof(edHeapEntry));
			pHeap->field_0x1c = sVar1;
			pHeap->nextFreeBlock = sVar1;
		}
	}
	return peVar2;
}

edHeapEntry* edmemGetMainHeader(void* heapID)
{
	short sVar1;

	sVar1 = (short)heapID;
	if ((ulong)g_TotalHeaders_00424d64 <= (ulong)heapID) {
		sVar1 = *(short*)((ulong)heapID + -0xc);
	}
	return MemoryMasterBlock + sVar1;
}

edHeapEntry* GetBlock_0028fc40(edHeapEntry* pHeap)
{
	short sVar1;
	edHeapEntry* peVar2;

	sVar1 = pHeap->field_0xa;
	while (peVar2 = MemoryMasterBlock + sVar1, MemoryMasterBlock + peVar2->heapID == pHeap) {
		sVar1 = peVar2->field_0xa;
		pHeap = peVar2;
	}
	return peVar2;
}

void FreeDataBlock_0028fc90(edHeapEntry* pHeap)
{
	short sVar1;
	edHeapEntry* peVar2;
	short sVar3;
	edHeapEntry* peVar4;

	peVar2 = MemoryMasterBlock;
	sVar3 = (short)((uint)((int)pHeap - (int)MemoryMasterBlock) / sizeof(edHeapEntry));
	if (pHeap->field_0xa != -1) {
		sVar1 = pHeap->heapID;
		peVar4 = MemoryMasterBlock + pHeap->field_0xa;
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
	pHeap->heapID = g_MaxHeadersUsed_00424d6a;
	pHeap->field_0x2 = -1;
	pHeap->nextBlock = -1;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + 1;
	g_MaxHeadersUsed_00424d6a = sVar3;
	return;
}

bool AllocateNewBlock_0028fd60(edHeapEntry* pHeap)
{
	bool bVar1;
	edHeapEntry* peVar2;

	if ((pHeap->flags & 0x200) == 0) {
		if ((pHeap->flags & 0x800) == 0) {
			peVar2 = AllocateHeapMemory_0028fac0(pHeap, 1);
			if (peVar2 == (edHeapEntry*)0x0) {
				bVar1 = false;
			}
			else {
				pHeap->flags = pHeap->flags | 8;
				peVar2->pStartAddr = pHeap->pStartAddr;
				peVar2->freeBytes = pHeap->freeBytes;
				if ((pHeap->flags & 0x10) != 0) {
					SetDataBlock_0028f440((uint*)peVar2->pStartAddr, peVar2->freeBytes, 0);
				}
				bVar1 = true;
			}
		}
		else {
			bVar1 = false;
		}
	}
	else {
		bVar1 = false;
	}
	return bVar1;
}

void edMemSetFlags(EHeap heapID, ushort newFlags)
{
	edHeapEntry* pHeap;
	ushort uVar1;

	if (heapID == TO_HEAP(H_INVALID)) {
		/* emSetFlags */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemSetFlags_00432b28);
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		uVar1 = newFlags & 0x85e0;
		if ((newFlags & 0x20) != 0) {
			uVar1 = uVar1 ^ 0x20;
			SetHeapFlag_0028f4e0(pHeap);
		}
		pHeap->flags = pHeap->flags | uVar1;
	}
	return;
}

void edMemClearFlags(EHeap heapID, ushort flags)
{
	edHeapEntry* pHeap;
	ushort uVar2;

	if (heapID == TO_HEAP(H_INVALID)) {
		/* edMemClearFlags */
		CallHandlerFunction(&edSysHandlerMemory_004890c0, 1, g_szEdMemClearFlags_00432b40);
	}
	else {
		pHeap = edmemGetMainHeader((void*)heapID);
		uVar2 = flags & 0x91e0;
		if ((flags & 0x20) != 0) {
			uVar2 = 0x1020;
		}
		pHeap->flags = pHeap->flags & ~uVar2;
	}
	return;
}

void ClearHeaps_0028ff10(void)
{
	short sVar1;
	edHeapEntry* peVar2;

	peVar2 = MemoryMasterBlock;
	memset((char*)MemoryMasterBlock, 0, g_TotalHeaders_00424d64 * sizeof(edHeapEntry));
	for (sVar1 = 0; sVar1 < g_TotalHeaders_00424d64; sVar1 = sVar1 + 1) {
		peVar2->heapID = sVar1 + 1;
		peVar2 = peVar2 + 1;
	}

#ifdef PLATFORM_WIN
	// Poison the end of the heap so we can see it in memory
	memset(&MemoryMasterBlock[0xfff], 0x5, sizeof(edHeapEntry));
#endif

	return;
}

void SetupDefaultHeaps_0028ff90(void)
{
	edHeapEntry* peVar1;
	edHeapEntry* puVar1;

	peVar1 = MemoryMasterBlock;
	MemoryMasterBlock->flags = 2;
	peVar1->funcTableID = 0xff;
	peVar1->field_0xf = 0xff;
	peVar1->pStartAddr = (char*)0xffffffff;
	peVar1->freeBytes = -1;
	peVar1->field_0xa = 0xffff;
	peVar1->heapID = 0xffff;
	peVar1->align = -1;
	peVar1->offset = 0xffffffff;
	peVar1->field_0x1c = -1;
	peVar1->nextFreeBlock = -1;
	peVar1->field_0x2 = 0xffff;
	peVar1->nextBlock = 0xffff;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
	peVar1[1].flags = 0x96;
	peVar1[1].funcTableID = 0;
	peVar1[1].field_0xf = 0;
	peVar1[1].pStartAddr = (char*)(MemoryMasterBlock + g_TotalHeaders_00424d64);
	int initialAlloc = (int)((char*)peVar1[1].pStartAddr - (char*)MemoryMasterBlock);
	peVar1[1].freeBytes = g_MemWorkSizeB - initialAlloc;
	peVar1[1].field_0xa = 0xffff;
	peVar1[1].heapID = 2;
	peVar1[1].align = g_SystemHeap_0042df0.align;
	peVar1[1].offset = g_SystemHeap_0042df0.offset;
	peVar1[1].field_0x1c = -1;
	peVar1[1].nextFreeBlock = -1;
	peVar1[1].field_0x2 = 0xffff;
	peVar1[1].nextBlock = 0xffff;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
	peVar1[2].flags = 0x96;
	peVar1[2].funcTableID = 0;
	peVar1[2].field_0xf = 0;
	peVar1[2].pStartAddr = g_Heap3_00424e00.startAddress;
	peVar1[2].freeBytes = g_Heap3_00424e00.size;
	peVar1[2].field_0xa = 1;
	peVar1[2].heapID = 3;
	peVar1[2].align = g_Heap3_00424e00.align;
	peVar1[2].offset = g_Heap3_00424e00.offset;
	peVar1[2].field_0x1c = -1;
	peVar1[2].nextFreeBlock = -1;
	peVar1[2].field_0x2 = 0xffff;
	peVar1[2].nextBlock = 0xffff;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
	peVar1[3].flags = 0x96;
	peVar1[3].funcTableID = 0;
	peVar1[3].field_0xf = 0;
	peVar1[3].pStartAddr = g_ScratchpadHeap_00424e10.startAddress;
	peVar1[3].freeBytes = g_ScratchpadHeap_00424e10.size;
	peVar1[3].field_0xa = 2;
	peVar1[3].heapID = 4;
	peVar1[3].align = g_ScratchpadHeap_00424e10.align;
	peVar1[3].offset = g_ScratchpadHeap_00424e10.offset;
	peVar1[3].field_0x1c = -1;
	peVar1[3].nextFreeBlock = -1;
	peVar1[3].field_0x2 = 0xffff;
	peVar1[3].nextBlock = 0xffff;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
	peVar1[4].flags = 0x86;
	peVar1[4].funcTableID = 1;
	peVar1[4].field_0xf = 0;
	peVar1[4].pStartAddr = g_Heap4_00424e20.startAddress;
	peVar1[4].freeBytes = g_Heap4_00424e20.size;
	peVar1[4].field_0xa = 3;
	peVar1[4].heapID = 0xffff;
	peVar1[4].align = g_Heap4_00424e20.align;
	peVar1[4].offset = g_Heap4_00424e20.offset;
	peVar1[4].field_0x1c = -1;
	peVar1[4].nextFreeBlock = -1;
	peVar1[4].field_0x2 = 0xffff;
	peVar1[4].nextBlock = 0xffff;
	g_MaxHeadersUsed_00424d68 = g_MaxHeadersUsed_00424d68 + 1;
	g_FreeHeaders_00424d66 = g_FreeHeaders_00424d66 + -1;
	g_MaxHeadersUsed_00424d6a = g_MaxHeadersUsed_00424d68;
	return;
}

void SetHeapFunctionsA(void)
{
	g_HeapFunctionTableArray_00489030[0].alloc = Alloc_0028e140;
	g_HeapFunctionTableArray_00489030[0].field_0x4 = FUN_0028e820;
	g_HeapFunctionTableArray_00489030[0].free = Free_0028ecc0;
	g_HeapFunctionTableArray_00489030[0].shrink = Shrink_0028f120;
	SetHeapFunctionsB();
	return;
}

void EmptyFunction_00290290(void)
{
	return;
}

void edMemInit(short headerCount)
{
	g_MaxHeadersUsed_00424d68 = 0;
	g_MaxHeadersUsed_00424d6a = 0;
	BYTE_00424d6c = 0;
	BYTE_00424d6d = 0;
	MemoryMasterBlock = (edHeapEntry*)g_SystemHeap_0042df0.startAddress;
	g_TotalHeaders_00424d64 = headerCount;
	g_FreeHeaders_00424d66 = headerCount;
	IncrementCounter_002903f0();
	ClearHeaps_0028ff10();
	SetupDefaultHeaps_0028ff90();
	SetHeapFunctionsA();
	EmptyFunction_00290290();
	return;
}

int GetHeadersUsed_00290320(void)
{
	return (int)g_TotalHeaders_00424d64 - (int)g_FreeHeaders_00424d66;
}

int GetHeadersLeft_00290340(void)
{
	return (int)g_FreeHeaders_00424d66;
}

int GetMaxHeadersUsed_00290350(void)
{
	return (int)g_MaxHeadersUsed_00424d68;
}

void IncrementCounter_002903f0(void)
{
	BYTE_00424d6c = BYTE_00424d6c + 1;
	return;
}

bool Cleanup_00290410(void)
{
	ushort uVar1;
	bool bVar2;
	edHeapEntry* peVar3;
	short sVar4;

	bVar2 = true;
	BYTE_00424d6c = BYTE_00424d6c - 1;
	peVar3 = MemoryMasterBlock;
	for (sVar4 = 0; sVar4 < g_TotalHeaders_00424d64; sVar4 = sVar4 + 1) {
		uVar1 = peVar3->flags;
		if ((((uVar1 & 2) != 0) && ((uVar1 & 4) != 0)) && (BYTE_00424d6c < peVar3->field_0xf)) {
			bVar2 = false;
			peVar3->flags = uVar1 | 0x4000;
		}
		peVar3 = peVar3 + 1;
	}
	return bVar2;
}

void* GetScratchPadPtr_00424e10(void)
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

void SetHeapFunctionsB(void)
{
	g_HeapFunctionTableArray_00489030[1].alloc = Alloc_0028e140;
	g_HeapFunctionTableArray_00489030[1].field_0x4 = FUN_0028e820;
	g_HeapFunctionTableArray_00489030[1].free = Free_0028ecc0;
	g_HeapFunctionTableArray_00489030[1].shrink = Shrink_0028f120;
	return;
}