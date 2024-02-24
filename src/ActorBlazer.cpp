#include "ActorBlazer.h"
#include "MemoryStream.h"

void CActorBlazer::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	uint uVar3;
	int iVar4;
	float fVar5;

	CActor::Create(pByteCode);
	this->vision.Create(this, pByteCode);
	uVar3 = pByteCode->GetU32();
	//this->field_0x1d0 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1d4 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1d8 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1dc = uVar3;
	this->pathFollowReader.Create(pByteCode);
	fVar5 = pByteCode->GetF32();
	//this->field_0x1e0 = fVar5;

	pByteCode->GetF32();
	//FUN_00115040(&this->field_0x1e4, pByteCode);

	fVar5 = pByteCode->GetF32();
	//this->field_0x1ec = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x1f0 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x1f4 = fVar5;
	uVar3 = pByteCode->GetU32();
	//this->field_0x1f8 = uVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//this->field_0x1fc = piVar1;
	iVar4 = pByteCode->GetS32();
	//this->field_0x200 = iVar4;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x204 = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x208 = pcVar2;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x20c = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x210 = pcVar2;
	iVar4 = pByteCode->GetS32();
	//this->field_0x214 = iVar4;
	addOnGenerator.Create(this, pByteCode);
	return;
}
