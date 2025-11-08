#include "ActorShoot.h"
#include "MemoryStream.h"

void CActorShoot::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;
	float fVar3;

	CActorAutonomous::Create(pByteCode);
	uVar1 = pByteCode->GetU32();
	//this->field_0x350 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x354 = uVar1;
	iVar2 = pByteCode->GetS32();
	//this->field_0x364 = iVar2;
	iVar2 = pByteCode->GetS32();
	//this->field_0x368 = iVar2;
	//(**(code**)(this->field_0x360 + 0x10))(&this->field_0x360);
	fVar3 = pByteCode->GetF32();
	//this->field_0x3c0 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3c4 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3c8 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3cc = fVar3;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3d0 = uVar1;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3d4 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3d8 = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3dc = fVar3;
	fVar3 = pByteCode->GetF32();
	//this->field_0x3e0 = fVar3;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3e8 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3e4 = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3ec = uVar1;
	uVar1 = pByteCode->GetU32();
	//this->field_0x3f0 = uVar1;
	this->addOnGenerator.Create(this, pByteCode);
	return;
}

void CActorShoot::Init()
{
	CActorAutonomous::Init();
	this->addOnGenerator.Init(0);
	return;
}