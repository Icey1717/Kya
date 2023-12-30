#include "ActorAmortos.h"
#include "MemoryStream.h"

void CActorAmortos::Create(ByteCode* pByteCode)
{
	ActorAndWaypoint* pAVar1;
	int* piVar2;
	char* pcVar3;
	int iVar4;
	uint uVar5;
	float fVar6;

	CActor::Create(pByteCode);
	pAVar1 = (ActorAndWaypoint*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)&pAVar1->pWaypoint;
	if (pAVar1->pActor != (CActor*)0x0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (int)pAVar1->pActor * 4;
	}
	//this->pCheckpoint_0x184 = pAVar1;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	//this->field_0x188 = piVar2;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 4;
	}
	//this->field_0x18c = piVar2;
	iVar4 = pByteCode->GetS32();
	//this->field_0x190 = iVar4;
	fVar6 = pByteCode->GetF32();
	//this->field_0x164 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x168 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x160 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x16c = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x170 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x174 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x178 = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x17c = fVar6;
	fVar6 = pByteCode->GetF32();
	//this->field_0x180 = fVar6;
	this->addOnGenerator.Create(this, pByteCode);
	uVar5 = pByteCode->GetU32();
	//this->field_0x1c8 = uVar5;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
	}
	//this->field_0x1cc = piVar2;
	pcVar3 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar3 + 0x20;
	//this->field_0x1d0 = (int)pcVar3;
	piVar2 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar2 + 1);
	if (*piVar2 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar2 * 0x1c;
	}
	//this->field_0x1d4 = piVar2;
	pcVar3 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar3 + 0x20;
	//this->field_0x1d8 = (int)pcVar3;
	return;
}
