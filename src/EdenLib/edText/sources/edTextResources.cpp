#include "edTextResources.h"
#include "edMem.h"

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

	MY_LOG("edCTextResourcePool::AddTextEntry key: %u value: %s\n", key, value);

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

bool edCTextResourcePool::BitmapAdd(char* label, void* param_3)
{
	int characterCount;
	int iVar1;
	edCTextResource* peVar2;
	ulong uVar3;
	char currentChar;
	char* nextCharPtr;

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
	characterCount = this->currentEntries;
	peVar2 = this->pEntries;
	iVar1 = 0;
	if (0 < characterCount) {
		do {
			if ((peVar2->key == uVar3) && (peVar2->index == 2)) {
				peVar2 = (edCTextResource*)0x0;
				goto LAB_0028da78;
			}
			iVar1 = iVar1 + 1;
			peVar2 = peVar2 + 1;
		} while (iVar1 < characterCount);
	}
	if (characterCount == this->maxEntries) {
		peVar2 = (edCTextResource*)0x0;
	}
	else {
		this->currentEntries = this->currentEntries + 1;
	}
LAB_0028da78:
	if (peVar2 != (edCTextResource*)0x0) {
		peVar2->key = uVar3;
		peVar2->pMessageData = param_3;
		peVar2->index = 2;
	}
	return peVar2 != (edCTextResource*)0x0;
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