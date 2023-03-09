#include "edCFiler_BNK.h"
#include <string>
#include "edCBank.h"
#include "edCFiler_CDVD.h"

edCFiler_BNK g_edCFiler_BNK_00467fe0;

char* sz_BNK_DriveName_0042ffc8 = "<BNK>";
char* sz_BNK_DriveLetter_0042ffd0 = "0:";

edCFiler_BNK::edCFiler_BNK()
{
	int iVar2;

	iVar2 = 0;
	do {
		bnkInUse[iVar2] = 0;
		memset(&field_0x218[iVar2], 0, sizeof(edCFiler_BNK_8));
		iVar2 = iVar2 + 1;
	} while (iVar2 < 4);
	//SetupEd10_00217720((undefined*)&Singleton_00467fe0, Free_00246df0, &EdFileGlobal_10_00467fd0);
	return;
}

bool edCFiler_BNK::Init()
{
	/* <BNK> */
	baseData.pDriveName_0x0 = sz_BNK_DriveName_0042ffc8;
	baseData.field_0x4 = 1;
	g_edCFiler_BNK_00467fe0.SetDriveLetter(sz_BNK_DriveLetter_0042ffd0);
	return true;
}

bool edCFiler_BNK::FormatStreamPath(char* outFilePath, char* pathBuff)
{
	bool bVar1;

	if ((outFilePath == (char*)0x0) || (pathBuff == (char*)0x0)) {
		bVar1 = false;
	}
	else {
		*outFilePath = '\0';
		bVar1 = true;
	}
	return bVar1;
}

edCFiler_Bnk_static g_BnkInternalLoadedData_00466fc0[4];
DebugBankData_234* g_LastBank_00448f50 = NULL;

bool edCFiler_BNK::Open(DebugBankData_234* pOutData, char* filePath)
{
	bool bVar1;
	int iVar2;
	char* pcVar3;
	int iVar4;
	DebugBankData_234* pDVar5;
	uint* puVar6;
	ulong uVar7;
	byte* pbVar8;
	edCFiler_Bnk_static* pAlreadyLoadedData;
	edCFiler_BNK_8* bufferStart;
	char acStack1024[512];
	char acStack512[512];

	MY_LOG("edCFiler_BNK::Open %s\n", filePath);

	iVar4 = 0;
	do {
		if (bnkInUse[iVar4] == 0) {
			bnkInUse[iVar4] = 1;
			bufferStart = field_0x218 + iVar4;
			memset(bufferStart, 0, 8);
			break;
		}
		iVar4 = iVar4 + 1;
		bufferStart = (edCFiler_BNK_8*)0x0;
	} while (iVar4 < 4);
	bVar1 = false;
	if (bufferStart != (edCFiler_BNK_8*)0x0) {
		FormatFilePath_002618e0((char*)0x0, acStack512, (char*)0x0, (char*)0x0, filePath);
		iVar4 = 0;
		/* Look to see if we already loaded the file. */
		pAlreadyLoadedData = g_BnkInternalLoadedData_00466fc0;
		do {
			iVar2 = strcmp(pAlreadyLoadedData->path, acStack512);
			if (iVar2 == 0) goto LAB_00247280;
			iVar4 = iVar4 + 1;
			pAlreadyLoadedData = pAlreadyLoadedData + 1;
		} while (iVar4 < 4);
		pAlreadyLoadedData = (edCFiler_Bnk_static*)0x0;
	LAB_00247280:
		bVar1 = false;
		if (pAlreadyLoadedData != (edCFiler_Bnk_static*)0x0) {
			pcVar3 = SearchString(filePath, '>');
			pcVar3 = SearchString(pcVar3 + 1, ':');
			edStringCpyL(acStack1024, pcVar3 + 1);
			pcVar3 = pAlreadyLoadedData->pFileData;
			/* Look for the index of the file we want. */
			iVar4 = GetIndexFromFileHeader((edCBankFileHeader*)(pcVar3 + 8), acStack1024);
			if (iVar4 == -1) {
				bVar1 = false;
			}
			else {
				/* Okay we found the index, load the file. */
				pDVar5 = LoadFile(pAlreadyLoadedData->diskPath, 1);
				iVar2 = 0;
				if (pDVar5 == (DebugBankData_234*)0x0) {
					pbVar8 = bnkInUse;
					do {
						if ((edCFiler_BNK_8*)(pbVar8 + 4) == bufferStart) {
							bnkInUse[iVar2] = 0;
							memset(field_0x218 + iVar2, 0, 8);
							break;
						}
						iVar2 = iVar2 + 1;
						pbVar8 = pbVar8 + 8;
					} while (iVar2 < 4);
					bVar1 = false;
				}
				else {
					bufferStart->pDebugBankData = pDVar5;
					g_LastBank_00448f50 = pDVar5;
					puVar6 = (uint*)edCBankFileHeader_FindFileDataInHeader((char*)(edCBankFileHeader*)(pcVar3 + 8), iVar4);
					uVar7 = DebugBankSeek(bufferStart->pDebugBankData, *puVar6, ED_SEEK_SET);
					if (uVar7 == 0) {
						pDVar5->pOwningFiler->Close(pDVar5);
						iVar4 = 0;
						pbVar8 = bnkInUse;
						do {
							if ((edCFiler_BNK_8*)(pbVar8 + 4) == bufferStart) {
								bnkInUse[iVar4] = 0;
								memset(field_0x218 + iVar4, 0, 8);
								break;
							}
							iVar4 = iVar4 + 1;
							pbVar8 = pbVar8 + 8;
						} while (iVar4 < 4);
						bVar1 = false;
					}
					else {
						bufferStart->pFileData = (char*)puVar6;
						memset(&pOutData->field_0x10, 0, 0x218);
						pcVar3 = SearchForColon(filePath);
						edStringCpyL((pOutData->field_0x10).name, pcVar3 + 1);
						bVar1 = true;
						(pOutData->field_0x10).field_0x14 = 0x21;
						(pOutData->field_0x10).fileSize = *(uint*)(bufferStart->pFileData + 4);
						pOutData->pFileData = (CDFileContainer*)bufferStart;
					}
				}
			}
		}
	}
	return bVar1;
}

bool edCFiler_BNK::Close(DebugBankData_234* pDebugBank)
{
	edCFiler* peVar1;
	edCFiler_BNK_8* ppiVar1;

	MY_LOG("edCFiler_BNK::Close\n");

	ppiVar1 = (edCFiler_BNK_8*)GetInternalData_0025b2e0(pDebugBank);
	if (ppiVar1 != (edCFiler_BNK_8*)0x0) {
		peVar1 = ppiVar1->pDebugBankData->pOwningFiler;
		peVar1->Close(ppiVar1->pDebugBankData);
	}
	return ppiVar1 != (edCFiler_BNK_8*)0x0;
}

uint edCFiler_BNK::ReadStream(DebugBankData_234* pDebugBank, char* destination, uint requiredSize)
{
	edCFiler* peVar1;
	edCFiler_BNK_8* ppiVar2;
	uint uVar2;

	ppiVar2 = (edCFiler_BNK_8*)GetInternalData_0025b2e0(pDebugBank);
	if (ppiVar2 == (edCFiler_BNK_8*)0x0) {
		uVar2 = 0;
	}
	else {
		if (requiredSize == (pDebugBank->field_0x10).fileSize) {
			peVar1 = ppiVar2->pDebugBankData->pOwningFiler;
			uVar2 = peVar1->ReadStream(ppiVar2->pDebugBankData, destination, requiredSize);
		}
		else {
			uVar2 = 0;
		}
	}
	return uVar2;
}

bool edCFiler_BNK::Seek(DebugBankData_234* pDebugBank)
{
	DebugBankData_234* pData;
	bool uVar1;
	edCFiler_BNK_8* ppiVar2;

	ppiVar2 = (edCFiler_BNK_8*)GetInternalData_0025b2e0(pDebugBank);
	if (ppiVar2 == (edCFiler_BNK_8*)0x0) {
		uVar1 = false;
	}
	else {
		if (*(uint*)((int)ppiVar2->pFileData + 4) < (uint)pDebugBank->seekOffset) {
			uVar1 = false;
		}
		else {
			pData = ppiVar2->pDebugBankData;
			pData->seekOffset = *(int*)ppiVar2->pFileData + pDebugBank->seekOffset;
			uVar1 = pData->pOwningFiler->Seek(pData);
		}
	}
	return uVar1;
}

bool edCFiler_BNK::ReadCallback(edCFiler_28_Internal* pEdFilerInternal)
{
	DebugBankData_234* pDebugBankData;
	DebugBankData_234* pDVar1;
	edCFiler* peVar2;
	EBankAction EVar3;
	bool bVar4;
	edCFiler_BNK_8* pCVar4;
	long lVar5;
	byte* pbVar6;
	int iVar7;

	pDebugBankData = pEdFilerInternal->pDataBank;
	pCVar4 = (edCFiler_BNK_8*)GetInternalData_0025b2e0(pDebugBankData);
	if (pCVar4 == (edCFiler_BNK_8*)0x0) {
		bVar4 = false;
	}
	else {
		pDVar1 = pCVar4->pDebugBankData;
		peVar2 = pDVar1->pOwningFiler;
		pDVar1->action = pEdFilerInternal->nextAction;
		EVar3 = pEdFilerInternal->nextAction;
		if (((EVar3 == BANK_ACTION_3) || (EVar3 == READ_STREAM)) || (bVar4 = true, EVar3 == SEEK)) {
			pEdFilerInternal->pDataBank = pDVar1;
			lVar5 = peVar2->ReadCallback(pEdFilerInternal);
			if (lVar5 == 0) {
				pEdFilerInternal->pDataBank = pDebugBankData;
				bVar4 = false;
			}
			else {
				pEdFilerInternal->pDataBank = pDebugBankData;
				bVar4 = true;
			}
		}
		else {
			if (EVar3 == CLOSE) {
				pEdFilerInternal->pDataBank = pDVar1;
				lVar5 = peVar2->ReadCallback(pEdFilerInternal);
				if (lVar5 == 0) {
					pEdFilerInternal->pDataBank = pDebugBankData;
					bVar4 = false;
				}
				else {
					pEdFilerInternal->pDataBank = pDebugBankData;
					iVar7 = 0;
					do {
						if (&field_0x218[iVar7] == pCVar4) {
							this->bnkInUse[iVar7] = 0;
							memset(&field_0x218[iVar7], 0, sizeof(edCFiler_BNK_8));
							break;
						}
						iVar7 = iVar7 + 1;
					} while (iVar7 < 4);
					iVar7 = 0;
					do {
						if (&g_DebugBankDataArray_00469bf0[iVar7] == pDVar1) {
							g_DebugBankLoadFlag_00469be0[iVar7] = 0;
							memset(&g_DebugBankDataArray_00469bf0[iVar7], 0, sizeof(DebugBankData_234));
							break;
						}
						iVar7 = iVar7 + 1;
					} while (iVar7 < 0x10);
					bVar4 = true;
				}
			}
		}
	}
	return bVar4;
}

bool edCFiler_BNK::LoadAndStoreInternal(char* filePath, char* bankPath)
{
	bool bSuccess;
	edCFiler* peVar1;
	int iVar2;
	char* pcVar3;
	int iVar4;
	edCFiler_Bnk_static* __s;
	edCFiler_Bnk_static* peVar5;
	char formattedPath[512];
	char outString[512];

	peVar1 = FindEdCFiler(outString, bankPath, 0);
	if (peVar1 == (edCFiler*)0x0) {
		bSuccess = false;
	}
	else {
		FormatFilePath_002618e0((char*)0x0, formattedPath, (char*)0x0, (char*)0x0, filePath);
		iVar4 = 0;
		/* Check if the bank is already loaded. */
		peVar5 = g_BnkInternalLoadedData_00466fc0;
		do {
			iVar2 = strcmp(peVar5->path, formattedPath);
			if (iVar2 == 0) goto LAB_002475e0;
			iVar4 = iVar4 + 1;
			peVar5 = peVar5 + 1;
		} while (iVar4 < 4);
		peVar5 = (edCFiler_Bnk_static*)0x0;
	LAB_002475e0:
		__s = (edCFiler_Bnk_static*)0x0;
		if (peVar5 == (edCFiler_Bnk_static*)0x0) {
			__s = g_BnkInternalLoadedData_00466fc0;
			iVar4 = 0;
			do {
				if (__s->path[0] == '\0') {
					memset(__s, 0, 0x404);
					edStringCpyL(__s->path, formattedPath);
					goto LAB_00247648;
				}
				iVar4 = iVar4 + 1;
				__s = __s + 1;
			} while (iVar4 < 4);
			__s = (edCFiler_Bnk_static*)0x0;
		}
	LAB_00247648:
		bSuccess = false;
		if (__s != (edCFiler_Bnk_static*)0x0) {
			edStringCpyL(__s->diskPath, outString);
			pcVar3 = ReadFileToBuffer(1, bankPath, 1, (DebugBankData_234**)0x0);
			__s->pFileData = pcVar3;
			bSuccess = true;
		}
	}
	return bSuccess;
}

void Link_00247700(void)
{
	Link_00260ec0(&g_edCFiler_MCPtr_00448fd8, &g_edCFiler_BNK_00467fe0);
	/* <BNK> */
	g_edCFiler_BNK_00467fe0.baseData.pDriveName_0x0 = sz_BNK_DriveName_0042ffc8;
	g_edCFiler_BNK_00467fe0.baseData.field_0x4 = 1;
	g_edCFiler_BNK_00467fe0.SetDriveLetter(sz_BNK_DriveLetter_0042ffd0);
	return;
}
