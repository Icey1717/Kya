#include "ActorShoot.h"
#include "MemoryStream.h"

CActorShoot::~CActorShoot()
{
	//(this->base).base.base.pVTable = (CActorFighterVTable*)&_vt;
	//lVar1 = (**(code**)(*(int*)&this->field_0x360 + 0x14))();
	//if (lVar1 != 0) {
	//	StaticMeshComponent::Term((StaticMeshComponent*)&this->field_0x360, CScene::_scene_handleA);
	//}
	this->addOnGenerator.Term();
	//if (this != (CActorShoot*)0xfffff810) {
	//	*(CBehaviourVtable**)&this->field_0x7f0 = &CBehaviourShootFireWave::_vt;
	//	CConicalWaveShoot::~CConicalWaveShoot((long)(int)&this->conicalWaveShoot, 0xffffffffffffffff);
	//	if ((this != (CActorShoot*)0xfffff810) && (*(CBehaviourVtable**)&this->field_0x7f0 = &CBehaviourShoot::_vt, this != (CActorShoot*)0xfffff810)) {
	//		*(CBehaviourVtable**)&this->field_0x7f0 = &CBehaviour::_vt;
	//	}
	//}
	//if (((this != (CActorShoot*)0xfffffad0) && (*(CBehaviourVtable**)&this->field_0x530 = &CBehaviourShootFire::_vt, this != (CActorShoot*)0xfffffad0)) &&
	//	(*(CBehaviourVtable**)&this->field_0x530 = &CBehaviourShoot::_vt, this != (CActorShoot*)0xfffffad0)) {
	//	*(CBehaviourVtable**)&this->field_0x530 = &CBehaviour::_vt;
	//}
	//StaticMeshComponent::~StaticMeshComponent((StaticMeshComponent*)&this->field_0x360, -1);
	//if (lVar2 != 0) {
	//	(this->base).base.base.pVTable = (CActorFighterVTable*)&CActorAutonomous::_vt;
	//	CLifeInterface::~CLifeInterface((long)(int)&(this->base).lifeInterface, 0xffffffffffffffff);
	//	CActorMovable::~CActorMovable(lVar2, 0);
	//}
}

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