#include "ActorHedgehog.h"
#include "MemoryStream.h"

void CActorHedgehog::Create(ByteCode* pByteCode)
{
	CCollision* pCol;

	CActorAutonomous::Create(pByteCode);

	this->walkSpeed = pByteCode->GetF32();
	this->walkAcceleration = pByteCode->GetF32();
	this->walkRotSpeed = pByteCode->GetF32();
	this->runSpeed = pByteCode->GetF32();
	this->field_0x374 = pByteCode->GetU32();
	this->field_0x378 = pByteCode->GetU32();
	this->field_0x368 = pByteCode->GetF32();
	this->field_0x36c = pByteCode->GetF32();
	this->field_0x370 = pByteCode->GetF32();

	this->addOnGenerator.Create(this, pByteCode);

	this->field_0x380 = pByteCode->GetU32();
	pCol = this->pCollisionData;
	pCol->flags_0x0 = pCol->flags_0x0 | 0x8000;

	return;
}

void CActorHedgehog::Init()
{
	CActorAutonomous::Init();

	this->field_0x350 = 0;
	this->field_0x354 = 0;

	this->addOnGenerator.Init(0);

	return;
}

void CActorHedgehog::Term()
{
	CActorAutonomous::Term();

	this->addOnGenerator.Term();

	return;
}

void CActorHedgehog::Reset()
{
	this->field_0x350 = 0;

	CActorAutonomous::Reset();

	return;
}

CBehaviour* CActorHedgehog::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 6) {
		pBehaviour = &this->behaviourWatchDogArmor;
	}
	else {
		if (behaviourType == 5) {
			pBehaviour = &this->behaviourGuardAreaArmor;
		}
		else {
			if (behaviourType == 4) {
				pBehaviour = &this->behaviourWatchDog;
			}
			else {
				if (behaviourType == 3) {
					pBehaviour = &this->behaviourGuardArea;
				}
				else {
					pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorHedgehog::_gStateCfg_ABV[28] =
{
	StateConfig(0x6, 0x4),
	StateConfig(0x7, 0x1),
	StateConfig(0x0, 0x4),
	StateConfig(0x7, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x7, 0x4),
	StateConfig(0xD, 0x0),
	StateConfig(0xE, 0x0),
	StateConfig(0xF, 0x0),
	StateConfig(0x10, 0x0),
	StateConfig(0x11, 0x0),
	StateConfig(0x12, 0x0),
	StateConfig(0x7, 0x4),
	StateConfig(0x7, 0x4),
	StateConfig(0xC, 0x4),
	StateConfig(0x13, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x15, 0x4),
	StateConfig(0x16, 0x4),
	StateConfig(0x17, 0x4),
	StateConfig(0x1B, 0x4),
	StateConfig(0x18, 0x4),
	StateConfig(0x18, 0x4),
	StateConfig(0x19, 0x4),
	StateConfig(0x1A, 0x4),
	StateConfig(0x1C, 0x0),
	StateConfig(0x0, 0x1),
};

StateConfig* CActorHedgehog::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_ABV + state + -4;
	}

	return pStateConfig;
}

float CActorHedgehog::GetWalkSpeed()
{
	return this->walkSpeed;
}

float CActorHedgehog::GetWalkRotSpeed()
{
	return this->walkRotSpeed;
}

float CActorHedgehog::GetWalkAcceleration()
{
	return this->walkAcceleration;
}

float CActorHedgehog::GetRunSpeed()
{
	return this->runSpeed;
}

float CActorHedgehog::GetRunRotSpeed()
{
	return GetWalkRotSpeed();
}

float CActorHedgehog::GetRunAcceleration()
{
	return GetWalkAcceleration();
}