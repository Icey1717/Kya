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

int CActorMovable::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int uVar1;

	if (msg == 0x48) {
		uVar1 = 1;
		(this->dynamic).field_0x58 = 1.2f;
	}
	else {
		if (msg == 0x47) {
			(this->dynamic).field_0x58 = 1.0f;
			uVar1 = 1;
		}
		else {
			if (msg == 0x46) {
				(this->dynamic).field_0x58 = 0.5f;
				uVar1 = 1;
			}
			else {
				if (msg == 0x45) {
					uVar1 = 1;
					(this->dynamic).field_0x58 = 0.8f;
				}
				else {
					uVar1 = CActor::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}
	return uVar1;
}

void CActorMovable::SV_MOV_UpdateSpeedIntensity(float param_1, float param_2)

{
	Timer* pTVar1;
	float fVar2;
	float fVar3;

	pTVar1 = GetTimer();
	fVar3 = (this->dynamic).speed;
	fVar2 = param_2 * pTVar1->cutsceneDeltaTime;
	if (fVar3 < param_1) {
		fVar3 = fVar3 + fVar2;
		(this->dynamic).speed = fVar3;
		if (param_1 < fVar3) {
			(this->dynamic).speed = param_1;
		}
	}
	else {
		fVar3 = fVar3 - fVar2;
		(this->dynamic).speed = fVar3;
		if (fVar3 < param_1) {
			(this->dynamic).speed = param_1;
		}
	}
	return;
}

void CActorMovable::SV_MOV_DecreaseSpeedIntensity(float param_1)
{
	Timer* pTVar1;
	float fVar2;

	pTVar1 = GetTimer();
	fVar2 = (this->dynamic).speed - param_1 * pTVar1->cutsceneDeltaTime;
	(this->dynamic).speed = fVar2;
	if (fVar2 < 0.0f) {
		(this->dynamic).speed = 0.0f;
	}
	return;
}

void CActorMovable::ComputeRealMoving(edF32VECTOR4* delta, float directionDot)
{
	Timer* pTVar1;
	float fVar2;
	edF32VECTOR4 eStack16;

	pTVar1 = GetTimer();
	edF32Vector4SubHard(&eStack16, &this->currentLocation, delta);
	fVar2 = edF32Vector4SafeNormalize1Hard(&(this->dynamic).velocityDirectionEuler, &eStack16);
	(this->dynamic).linearAcceleration = fVar2;
	(this->dynamic).linearAcceleration = (this->dynamic).linearAcceleration / pTVar1->cutsceneDeltaTime;

	eStack16.y = 0.0f;

	fVar2 = edF32Vector4SafeNormalize1Hard(&(this->dynamic).horizontalVelocityDirectionEuler, &eStack16);
	(this->dynamic).linearJerk = fVar2;
	(this->dynamic).linearJerk = (this->dynamic).linearJerk / pTVar1->cutsceneDeltaTime;
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
	

	this->velocityDirectionEuler = gF32Vector4UnitZ;
	this->horizontalVelocityDirectionEuler = gF32Vector4UnitZ;
	this->field_0x10 = gF32Vector4Zero;

	this->speed = 0.0f;
	this->linearAcceleration = 0;
	this->linearJerk = 0;
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