#include "edCFiler.h"

#include <string.h>
#include <assert.h>
#include "edCFiler_Static.h"
#include "edCFiler_CDVD.h"
#include "edSystem.h"
#include "edCBank.h"
#include "../edVideo/VideoD.h"

edCFiler* g_edCFiler_MCPtr_00448fd8;

edSysHandlerFile g_edSysHandlerFile_00469b84 = edSysHandlerFile(&g_SysHandlersNodeTable_00489170, 0x10, 6);

char* FindEmptyChar(char* param_1)
{
	for (; *param_1 != '\0'; param_1 = param_1 + 1) {
	}
	return param_1;
}

char s_DefaultFilePath_00431460[8] = "";

edCFiler* edFileOpen(char* outString, char* filePath, long mode)
{
    char* pcVar1;
    bool bVar2;
    char* pcVar3;
    char* bufferPos;
    int length;
    long lVar4;
    char cVar5;
    long lVar6;
    char cVar7;
    char* currentCharacter;
    char* pcVar8;
    edCFiler* peVar9;
    char local_200[512];

    /* This function takes in a raw file path, and puts it in the correct format for finding the file on the disc.
       Example:
       In: CDEURO/frontend/kyatitle.g2d
       Out: <CDVD>0:\CDEURO\FRONTEND\kyatitle.g2d
       Return: 3A (58) */
    if (filePath == (char*)0x0) {
        filePath = s_DefaultFilePath_00431460;
    }
    cVar7 = *filePath;
    if (cVar7 == '<') {
        filePath = filePath + 1;
        currentCharacter = outString;
        if (true) {
            do {
                bVar2 = false;
                if (('`' < cVar7) && (cVar7 < '{')) {
                    bVar2 = true;
                }
                cVar5 = cVar7;
                if (bVar2) {
                    cVar5 = cVar7 + -0x20;
                }
                *currentCharacter = cVar5;
                currentCharacter = currentCharacter + 1;
                if (cVar7 == '>') break;
                cVar7 = *filePath;
                filePath = filePath + 1;
            } while (cVar7 != '\0');
        }
        *currentCharacter = '\0';
        peVar9 = g_edCFiler_MCPtr_00448fd8;
    }
    else {
        length = g_edCFiler_Static_0046dea0.CopyDrivePath(outString);
        currentCharacter = outString + length;
        peVar9 = g_edCFiler_MCPtr_00448fd8;
    }
    for (; peVar9 != (edCFiler*)0x0; peVar9 = (peVar9->baseData).pNextEd) {
        bufferPos = (peVar9->baseData).pDriveName_0x0;
        if ((bufferPos != (char*)0x0) && (length = strcmp(outString, bufferPos), length == 0)) goto LAB_00261138;
    }
    peVar9 = (edCFiler*)0x0;
LAB_00261138:
    if (peVar9 == (edCFiler*)0x0) {
        *outString = '\0';
        peVar9 = (edCFiler*)0x0;
    }
    else {
        lVar4 = (long)*filePath;
        if (lVar4 == 0x7c) {
            bufferPos = filePath + 1;
            if (true) {
                do {
                    lVar6 = (lVar4 << 0x38) >> 0x38;
                    bVar2 = false;
                    if ((0x60 < lVar6) && (lVar6 < 0x7b)) {
                        bVar2 = true;
                    }
                    if (bVar2) {
                        lVar6 = (long)((int)((lVar4 << 0x38) >> 0x38) + -0x20);
                    }
                    *currentCharacter = (char)lVar6;
                    lVar4 = (long)*bufferPos;
                    currentCharacter = currentCharacter + 1;
                    bufferPos = bufferPos + 1;
                } while ((lVar4 != 0x7c) && (lVar4 != 0));
            }
            if (*bufferPos != '\0') {
                return (edCFiler*)0x0;
            }
        }
        else {
            bufferPos = SearchString(filePath, ':');
            if (bufferPos == (char*)0x0) {
                length = peVar9->AppendDriveLetter(currentCharacter);
                currentCharacter = currentCharacter + length;
            }
            else {
                cVar7 = *filePath;
                while (filePath = filePath + 1, cVar7 != '\0') {
                    bVar2 = false;
                    if (('`' < cVar7) && (cVar7 < '{')) {
                        bVar2 = true;
                    }
                    cVar5 = cVar7;
                    if (bVar2) {
                        cVar5 = cVar7 + -0x20;
                    }
                    *currentCharacter = cVar5;
                    currentCharacter = currentCharacter + 1;
                    if (cVar7 == ':') break;
                    cVar7 = *filePath;
                }
                *currentCharacter = '\0';
            }
            local_200[0] = '\0';
            bufferPos = (char*)0x0;
            /* Try and find the next / or \\ */
            if ((mode == 0) && ((bufferPos = SearchString(filePath, '/'), bufferPos != (char*)0x0 || (pcVar3 = SearchString(filePath, '\\'), bufferPos = filePath, pcVar3 != (char*)0x0)))) {
                /* Go back through the file and try find '/' and '\\' */
                for (bufferPos = FindEmptyChar(filePath); (*bufferPos != '/' && (*bufferPos != '\\')); bufferPos = bufferPos + -1) {
                }
                bufferPos = bufferPos + 1;
            }
            if ((*filePath == '/') || (*filePath == '\\')) {
                *currentCharacter = '\\';
                filePath = filePath + 1;
                currentCharacter = currentCharacter + 1;
            }
            else {
                g_edCFiler_Static_0046dea0.InitDriveType_00261530(outString);
                currentCharacter = FindEmptyChar(outString);
            }
            cVar7 = '\0';
            pcVar3 = local_200;
            for (; (filePath != bufferPos && ((long)*filePath != 0)); filePath = filePath + 1) {
                lVar4 = (long)*filePath << 0x38;
                lVar6 = lVar4 >> 0x38;
                bVar2 = false;
                if ((0x60 < lVar6) && (lVar6 < 0x7b)) {
                    bVar2 = true;
                }
                if (bVar2) {
                    lVar6 = (long)((int)(lVar4 >> 0x38) + -0x20);
                }
                cVar5 = (char)lVar6;
                if ((char)lVar6 == '/') {
                    cVar5 = '\\';
                }
                if ((cVar5 != '\\') || (cVar7 != '\\')) {
                    *pcVar3 = cVar5;
                    pcVar3 = pcVar3 + 1;
                    cVar7 = cVar5;
                }
            }
            if ((cVar7 != '\0') && (cVar7 != '\\')) {
                *pcVar3 = '\\';
                pcVar3 = pcVar3 + 1;
            }
            *pcVar3 = '\0';
            pcVar3 = local_200;
            while (local_200[0] != '\0') {
                if (local_200[0] == '.') {
                    if (pcVar3[1] == '\\') {
                        pcVar3 = pcVar3 + 2;
                    }
                    else {
                        if ((pcVar3[1] == '.') && (pcVar3 = pcVar3 + 3, pcVar8 = currentCharacter, currentCharacter[-2] != ':')) {
                            do {
                                currentCharacter = pcVar8 + -1;
                                pcVar1 = pcVar8 + -2;
                                pcVar8 = currentCharacter;
                            } while (*pcVar1 != '\\');
                        }
                    }
                }
                else {
                    do {
                        cVar7 = *pcVar3;
                        *currentCharacter = cVar7;
                        pcVar3 = pcVar3 + 1;
                        currentCharacter = currentCharacter + 1;
                        if (cVar7 == '\0') break;
                    } while (cVar7 != '\\');
                }
                local_200[0] = *pcVar3;
            }
            if (bufferPos != (char*)0x0) {
                cVar7 = *bufferPos;
                while (cVar7 != '\0') {
                    bufferPos = bufferPos + 1;
                    *currentCharacter = cVar7;
                    currentCharacter = currentCharacter + 1;
                    cVar7 = *bufferPos;
                }
            }
        }
        *currentCharacter = '\0';
    }
    return peVar9;
}

void edFileSetPath(char* mode)
{
	bool bVar1;
	char* pcVar2;
	edCFiler* peVar3;
	char cVar4;
	char* pRootDrivePathChar;
	char local_200[512];
    char weirdChange;

	peVar3 = edFileOpen(local_200, mode, 1);
	if (peVar3 != (edCFiler*)0x0) {
        g_edCFiler_Static_0046dea0.SetDefaultFileLoad_00261610(local_200);
		pcVar2 = local_200;
		pRootDrivePathChar = g_edCFiler_Static_0046dea0.rootDrivePath;
        weirdChange = local_200[0];
		while (pcVar2 = pcVar2 + 1, weirdChange != '\0') {
			bVar1 = false;
			if (('`' < weirdChange) && (weirdChange < '{')) {
				bVar1 = true;
			}
			cVar4 = weirdChange;
			if (bVar1) {
				cVar4 = weirdChange + -0x20;
			}
			*pRootDrivePathChar = cVar4;
			pRootDrivePathChar = pRootDrivePathChar + 1;
			if (weirdChange == '>') break;
            weirdChange = *pcVar2;
		}
		*pRootDrivePathChar = '\0';
		peVar3->SetDriveLetter(local_200);
	}
	return;
}

edCFiler* GetFirstEdFileHandler_00260e00(edCFiler** param_1)
{
	return *param_1;
}

void Link_00260ec0(edCFiler** param_1, edCFiler* param_2)
{
	edCFiler* peVar1;
	edCFiler* peVar2;

	if (param_2 != (edCFiler*)0x0) {
		peVar1 = *param_1;
		if (peVar1 == (edCFiler*)0x0) {
			(param_2->baseData).pPrevEd = (edCFiler*)0x0;
			(param_2->baseData).pNextEd = (edCFiler*)0x0;
			*param_1 = param_2;
		}
		else {
			for (peVar2 = (peVar1->baseData).pNextEd; peVar2 != (edCFiler*)0x0; peVar2 = (peVar2->baseData).pNextEd) {
				peVar1 = peVar2;
			}
			(param_2->baseData).pPrevEd = peVar1;
			(param_2->baseData).pNextEd = (edCFiler*)0x0;
			(peVar1->baseData).pNextEd = param_2;
		}
	}
	return;
}

void LinkCDVD(void)
{
    Link_00260ec0(&g_edCFiler_MCPtr_00448fd8, (edCFiler*)&g_edCFiler_CDVD_0046c360);
	return;
}

void _edFileInit(void)
{
	byte bVar1;

	//if ((DAT_00448fd1 != '\0') && (bVar1 = IsNetSetup_004893d0(), bVar1 != 0)) {
	//	SetFilePathMode_00261810(s_<net>_00431458);
	//	g_NetFilePtr_00448fd4 = (undefined*)edFileLoadSize(PTR_s_NetFile.log_00448930, 6);
	//	edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, (long)g_SysHandlersMainMaxEventID_0048cf10, New_Name, FUN_00260ae0, 1, 0);
	//}
	//FUN_00260ec0();
	//FUN_0025e630();
	LinkCDVD();
	//FUN_00260910();
	//FUN_00263870();
	return;
}

void ReadsBankFileHandler(int, int, char*)
{
	ReadsBankFile();
}

bool edFileInit(void)
{
	bool bVar1;
	edCFiler* piVar2;
	bool bSuccess;
	bool bVar3;

	//edFileDebugInit();
	_edFileInit();
	//EdFileBase::edFileMemAddFiler();
	piVar2 = GetFirstEdFileHandler_00260e00(&g_edCFiler_MCPtr_00448fd8);
	bVar3 = true;
	for (; piVar2 != (edCFiler*)0x0; piVar2 = (piVar2->baseData).pNextEd) {
        bSuccess = piVar2->Init();
		if (bSuccess == false) {
			bVar3 = false;
		}
	}
	//SHORT_00448fcc = 0x40;
	//INT_00448fc8 = 1;
	//SHORT_00448fce = 0;
	bVar1 = edSysHandlersAdd
	(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, ESHT_RenderScene, ReadsBankFileHandler, 3, 1);
	if (bVar1 == false) {
		bVar3 = false;
	}
	return bVar3;
}



void FormatFilePath_002618e0(char* param_1, char* param_2, char* param_3, char* param_4, char* param_5)
{
	char cVar1;
	char* pcVar2;

	if ((*param_5 != '\0') && (*param_5 == '<')) {
		do {
			cVar1 = *param_5;
			if (param_1 != (char*)0x0) {
				*param_1 = cVar1;
				param_1 = param_1 + 1;
			}
		} while ((cVar1 != '\0') && (param_5 = param_5 + 1, cVar1 != '>'));
	}
	if (param_1 != (char*)0x0) {
		*param_1 = '\0';
	}
	if ((*param_5 != '\0') && (pcVar2 = SearchString(param_5, ':'), pcVar2 != (char*)0x0)) {
		do {
			cVar1 = *param_5;
			if (param_2 != (char*)0x0) {
				*param_2 = cVar1;
				param_2 = param_2 + 1;
			}
		} while ((cVar1 != '\0') && (param_5 = param_5 + 1, cVar1 != ':'));
	}
	if (param_2 != (char*)0x0) {
		*param_2 = '\0';
	}
	if ((*param_5 != '\0') && (pcVar2 = SearchString(param_5, '\\'), pcVar2 != (char*)0x0)) {
		pcVar2 = FindEmptyChar(param_5);
		cVar1 = *pcVar2;
		while (cVar1 != '\\') {
			pcVar2 = pcVar2 + -1;
			cVar1 = *pcVar2;
		}
		while ((param_5 != pcVar2 + 1 && (cVar1 = *param_5, cVar1 != '\0'))) {
			if (param_3 != (char*)0x0) {
				*param_3 = cVar1;
				param_3 = param_3 + 1;
			}
			if (cVar1 != '\0') {
				param_5 = param_5 + 1;
			}
		}
	}
	if (param_3 != (char*)0x0) {
		*param_3 = '\0';
	}
	cVar1 = *param_5;
	if (cVar1 != '\0') {
		while (cVar1 != '\0') {
			if (param_4 != (char*)0x0) {
				*param_4 = cVar1;
				param_4 = param_4 + 1;
			}
			if (cVar1 != '\0') {
				param_5 = param_5 + 1;
			}
			cVar1 = *param_5;
		}
	}
	if (param_4 != (char*)0x0) {
		*param_4 = '\0';
	}
	return;
}

edCFiler::edCFiler()
{	
	baseData.pDriveName_0x0 = (char*)0x0;
	baseData.pPrevEd = (edCFiler*)0x0;
	baseData.pNextEd = (edCFiler*)0x0;
	baseData.field_0x4 = 0;
	filePath[0] = '\0';
	return;
}

edCFiler_28 edCFiler_28_ARRAY_004697a0[24] = { 0 };

edCFiler_28* edCFiler::GetGlobalC_0x1c()
{
	return edCFiler_28_ARRAY_004697a0;
}

void edCFiler::SetDriveLetter(char* szDriveLetter)
{
	FormatFilePath_002618e0((char*)0x0, filePath, (char*)0x0, (char*)0x0, szDriveLetter);
	return;
}

int edCFiler::AppendDriveLetter(char* outString)
{
	int iVar1;

	iVar1 = edStringCpyL(outString, filePath);
	return iVar1;
}

void ReadsBankFile(edCFiler_28* pFiler_28)
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
	DebugBankData_234* pDVar8;
	edCFiler_28_Internal* peVar9;

	if (pFiler_28->freeIndexes != 0) {
		EVar1 = pFiler_28->nextAction;
		pFilerSubObj = pFiler_28 + pFiler_28->currentIndex;
		if (EVar1 == LOAD) {
			iVar7 = (pFilerSubObj->internalBank).mode;
			pDVar8 = (DebugBankData_234*)0x0;
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
			lVar4 = pDVar5->ReadCallback(&pFilerSubObj->internalBank);
			if (lVar4 != 0) {
				if ((pFilerSubObj->internalBank).mode == 0) {
					if (pDVar8->bInUse == 0) {
						iVar7 = pFiler_28->freeIndexes;
						while (iVar7 != 0) {
							pDVar8 = pFiler_28[pFiler_28->currentIndex].internalBank.pDataBank;
							pDVar8->count_0x228 = pDVar8->count_0x228 + -1;
							assert(false);
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
					switch ((pFilerSubObj->internalBank).nextAction) {
					case SEEK:
						edSysHandlersCall(g_edSysHandlerFile_00469b84.mainIdentifier,
							g_edSysHandlerFile_00469b84.entries,
							g_edSysHandlerFile_00469b84.maxEventID, 6, (void*)0x0);
						break;
					case READ_STREAM:
						edSysHandlersCall(g_edSysHandlerFile_00469b84.mainIdentifier,
							g_edSysHandlerFile_00469b84.entries,
							g_edSysHandlerFile_00469b84.maxEventID, 4, (pFilerSubObj->internalBank).pReadBuffer)
							;
						break;
					case BANK_ACTION_3:
						edSysHandlersCall(g_edSysHandlerFile_00469b84.mainIdentifier,
							g_edSysHandlerFile_00469b84.entries,
							g_edSysHandlerFile_00469b84.maxEventID, 5,
							*(void**)&(pFilerSubObj->internalBank).field_0x18);
						break;
					case CLOSE:
						edSysHandlersCall(g_edSysHandlerFile_00469b84.mainIdentifier,
							g_edSysHandlerFile_00469b84.entries,
							g_edSysHandlerFile_00469b84.maxEventID, 3, (void*)0x0);
						if ((pDVar8->openFlags & 0x20) == 0) {
							iVar7 = 0;
							do {
								if (&g_DebugBankDataArray_00469bf0[iVar7] == pDVar8) {
									g_DebugBankLoadFlag_00469be0[iVar7] = 0;
									memset(&g_DebugBankDataArray_00469bf0[iVar7], 0, sizeof(DebugBankData_234));
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
					assert(false);
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
					if (EVar1 == CLOSE) {
						bVar2 = pFiler->Close(pDVar8);
						in_a3 = bVar2 & 0xff;
					}
					else {
						if (EVar1 == BANK_ACTION_3) {
							assert(false);
							uVar3 = 0; //(*(code*)pFiler->pVTable->field_0x44)(pFiler, pDVar8, *(undefined4*)&(pFilerSubObj->internalBank).field_0x18, pFilerSubObj[1].freeIndexes);
							in_a3 = uVar3 & 0xff;
							if (in_a3 != 0) {
								pDVar8->seekOffset = pDVar8->seekOffset + pFilerSubObj[1].freeIndexes;
							}
						}
						else {
							if (EVar1 == READ_STREAM) {
								uVar3 = pFiler->ReadStream(pDVar8, (pFilerSubObj->internalBank).pReadBuffer, (pFilerSubObj->internalBank).seekOffset2);
								in_a3 = uVar3 & 0xff;
								if (in_a3 != 0) {
									pDVar8->seekOffset = pDVar8->seekOffset + (pFilerSubObj->internalBank).seekOffset2;
								}
							}
							else {
								if (EVar1 == SEEK) {
									pDVar8->seekOffset = (pFilerSubObj->internalBank).seekOffset;
									bVar2 = pFiler->Seek(pDVar8);
									in_a3 = (long)bVar2 & 0xff;
								}
								else {
									if (EVar1 == OPEN) {
										assert(false);
										bVar2 = 0; //(*pFiler->pVTable->Open)(pFiler, pDVar8, (char*)pFilerSubObj[1].field_0x8);
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
					assert(false);
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

void edFileGetFiler(edCFiler* pFiler)
{
	bool bVar1;
	edCFiler* peVar2;
	edCFiler_28* peVar3;

	do {
		bVar1 = true;
		for (peVar2 = GetFirstEdFileHandler_00260e00(&g_edCFiler_MCPtr_00448fd8); peVar2 != (edCFiler*)0x0; peVar2 = (peVar2->baseData).pNextEd) {
            peVar3 = peVar2->GetGlobalC_0x1c();
			if ((peVar3 != (edCFiler_28*)0x0) && (peVar3->freeIndexes != 0)) {
				ReadsBankFile(peVar3);
				if (pFiler == (edCFiler*)0x0) {
					bVar1 = false;
				}
				else {
					if (peVar2 == pFiler) {
						bVar1 = false;
					}
				}
			}
		}
	} while (!bVar1);
	return;
}

bool edFileFilerConfigure(char* path, ETableOfContentsInitMode mode, void* param_3, int* param_4)
{
	bool bVar1;
	edCFiler_CDVD* peVar1;
	int iVar2;
	char acStack592[528];
	ETableOfContentsInitMode EStack64;
	long local_30;
	long local_28;

	local_28 = (long)(int)param_4;
	local_30 = (long)(int)param_3;
	EStack64 = mode;
	peVar1 = (edCFiler_CDVD*)edFileOpen(acStack592, path, 1);
	if (peVar1 == (edCFiler_CDVD*)0x0) {
		bVar1 = false;
	}
	else {
		if (true) {
			iVar2 = 0x30;
		}
		else {
			iVar2 = 0;
		}

        InitTableOfContentsParams params;
        params.field_0x0 = param_3;
        params.field_0x8 = param_4;

		bVar1 = peVar1->InitTableOfContents(acStack592, EStack64, &params);
	}
	return bVar1;
}

bool FormatStreamPath(char* filePathOut, char* filePathIn)
{
	bool uVar1;
	edCFiler* peVar1;
	char acStack512[512];

	if ((filePathOut == (char*)0x0) || (filePathIn == (char*)0x0)) {
		uVar1 = false;
	}
	else {
		*filePathOut = '\0';
		peVar1 = edFileOpen(acStack512, filePathIn, 0);
		if (peVar1 == (edCFiler*)0x0) {
			uVar1 = false;
		}
		else {
			uVar1 = peVar1->FormatStreamPath(filePathOut, acStack512);
		}
	}
	return uVar1;
}

void* GetInternalData_0025b2e0(DebugBankData_234* pDebugBankData)
{
	void* pvVar1;

	pvVar1 = (void*)0x0;
	if ((pDebugBankData != (DebugBankData_234*)0x0) &&
		(pvVar1 = pDebugBankData->pFileData, pvVar1 == (void*)0x0)) {
		pvVar1 = (void*)0x0;
	}
	return pvVar1;
}
