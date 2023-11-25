#include "PathManager.h"
#include "MemoryStream.h"

CPathManager::CPathManager()
{
	this->aPathFollow = (CPathFollow*)0x0;
	this->pathFollowCount = 0;
	this->field_0xc = (int*)0x0;
	return;
}


void CPathManager::Level_AddAll(ByteCode* pMemoryStream)
{
	uint count;
	int iVar1;
	int* piVar2;
	CPathFollow* pAVar3;
	undefined8 uVar4;
	int iVar5;

	pMemoryStream->GetChunk();
	iVar1 = pMemoryStream->GetS32();
	this->pathFollowCount = iVar1;
	count = this->pathFollowCount;
	if (count != 0) {
		pAVar3 = new CPathFollow[count];
		this->aPathFollow = pAVar3;

		for (int i = 0; i < this->pathFollowCount; i++) {
			this->aPathFollow[i].Create(pMemoryStream);
		}
	}
	//piVar2 = (int*)operator.new(0x434);
	//if ((long)(int)piVar2 != 0) {
	//	uVar4 = FUN_001bf5f0((long)(int)piVar2);
	//	piVar2 = (int*)uVar4;
	//}
	//this->field_0xc = piVar2;
	return;
}

void CPathFollow::Create(ByteCode* pByteCode)
{
	char* pcVar1;
	uint uVar2;
	int iVar3;
	int iVar4;

	pByteCode->Align(0x10);
	uVar2 = pByteCode->GetU32();
	pByteCode->GetS32();
	pByteCode->GetS32();
	iVar3 = pByteCode->GetS32();
	this->field_0x10 = iVar3;
	this->field_0x14 = 0;
	if ((uVar2 & 1) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 0x10;
		this->field_0x1c = pcVar1;
	}
	if ((uVar2 & 2) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 0x10;
		this->field_0x20 = pcVar1;
	}
	if ((uVar2 & 4) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 0x10;
		this->field_0x28 = pcVar1;
	}
	if ((uVar2 & 8) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 4;
		this->field_0x2c = pcVar1;
	}
	if ((uVar2 & 0x10) == 0) {
		this->field_0x18 = 0;
	}
	else {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 4;
		this->field_0x30 = pcVar1;
		this->field_0x18 = 0;
		iVar3 = 0;
		if (0 < (int)this->field_0x10) {
			iVar4 = 0;
			do {
				this->field_0x18 = this->field_0x18 | *(uint*)(this->field_0x30 + iVar4);
				if ((*(uint*)(this->field_0x30 + iVar4) & 1) != 0) {
					this->field_0x14 = iVar3;
				}
				iVar3 = iVar3 + 1;
				iVar4 = iVar4 + 4;
			} while (iVar3 < (int)this->field_0x10);
		}
	}
	if ((uVar2 & 0x20) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 4;
		this->field_0x34 = pcVar1;
	}
	if ((uVar2 & 0x40) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->field_0x10 * 4;
		this->field_0x38 = pcVar1;
	}
	uVar2 = pByteCode->GetU32();
	this->field_0x4 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->field_0x8 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->field_0xc = uVar2;
	return;
}
