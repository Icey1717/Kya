#include "ActorBasicBox.h"
#include "MemoryStream.h"

void CActorBasicBox::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	int iVar3;
	uint uVar4;
	float fVar5;

	CActor::Create(pByteCode);
	iVar3 = pByteCode->GetS32();
	//this->field_0x160 = iVar3;
	uVar4 = pByteCode->GetU32();
	//this->field_0x16c = (float)(int)(float)uVar4;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//this->field_0x164 = piVar1;
	fVar5 = pByteCode->GetF32();
	//this->field_0x170 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x174 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x178 = fVar5;
	fVar5 = pByteCode->GetF32();
	//this->field_0x17c = fVar5;
	uVar4 = pByteCode->GetU32();
	//this->field_0x180 = uVar4;
	iVar3 = pByteCode->GetS32();
	//this->field_0x18c = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x190 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x198 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x194 = iVar3;
	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 0x1c;
	}
	//this->field_0x184 = piVar1;
	pcVar2 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar2 + 0x20;
	//this->field_0x188 = pcVar2;
	this->addOnGenerator.Create(this, pByteCode);
	return;
}
