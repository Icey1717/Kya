#include "TranslatedTextData.h"
#include "edMem.h"
#include "edC/edCBankBuffer.h"
#include <stdio.h>
#include "edC/edCBank.h"
#include "edC/edCFiler.h"
#include <assert.h>

TranslatedTextData* g_TranslatedTextTRC_00449748 = NULL;

void RemoveLinkedTextData_00336a00(TranslatedTextData** ppTextData, TranslatedTextData* pToRemove)
{
	TranslatedTextData* pTVar1;

	if (pToRemove != (TranslatedTextData*)0x0) {
		for (pTVar1 = *ppTextData; (pTVar1 != (TranslatedTextData*)0x0 && (pTVar1 != pToRemove));
			pTVar1 = pTVar1->pNext) {
		}
		if (pTVar1->pPrev != (TranslatedTextData*)0x0) {
			pTVar1->pPrev->pNext = pTVar1->pNext;
		}
		if (pTVar1->pNext != (TranslatedTextData*)0x0) {
			pTVar1->pNext->pPrev = pTVar1->pPrev;
		}
		if (pTVar1 == *ppTextData) {
			if (pTVar1->pPrev == (TranslatedTextData*)0x0) {
				if (pTVar1->pNext == (TranslatedTextData*)0x0) {
					*ppTextData = (TranslatedTextData*)0x0;
				}
				else {
					*ppTextData = pTVar1->pNext;
				}
			}
			else {
				*ppTextData = pTVar1->pPrev;
			}
		}
	}
	return;
}

void FUN_00336d20(TranslatedTextData* pTextData)
{
	int* piVar1;
	uint uVar2;
	ulong* puVar3;

	piVar1 = (int*)pTextData->pFileData;
	pTextData->entryCount = *piVar1;
	puVar3 = (ulong*)(piVar1 + 2);
	pTextData->pDataA = puVar3;
	if (piVar1[1] == 0) {
		piVar1[1] = 1;
		uVar2 = 0;
		if (pTextData->entryCount != 0) {
			do {
				uVar2 = uVar2 + 1;
				*(char**)(puVar3 + 1) = pTextData->pFileData + *(int*)(puVar3 + 1);
				puVar3 = puVar3 + 2;
			} while (uVar2 < (uint)pTextData->entryCount);
		}
	}
	return;
}

TranslatedTextData::TranslatedTextData()
{
	pPrev = (TranslatedTextData*)0x0;
	pNext = (TranslatedTextData*)0x0;
	pFileData = (char*)0x0;
	entryCount = 0;
	languageID = (ELanguageID)0;
	pDataA = (ulong*)0x0;
	field_0x8[0] = '\0';
	pBankAccessObj = (edCBankBuffer*)0x0;
	return;
}

TranslatedTextData::~TranslatedTextData()
{
	char* dataPtr = pFileData;
	if ((dataPtr != (char*)0x0) && (dataPtr != (char*)0x0)) {
		edMemFree(dataPtr);
		pFileData = (char*)0x0;
		pDataA = (ulong*)0x0;
		entryCount = 0;
		pBankAccessObj = (edCBankBuffer*)0x0;
		RemoveLinkedTextData_00336a00(&g_TranslatedTextTRC_00449748, this);
	}
}

char* g_LanguageSuffixArray_00425840[5] = {
	"GB",
	"FR",
	"GE",
	"SP",
	"IT",
};

ELanguageID g_LanguageID_0044974c = GB;
short SHORT_00448fce = 0;
EHeap EHeap_00448fc8 = TO_HEAP(H_MAIN);
short SHORT_00448fcc = 0x40;

void TranslatedTextData::LoadTextTranslatedFromBank(edCBankBuffer* pBankAccess, char* pFilePath, ELanguageID languageID)
{
	bool bVar1;
	TranslatedTextData* pTVar2;
	TranslatedTextData* pTVar3;
	TranslatedTextData* pTVar4;
	bool bVar5;
	int iVar6;
	BankFileData BStack544;
	char acStack512[512];

	bVar1 = false;
	if (this->entryCount != 0) {
		this->pFileData = (char*)0x0;
		this->pDataA = (ulong*)0x0;
		this->entryCount = 0;
		this->pBankAccessObj = (edCBankBuffer*)0x0;
		if (pBankAccess == (edCBankBuffer*)0x0) {
			RemoveLinkedTextData_00336a00(&g_TranslatedTextTRC_00449748, this);
		}
		else {
			bVar1 = true;
		}
	}
	if (pBankAccess != (edCBankBuffer*)0x0) {
		if (languageID == AUTO) {
			languageID = g_LanguageID_0044974c;
		}
		if (pFilePath != (char*)0x0) {
			iVar6 = edStringCpyL(this->field_0x8, pFilePath);
			(this->field_0x8 + iVar6 + -8)[2] = '%';
			(this->field_0x8 + iVar6 + -8)[3] = 's';
		}
		sprintf(acStack512, this->field_0x8);
		iVar6 = GetIndexForFileName(pBankAccess, acStack512);
		if ((iVar6 != -1) && (bVar5 = GetFileDataForIndex(pBankAccess, iVar6, &BStack544, (char*)0x0), bVar5 != false)) {
			this->languageID = languageID;
			this->pFileData = BStack544.fileBufferStart;
			this->pBankAccessObj = pBankAccess;
			FUN_00336d20(this);
			if ((!bVar1) && (this != (TranslatedTextData*)0x0)) {
				if (g_TranslatedTextTRC_00449748 == (TranslatedTextData*)0x0) {
					this->pPrev = (TranslatedTextData*)0x0;
					this->pNext = (TranslatedTextData*)0x0;
					g_TranslatedTextTRC_00449748 = this;
				}
				else {
					pTVar4 = g_TranslatedTextTRC_00449748->pNext;
					pTVar3 = g_TranslatedTextTRC_00449748;
					while (pTVar2 = pTVar4, pTVar2 != (TranslatedTextData*)0x0) {
						pTVar3 = pTVar2;
						pTVar4 = pTVar2->pNext;
					}
					this->pPrev = pTVar3;
					this->pNext = (TranslatedTextData*)0x0;
					pTVar3->pNext = this;
				}
			}
		}
	}
	return;
}

char* LoadFromDisk_0025b960(char* filePath, uint* outSize, uint flags)
{
	byte bVar1;
	DebugBankData_234* pDebugBank;
	edCFiler* peVar2;
	uint uVar3;
	char* pReadBuffer;
	edCFiler_28* peVar4;
	byte* pbVar5;
	int iVar6;
	char acStack512[512];

	MY_LOG("TranslatedTextData::LoadFromDisk_0025b960 %s\n", filePath);

	*outSize = 0;
	pDebugBank = LoadFile(filePath, flags | 1);
	if (pDebugBank == (DebugBankData_234*)0x0) {
		pReadBuffer = (char*)0x0;
	}
	else {
		peVar2 = FindEdCFiler(acStack512, filePath, 0);
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

			uVar3 = peVar2->Function74(uVar3);
			*outSize = uVar3;
			pReadBuffer = (char*)edMemAlloc(EHeap_00448fc8, *outSize, (uint)(ushort)SHORT_00448fcc,
				(uint)(ushort)SHORT_00448fce);
			SHORT_00448fce = 0;
			EHeap_00448fc8 = TO_HEAP(H_MAIN);
			SHORT_00448fcc = 0x40;
			peVar4 = peVar2->GetGlobalC_0x1c();
			SetBankReadStream(peVar4, pDebugBank, pReadBuffer, *outSize);
			peVar4 = peVar2->GetGlobalC_0x1c();
			SetBankClose(peVar4, pDebugBank);
			if ((pDebugBank->openFlags & 8) == 0) {
				TriggerBankRead_0025b0c0(pDebugBank->pOwningFiler);
				iVar6 = 0;
				do {
					if (&g_DebugBankDataArray_00469bf0[iVar6] == pDebugBank) {
						bVar1 = g_DebugBankLoadFlag_00469be0[iVar6];
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

void TranslatedTextData::LoadTranslatedTextFromDisk(char* filePath, ELanguageID inLanguageID)
{
	TranslatedTextData* pTVar1;
	TranslatedTextData* pTVar2;
	TranslatedTextData* pTVar3;
	byte bVar4;
	DebugBankData_234* pDebugBank;
	char* pcVar5;
	char acStack512[512];

	if (inLanguageID == AUTO) {
		inLanguageID = g_LanguageID_0044974c;
	}

	if ((this->pFileData != (char*)0x0) && (inLanguageID == this->languageID)) {
		return;
	}
	if (filePath != (char*)0x0) {
		edStringCpyL(this->field_0x8, filePath);
	}

	MY_LOG("TranslatedTextData::LoadTranslatedTextFromDisk %s [%s]\n", filePath, g_LanguageSuffixArray_00425840[inLanguageID]);

	this->pBankAccessObj = (edCBankBuffer*)0x0;
	this->languageID = inLanguageID;
	sprintf(acStack512, this->field_0x8, g_LanguageSuffixArray_00425840[inLanguageID]);
	if (this->pFileData == (char*)0x0) {
		pcVar5 = LoadFromDisk_0025b960(acStack512, &this->size, 0);
		this->pFileData = pcVar5;
		if (this->pFileData == (char*)0x0) {
			return;
		}
		if (this != (TranslatedTextData*)0x0) {
			if (g_TranslatedTextTRC_00449748 == (TranslatedTextData*)0x0) {
				this->pPrev = (TranslatedTextData*)0x0;
				this->pNext = (TranslatedTextData*)0x0;
				g_TranslatedTextTRC_00449748 = this;
			}
			else {
				pTVar3 = g_TranslatedTextTRC_00449748->pNext;
				pTVar2 = g_TranslatedTextTRC_00449748;
				while (pTVar1 = pTVar3, pTVar1 != (TranslatedTextData*)0x0) {
					pTVar2 = pTVar1;
					pTVar3 = pTVar1->pNext;
				}
				this->pPrev = pTVar2;
				this->pNext = (TranslatedTextData*)0x0;
				pTVar2->pNext = this;
			}
		}
	}
	else {
		pDebugBank = LoadFile(acStack512, 1);
		if (pDebugBank == (DebugBankData_234*)0x0) {
			return;
		}
		bVar4 = UsedInFileLoad(pDebugBank, this->pFileData, this->size);
		if (bVar4 == 0) {
			this->entryCount = 0;
			DebugBankClose(pDebugBank);
			return;
		}
		DebugBankClose(pDebugBank);
	}
	FUN_00336d20(this);
	return;
}

char* g_szTextNotFound_00434bf0 = "NotFound";

char* TranslatedTextData::GetText_00336c10(ulong key, long mode)
{
	char* pcVar1;
	ulong* puVar2;
	uint uVar3;

	if (key == 0) {
		if (mode == 0) {
			pcVar1 = g_szTextNotFound_00434bf0;
		}
		else {
			pcVar1 = (char*)0x0;
		}
	}
	else {
		if (key >> 0x20 == 0xffffffff) {
			pcVar1 = (char*)key;
		}
		else {
			uVar3 = 0;
			if (this->entryCount != 0) {
				puVar2 = this->pDataA;
				do {
					if (key == *puVar2) {
						return *(char**)(this->pDataA + uVar3 * 2 + 1);
					}
					uVar3 = uVar3 + 1;
					puVar2 = puVar2 + 2;
				} while (uVar3 < (uint)this->entryCount);
			}
			if (mode == 0) {
				pcVar1 = g_szTextNotFound_00434bf0;
			}
			else {
				pcVar1 = (char*)0x0;
			}
		}
	}

	//MY_LOG("TranslatedTextData::GetText_00336c10 %c%c%c%c -> %s [%d]\n", LOC_KEY_TO_CHAR(key), pcVar1, mode);

	return pcVar1;
}

char* FindTranslatedTextFromKey_00336970(TranslatedTextData** ppTextData, ulong key)
{
	char* pcVar1;
	TranslatedTextData* pTextData;

	if (key != 0) {
		for (pTextData = *ppTextData; pTextData != (TranslatedTextData*)0x0; pTextData = pTextData->pNext) {
			pcVar1 = pTextData->GetText_00336c10(key, 1);
			if (pcVar1 != (char*)0x0) {
				return pcVar1;
			}
		}
	}
	return g_szTextNotFound_00434bf0;
}