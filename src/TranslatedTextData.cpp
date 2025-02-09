#include "TranslatedTextData.h"
#include "edMem.h"
#include "edBankBuffer.h"
#include <stdio.h>
#include "edFile.h"
#include <assert.h>
#include "edStr.h"
#include "PauseManager.h"

MessageManager gMessageManager = { NULL };

void MessageManager::remove_entry(CMessageFile* pToRemove)
{
	CMessageFile* pTVar1;

	if (pToRemove != (CMessageFile*)0x0) {
		for (pTVar1 = pMessage; (pTVar1 != (CMessageFile*)0x0 && (pTVar1 != pToRemove));
			pTVar1 = pTVar1->pNext) {
		}
		if (pTVar1->pPrev != (CMessageFile*)0x0) {
			pTVar1->pPrev->pNext = pTVar1->pNext;
		}
		if (pTVar1->pNext != (CMessageFile*)0x0) {
			pTVar1->pNext->pPrev = pTVar1->pPrev;
		}
		if (pTVar1 == pMessage) {
			if (pTVar1->pPrev == (CMessageFile*)0x0) {
				if (pTVar1->pNext == (CMessageFile*)0x0) {
					pMessage = (CMessageFile*)0x0;
				}
				else {
					pMessage = pTVar1->pNext;
				}
			}
			else {
				pMessage = pTVar1->pPrev;
			}
		}
	}
	return;
}

void CMessageFile::prepare_buffer()
{
	int* piVar1;
	uint uVar2;
	ulong* puVar3;

	piVar1 = (int*)this->pFileData;
	this->entryCount = *piVar1;
	puVar3 = (ulong*)(piVar1 + 2);
	this->pDataA = puVar3;
	if (piVar1[1] == 0) {
		piVar1[1] = 1;
		uVar2 = 0;
		if (this->entryCount != 0) {
			do {
				uVar2 = uVar2 + 1;
				*(char**)(puVar3 + 1) = this->pFileData + *(int*)(puVar3 + 1);
				puVar3 = puVar3 + 2;
			} while (uVar2 < (uint)this->entryCount);
		}
	}
	return;
}

int CMessageFile::get_entry_count()
{
	return this->entryCount;
}

CMessageFile::CMessageFile()
{
	pPrev = (CMessageFile*)0x0;
	pNext = (CMessageFile*)0x0;
	pFileData = (char*)0x0;
	entryCount = 0;
	languageID = (LANGUAGE)0;
	pDataA = (ulong*)0x0;
	field_0x8[0] = '\0';
	pBankAccessObj = (edCBankBufferEntry*)0x0;
	return;
}

CMessageFile::~CMessageFile()
{
	char* dataPtr = pFileData;
	if ((dataPtr != (char*)0x0) && (dataPtr != (char*)0x0)) {
		edMemFree(dataPtr);
		pFileData = (char*)0x0;
		pDataA = (ulong*)0x0;
		entryCount = 0;
		pBankAccessObj = (edCBankBufferEntry*)0x0;
		gMessageManager.remove_entry(this);
	}
}

char* g_LanguageSuffixArray_00425840[5] = {
	"GB",
	"FR",
	"GE",
	"SP",
	"IT",
};

LANGUAGE CMessageFile::sm_default_language = GB;

void CMessageFile::set_default_language(LANGUAGE newID)
{
	sm_default_language = newID;
	return;
}

LANGUAGE CMessageFile::get_default_language(void)
{
	return sm_default_language;
}

void CMessageFile::select_language(edCBankBufferEntry* pBankAccess, const char* pFilePath, LANGUAGE languageID)
{
	bool bVar1;
	CMessageFile* pTVar2;
	CMessageFile* pTVar3;
	CMessageFile* pTVar4;
	bool bVar5;
	int iVar6;
	edBANK_ENTRY_INFO BStack544;
	char acStack512[512];

	MY_LOG("select_language: {}\n", pFilePath);

	bVar1 = false;
	if (this->entryCount != 0) {
		this->pFileData = (char*)0x0;
		this->pDataA = (ulong*)0x0;
		this->entryCount = 0;
		this->pBankAccessObj = (edCBankBufferEntry*)0x0;
		if (pBankAccess == (edCBankBufferEntry*)0x0) {
			gMessageManager.remove_entry(this);
		}
		else {
			bVar1 = true;
		}
	}

	if (pBankAccess != (edCBankBufferEntry*)0x0) {
		if (languageID == AUTO) {
			languageID = CMessageFile::sm_default_language;
		}

		if (pFilePath != (char*)0x0) {
			iVar6 = edStrCopy(this->field_0x8, pFilePath);
			(this->field_0x8 + iVar6 + -8)[2] = '%';
			(this->field_0x8 + iVar6 + -8)[3] = 's';
		}

		sprintf(acStack512, this->field_0x8, g_LanguageSuffixArray_00425840[languageID]);
		iVar6 = pBankAccess->get_index(acStack512);
		if ((iVar6 != -1) && (bVar5 = pBankAccess->get_info(iVar6, &BStack544, (char*)0x0), bVar5 != false)) {
			this->languageID = languageID;
			this->pFileData = BStack544.fileBufferStart;
			this->pBankAccessObj = pBankAccess;
			prepare_buffer();
			if ((!bVar1) && (this != (CMessageFile*)0x0)) {
				if (gMessageManager.pMessage == (CMessageFile*)0x0) {
					this->pPrev = (CMessageFile*)0x0;
					this->pNext = (CMessageFile*)0x0;
					gMessageManager.pMessage = this;
					MY_LOG("Linking first message: %p\n", (void*)this);
				}
				else {
					pTVar4 = gMessageManager.pMessage->pNext;
					pTVar3 = gMessageManager.pMessage;
					while (pTVar2 = pTVar4, pTVar2 != (CMessageFile*)0x0) {
						pTVar3 = pTVar2;
						pTVar4 = pTVar2->pNext;
					}
					this->pPrev = pTVar3;
					this->pNext = (CMessageFile*)0x0;
					pTVar3->pNext = this;
					MY_LOG("Linking next message: %p\n", (void*)this);
				}
			}
		}
	}
	return;
}

void CMessageFile::select_language(const char* filePath, LANGUAGE inLanguageID)
{
	CMessageFile* pTVar1;
	CMessageFile* pTVar2;
	CMessageFile* pTVar3;
	byte bVar4;
	edFILEH* pDebugBank;
	char* pcVar5;
	char acStack512[512];

	if (inLanguageID == AUTO) {
		inLanguageID = CMessageFile::sm_default_language;
	}

	if ((this->pFileData != (char*)0x0) && (inLanguageID == this->languageID)) {
		return;
	}
	if (filePath != (char*)0x0) {
		edStrCopy(this->field_0x8, filePath);
	}

	MY_LOG("MessageFile::select_language {} [{}]\n", filePath, g_LanguageSuffixArray_00425840[inLanguageID]);

	this->pBankAccessObj = (edCBankBufferEntry*)0x0;
	this->languageID = inLanguageID;
	sprintf(acStack512, this->field_0x8, g_LanguageSuffixArray_00425840[inLanguageID]);
	if (this->pFileData == (char*)0x0) {
		pcVar5 = edFileOpen(acStack512, &this->size, 0);
		this->pFileData = pcVar5;
		if (this->pFileData == (char*)0x0) {
			MY_LOG("MessageFile::select_language FAILED TO OPEN FILE: {}\n", acStack512);
			return;
		}
		if (this != (CMessageFile*)0x0) {
			if (gMessageManager.pMessage == (CMessageFile*)0x0) {
				this->pPrev = (CMessageFile*)0x0;
				this->pNext = (CMessageFile*)0x0;
				gMessageManager.pMessage = this;
				MY_LOG("Linking first message: %p\n", (void*)this);
			}
			else {
				pTVar3 = gMessageManager.pMessage->pNext;
				pTVar2 = gMessageManager.pMessage;
				while (pTVar1 = pTVar3, pTVar1 != (CMessageFile*)0x0) {
					pTVar2 = pTVar1;
					pTVar3 = pTVar1->pNext;
				}
				this->pPrev = pTVar2;
				this->pNext = (CMessageFile*)0x0;
				pTVar2->pNext = this;
				MY_LOG("Linking next message: %p\n", (void*)this);
			}
		}
	}
	else {
		pDebugBank = edFileOpen(acStack512, 1);
		if (pDebugBank == (edFILEH*)0x0) {
			return;
		}
		bVar4 = edFileRead(pDebugBank, this->pFileData, this->size);
		if (bVar4 == 0) {
			this->entryCount = 0;
			edFileClose(pDebugBank);
			return;
		}
		edFileClose(pDebugBank);
	}
	prepare_buffer();
	return;
}

char* g_szTextNotFound_00434bf0 = "NotFound";

char* CMessageFile::get_message(ulong key, long mode)
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

	//MY_LOG("MessageFile::get_message %c%c%c%c -> {} [%d]\n", LOC_KEY_TO_CHAR(key), pcVar1, mode);

	return pcVar1;
}

char* MessageManager::get_message(ulong key)
{
	char* pcVar1;
	CMessageFile* pTextData;

	if (key != 0) {
		for (pTextData = pMessage; pTextData != (CMessageFile*)0x0; pTextData = pTextData->pNext) {
			pcVar1 = pTextData->get_message(key, 1);
			if (pcVar1 != (char*)0x0) {
				return pcVar1;
			}
		}
	}
	return g_szTextNotFound_00434bf0;
}