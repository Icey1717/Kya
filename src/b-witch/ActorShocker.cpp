#include "ActorShocker.h"
#include "MemoryStream.h"

void CActorShocker::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	uint uVar3;
	int iVar4;
	float fVar5;

	CActorAutonomous::Create(pByteCode);
	fVar5 = pByteCode->GetF32();
	//this->field_0x350 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x354 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x358 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x35c = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x364 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x368 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x360 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0xf0 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x36c = fVar5;
	uVar3 = pByteCode->GetU32();
	//this->field_0x374 = uVar3;
	uVar3 = pByteCode->GetU32();
	//this->field_0x370 = uVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x378 = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x37c = pcVar2;
	uVar3 = pByteCode->GetU32();
	//this->field_0x380 = uVar3;
	iVar4 = pByteCode->GetS32();
	//this->field_0x384 = iVar4;
	iVar4 = pByteCode->GetS32();
	//this->field_0x388 = iVar4;
	iVar4 = pByteCode->GetS32();
	//this->field_0x38c = iVar4;
	iVar4 = pByteCode->GetS32();
	//this->field_0x390 = iVar4;
	iVar4 = pByteCode->GetS32();
	//this->field_0x394 = iVar4;
	iVar4 = pByteCode->GetS32();
	//this->field_0x398 = iVar4;
	uVar3 = pByteCode->GetU32();
	//this->field_0x39c = uVar3;
	//ActorFunc_001157e0((Actor*)this, this->field_0x394, 0, (long)&this->field_0x784);
	//ActorFunc_001157e0((Actor*)this, this->field_0x394, 1, (long)&this->field_0x794);
	this->addOnGenerator.Create(this, pByteCode);
	return;
}

void CActorShocker::Init()
{
	CActorAutonomous::Init();
	this->addOnGenerator.Init(0);
	return;
}