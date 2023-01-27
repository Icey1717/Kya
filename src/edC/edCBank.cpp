#include "edCBank.h"

#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edCFiler.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#endif
#include "edSystem.h"

edBank_Manager g_edCBank_00466d30;
byte g_edCBank_DebugAllowNoWait_00448900;
byte g_DebugBankLoadFlag_00469be0[16];
DebugBankData_234 g_DebugBankDataArray_00469bf0[16];

edCBankBuffer* edCBank_GetBankBuffer(edCBank* bankObj)
{
	edCBankBuffer* returnBankPtr;
	edCBankBuffer* peVar1;
	uint uVar2;

	uVar2 = 0;
	for (returnBankPtr = bankObj->pBankBuffer; (uVar2 < (uint)bankObj->createFlagA && (returnBankPtr->field_0x4 != (int*)0x0)); returnBankPtr = (edCBankBuffer*)&returnBankPtr->field_0x24) {
		uVar2 = uVar2 + 1;
	}
	if (uVar2 == bankObj->createFlagA) {
		returnBankPtr = (edCBankBuffer*)0x0;
	}
	else {
		returnBankPtr->headerComponent = 0;
		returnBankPtr->header = bankObj;
		uVar2 = bankObj->createFlagA * 0x24;
		if ((uVar2 & 0x7ff) == 0) {
			peVar1 = bankObj->pBankBuffer;
		}
		else {
			uVar2 = ((uVar2 >> 0xb) + 1) * 0x800;
			peVar1 = bankObj->pBankBuffer;
		}
		returnBankPtr->field_0x4 = (int*)((int)&peVar1->header + bankObj->flagB + uVar2);
		bankObj->flagC = bankObj->flagC + 1;
	}
	return returnBankPtr;
}

void edCBank_Link_00245420(edBank_Manager* pEdBank, edCBank* bankAllocateAddress)
{
	if (bankAllocateAddress != (edCBank*)0x0) {
		if (pEdBank->bankCount == 1) {
			pEdBank->pNextBank->pNextBank = bankAllocateAddress;
			pEdBank->pNextBank->pPrevBank = bankAllocateAddress;
			bankAllocateAddress->pNextBank = pEdBank->pNextBank;
			bankAllocateAddress->pPrevBank = pEdBank->pNextBank;
		}
		else {
			if (pEdBank->bankCount == 0) {
				pEdBank->pNextBank = bankAllocateAddress;
				bankAllocateAddress->pNextBank = bankAllocateAddress;
				bankAllocateAddress->pPrevBank = bankAllocateAddress;
			}
			else {
				bankAllocateAddress->pNextBank = pEdBank->pNextBank;
				bankAllocateAddress->pPrevBank = pEdBank->pNextBank->pPrevBank;
				pEdBank->pNextBank->pPrevBank->pNextBank = bankAllocateAddress;
				pEdBank->pNextBank->pPrevBank = bankAllocateAddress;
			}
		}
		pEdBank->bankCount = pEdBank->bankCount + 1;
	}
	return;
}

DebugBankData_234* LoadFile(char* filePath, uint flags)
{
	bool bVar1;
	edCFiler* pFiler;
	DebugBankData_234* pDVar2;
	byte* pbVar3;
	int iVar4;
	DebugBankData_234* pDebugBank;
	char outCdPath[512];

	MY_LOG("LoadFile %s\n", filePath);

	pFiler = FindEdCFiler(outCdPath, filePath, 0);
	if (pFiler == (edCFiler*)0x0) {
		pDVar2 = (DebugBankData_234*)0x0;
	}
	else {
		if (((flags & 8) != 0) && (((pFiler->baseData).field_0x4 & 0x20U) == 0)) {
			flags = flags & 0xfffffff7;
		}
		if ((((flags & 2) == 0) || (pDVar2 = (DebugBankData_234*)0x0, ((pFiler->baseData).field_0x4 & 2U) != 0)) &&
			(((flags & 4) == 0 || (pDVar2 = (DebugBankData_234*)0x0, ((pFiler->baseData).field_0x4 & 4U) != 0)))) {
			if (((flags & 8) == 0) || (((pFiler->baseData).field_0x4 & 0x80U) != 0)) {
				Func_0025b0c0(pFiler);
			}
			iVar4 = 0;
			pbVar3 = g_DebugBankLoadFlag_00469be0;
			do {
				if (*pbVar3 == 0) {
					g_DebugBankLoadFlag_00469be0[iVar4] = 1;
					pDebugBank = g_DebugBankDataArray_00469bf0 + iVar4;
					memset(pDebugBank, 0, sizeof(DebugBankData_234));
					goto LAB_0025c168;
				}
				iVar4 = iVar4 + 1;
				pbVar3 = pbVar3 + 1;
			} while (iVar4 < 0x10);
			pDebugBank = (DebugBankData_234*)0x0;
		LAB_0025c168:
			pDVar2 = (DebugBankData_234*)0x0;
			if (pDebugBank != (DebugBankData_234*)0x0) {
				pDebugBank->pOwningFiler = pFiler;
				pDebugBank->openFlags = flags;
				/* May call edCFiler_CDVD_open */
				bVar1 = pFiler->Open(pDebugBank, outCdPath);
				if (bVar1 == false) {
					iVar4 = 0;
					pbVar3 = g_DebugBankLoadFlag_00469be0;
					do {
						if ((DebugBankData_234*)(pbVar3 + 0x10) == pDebugBank) {
							g_DebugBankLoadFlag_00469be0[iVar4] = 0;
							memset(g_DebugBankDataArray_00469bf0 + iVar4, 0, sizeof(DebugBankData_234));
							break;
						}
						iVar4 = iVar4 + 1;
						pbVar3 = pbVar3 + sizeof(DebugBankData_234);
					} while (iVar4 < 0x10);
					pDVar2 = (DebugBankData_234*)0x0;
				}
				else {
					pDebugBank->bInUse = 1;
					pDVar2 = pDebugBank;
				}
			}
		}
	}
	return pDVar2;
}

bool edCFiler_28_Func_0025a8e0(edCFiler_28* param_1, DebugBankData_234* pDebugBank, char* pReadBuffer, uint someSize)
{
	uint uVar1;
	int iVar2;
	edCFiler_28_Internal* puVar3;

	uVar1 = param_1->freeIndexes;
	puVar3 = (edCFiler_28_Internal*)0x0;
	if (uVar1 < 0x18) {
		iVar2 = param_1->currentIndex;
		param_1[(int)(iVar2 + uVar1) % 0x18].internalBank.lastResult = 2;
		puVar3 = &param_1[(int)(iVar2 + uVar1) % 0x18].internalBank;
		if (param_1->freeIndexes == 0) {
			param_1->field_0x8 = param_1[param_1->currentIndex].internalBank.lastResult;
		}
		param_1->freeIndexes = param_1->freeIndexes + 1;
	}
	if (puVar3 == (edCFiler_28_Internal*)0x0) {
		puVar3 = (edCFiler_28_Internal*)0x0;
	}
	else {
		puVar3->mode = 0;
		puVar3->pDataBank = pDebugBank;
		pDebugBank->field_0x228 = pDebugBank->field_0x228 + 1;
	}
	if (puVar3 != (edCFiler_28_Internal*)0x0) {
		puVar3->pReadBuffer = pReadBuffer;
		puVar3->seekOffset2 = someSize;
	}
	return puVar3 != (edCFiler_28_Internal*)0x0;
}

byte UsedInFileLoad(DebugBankData_234* pDebugBank, char* pReadBuffer, uint someSize)
{
	byte bVar1;
	edCFiler_28* peVar2;
	byte* pbVar3;
	int iVar4;

	peVar2 = pDebugBank->pOwningFiler->GetGlobalC_0x1c();
	edCFiler_28_Func_0025a8e0(peVar2, pDebugBank, pReadBuffer, someSize);
	if ((pDebugBank->openFlags & 8) == 0) {
		Func_0025b0c0(pDebugBank->pOwningFiler);
		iVar4 = 0;
		pbVar3 = g_DebugBankLoadFlag_00469be0;
		do {
			if ((DebugBankData_234*)(pbVar3 + 0x10) == pDebugBank) {
				bVar1 = g_DebugBankLoadFlag_00469be0[iVar4];
				goto LAB_0025b580;
			}
			iVar4 = iVar4 + 1;
			pbVar3 = pbVar3 + sizeof(DebugBankData_234);
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

char* ReadFileToBuffer(short heapID, char* filePath, uint flags, DebugBankData_234** outLoadedData)
{
	DebugBankData_234* pDVar1;
	uint uVar2;
	char* __dest = 0;
	ulong uVar3;
	long lVar4;
	ulong __n;
	uint* __s;
	uint auStack4096[1024];

	__s = auStack4096;
	if (((uint)__s & 0x7ff) != 0) {
		__s = (uint*)((((uint)__s >> 0xb) + 1) * 0x800);
	}
	pDVar1 = LoadFile(filePath, flags);
	//if (pDVar1 == (DebugBankData_234*)0x0) {
	//	__dest = (char*)0x0;
	//}
	//else {
	//	uVar2 = FUN_0025b330(pDVar1);
	//	if (uVar2 < 0x40) {
	//		__dest = (char*)0x0;
	//	}
	//	else {
	//		memset(__s, 0, 0x800);
	//		uVar3 = UsedInFileLoad(pDVar1, __s, 0x800);
	//		if (uVar3 == 0) {
	//			__dest = (char*)0x0;
	//		}
	//		else {
	//			FUN_0025bcc0();
	//			__n = SEXT48((int)__s[0x11]);
	//			uVar3 = __n;
	//			if ((0x800 < __n) && ((__n & 0x7ff) != 0)) {
	//				uVar3 = (long)(int)(((__s[0x11] >> 0xb) + 1) * 0x800);
	//			}
	//			__dest = (char*)edMemAlloc(heapID, (int)uVar3);
	//			if (__dest == (char*)0x0) {
	//				FUN_0025b420(pDVar1);
	//				__dest = (char*)0x0;
	//			}
	//			else {
	//				if (__n < 0x801) {
	//					memcpy(__dest, __s, __n);
	//				}
	//				else {
	//					memcpy(__dest, __s, 0x800);
	//					uVar2 = (int)uVar3 - 0x800;
	//					lVar4 = (long)(int)uVar2;
	//					UsedInFileLoad(pDVar1, (uint*)(__dest + 0x800), uVar2);
	//					FUN_0025bcc0();
	//					if (__n != uVar3) {
	//						edMemShrink(__dest, __n, lVar4);
	//					}
	//				}
	//				if (outLoadedData == (DebugBankData_234**)0x0) {
	//					FUN_0025b420(pDVar1);
	//				}
	//				else {
	//					*outLoadedData = pDVar1;
	//				}
	//			}
	//		}
	//	}
	//}
	return __dest;
}

void edCBank_Setup(edCBank* pBankObj, int size, int param_3, BankFilePathContainer* bankPathContainer)
{
	int iVar1;
	edCBankBuffer* peVar2;
	int* piVar3;
	edCBankBuffer* peVar4;
	uint bankFlags;
	uint uVar5;
	edCBank* local_s2_lo_56;

	edCBank_Link_00245420(&g_edCBank_00466d30, pBankObj);
	pBankObj->size = size;
	pBankObj->fileFlagA = bankPathContainer->fileFlagA;
	pBankObj->createFlagA = param_3;
	pBankObj->pBankTypePairData = (TypePairData*)0x0;
	pBankObj->flagB = 0;
	pBankObj->flagC = 0;
	pBankObj->firstField = 0;
	pBankObj->heapID = bankPathContainer->heapID;
	if (pBankObj->heapID == 0) {
		pBankObj->heapID = 1;
	}
	if (g_edCBank_DebugAllowNoWait_00448900 == 0) {
		pBankObj->fileFlagA = pBankObj->fileFlagA & 0xfffffffb;
	}
	if ((pBankObj->fileFlagA & 1U) != 0) {
		pBankObj->createFlagA = pBankObj->createFlagA + 1;
	}
	uVar5 = pBankObj->createFlagA * 0x24;
	if ((uVar5 & 0x7ff) == 0) {
		iVar1 = pBankObj->size;
	}
	else {
		uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
		iVar1 = pBankObj->size;
	}
	peVar2 = (edCBankBuffer*)edMemAllocAlign(pBankObj->heapID, (long)(int)(iVar1 + uVar5), 0x800);
	pBankObj->pBankBuffer = peVar2;
	memset(pBankObj->pBankBuffer, 0, uVar5);
	if ((pBankObj->fileFlagA & 1U) != 0) {
		bankFlags = 0;
		for (peVar2 = pBankObj->pBankBuffer; (bankFlags < (uint)pBankObj->createFlagA && (peVar2->field_0x4 != (int*)0x0)); peVar2 = (edCBankBuffer*)&peVar2->field_0x24) {
			bankFlags = bankFlags + 1;
		}
		if (bankFlags == pBankObj->createFlagA) {
			peVar2 = (edCBankBuffer*)0x0;
		}
		else {
			peVar2->headerComponent = 0;
			peVar2->header = pBankObj;
			uVar5 = pBankObj->createFlagA * 0x24;
			if ((uVar5 & 0x7ff) == 0) {
				peVar4 = pBankObj->pBankBuffer;
			}
			else {
				uVar5 = ((uVar5 >> 0xb) + 1) * 0x800;
				peVar4 = pBankObj->pBankBuffer;
			}
			peVar2->field_0x4 = (int*)((int)&peVar4->header + pBankObj->flagB + uVar5);
			pBankObj->flagC = pBankObj->flagC + 1;
		}
		uVar5 = 1;
		peVar2->headerComponent = 1;
		if ((pBankObj->fileFlagA & 4U) != 0) {
			uVar5 = 9;
		}
		piVar3 = (int*)ReadFileToBuffer((short)pBankObj->heapID, bankPathContainer->filePath, uVar5, &peVar2->pLoadedData);
		peVar2->field_0x4 = piVar3;
		peVar2->fileBuffer = (char*)(piVar3 + 2);
		peVar2->field_0x14 = piVar3[0x11];
		//EmptyFunction();
	}
	return;
}

void edCBank_SetDeserializeData(edCBank* pBank, TypePairData* pTypePairData)
{
	pBank->pBankTypePairData = pTypePairData;
	return;
}

void ClearBankLink_002453b0(edBank_Manager* pBank, edCBank* pBankToRemove)
{
	if (pBankToRemove != (edCBank*)0x0) {
		pBank->bankCount = pBank->bankCount + -1;
		pBankToRemove->pNextBank->pPrevBank = pBankToRemove->pPrevBank;
		pBankToRemove->pPrevBank->pNextBank = pBankToRemove->pNextBank;
		if (pBank->bankCount == 0) {
			pBank->pNextBank = (edCBank*)0x0;
		}
		else {
			if (pBank->pNextBank == pBankToRemove) {
				pBank->pNextBank = pBankToRemove->pNextBank;
			}
		}
		pBankToRemove->pNextBank = (edCBank*)0x0;
		pBankToRemove->pPrevBank = (edCBank*)0x0;
	}
	return;
}

void edCBank_Free_00244e10(edCBank* pBank)
{
	ClearBankLink_002453b0(&g_edCBank_00466d30, pBank);
	if ((pBank->fileFlagA & 1U) != 0) {
		assert(false);
		//FUN_0025b420(pBank->pBankBuffer->pLoadedData);
		edMemFree(pBank->pBankBuffer->field_0x4);
		memset(pBank->pBankBuffer, 0, 0x24);
	}
	edMemFree(pBank->pBankBuffer);
	memset(pBank, 0, sizeof(edCBank));
	return;
}

int g_FileSystemSysHandlerID_00469bc4 = 0x10;
int g_FileSystemHandlers_00469bc8 = 6;
ReadBank_158 g_ReadBank_00466e60;

edSysHandlersNodeTable* PTR_g_SysHandlersNodeTable_00469b80 = &g_SysHandlersNodeTable_00489170;

void ReadBankFileSysFunc(int idA, int idB, char* pReadBuffer)
{
	char* pcVar1;
	TypePairData* pTypePairData;
	undefined* puVar2;
	void* pcVar3;
	DebugBankData_234* pDVar4;
	int iVar5;
	uint uVar6;

	iVar5 = g_ReadBank_00466e60.field_0x4;
	if (g_ReadBank_00466e60.loadedBanks != 0) {
		pcVar1 = g_ReadBank_00466e60.field_0x8[g_ReadBank_00466e60.field_0x4].pReadBuffer;
		if (pReadBuffer == pcVar1) {
			pTypePairData = (TypePairData*)g_ReadBank_00466e60.field_0x8[g_ReadBank_00466e60.field_0x4].pBankTypePairData_0xc;
			puVar2 = g_ReadBank_00466e60.field_0x8[g_ReadBank_00466e60.field_0x4].pObjectReference_0x10;
			(g_ReadBank_00466e60.field_0x8[g_ReadBank_00466e60.field_0x4].pBankHeader_0x0)->accessFlag = 0;
			if ((*(uint*)(pcVar1 + 0xc) & 1) != 0) {
				assert(false);
				//UnpackSomething((int)(pcVar1 + 8));
			}
			if ((g_ReadBank_00466e60.field_0x8[iVar5].fileFlagB_0x18 & 8U) == 0) {
				pcVar3 = (void*)g_ReadBank_00466e60.field_0x8[iVar5].fileFunc_0x14;
				if (pcVar3 != (void*)0x0) {
					assert(false);
					//(*pcVar3)(0, puVar2);
				}
				edCBankFileHeader_get_entry_typepair(pcVar1 + 8, pTypePairData, 0);
				if (pcVar3 != (void*)0x0) {
					assert(false);
				//	(*pcVar3)(1, puVar2);
				}
				//EmptyFunction();
			}
			pDVar4 = g_ReadBank_00466e60.field_0x8[iVar5].pDebugBankData;
			if (pDVar4 != (DebugBankData_234*)0x0) {
				uVar6 = GetFileSize_0025b330(pDVar4);
				*(uint*)(pcVar1 + 0x18) = uVar6;
			}
			g_ReadBank_00466e60.loadedBanks = g_ReadBank_00466e60.loadedBanks + -1;
			if ((g_ReadBank_00466e60.loadedBanks != 0) && (g_ReadBank_00466e60.field_0x4 = g_ReadBank_00466e60.field_0x4 + 1, g_ReadBank_00466e60.field_0x4 == 0xc)) {
				g_ReadBank_00466e60.field_0x4 = 0;
			}
		}
	}
	return;
}

void SetupReadBank_002445a0(ReadBank_158* pReadBank)
{
	memset(pReadBank, 0, sizeof(ReadBank_158));
	edSysHandlersAdd
	(PTR_g_SysHandlersNodeTable_00469b80, (edSysHandlersPoolEntry**)edSysHandlersPoolEntry_ARRAY_00469b84, g_FileSystemSysHandlerID_00469bc4, ESHT_LoadFile, ReadBankFileSysFunc, 1, 1);
	return;
}

void Init_edBank(void)
{
	//edCBank_LoadLib_002454d0(&g_edCBank_00466d30);
	SetupReadBank_002445a0(&g_ReadBank_00466e60);
	//FUN_00247700();
	return;
}
