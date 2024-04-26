#include "edFile.h"
#include "edSystem.h"
#include "edMem.h"

#include "_ps2/_edFileFilerCDVD.h"
#include "edVideo/VideoD.h"

byte edFileHandleTable[16];
edFILEH edFileHandleData[16];

edSysHandlerFile edFileHandlers = edSysHandlerFile(&g_SysHandlersNodeTable_00489170, 0x10, 6);

void* GetInternalData_0025b2e0(edFILEH* pDebugBankData)
{
	void* pvVar1;

	pvVar1 = (void*)0x0;
	if ((pDebugBankData != (edFILEH*)0x0) &&
		(pvVar1 = pDebugBankData->pFileData, pvVar1 == (void*)0x0)) {
		pvVar1 = (void*)0x0;
	}
	return pvVar1;
}

char* edFileOpen(char* filePath, uint* outSize, uint flags)
{
	byte bVar1;
	edFILEH* pDebugBank;
	edCFiler* peVar2;
	uint uVar3;
	char* pReadBuffer;
	edCFiler_28* peVar4;
	byte* pbVar5;
	int iVar6;
	char acStack512[512];

	MY_LOG("MessageFile::edFileOpen {}\n", filePath);

	*outSize = 0;
	pDebugBank = edFileOpen(filePath, flags | 1);
	if (pDebugBank == (edFILEH*)0x0) {
		pReadBuffer = (char*)0x0;
	}
	else {
		peVar2 = edFileGetFiler(acStack512, filePath, 0);
		if (peVar2 == (edCFiler*)0x0) {
			pReadBuffer = (char*)0x0;
		}
		else {
			uVar3 = 0;
			if ((pDebugBank->openFlags & 6) == 0) {
				uVar3 = (pDebugBank->field_0x10).fileSize;
				//peVar7 = peVar2->pVTable;
			}
			else {
				//peVar7 = peVar2->pVTable;
			}

			uVar3 = peVar2->getalignedsize(uVar3);
			*outSize = uVar3;
			pReadBuffer = (char*)edMemAllocAlignBoundary(edFileLoadInfo.heap, *outSize, (uint)edFileLoadInfo.align,
				(uint)edFileLoadInfo.offset);
			edFileLoadInfo.offset = 0;
			edFileLoadInfo.heap = TO_HEAP(H_MAIN);
			edFileLoadInfo.align = 0x40;
			peVar4 = peVar2->GetGlobalC_0x1c();
			SetBankReadStream(peVar4, pDebugBank, pReadBuffer, *outSize);
			peVar4 = peVar2->GetGlobalC_0x1c();
			SetBankClose(peVar4, pDebugBank);
			if ((pDebugBank->openFlags & 8) == 0) {
				edFileGetFiler(pDebugBank->pOwningFiler);
				iVar6 = 0;
				do {
					if (&edFileHandleData[iVar6] == pDebugBank) {
						bVar1 = edFileHandleTable[iVar6];
						goto LAB_0025bac8;
					}
					iVar6 = iVar6 + 1;
				} while (iVar6 < 0x10);
				bVar1 = 0;
			LAB_0025bac8:
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
			if (bVar1 == 0) {
				*outSize = 0;
				edMemFree(pReadBuffer);
				pReadBuffer = (char*)0x0;
			}
		}
	}
	return pReadBuffer;
}

edFILEH* edFileOpen(char* szFilePath, uint flags)
{
	bool bOpenSuccess;
	edCFiler* pFiler;
	edFILEH* pFile;
	int bankIndex;
	edFILEH* pDebugBank;
	char szFilerPath[512];

	MY_LOG("edFileOpen {}\n", szFilePath);

	pFiler = edFileGetFiler(szFilerPath, szFilePath, 0);

	MY_LOG("edFileOpen Filer Path {}\n", szFilerPath);

	if (pFiler == (edCFiler*)0x0) {
		pFile = (edFILEH*)0x0;
	}
	else {
		if (((flags & 8) != 0) && ((pFiler->field_0x4 & 0x20U) == 0)) {
			flags = flags & 0xfffffff7;
		}
		if ((((flags & 2) == 0) || (pFile = (edFILEH*)0x0, (pFiler->field_0x4 & 2U) != 0)) &&
			(((flags & 4) == 0 || (pFile = (edFILEH*)0x0, (pFiler->field_0x4 & 4U) != 0)))) {
			if (((flags & 8) == 0) || ((pFiler->field_0x4 & 0x80U) != 0)) {
				edFileGetFiler(pFiler);
			}
			bankIndex = 0;
			do {
				if (edFileHandleTable[bankIndex] == 0) {
					edFileHandleTable[bankIndex] = 1;
					pDebugBank = &edFileHandleData[bankIndex];
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
						if (&edFileHandleData[bankIndex] == pDebugBank) {
							edFileHandleTable[bankIndex] = 0;
							memset(&edFileHandleData[bankIndex], 0, sizeof(edFILEH));
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
			if (&edFileHandleData[iVar5] == pDebugBank) {
				bVar2 = edFileHandleTable[iVar5];
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
			if (&edFileHandleData[iVar5] == pDebugBank) {
				bVar2 = edFileHandleTable[iVar5];
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
		pbVar3 = edFileHandleTable;
		do {
			if ((edFILEH*)(pbVar3 + 0x10) == pDebugBank) {
				bVar1 = edFileHandleTable[iVar4];
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

bool edFileFlush(void)
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
			if (&edFileHandleData[iVar5] == pDebugBank) {
				bVar2 = edFileHandleTable[iVar5];
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

const char* sz_edCBankBuffer_Wait_0042fb70 = "edCBankBufferEntry::file_access: Wait for end of previous loading operation \n";

void edFileNoWaitStackFlush(void)
{
	edCFiler* peVar1;
	edCFiler_28* peVar2;

	for (peVar1 = edFilerList.get_root(); peVar1 != (edCFiler*)0x0; peVar1 = peVar1->pNextEd) {
		peVar2 = peVar1->GetGlobalC_0x1c();
		if (peVar2 != (edCFiler_28*)0x0) {
			edFileNoWaitStackCallBack(peVar2);
		}
	}
	return;
}

uint edFileGetSize(edFILEH* param_1)
{
	uint uVar1;

	uVar1 = 0;
	if ((param_1->openFlags & 6) == 0) {
		uVar1 = (param_1->field_0x10).fileSize;
	}
	return uVar1;
}

void _edFileInit(void)
{
	byte bVar1;

	//if ((DAT_00448fd1 != '\0') && (bVar1 = edPsx2IsNetConnection(), bVar1 != 0)) {
	//	edFileSetPath(s_<net>_00431458);
	//	g_NetFilePtr_00448fd4 = (undefined*)edFileOpen(PTR_s_NetFile.log_00448930, 6);
	//	edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, (long)g_SysHandlersMainMaxEventID_0048cf10, New_Name, FUN_00260ae0, 1, 0);
	//}
	//_edFileMCardAddFiler();
	//_edFileHostPcAddFiler();
	_edFileCDVDAddFiler();
	//_edFileNetAddFiler();
	//_edFileHDDAddFiler();
	return;
}

void ReadsBankFileHandler(int, int, char*)
{
	edFileNoWaitStackFlush();
}

edFileLoadConfig edFileLoadInfo;

bool edFileInit(void)
{
	bool bVar1;
	edCFiler* piVar2;
	bool bSuccess;
	bool bVar3;

	//edFileDebugInit();
	_edFileInit();
	//EdFileBase::edFileMemAddFiler();
	piVar2 = edFilerList.get_root();
	bVar3 = true;
	for (; piVar2 != (edCFiler*)0x0; piVar2 = piVar2->pNextEd) {
		bSuccess = piVar2->initialize();
		if (bSuccess == false) {
			bVar3 = false;
		}
	}
	edFileLoadInfo.offset = 0;
	edFileLoadInfo.heap = TO_HEAP(H_MAIN);
	edFileLoadInfo.align = 0x40;
	bVar1 = edSysHandlersAdd
	(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, ESHT_RenderScene, ReadsBankFileHandler, 3, 1);
	if (bVar1 == false) {
		bVar3 = false;
	}
	return bVar3;
}

// #Hack
char* g_FileActionNames[] = {
	"BANK_ACTION_3",
	"BANK_ACTION_5",
	"CLOSE",
	"LOAD",
	"OPEN",
	"READ_STREAM",
	"SEEK"
};

void edFileNoWaitStackCallBack(edCFiler_28* pFiler_28)
{
	EBankAction EVar1;
	int puVar1;
	edCFiler* pFiler;
	bool bVar2;
	ulong uVar3;
	long lVar4;
	edCFiler_28* pFilerSubObj;
	edCFiler* pDVar5;
	byte* pbVar6;
	int iVar7;
	ulong in_a3;
	edFILEH* pDVar8;
	edCFiler_28_Internal* peVar9;

	if (pFiler_28->freeIndexes != 0) {
		EVar1 = pFiler_28->nextAction;
		pFilerSubObj = pFiler_28 + pFiler_28->currentIndex;
		if (EVar1 == LOAD) {
			iVar7 = (pFilerSubObj->internalBank).mode;
			pDVar8 = (edFILEH*)0x0;
			pDVar5 = (edCFiler*)0x0;
			if (iVar7 == 0) {
				pDVar8 = (pFilerSubObj->internalBank).pDataBank;
				pDVar5 = pDVar8->pOwningFiler;
			}
			else {
				if (iVar7 == 1) {
					pDVar5 = (edCFiler*)(pFilerSubObj->internalBank).pDataBank;
				}
			}
			lVar4 = pDVar5->isnowaitcmdend(&pFilerSubObj->internalBank);
			if (lVar4 != 0) {
				if ((pFilerSubObj->internalBank).mode == 0) {
					if (pDVar8->bInUse == 0) {
						iVar7 = pFiler_28->freeIndexes;
						while (iVar7 != 0) {
							pDVar8 = pFiler_28[pFiler_28->currentIndex].internalBank.pDataBank;
							pDVar8->count_0x228 = pDVar8->count_0x228 + -1;
							IMPLEMENTATION_GUARD();
							//CallFilerFunction_0025b7c0(pDVar8);
							pFiler_28->freeIndexes = pFiler_28->freeIndexes + -1;
							pFiler_28->currentIndex = pFiler_28->currentIndex + 1;
							if (pFiler_28->currentIndex == 0x18) {
								pFiler_28->currentIndex = 0;
							}
							iVar7 = pFiler_28->freeIndexes;
						}
						return;
					}
					pDVar8->count_0x228 = pDVar8->count_0x228 + -1;
				}
				if (true) {
					MY_LOG("Doing file action {}\n", g_FileActionNames[(int)(pFilerSubObj->internalBank).nextAction]);
					switch ((pFilerSubObj->internalBank).nextAction) {
					case SEEK:
						edSysHandlersCall(edFileHandlers.mainIdentifier,
							edFileHandlers.entries,
							edFileHandlers.maxEventID, 6, (void*)0x0);
						break;
					case READ_STREAM:
						edSysHandlersCall(edFileHandlers.mainIdentifier,
							edFileHandlers.entries,
							edFileHandlers.maxEventID, 4, (pFilerSubObj->internalBank).pReadBuffer)
							;
						break;
					case BANK_ACTION_3:
						edSysHandlersCall(edFileHandlers.mainIdentifier,
							edFileHandlers.entries,
							edFileHandlers.maxEventID, 5,
							*(void**)&(pFilerSubObj->internalBank).field_0x18);
						break;
					case CLOSE:
						edSysHandlersCall(edFileHandlers.mainIdentifier,
							edFileHandlers.entries,
							edFileHandlers.maxEventID, 3, (void*)0x0);
						if ((pDVar8->openFlags & 0x20) == 0) {
							iVar7 = 0;
							do {
								if (&edFileHandleData[iVar7] == pDVar8) {
									edFileHandleTable[iVar7] = 0;
									memset(&edFileHandleData[iVar7], 0, sizeof(edFILEH));
									break;
								}
								iVar7 = iVar7 + 1;
							} while (iVar7 < 0x10);
						}
					}
				}
				pFiler_28->freeIndexes = pFiler_28->freeIndexes + -1;
				if (pFiler_28->freeIndexes != 0) {
					pFiler_28->currentIndex = pFiler_28->currentIndex + 1;
					if (pFiler_28->currentIndex == 0x18) {
						pFiler_28->currentIndex = 0;
						peVar9 = &pFiler_28->internalBank;
					}
					else {
						peVar9 = &pFilerSubObj[1].internalBank;
					}
					pFiler_28->nextAction = peVar9->nextAction;
				}
			}
		}
		else {
			iVar7 = (pFilerSubObj->internalBank).mode;
			if (iVar7 == 1) {
				if (EVar1 == BANK_ACTION_5) {
					IMPLEMENTATION_GUARD();
					uVar3 = 0; //(*(code*)((pFilerSubObj->internalBank).pDataBank)->pOwningFiler[2].baseData.field_0x4)();
					in_a3 = uVar3 & 0xff;
				}
				else {
					in_a3 = 0;
				}
			}
			else {
				if (iVar7 == 0) {
					pDVar8 = (pFilerSubObj->internalBank).pDataBank;
					pFiler = pDVar8->pOwningFiler;
					pDVar8->action = EVar1;
					EVar1 = pFiler_28->nextAction;
					MY_LOG("Doing file action {}\n", g_FileActionNames[(int)EVar1]);
					if (EVar1 == CLOSE) {
						bVar2 = pFiler->close(pDVar8);
						in_a3 = bVar2 & 0xff;
					}
					else {
						if (EVar1 == BANK_ACTION_3) {
							IMPLEMENTATION_GUARD();
							uVar3 = 0; //(*(code*)pFiler->pVTable->field_0x44)(pFiler, pDVar8, *(undefined4*)&(pFilerSubObj->internalBank).field_0x18, pFilerSubObj[1].freeIndexes);
							in_a3 = uVar3 & 0xff;
							if (in_a3 != 0) {
								pDVar8->seekOffset = pDVar8->seekOffset + pFilerSubObj[1].freeIndexes;
							}
						}
						else {
							if (EVar1 == READ_STREAM) {
								uVar3 = pFiler->read(pDVar8, (pFilerSubObj->internalBank).pReadBuffer, (pFilerSubObj->internalBank).seekOffset2);
								in_a3 = uVar3 & 0xff;
								if (in_a3 != 0) {
									pDVar8->seekOffset = pDVar8->seekOffset + (pFilerSubObj->internalBank).seekOffset2;
								}
							}
							else {
								if (EVar1 == SEEK) {
									pDVar8->seekOffset = (pFilerSubObj->internalBank).seekOffset;
									bVar2 = pFiler->seek(pDVar8);
									in_a3 = (long)bVar2 & 0xff;
								}
								else {
									if (EVar1 == OPEN) {
										IMPLEMENTATION_GUARD();
										bVar2 = 0; //(*pFiler->pVTable->open)(pFiler, pDVar8, (char*)pFilerSubObj[1].field_0x8);
										in_a3 = (long)bVar2 & 0xff;
									}
									else {
										in_a3 = 0;
									}
								}
							}
						}
					}
					if (in_a3 != 0) {
						pDVar8->bInUse = 1;
					}
				}
			}
			if (in_a3 == 0) {
				iVar7 = pFiler_28->freeIndexes;
				while (iVar7 != 0) {
					pDVar8 = pFiler_28[pFiler_28->currentIndex].internalBank.pDataBank;
					pDVar8->count_0x228 = pDVar8->count_0x228 + -1;
					IMPLEMENTATION_GUARD();
					//CallFilerFunction_0025b7c0(pDVar8);
					pFiler_28->freeIndexes = pFiler_28->freeIndexes + -1;
					pFiler_28->currentIndex = pFiler_28->currentIndex + 1;
					if (pFiler_28->currentIndex == 0x18) {
						pFiler_28->currentIndex = 0;
					}
					iVar7 = pFiler_28->freeIndexes;
				}
			}
			else {
				pFiler_28->nextAction = LOAD;
			}
		}
	}
	return;
}

bool edFileGetPhysicalFileName(char* filePathOut, char* filePathIn)
{
	bool uVar1;
	edCFiler* peVar1;
	char acStack512[512];

	if ((filePathOut == (char*)0x0) || (filePathIn == (char*)0x0)) {
		uVar1 = false;
	}
	else {
		*filePathOut = '\0';
		peVar1 = edFileGetFiler(acStack512, filePathIn, 0);
		if (peVar1 == (edCFiler*)0x0) {
			uVar1 = false;
		}
		else {
			uVar1 = peVar1->get_physical_filename(filePathOut, acStack512);
		}
	}
	return uVar1;
}