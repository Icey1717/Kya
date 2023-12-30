#include "ActorBox.h"
#include "MemoryStream.h"

void CActorBox::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	int iVar3;
	uint uVar4;
	float fVar5;

	CActorAutonomous::Create(pByteCode);
	iVar3 = pByteCode->GetS32();
	//this->field_0x350 = iVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//this->field_0x380 = piVar1;
	fVar5 = pByteCode->GetF32();
	//this->field_0x354 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x358 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x35c = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x360 = fVar5;
	uVar4 = pByteCode->GetU32();
	//this->field_0x364 = uVar4;
	iVar3 = pByteCode->GetS32();
	//this->field_0x370 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x374 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x378 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x37c = iVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x368 = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x36c = pcVar2;
	addOnGenerator.Create(this, pByteCode);
	//this->field_0x2c->flags_0x0 = this->field_0x2c->flags_0x0 | 0x180;
	return;
}
