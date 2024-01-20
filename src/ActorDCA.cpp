#include "ActorDCA.h"
#include "MemoryStream.h"

void FUN_003379e0(ByteCode* pByteCode)
{
	int* piVar1;

	piVar1 = (int*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(piVar1 + 1);
	if (*piVar1 != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + *piVar1 * 4;
	}
	//*param_1 = piVar1;
	return;
}

void CActorDCA::Create(ByteCode* pByteCode)
{
	CCollision* pCVar1;
	float fVar2;
	int iVar3;
	uint uVar4;
	int* piVar5;
	float fVar6;

	CActor::Create(pByteCode);
	//this->field_0x4f4 = 0;
	iVar3 = pByteCode->GetS32();
	//this->field_0x504 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x500 = iVar3;
	fVar6 = pByteCode->GetF32();
	//this->field_0x464 = fVar6;
	iVar3 = pByteCode->GetS32();
	//this->field_0x508 = iVar3;
	iVar3 = pByteCode->GetS32();
	//this->field_0x50c = iVar3;
	fVar6 = pByteCode->GetF32();
	//this->field_0x160 = fVar6 * 0.01745329;
	FUN_003379e0(/*&this->field_0x1b0, */pByteCode);
	uVar4 = pByteCode->GetU32();
	//this->field_0x450 = uVar4;
	uVar4 = pByteCode->GetU32();
	//this->field_0x454 = uVar4;
	//pCVar1 = (this->base).data.pCollisionData;
	//if (pCVar1 != (CollisionData*)0x0) {
	//	pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffffc4;
	//	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x800480;
	//}
	//piVar5 = CActor::CreateActorSound((CActor*)this, 4);
	//this->field_0x4fc = piVar5;
	//fVar2 = gF32Vertex4Zero.z;
	//fVar6 = gF32Vertex4Zero.y;
	//this->field_0x164 = gF32Vertex4Zero.x;
	//this->field_0x168 = fVar6;
	//this->field_0x16c = fVar2;
	//fVar2 = gF32Vertex4Zero.z;
	//fVar6 = gF32Vertex4Zero.y;
	//this->field_0x170 = gF32Vertex4Zero.x;
	//this->field_0x174 = fVar6;
	//this->field_0x178 = fVar2;
	//this->field_0x1a0 = 0;
	//this->field_0x4f0 = 3.0;
	return;
}
