#include "ActorMovable.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"

int INT_00448e04 = 0;

float CDynamic::gMaxSpeed_Horiz = 32.0f;
float CDynamic::gMaxSpeed_Vert = 20.0f;

void CActorMovable::Create(ByteCode* pByteCode)
{
	float fVar1;

	INT_00448e04 = INT_00448e04 + 1;
	CActor::Create(pByteCode);
	this->dynamic.Reset(this);
	fVar1 = pByteCode->GetF32();
	(this->dynamic).field_0x54 = fVar1;
	this->field_0x1c0 = 1e+30f;
	return;
}


void CActorMovable::SetState(int newState, int animType)
{
	if (newState != this->actorState) {
		this->dynamic.field_0x58 = 1.0f;
	}
	CActor::SetState(newState, animType);
	return;
}

void CActorMovable::SV_MOV_UpdateSpeedIntensity(float param_1, float param_2)

{
	Timer* pTVar1;
	float fVar2;
	float fVar3;

	pTVar1 = GetTimer();
	fVar3 = (this->dynamic).intensity;
	fVar2 = param_2 * pTVar1->cutsceneDeltaTime;
	if (fVar3 < param_1) {
		fVar3 = fVar3 + fVar2;
		(this->dynamic).intensity = fVar3;
		if (param_1 < fVar3) {
			(this->dynamic).intensity = param_1;
		}
	}
	else {
		fVar3 = fVar3 - fVar2;
		(this->dynamic).intensity = fVar3;
		if (fVar3 < param_1) {
			(this->dynamic).intensity = param_1;
		}
	}
	return;
}

void CActorMovable::ComputeRealMoving(edF32VECTOR4* delta, float param_3)
{
	Timer* pTVar1;
	float fVar2;
	edF32VECTOR4 eStack16;

	pTVar1 = GetTimer();
	edF32Vector4SubHard(&eStack16, &this->currentLocation, delta);
	edF32Vector4SafeNormalize1Hard(&(this->dynamic).currentLocation, &eStack16);
	(this->dynamic).field_0x44 = param_3;
	fVar2 = (this->dynamic).field_0x44 / pTVar1->cutsceneDeltaTime;
	(this->dynamic).field_0x44 = fVar2;
	eStack16.y = 0.0;
	edF32Vector4SafeNormalize1Hard(&(this->dynamic).horizontalLocation, &eStack16);
	(this->dynamic).field_0x40 = fVar2;
	(this->dynamic).field_0x40 = (this->dynamic).field_0x40 / pTVar1->cutsceneDeltaTime;
	return;
}

CDynamic::CDynamic()
{
	field_0x54 = 0.0f;
}

void CDynamic::Reset(CActor* pActor)
{
	float fVar1;
	float fVar2;
	float fVar3;

	this->field_0x4c = 0;
	this->flags = 0;
	

	this->currentLocation = gF32Vector4UnitZ;
	this->horizontalLocation = gF32Vector4UnitZ;
	this->field_0x10 = gF32Vector4Zero;

	this->intensity = 0.0f;
	this->field_0x44 = 0;
	this->field_0x40 = 0;
	this->field_0x58 = 1.0f;
	if (pActor == (CActor*)0x0) {
		(this->rotationQuat).x = 1.0f;
		(this->rotationQuat).y = 0.0f;
		(this->rotationQuat).z = 0.0f;
		(this->rotationQuat).w = 0.0f;
	}
	else {
		this->rotationQuat = pActor->rotationQuat;
	}
	return;
}