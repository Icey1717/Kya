#include "TranslatedTextData.h"
#include "edMem.h"
#include "edC/edCBankBuffer.h"
#include <stdio.h>
#include "edC/edCBank.h"
#include "edC/edCFiler.h"
#include <assert.h>
#include "edStr.h"

MessageManager gMessageManager = { NULL };

void MessageManager::remove_entry(MessageFile* pToRemove)
{
	MessageFile* pTVar1;

	if (pToRemove != (MessageFile*)0x0) {
		for (pTVar1 = pMessage; (pTVar1 != (MessageFile*)0x0 && (pTVar1 != pToRemove));
			pTVar1 = pTVar1->pNext) {
		}
		if (pTVar1->pPrev != (MessageFile*)0x0) {
			pTVar1->pPrev->pNext = pTVar1->pNext;
		}
		if (pTVar1->pNext != (MessageFile*)0x0) {
			pTVar1->pNext->pPrev = pTVar1->pPrev;
		}
		if (pTVar1 == pMessage) {
			if (pTVar1->pPrev == (MessageFile*)0x0) {
				if (pTVar1->pNext == (MessageFile*)0x0) {
					pMessage = (MessageFile*)0x0;
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

void MessageFile::prepare_buffer()
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

MessageFile::MessageFile()
{
	pPrev = (MessageFile*)0x0;
	pNext = (MessageFile*)0x0;
	pFileData = (char*)0x0;
	entryCount = 0;
	languageID = (LANGUAGE)0;
	pDataA = (ulong*)0x0;
	field_0x8[0] = '\0';
	pBankAccessObj = (edCBankBufferEntry*)0x0;
	return;
}

MessageFile::~MessageFile()
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

LANGUAGE g_LanguageID_0044974c = GB;

void MessageFile::select_language(edCBankBufferEntry* pBankAccess, char* pFilePath, LANGUAGE languageID)
{
	bool bVar1;
	MessageFile* pTVar2;
	MessageFile* pTVar3;
	MessageFile* pTVar4;
	bool bVar5;
	int iVar6;
	edBANK_ENTRY_INFO BStack544;
	char acStack512[512];

	MY_LOG("select_language: %s\n", pFilePath);

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
			languageID = g_LanguageID_0044974c;
		}
		if (pFilePath != (char*)0x0) {
			iVar6 = edStrCopy(this->field_0x8, pFilePath);
			(this->field_0x8 + iVar6 + -8)[2] = '%';
			(this->field_0x8 + iVar6 + -8)[3] = 's';
		}
		sprintf(acStack512, this->field_0x8);
		iVar6 = get_index(pBankAccess, acStack512);
		if ((iVar6 != -1) && (bVar5 = get_info(pBankAccess, iVar6, &BStack544, (char*)0x0), bVar5 != false)) {
			this->languageID = languageID;
			this->pFileData = BStack544.fileBufferStart;
			this->pBankAccessObj = pBankAccess;
			prepare_buffer();
			if ((!bVar1) && (this != (MessageFile*)0x0)) {
				if (gMessageManager.pMessage == (MessageFile*)0x0) {
					this->pPrev = (MessageFile*)0x0;
					this->pNext = (MessageFile*)0x0;
					gMessageManager.pMessage = this;
					MY_LOG("Linking first message: %p\n", this);
				}
				else {
					pTVar4 = gMessageManager.pMessage->pNext;
					pTVar3 = gMessageManager.pMessage;
					while (pTVar2 = pTVar4, pTVar2 != (MessageFile*)0x0) {
						pTVar3 = pTVar2;
						pTVar4 = pTVar2->pNext;
					}
					this->pPrev = pTVar3;
					this->pNext = (MessageFile*)0x0;
					pTVar3->pNext = this;
					MY_LOG("Linking next message: %p\n", this);
				}
			}
		}
	}
	return;
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

	MY_LOG("MessageFile::edFileOpen %s\n", filePath);

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

void MessageFile::select_language(char* filePath, LANGUAGE inLanguageID)
{
	MessageFile* pTVar1;
	MessageFile* pTVar2;
	MessageFile* pTVar3;
	byte bVar4;
	edFILEH* pDebugBank;
	char* pcVar5;
	char acStack512[512];

	if (inLanguageID == AUTO) {
		inLanguageID = g_LanguageID_0044974c;
	}

	if ((this->pFileData != (char*)0x0) && (inLanguageID == this->languageID)) {
		return;
	}
	if (filePath != (char*)0x0) {
		edStrCopy(this->field_0x8, filePath);
	}

	MY_LOG("MessageFile::select_language %s [%s]\n", filePath, g_LanguageSuffixArray_00425840[inLanguageID]);

	this->pBankAccessObj = (edCBankBufferEntry*)0x0;
	this->languageID = inLanguageID;
	sprintf(acStack512, this->field_0x8, g_LanguageSuffixArray_00425840[inLanguageID]);
	if (this->pFileData == (char*)0x0) {
		pcVar5 = edFileOpen(acStack512, &this->size, 0);
		this->pFileData = pcVar5;
		if (this->pFileData == (char*)0x0) {
			MY_LOG("MessageFile::select_language FAILED TO OPEN FILE: %s\n", acStack512);
			return;
		}
		if (this != (MessageFile*)0x0) {
			if (gMessageManager.pMessage == (MessageFile*)0x0) {
				this->pPrev = (MessageFile*)0x0;
				this->pNext = (MessageFile*)0x0;
				gMessageManager.pMessage = this;
				MY_LOG("Linking first message: %p\n", this);
			}
			else {
				pTVar3 = gMessageManager.pMessage->pNext;
				pTVar2 = gMessageManager.pMessage;
				while (pTVar1 = pTVar3, pTVar1 != (MessageFile*)0x0) {
					pTVar2 = pTVar1;
					pTVar3 = pTVar1->pNext;
				}
				this->pPrev = pTVar2;
				this->pNext = (MessageFile*)0x0;
				pTVar2->pNext = this;
				MY_LOG("Linking next message: %p\n", this);
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

char* MessageFile::get_message(ulong key, long mode)
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

	//MY_LOG("MessageFile::get_message %c%c%c%c -> %s [%d]\n", LOC_KEY_TO_CHAR(key), pcVar1, mode);

	return pcVar1;
}

char* MessageManager::get_message(ulong key)
{
	char* pcVar1;
	MessageFile* pTextData;

	if (key != 0) {
		for (pTextData = pMessage; pTextData != (MessageFile*)0x0; pTextData = pTextData->pNext) {
			pcVar1 = pTextData->get_message(key, 1);
			if (pcVar1 != (char*)0x0) {
				return pcVar1;
			}
		}
	}
	return g_szTextNotFound_00434bf0;
}