#include "edTextResources.h"
#include "edMem.h"

#define TEXT_RESOURCES_LOG(level, format, ...) MY_LOG_CATEGORY("textresources", level, format, ##__VA_ARGS__)

edCTextResourcePool edTextResources;

bool edCTextResourcePool::CallbackAdd(char* name, edResourceFunc pFunc)
{
	char cVar1;
	char* pcVar2;
	int iVar3;
	int iVar4;
	edCTextResource* peVar5;
	ulong uVar6;

	iVar3 = 0;
	uVar6 = 0;
	cVar1 = *name;
	pcVar2 = name + 1;
	while (cVar1 != '\0') {
		iVar3 = iVar3 + 1;
		uVar6 = uVar6 * 0x100 + (long)cVar1;
		if (8 < iVar3) {
			uVar6 = 0;
			break;
		}
		cVar1 = *pcVar2;
		pcVar2 = pcVar2 + 1;
	}
	iVar3 = this->currentEntries;
	peVar5 = this->pEntries;
	iVar4 = 0;
	if (0 < iVar3) {
		do {
			if ((peVar5->key == uVar6) && (peVar5->index == 3)) {
				peVar5 = (edCTextResource*)0x0;
				goto LAB_0028d878;
			}
			iVar4 = iVar4 + 1;
			peVar5 = peVar5 + 1;
		} while (iVar4 < iVar3);
	}
	if (iVar3 == this->maxEntries) {
		peVar5 = (edCTextResource*)0x0;
	}
	else {
		this->currentEntries = this->currentEntries + 1;
	}
LAB_0028d878:
	if (peVar5 != (edCTextResource*)0x0) {
		peVar5->key = uVar6;
		peVar5->pMessageData = (void*)pFunc;
		peVar5->index = 3;
	}
	return peVar5 != (edCTextResource*)0x0;
}

bool edCTextResourcePool::TextAdd(char* key, char* value)
{
	char cVar1;
	char* pcVar2;
	int iVar3;
	int iVar4;
	edCTextResource* peVar5;
	ulong uVar6;

	TEXT_RESOURCES_LOG(LogLevel::Info, "edCTextResourcePool::TextAdd key: %u value: %s\n", key, value);

	iVar3 = 0;
	uVar6 = 0;
	cVar1 = *key;
	pcVar2 = key + 1;
	while (cVar1 != '\0') {
		iVar3 = iVar3 + 1;
		uVar6 = uVar6 * 0x100 + (long)cVar1;
		if (8 < iVar3) {
			uVar6 = 0;
			break;
		}
		cVar1 = *pcVar2;
		pcVar2 = pcVar2 + 1;
	}
	iVar3 = this->currentEntries;
	peVar5 = this->pEntries;
	iVar4 = 0;
	if (0 < iVar3) {
		do {
			if ((peVar5->key == uVar6) && (peVar5->index == 1)) {
				peVar5 = (edCTextResource*)0x0;
				goto LAB_0028d978;
			}
			iVar4 = iVar4 + 1;
			peVar5 = peVar5 + 1;
		} while (iVar4 < iVar3);
	}
	if (iVar3 == this->maxEntries) {
		peVar5 = (edCTextResource*)0x0;
	}
	else {
		this->currentEntries = this->currentEntries + 1;
	}
LAB_0028d978:
	if (peVar5 != (edCTextResource*)0x0) {
		peVar5->key = uVar6;
		peVar5->pMessageData = value;
		peVar5->index = 1;
	}
	return peVar5 != (edCTextResource*)0x0;
}

bool edCTextResourcePool::BitmapAdd(char* label, edTextBitmap* pBitmap)
{
	int characterCount;
	int curEntryIndex;
	edCTextResource* pNewResource;
	ulong uVar3;
	char currentChar;
	char* nextCharPtr;

	TEXT_RESOURCES_LOG(LogLevel::Info, "edCTextResourcePool::BitmapAdd key: %s\n", label);

	characterCount = 0;
	uVar3 = 0;
	currentChar = *label;
	nextCharPtr = label + 1;
	while (currentChar != '\0') {
		characterCount = characterCount + 1;
		uVar3 = uVar3 * 0x100 + (long)currentChar;
		if (8 < characterCount) {
			uVar3 = 0;
			break;
		}
		currentChar = *nextCharPtr;
		nextCharPtr = nextCharPtr + 1;
	}

	int nbEntries = this->currentEntries;
	pNewResource = this->pEntries;
	curEntryIndex = 0;
	if (0 < nbEntries) {
		do {
			if ((pNewResource->key == uVar3) && (pNewResource->index == 2)) {
				pNewResource = (edCTextResource*)0x0;
				goto LAB_0028da78;
			}
			curEntryIndex = curEntryIndex + 1;
			pNewResource = pNewResource + 1;
		} while (curEntryIndex < nbEntries);
	}

	if (nbEntries == this->maxEntries) {
		pNewResource = (edCTextResource*)0x0;
	}
	else {
		this->currentEntries = this->currentEntries + 1;
	}

LAB_0028da78:
	if (pNewResource != (edCTextResource*)0x0) {
		pNewResource->key = uVar3;
		pNewResource->pMessageData = pBitmap;
		pNewResource->index = 2;
	}

	return pNewResource != (edCTextResource*)0x0;
}

edTextBitmap* edCTextResourcePool::GetResourcePtr(ulong key, int index)
{
	edTextBitmap* pvVar1;
	int iVar2;
	edCTextResource* peVar3;

	iVar2 = 0;
	peVar3 = edTextResources.pEntries;

	if (0 < edTextResources.currentEntries) {
		do {
			if ((peVar3->key == key) && (peVar3->index == index)) goto LAB_0028db08;
			iVar2 = iVar2 + 1;
			peVar3 = peVar3 + 1;
		} while (iVar2 < edTextResources.currentEntries);
	}

	peVar3 = (edCTextResource*)0x0;
LAB_0028db08:
	pvVar1 = (edTextBitmap*)0x0;
	if (peVar3 != (edCTextResource*)0x0) {
		pvVar1 = reinterpret_cast<edTextBitmap*>(peVar3->pMessageData);
	}

	return pvVar1;
}

edCTextResourcePool::edCTextResourcePool()
{
	pEntries = (edCTextResource*)0x0;
	maxEntries = 0;
	currentEntries = 0;
}

bool edCTextResourcePool::Init(int maxEntries)
{
	bool bVar1;
	uint count;

	this->currentEntries = 0;
	this->maxEntries = maxEntries;
	count = this->maxEntries;
	this->pEntries = new edCTextResource[count];
	bVar1 = this->pEntries != (edCTextResource*)0x0;
	if (!bVar1) {
		this->maxEntries = 0;
	}
	return bVar1;
}

edCTextResourcePool::edCTextResource::edCTextResource()
{
	this->key = 0;
	this->pMessageData = (void*)0x0;
	this->index = 0;
	return;
}