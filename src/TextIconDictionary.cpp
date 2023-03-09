#include "TextIconDictionary.h"
#include "edMem.h"

TextIconDictionary g_TextIconDictionary;

TextIconDictionary::TextIconDictionary()
{
	pEntries = (TextIconDictionaryEntry*)0x0;
	maxEntries = 0;
	currentEntries = 0;
}

bool TextIconDictionary::Init(int maxEntries)
{
	bool bVar1;
	uint count;
	int* pBase;
	TextIconDictionaryEntry* pTVar2;

	this->currentEntries = 0;
	this->maxEntries = maxEntries;
	count = this->maxEntries;
	pBase = (int*)AllocateFunc_001002a0((long)(int)(count * 0x10 + 0x10));
	pTVar2 = (TextIconDictionaryEntry*)pBase;
	//pTVar2 = (TextIconDictionaryEntry*)
	//	RunActorInitFunctions_00217580
	//	(pBase, TextIconDictionaryEntry::Constructor_0028dcb0,
	//		TextIconDictionaryEntry::Destructor_0028dc60, 0x10, count);
	this->pEntries = pTVar2;
	bVar1 = this->pEntries != (TextIconDictionaryEntry*)0x0;
	if (!bVar1) {
		this->maxEntries = 0;
	}
	return bVar1;
}

bool TextIconDictionary::AddTextEntry(char* key, char* value)
{
	char cVar1;
	char* pcVar2;
	int iVar3;
	int iVar4;
	TextIconDictionaryEntry* pTVar5;
	long long lVar6;

	MY_LOG("TextIconDictionary::AddTextEntry key: %s value: %s\n", key, value);

	iVar3 = 0;
	lVar6 = 0;
	cVar1 = *key;
	pcVar2 = key + 1;
	while (cVar1 != '\0') {
		iVar3 = iVar3 + 1;
		lVar6 = lVar6 * 0x100 + (long)cVar1;
		if (8 < iVar3) {
			lVar6 = 0;
			break;
		}
		cVar1 = *pcVar2;
		pcVar2 = pcVar2 + 1;
	}
	iVar3 = this->currentEntries;
	pTVar5 = this->pEntries;
	iVar4 = 0;
	if (0 < iVar3) {
		do {
			if ((*(long*)pTVar5->key == lVar6) && (pTVar5->index == 1)) {
				pTVar5 = (TextIconDictionaryEntry*)0x0;
				goto LAB_0028d978;
			}
			iVar4 = iVar4 + 1;
			pTVar5 = pTVar5 + 1;
		} while (iVar4 < iVar3);
	}
	if (iVar3 == this->maxEntries) {
		pTVar5 = (TextIconDictionaryEntry*)0x0;
	}
	else {
		this->currentEntries = this->currentEntries + 1;
	}
LAB_0028d978:
	if (pTVar5 != (TextIconDictionaryEntry*)0x0) {
		*(long long*)pTVar5->key = lVar6;
		pTVar5->pMessageData = value;
		pTVar5->index = 1;
	}
	return pTVar5 != (TextIconDictionaryEntry*)0x0;
}

