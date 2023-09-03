#include "edCBank.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edCFiler.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif
#include "edSystem.h"
#include "edCFiler_BNK.h"

edCBankDebug edBankDebug;
byte g_edCBank_DebugAllowNoWait_00448900;
byte g_DebugBankLoadFlag_00469be0[16];
edFILEH g_DebugBankDataArray_00469bf0[16];

edCBankBufferEntry* edCBankBuffer::get_free_entry()
{
	edCBankBufferEntry* returnBankPtr;
	edCBankBufferEntry* peVar1;
	uint uVar2;

	uVar2 = 0;
	for (returnBankPtr = this->pBankBuffer; (uVar2 < (uint)this->createFlagA && (returnBankPtr->field_0x4 != (int*)0x0)); returnBankPtr = (edCBankBufferEntry*)&returnBankPtr->field_0x24) {
		uVar2 = uVar2 + 1;
	}
	if (uVar2 == this->createFlagA) {
		returnBankPtr = (edCBankBufferEntry*)0x0;
	}
	else {
		returnBankPtr->headerComponent = 0;
		returnBankPtr->header = this;
		uVar2 = this->createFlagA * 0x24;
		if ((uVar2 & 0x7ff) == 0) {
			peVar1 = this->pBankBuffer;
		}
		else {
			uVar2 = ((uVar2 >> 0xb) + 1) * 0x800;
			peVar1 = this->pBankBuffer;
		}
		returnBankPtr->field_0x4 = (int*)((int)&peVar1->header + this->flagB + uVar2);
		this->flagC = this->flagC + 1;
	}
	return returnBankPtr;
}

void edCBankDebug::buffer_link(edCBankBuffer* bankAllocateAddress)
{
	if (bankAllocateAddress != (edCBankBuffer*)0x0) {
		if (this->bankCount == 1) {
			this->pNextBank->pNextBank = bankAllocateAddress;
			this->pNextBank->pPrevBank = bankAllocateAddress;
			bankAllocateAddress->pNextBank = this->pNextBank;
			bankAllocateAddress->pPrevBank = this->pNextBank;
		}
		else {
			if (this->bankCount == 0) {
				this->pNextBank = bankAllocateAddress;
				bankAllocateAddress->pNextBank = bankAllocateAddress;
				bankAllocateAddress->pPrevBank = bankAllocateAddress;
			}
			else {
				bankAllocateAddress->pNextBank = this->pNextBank;
				bankAllocateAddress->pPrevBank = this->pNextBank->pPrevBank;
				this->pNextBank->pPrevBank->pNextBank = bankAllocateAddress;
				this->pNextBank->pPrevBank = bankAllocateAddress;
			}
		}
		this->bankCount = this->bankCount + 1;
	}
	return;
}

edFILEH* edFileOpen(char* szFilePath, uint flags)
{
	bool bOpenSuccess;
	edCFiler* pFiler;
	edFILEH* pFile;
	int bankIndex;
	edFILEH* pDebugBank;
	char szFilerPath[512];

	MY_LOG("edFileOpen %s\n", szFilePath);

	pFiler = edFileGetFiler(szFilerPath, szFilePath, 0);

	MY_LOG("edFileOpen Filer Path %s\n", szFilerPath);

	if (pFiler == (edCFiler*)0x0) {
		pFile = (edFILEH*)0x0;
	}
	else {
		if (((flags & 8) != 0) && (((pFiler->baseData).field_0x4 & 0x20U) == 0)) {
			flags = flags & 0xfffffff7;
		}
		if ((((flags & 2) == 0) || (pFile = (edFILEH*)0x0, ((pFiler->baseData).field_0x4 & 2U) != 0)) &&
			(((flags & 4) == 0 || (pFile = (edFILEH*)0x0, ((pFiler->baseData).field_0x4 & 4U) != 0)))) {
			if (((flags & 8) == 0) || (((pFiler->baseData).field_0x4 & 0x80U) != 0)) {
				edFileGetFiler(pFiler);
			}
			bankIndex = 0;
			do {
				if (g_DebugBankLoadFlag_00469be0[bankIndex] == 0) {
					g_DebugBankLoadFlag_00469be0[bankIndex] = 1;
					pDebugBank = &g_DebugBankDataArray_00469bf0[bankIndex];
					memset(pDebugBank, 0, sizeof(edFILEH));
					goto LAB_0025c168;
				}
				bankIndex = bankIndex + 1;
			} while (bankIndex < 0x10);
			pDebugBank = (edFILEH*)0x0;
		LAB_0025c168:
			pFile = (edFILEH*)0x0;
			if (pDebugBank != (edFILEH*)0x0) {
				pDebugBank->pOwningFiler = pFiler;
				pDebugBank->openFlags = flags;
				/* May call edCFiler_CDVD_open */
				bOpenSuccess = pFiler->open(pDebugBank, szFilerPath);
				if (bOpenSuccess == false) {
					bankIndex = 0;
					do {
						if (&g_DebugBankDataArray_00469bf0[bankIndex] == pDebugBank) {
							g_DebugBankLoadFlag_00469be0[bankIndex] = 0;
							memset(&g_DebugBankDataArray_00469bf0[bankIndex], 0, sizeof(edFILEH));
							break;
						}
						bankIndex = bankIndex + 1;
					} while (bankIndex < 0x10);
					pFile = (edFILEH*)0x0;
				}
				else {
					pDebugBank->bInUse = 1;
					pFile = pDebugBank;
				}
			}
		}
	}
	return pFile;
}

uint GetFileSize_0025bd70(edFILEH* pDebugBank)
{
	uint uVar1;

	uVar1 = 0;
	if ((pDebugBank->openFlags & 6) == 0) {
		uVar1 = (pDebugBank->field_0x10).fileSize;
	}
	return uVar1;
}

bool SetRead_0025be80(edFILEH* pDebugBank, char* param_2, uint size)
{
	byte bVar2;
	bool bVar1;
	edCFiler_28* peVar3;
	byte* pbVar4;
	int iVar5;

	peVar3 = pDebugBank->pOwningFiler->GetGlobalC_0x1c();
	SetBankReadStream(peVar3, pDebugBank, param_2, size);
	if ((pDebugBank->openFlags & 8) == 0) {
		edFileGetFiler(pDebugBank->pOwningFiler);
		iVar5 = 0;
		do {
			if (&g_DebugBankDataArray_00469bf0[iVar5] == pDebugBank) {
				bVar2 = g_DebugBankLoadFlag_00469be0[iVar5];
				goto LAB_0025bf20;
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < 0x10);
		bVar2 = 0;
	LAB_0025bf20:
		if (bVar2 == 0) {
			bVar1 = true;
		}
		else {
			bVar1 = pDebugBank->bInUse;
		}
	}
	else {
		bVar1 = true;
	}
	return bVar1;
}

bool SetClose_0025bf60(edFILEH* pDebugBank)
{
	byte bVar1;
	bool uVar2;
	edCFiler_28* peVar2;
	byte* pbVar3;
	int iVar4;

	peVar2 = pDebugBank->pOwningFiler->GetGlobalC_0x1c();
	SetBankClose(peVar2, pDebugBank);
	if ((pDebugBank->openFlags & 8) == 0) {
		edFileGetFiler(pDebugBank->pOwningFiler);
		iVar4 = 0;
		do {
			if (&g_DebugBankDataArray_00469bf0[iVar4] == pDebugBank) {
				bVar1 = g_DebugBankLoadFlag_00469be0[iVar4];
				goto LAB_0025bff0;
			}
			iVar4 = iVar4 + 1;
		} while (iVar4 < 0x10);
		bVar1 = 0;
	LAB_0025bff0:
		if (bVar1 == 0) {
			uVar2 = true;
		}
		else {
			uVar2 = pDebugBank->bInUse;
		}
	}
	else {
		uVar2 = true;
	}
	return uVar2;
}

bool SetBankReadStream(edCFiler_28* param_1, edFILEH* pDebugBank, char* pReadBuffer, uint someSize)
{
	uint uVar1;
	int iVar2;
	edCFiler_28_Internal* puVar3;

	uVar1 = param_1->freeIndexes;
	puVar3 = (edCFiler_28_Internal*)0x0;
	if (uVar1 < 0x18) {
		iVar2 = param_1->currentIndex;
		param_1[(int)(iVar2 + uVar1) % 0x18].internalBank.nextAction = READ_STREAM;
		puVar3 = &param_1[(int)(iVar2 + uVar1) % 0x18].internalBank;
		if (param_1->freeIndexes == 0) {
			param_1->nextAction = param_1[param_1->currentIndex].internalBank.nextAction;
		}
		param_1->freeIndexes = param_1->freeIndexes + 1;
	}
	if (puVar3 == (edCFiler_28_Internal*)0x0) {
		puVar3 = (edCFiler_28_Internal*)0x0;
	}
	else {
		puVar3->mode = 0;
		puVar3->pDataBank = pDebugBank;
		pDebugBank->count_0x228 = pDebugBank->count_0x228 + 1;
	}
	if (puVar3 != (edCFiler_28_Internal*)0x0) {
		puVar3->pReadBuffer = pReadBuffer;
		puVar3->seekOffset2 = someSize;
	}
	return puVar3 != (edCFiler_28_Internal*)0x0;
}

bool SetBankSeek(edCFiler_28* param_1, edFILEH* pDebugBank, uint seekOffset)
{
	uint uVar1;
	int iVar2;
	edCFiler_28_Internal* peVar3;

	uVar1 = param_1->freeIndexes;
	peVar3 = (edCFiler_28_Internal*)0x0;
	if (uVar1 < 0x18) {
		iVar2 = param_1->currentIndex;
		param_1[(int)(iVar2 + uVar1) % 0x18].internalBank.nextAction = SEEK;
		peVar3 = &param_1[(int)(iVar2 + uVar1) % 0x18].internalBank;
		if (param_1->freeIndexes == 0) {
			param_1->nextAction = param_1[param_1->currentIndex].internalBank.nextAction;
		}
		param_1->freeIndexes = param_1->freeIndexes + 1;
	}
	if (peVar3 == (edCFiler_28_Internal*)0x0) {
		peVar3 = (edCFiler_28_Internal*)0x0;
	}
	else {
		peVar3->mode = 0;
		peVar3->pDataBank = pDebugBank;
		pDebugBank->count_0x228 = pDebugBank->count_0x228 + 1;
	}
	if (peVar3 != (edCFiler_28_Internal*)0x0) {
		peVar3->seekOffset = seekOffset;
	}
	return peVar3 != (edCFiler_28_Internal*)0x0;
}

bool edFileSeek(edFILEH* pDebugBank, uint seekOffset, ESeekMode mode)
{
	byte bVar2;
	bool bVar1;
	edCFiler_28* peVar3;
	byte* pbVar4;
	int iVar5;

	if (mode == ED_SEEK_END) {
		seekOffset = (pDebugBank->field_0x10).fileSize + seekOffset;
	}
	else {
		if (mode == ED_SEEK_CUR) {
			seekOffset = pDebugBank->seekOffset + seekOffset;
		}
		else {
			if (mode != ED_SEEK_SET) {
				seekOffset = 0;
			}
		}
	}
	peVar3 = pDebugBank->pOwningFiler->GetGlobalC_0x1c();
	SetBankSeek(peVar3, pDebugBank, seekOffset);
	if ((pDebugBank->openFlags & 8) == 0) {
		edFileGetFiler(pDebugBank->pOwningFiler);
		iVar5 = 0;
		do {
			if (&g_DebugBankDataArray_00469bf0[iVar5] == pDebugBank) {
				bVar2 = g_DebugBankLoadFlag_00469be0[iVar5];
				goto LAB_0025b788;
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < 0x10);
		bVar2 = 0;
	LAB_0025b788:
		if (bVar2 == 0) {
			bVar1 = true;
		}
		else {
			bVar1 = (bool)pDebugBank->bInUse;
		}
	}
	else {
		bVar1 = true;
	}
	return bVar1;
}

byte edFileRead(edFILEH* pDebugBank, char* pReadBuffer, uint someSize)
{
	byte bVar1;
	edCFiler_28* peVar2;
	byte* pbVar3;
	int iVar4;

	peVar2 = pDebugBank->pOwningFiler->GetGlobalC_0x1c();
	SetBankReadStream(peVar2, pDebugBank, pReadBuffer, someSize);
	if ((pDebugBank->openFlags & 8) == 0) {
		edFileGetFiler(pDebugBank->pOwningFiler);
		iVar4 = 0;
		pbVar3 = g_DebugBankLoadFlag_00469be0;
		do {
			if ((edFILEH*)(pbVar3 + 0x10) == pDebugBank) {
				bVar1 = g_DebugBankLoadFlag_00469be0[iVar4];
				goto LAB_0025b580;
			}
			iVar4 = iVar4 + 1;
			pbVar3 = pbVar3 + sizeof(edFILEH);
		} while (iVar4 < 0x10);
		bVar1 = 0;
	LAB_0025b580:
		if (bVar1 == 0) {
			bVar1 = 1;
		}
		else {
			bVar1 = pDebugBank->bInUse;
		}
	}
	else {
		bVar1 = 1;
	}
	return bVar1;
}

bool CompleteRead_0025bcc0(void)
{
	edFileGetFiler((edCFiler*)0x0);
	return true;
}

bool SetBankClose(edCFiler_28* param_1, edFILEH* pDataBank)
{
	uint uVar1;
	int iVar2;
	edCFiler_28_Internal* peVar3;

	uVar1 = param_1->freeIndexes;
	peVar3 = (edCFiler_28_Internal*)0x0;
	if (uVar1 < 0x18) {
		iVar2 = param_1->currentIndex;
		param_1[(int)(iVar2 + uVar1) % 0x18].internalBank.nextAction = CLOSE;
		peVar3 = &param_1[(int)(iVar2 + uVar1) % 0x18].internalBank;
		if (param_1->freeIndexes == 0) {
			param_1->nextAction = param_1[param_1->currentIndex].internalBank.nextAction;
		}
		param_1->freeIndexes = param_1->freeIndexes + 1;
	}
	if (peVar3 == (edCFiler_28_Internal*)0x0) {
		peVar3 = (edCFiler_28_Internal*)0x0;
	}
	else {
		peVar3->mode = 0;
		peVar3->pDataBank = pDataBank;
		pDataBank->count_0x228 = pDataBank->count_0x228 + 1;
	}
	return peVar3 != (edCFiler_28_Internal*)0x0;
}

bool edFileClose(edFILEH* pDebugBank)
{
	byte bVar2;
	bool bVar1;
	edCFiler_28* peVar3;
	int iVar5;

	peVar3 = pDebugBank->pOwningFiler->GetGlobalC_0x1c();
	SetBankClose(peVar3, pDebugBank);
	if ((pDebugBank->openFlags & 8) == 0) {
		edFileGetFiler(pDebugBank->pOwningFiler);
		iVar5 = 0;
		do {
			if (&g_DebugBankDataArray_00469bf0[iVar5] == pDebugBank) {
				bVar2 = g_DebugBankLoadFlag_00469be0[iVar5];
				goto LAB_0025b4b0;
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < 0x10);
		bVar2 = 0;
	LAB_0025b4b0:
		if (bVar2 == 0) {
			bVar1 = true;
		}
		else {
			bVar1 = (bool)pDebugBank->bInUse;
		}
	}
	else {
		bVar1 = true;
	}
	return bVar1;
}

char* edBankFilerReadHeader(EHeap heapID, char* filePath, uint flags, edFILEH** outLoadedData)
{
	byte bVar1;
	edFILEH* pDebugBank;
	uint uVar2;
	char* __dest;
	char* pReadBuffer;
	int size;
	char acStack4096[4096];
	uint __n;

	pReadBuffer = acStack4096;
	if (((uint)pReadBuffer & 0x7ff) != 0) {
		pReadBuffer = (char*)((((long long)pReadBuffer >> 0xb) + 1) * 0x800);
	}
	pDebugBank = edFileOpen(filePath, flags);
	if (pDebugBank == (edFILEH*)0x0) {
		__dest = (char*)0x0;
	}
	else {
		uVar2 = edFileGetSize(pDebugBank);
		if (uVar2 < 0x40) {
			__dest = (char*)0x0;
		}
		else {
			memset(pReadBuffer, 0, 0x800);
			bVar1 = edFileRead(pDebugBank, pReadBuffer, 0x800);
			if (bVar1 == 0) {
				__dest = (char*)0x0;
			}
			else {
				CompleteRead_0025bcc0();
				__n = *(uint*)(pReadBuffer + 0x44);
				size = __n;
				if ((0x800 < __n) && ((__n & 0x7ff) != 0)) {
					size = ((__n >> 0xb) + 1) * 0x800;
				}
				__dest = (char*)edMemAlloc(heapID, size);
				if (__dest == (char*)0x0) {
					IMPLEMENTATION_GUARD();
					//FUN_0025b420(pDebugBank);
					__dest = (char*)0x0;
				}
				else {
					if (__n < 0x801) {
						memcpy(__dest, pReadBuffer, __n);
					}
					else {
						memcpy(__dest, pReadBuffer, 0x800);
						edFileRead(pDebugBank, __dest + 0x800, size - 0x800);
						CompleteRead_0025bcc0();
						if (__n != size) {
							edMemShrink(__dest, __n);
						}
					}
					if (outLoadedData == (edFILEH**)0x0) {
						edFileClose(pDebugBank);
					}
					else {
						*outLoadedData = pDebugBank;
					}
				}
			}
		}
	}
	return __dest;
}

void edCBankBuffer::initialize(int size, int param_3, edCBankInstall* bankPathContainer)
{
	int iVar1;
	edCBankBufferEntry* peVar2;
	int* piVar3;
	edCBankBufferEntry* peVar4;
	uint bankFlags;
	uint uVar5;
	edCBankBuffer* local_s2_lo_56;

	edBankDebug.buffer_link(this);
	this->size = size;
	this->fileFlagA = bankPathContainer->fileFlagA;
	this->createFlagA = param_3;
	this->pBankTypePairData = (TypePairData*)0x0;
	this->flagB = 0;
	this->flagC = 0;
	this->firstField = 0;
	this->heapID = bankPathContainer->heapID;
	if (this->heapID == TO_HEAP(H_INVALID)) {
		this->heapID = TO_HEAP(H_MAIN);
	}
	if (g_edCBank_DebugAllowNoWait_00448900 == 0) {
		this->fileFlagA = this->fileFlagA & 0xfffffffb;
	}
	if ((this->fileFlagA & 1U) != 0) {
		this->createFlagA = this->createFlagA + 1;
	}
	uVar5 = this->createFlagA * 0x24;
	if ((uVar5 & 0x7ff) == 0) {
		iVar1 = this->size;
	}
	else {
		uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
		iVar1 = this->size;
	}
	peVar2 = (edCBankBufferEntry*)edMemAllocAlign(this->heapID, (long)(int)(iVar1 + uVar5), 0x800);
	this->pBankBuffer = peVar2;
	memset(this->pBankBuffer, 0, uVar5);
	if ((this->fileFlagA & 1U) != 0) {
		bankFlags = 0;
		for (peVar2 = this->pBankBuffer; (bankFlags < (uint)this->createFlagA && (peVar2->field_0x4 != (int*)0x0)); peVar2 = (edCBankBufferEntry*)&peVar2->field_0x24) {
			bankFlags = bankFlags + 1;
		}
		if (bankFlags == this->createFlagA) {
			peVar2 = (edCBankBufferEntry*)0x0;
		}
		else {
			peVar2->headerComponent = 0;
			peVar2->header = this;
			uVar5 = this->createFlagA * 0x24;
			if ((uVar5 & 0x7ff) == 0) {
				peVar4 = this->pBankBuffer;
			}
			else {
				uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
				peVar4 = this->pBankBuffer;
			}
			peVar2->field_0x4 = (int*)((int)&peVar4->header + this->flagB + uVar5);
			this->flagC = this->flagC + 1;
		}
		uVar5 = 1;
		peVar2->headerComponent = 1;
		if ((this->fileFlagA & 4U) != 0) {
			uVar5 = 9;
		}
		piVar3 = (int*)edBankFilerReadHeader(this->heapID, bankPathContainer->filePath, uVar5, &peVar2->pLoadedData);
		peVar2->field_0x4 = piVar3;
		peVar2->fileBuffer = (edCBankFileHeader*)(piVar3 + 2);
		peVar2->field_0x14 = piVar3[0x11];
		((edCBankFileHeader*)(piVar3 + 2))->analyse();
	}
	return;
}

void edCBankBuffer::bank_buffer_setcb(TypePairData* pTypePairData)
{
	this->pBankTypePairData = pTypePairData;
	return;
}

void edCBankDebug::buffer_unlink(edCBankBuffer* pBankToRemove)
{
	if (pBankToRemove != (edCBankBuffer*)0x0) {
		this->bankCount = this->bankCount + -1;
		pBankToRemove->pNextBank->pPrevBank = pBankToRemove->pPrevBank;
		pBankToRemove->pPrevBank->pNextBank = pBankToRemove->pNextBank;
		if (this->bankCount == 0) {
			this->pNextBank = (edCBankBuffer*)0x0;
		}
		else {
			if (this->pNextBank == pBankToRemove) {
				this->pNextBank = pBankToRemove->pNextBank;
			}
		}
		pBankToRemove->pNextBank = (edCBankBuffer*)0x0;
		pBankToRemove->pPrevBank = (edCBankBuffer*)0x0;
	}
	return;
}

void edCBankBuffer::terminate()
{
	edBankDebug.buffer_unlink(this);
	if ((this->fileFlagA & 1U) != 0) {
		edFileClose(this->pBankBuffer->pLoadedData);
		edMemFree(this->pBankBuffer->field_0x4);
		memset(this->pBankBuffer, 0, sizeof(edCBankBufferEntry));
	}
	edMemFree(this->pBankBuffer);
	memset(this, 0, sizeof(edCBankBuffer));
	return;
}

int g_FileSystemSysHandlerID_00469bc4 = 0x10;
int g_FileSystemHandlers_00469bc8 = 6;
edCBankStack g_edCBankStack_00466e60;

edSysHandlersNodeTable* PTR_g_SysHandlersNodeTable_00469b80 = &g_SysHandlersNodeTable_00489170;

PACK (struct BankFileHeader {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	edCBankFileHeader body;
});

char* sz_Unpacking_0042ff40 = "\n ==> UNPACKING \n";

void edDataLZ77Unpack(byte* param_1, byte* param_2, int param_3)
{
	byte bVar1;
	byte bVar2;
	long lVar3;
	uint uVar4;
	uint in_t2_lo;

	uVar4 = 0;
	if (param_3 != 0) {
		do {
			if (uVar4 == 0) {
				in_t2_lo = (uint)*param_2;
				uVar4 = 1;
				param_2 = param_2 + 1;
			}
			if ((in_t2_lo & 0xff & uVar4) == 0) {
				bVar1 = param_2[1];
				bVar2 = *param_2;
				param_2 = param_2 + 2;
				lVar3 = ((ulong)bVar2 & 0xf) + 3;
				param_3 = param_3 - (int)lVar3;
				if (lVar3 != 0) {
					do {
						lVar3 = lVar3 + -1;
						*param_1 = param_1[-((int)((long)((ulong)bVar2 | (long)(int)((uint)bVar1 << 8)) >> 4) + 1)];
						param_1 = param_1 + 1;
					} while (0 < lVar3);
				}
			}
			else {
				param_3 = param_3 + -1;
				*param_1 = *param_2;
				param_2 = param_2 + 1;
				param_1 = param_1 + 1;
			}
			uVar4 = (uVar4 & 0x7f) << 1;
		} while (param_3 != 0);
	}
	return;
}

void edCBankFileHeader::unpack()
{
	int iVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	char* pcVar5;
	uint uVar6;
	char* pcVar7;
	uint uVar8;
	uint uVar9;
	int iVar10;
	uint* piVar11;
	int iVar12;
	uint uVar13;
	byte* pbVar14;
	char* piVar10;
	byte* __src;
	undefined auStack256[256];

	if ((this->flags_0x4 & 1) != 0) {
		/* \n ==> UNPACKING \n */
		edDebugPrintf(sz_Unpacking_0042ff40);
		uVar6 = this->fileCount;
		piVar10 = (char*)&this[-1].field_0x34;
		if (uVar6 != 0) {
			iVar12 = uVar6 << 4;
			while (uVar6 != 0) {
				iVar1 = this->fileHeaderDataOffset;
				iVar2 = iVar12 + -0x10;
				uVar13 = uVar6 - 1;
				if (iVar1 == 0) {
					pcVar5 = (char*)0x0;
				}
				else {
					pcVar5 = piVar10 + iVar1;
				}
				if (iVar1 == 0) {
					pcVar7 = (char*)0x0;
				}
				else {
					pcVar7 = piVar10 + iVar1;
				}
				uVar8 = *(int*)(pcVar7 + uVar6 * 0x10 + 8) - *(int*)(pcVar5 + iVar2 + 8);
				if (iVar1 == 0) {
					pcVar7 = (char*)0x0;
				}
				else {
					pcVar7 = piVar10 + iVar1;
				}
				iVar3 = *(int*)(pcVar7 + iVar12 + -8);
				if ((this->flags_0x4 & 1) == 0) {
					if (iVar1 == 0) {
						pcVar7 = (char*)0x0;
					}
					else {
						pcVar7 = piVar10 + iVar1;
					}
					iVar12 = *(int*)(pcVar7 + iVar12 + -8);
				}
				else {
					pcVar7 = (char*)0x0;
					if (iVar1 != 0) {
						pcVar7 = piVar10 + iVar1;
					}
					iVar12 = *(int*)(pcVar7 + 8);
					uVar6 = 0;
					if (uVar13 != 0) {
						iVar10 = 0;
						uVar9 = this->field_0x1c - 1;
						do {
							pcVar7 = (char*)0x0;
							if (iVar1 != 0) {
								pcVar7 = piVar10 + iVar1;
							}
							uVar6 = uVar6 + 1;
							iVar4 = iVar10 + 0xc;
							iVar10 = iVar10 + 0x10;
							iVar12 = iVar12 + (~uVar9 & *(int*)(pcVar7 + iVar4) + uVar9);
						} while (uVar6 < uVar13);
					}
				}
				uVar9 = *(uint*)(pcVar5 + iVar2 + 0xc);
				pbVar14 = (byte*)(piVar10 + iVar12);
				uVar6 = uVar13;
				iVar12 = iVar2;
				if (uVar8 < uVar9) {
					__src = pbVar14 + uVar9;
					memcpy(auStack256, __src, 0x100);
					iVar1 = 0x100 - uVar8;
					while (uVar8 != 0) {
						uVar8 = uVar8 - 1;
						(__src + iVar1)[uVar8] = (piVar10 + iVar3)[uVar8];
					}
					edDataLZ77Unpack(pbVar14, __src + iVar1, uVar9);
					memcpy(__src, auStack256, 0x100);
				}
				else {
					while (uVar8 != 0) {
						uVar8 = uVar8 - 1;
						pbVar14[uVar8] = (piVar10 + iVar3)[uVar8];
					}
				}
			}
		}
		uVar6 = 0;
		iVar12 = 0;
		piVar11 = &this[-1].field_0x34;
		do {
			iVar1 = this->fileHeaderDataOffset;
			iVar2 = 0;
			if (iVar1 != 0) {
				iVar2 = (int)piVar11 + iVar1;
			}
			if ((this->flags_0x4 & 1) == 0) {
				iVar3 = 0;
				if (iVar1 != 0) {
					iVar3 = (int)piVar11 + iVar1;
				}
				iVar3 = *(int*)(iVar3 + iVar12 + 8);
			}
			else {
				iVar3 = 0;
				if (iVar1 != 0) {
					iVar3 = (int)piVar11 + iVar1;
				}
				iVar3 = *(int*)(iVar3 + 8);
				uVar13 = 0;
				if (uVar6 != 0) {
					iVar10 = 0;
					uVar8 = this->field_0x1c - 1;
					do {
						iVar4 = 0;
						if (iVar1 != 0) {
							iVar4 = (int)piVar11 + iVar1;
						}
						iVar4 = iVar4 + iVar10;
						uVar13 = uVar13 + 1;
						iVar10 = iVar10 + 0x10;
						iVar3 = iVar3 + (~uVar8 & *(int*)(iVar4 + 0xc) + uVar8);
					} while (uVar13 < uVar6);
				}
			}
			*(int*)(iVar2 + iVar12 + 8) = iVar3;
			uVar6 = uVar6 + 1;
			iVar12 = iVar12 + 0x10;
		} while (uVar6 <= this->fileCount);
	}
	return;
}

void edBankStackFileCallBack(int idA, int idB, char* pReadBuffer)
{
	TypePairData* pTypePairData;
	void* puVar1;
	LoadBankFileFunc pLVar2;
	edFILEH* pDVar3;
	int iVar4;
	uint uVar5;
	BankFileHeader* pcVar1;

	iVar4 = g_edCBankStack_00466e60.currentIndex;
	if (g_edCBankStack_00466e60.loadedBanks != 0) {
		pcVar1 = (BankFileHeader*)g_edCBankStack_00466e60.aBankQueue[g_edCBankStack_00466e60.currentIndex].pReadBuffer;
		if ((BankFileHeader*)pReadBuffer == pcVar1) {
			pTypePairData = (TypePairData*)g_edCBankStack_00466e60.aBankQueue[g_edCBankStack_00466e60.currentIndex].pBankTypePairData_0xc;
			puVar1 = g_edCBankStack_00466e60.aBankQueue[g_edCBankStack_00466e60.currentIndex].pObjectReference_0x10;
			(g_edCBankStack_00466e60.aBankQueue[g_edCBankStack_00466e60.currentIndex].pBankHeader_0x0)->accessFlag = 0;

			if (((pcVar1->body).flags_0x4 & 1) != 0) {
				pcVar1->body.unpack();
			}
			if ((g_edCBankStack_00466e60.aBankQueue[iVar4].fileFlagB_0x18 & 8U) == 0) {
				pLVar2 = g_edCBankStack_00466e60.aBankQueue[iVar4].fileFunc_0x14;
				if (pLVar2 != (LoadBankFileFunc)0x0) {
					(pLVar2)(false, puVar1);
				}
				pcVar1->body.apply_callback(pTypePairData, 0);
				if (pLVar2 != (LoadBankFileFunc)0x0) {
					(pLVar2)(true, puVar1);
				}
				pcVar1->body.analyse();
			}
			pDVar3 = g_edCBankStack_00466e60.aBankQueue[iVar4].pDebugBankData;
			if (pDVar3 != (edFILEH*)0x0) {
				uVar5 = edFileGetSize(pDVar3);
				(pcVar1->body).sizePacked = uVar5;
			}
			g_edCBankStack_00466e60.loadedBanks = g_edCBankStack_00466e60.loadedBanks + -1;
			if ((g_edCBankStack_00466e60.loadedBanks != 0) &&
				(g_edCBankStack_00466e60.currentIndex = g_edCBankStack_00466e60.currentIndex + 1, g_edCBankStack_00466e60.currentIndex == 0xc)) {
				g_edCBankStack_00466e60.currentIndex = 0;
			}
		}
	}
	return;
}

void edCBankStack::initialize()
{
	memset(this, 0, sizeof(edCBankStack));
	edSysHandlersAdd(edFileHandlers.nodeParent, edFileHandlers.entries, edFileHandlers.maxEventID
		, ESHT_LoadFile, edBankStackFileCallBack, 1, 1);
	return;
}

void edBankInit(void)
{
	//edCBank_LoadLib_002454d0(&g_edCBank_00466d30);
	g_edCBankStack_00466e60.initialize();
	edBankFilerInstall();
	return;
}
