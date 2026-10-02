#include "ActorHedgehog.h"
#include "ActorHero.h"
#include "ActorBox.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "EventManager.h"

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

void CActorHedgehog::BehaviourGuardArea_Manage(CBehaviourHedgehogGuardArea* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	uint uVar5;
	edF32VECTOR4* v0;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack240;
	float local_e0;
	float fStack220;
	float fStack216;
	float fStack212;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 local_c0;
	_msg_hit_param local_b0;
	_msg_impulse_params local_30;

	switch (this->actorState) {
	case 6:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		if ((this->field_0x350 & 0x10) == 0) {
			if ((this->field_0x350 & 8) == 0) {
				bVar3 = pBehaviour->pathFollowReader.AtGoal((pBehaviour->pathFollowReader).splinePointIndex, (pBehaviour->pathFollowReader).field_0xc);
				if (bVar3 == false) {
					SetState(0x10, -1);
				}
			}
			else {
				SetState(0x12, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 7:
		StateGuardChase(pBehaviour);
		break;
	case 8:
		StateGuardChaseStand(pBehaviour);
		break;
	case 9:
		StateGuardComeBack(pBehaviour);
		break;
	case 10:
		this->dynamic.speed = 0.0f;

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0xb, -1);
		}
		break;
	case 0xb:
		StateGuardUpsideDown();
		break;
	case 0xc:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->field_0x350 = this->field_0x350 & 0xffffffee;
			SetState(9, -1);
		}
		break;
	case 0xd:
		this->dynamic.speed = 0.0f;

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			local_30.field_0x10 = 500.0f;
			local_30.field_0x0.y = 1.0f;
			local_30.field_0x0.x = 0.0f;
			local_30.field_0x0.z = 0.0f;
			local_30.field_0x0.w = 0.0f;
			DoMessage(CActorHero::_gThis, MESSAGE_IMPULSE, &local_30);
			SetState(0xe, -1);
		}
		break;
	case 0xe:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		local_e0 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
		fStack220 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
		fStack216 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
		fStack212 = CActorHero::_gThis->currentLocation.w - this->currentLocation.w;
		if (this->field_0x368 <= sqrtf(local_e0 * local_e0 + fStack220 * fStack220 + fStack216 * fStack216)) {
			SetState(0xf, -1);
		}
		break;
	case 0xf:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->field_0x350 = this->field_0x350 & 0xffffffee;
			SetState(9, -1);
		}
		break;
	case 0x10:
		State_0x10(pBehaviour);
		break;
	case 0x11:
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if ((this->field_0x350 & 0x10) == 0) {
			if ((this->field_0x350 & 8) == 0) {
				fVar7 = this->timeInAir;
				fVar6 = pBehaviour->pathFollowReader.GetDelay();
				if (fVar6 < fVar7) {
					bVar3 = pBehaviour->pathFollowReader.AtGoal(pBehaviour->pathFollowReader.splinePointIndex, pBehaviour->pathFollowReader.field_0xc);
					if (bVar3 == false) {
						pBehaviour->pathFollowReader.NextWayPoint();
						SetState(0x10, -1);
					}
					else {
						SetState(6, -1);
					}
				}
			}
			else {
				SetState(0x12, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x12:
		StateGuardSurprised(pBehaviour);
		break;
	case 0x13:
		ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(7, -1);
		}
		break;
	case 0x14:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		local_b0.projectileType = 0;
		local_b0.flags = 1;
		local_b0.damage = this->field_0x370;
		DoMessage(this->field_0x354, MESSAGE_KICKED, &local_b0);
		this->dynamic.speed = 0.0f;
		SetState(0x15, -1);
		break;
	case 0x15:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		if (1.0f <= this->timeInAir) {
			this->field_0x350 = this->field_0x350 & 0xfffffffe;
			if ((this->field_0x350 & 8) == 0) {
				SetState(9, -1);
			}
			else {
				SetState(7, -1);
			}
		}
		else {
			if ((this->field_0x350 & 0x10) != 0) {
				uVar5 = TreatBoomyHit(pBehaviour);
				SetState(uVar5, -1);
			}
		}
		break;
	case 0x16:
		this->dynamic.speed = 0.0f;
		if (this->field_0x354 != (CActor*)0x0) {
			fVar6 = GetRunRotSpeed();
			SV_UpdateOrientationToPosition2D(fVar6, &this->field_0x354->currentLocation);
		}
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				local_c0.z = this->rotationQuat.z;
				local_c0.w = this->rotationQuat.w;
				local_c0.x = (float)*(undefined8*)&this->rotationQuat;
				local_c0.y = 0.0f;
				edF32Vector4NormalizeHard(&local_c0, &local_c0);
				local_c0.y = 1.5f;
				edF32Vector4NormalizeHard(&local_c0, &local_c0);
				edF32Vector4ScaleHard(600.0f, &eStack208, &local_c0);
				this->dynamic.speed = 0.0f;
				edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack240, &eStack208);
				v0 = this->dynamicExt.aImpulseVelocities;
				edF32Vector4AddHard(v0, v0, &eStack240);
				fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
				this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;
				SetState(0x17, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x17:
		StateFly(pBehaviour);
		break;
	case 0x18:
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if ((((this->pCollisionData)->flags_0x4 & 2) != 0) && (fabsf(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) < 1.0f)) {
				this->dynamic.speed = 0.0f;
				SetState(0x19, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x19:
		this->dynamic.speed = 0.0f;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(7, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1a:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (3.0f <= this->timeInAir) {
				SetState(0x1c, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1b:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (3.0f <= this->timeInAir) {
				TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
				SetState(0x1c, -1);
			}
			else {
				if ((this->field_0x37c->typeID == 0x35) && (this->field_0x37c->actorState == 0xd)) {
					TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
					SetState(0x1c, -1);
				}
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1c:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(0x1d, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1d:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(0x18, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1e:
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0x1f, -1);
		}
	}

	CheckArea();

	uVar5 = this->field_0x350;
	if ((uVar5 & 4) == 0) {
		if ((uVar5 & 2) == 0) {
			this->field_0x350 = uVar5 & 0xfffffff7;
			this->field_0x354 = (CActor*)0x0;
		}
	}
	else {
		this->field_0x350 = uVar5 | 8;
	}

	return;
}

void CActorHedgehog::BehaviourWatchDog_Manage(CBehaviourHedgehogWatchDog* pBehaviour)
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	uint uVar5;
	edF32VECTOR4* v0;
	float fVar6;
	edF32VECTOR4 eStack240;
	float local_e0;
	float fStack220;
	float fStack216;
	float fStack212;
	edF32VECTOR4 eStack208;
	edF32VECTOR4 local_c0;
	_msg_hit_param local_b0;
	_msg_impulse_params local_30;

	switch (this->actorState) {
	case 6:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if ((this->field_0x350 & 0x10) == 0) {
			if ((this->field_0x350 & 8) != 0) {
				SetState(0x12, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 7:
		StateGuardChase(pBehaviour);
		break;
	case 8:
		StateGuardChaseStand(pBehaviour);
		break;
	case 9:
		StateGuardComeBack(pBehaviour);
		break;
	case 10:
		this->dynamic.speed = 0.0f;

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0xb, -1);
		}
		break;
	case 0xb:
		StateGuardUpsideDown();
		break;
	case 0xc:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->field_0x350 = this->field_0x350 & 0xffffffee;
			SetState(9, -1);
		}
		break;
	case 0xd:
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		pCVar1 = this->pAnimationController;
		peVar2 = pCVar1->anmBinMetaAnimator.aAnimData;
		if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType) {
			bVar3 = false;
			if (peVar2->animPlayState != 0) {
				bVar3 = (peVar2->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar3 = false;
		}

		if (bVar3) {
			local_30.field_0x10 = 500.0;
			local_30.field_0x0.y = 1.0;
			local_30.field_0x0.x = 0.0;
			local_30.field_0x0.z = 0.0;
			local_30.field_0x0.w = 0.0;
			DoMessage(CActorHero::_gThis, MESSAGE_IMPULSE, &local_30);
			SetState(0xe, -1);
		}
		break;
	case 0xe:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		local_e0 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
		fStack220 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
		fStack216 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
		fStack212 = CActorHero::_gThis->currentLocation.w - this->currentLocation.w;
		if (this->field_0x368 <= sqrtf(local_e0 * local_e0 + fStack220 * fStack220 + fStack216 * fStack216)) {
			SetState(0xf, -1);
		}
		break;
	case 0xf:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->field_0x350 = this->field_0x350 & 0xffffffee;
			SetState(9, -1);
		}
		break;
	case 0x12:
		StateGuardSurprised(pBehaviour);
		break;
	case 0x13:
		ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(7, -1);
		}
		break;
	case 0x14:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		local_b0.projectileType = 0;
		local_b0.flags = 1;
		local_b0.damage = this->field_0x370;
		DoMessage(this->field_0x354, MESSAGE_KICKED, &local_b0);
		this->dynamic.speed = 0.0f;
		SetState(0x15, -1);
		break;
	case 0x15:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (1.0f <= this->timeInAir) {
			this->field_0x350 = this->field_0x350 & 0xfffffffe;
			if ((this->field_0x350 & 8) == 0) {
				SetState(9, -1);
			}
			else {
				SetState(7, -1);
			}
		}
		else {
			if ((this->field_0x350 & 0x10) != 0) {
				uVar5 = TreatBoomyHit(pBehaviour);
				SetState(uVar5, -1);
			}
		}
		break;
	case 0x16:
		this->dynamic.speed = 0.0f;
		if (this->field_0x354 != 0) {
			fVar6 = GetRunRotSpeed();
			SV_UpdateOrientationToPosition2D(fVar6, &this->field_0x354->currentLocation);
		}

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				local_c0.z = this->rotationQuat.z;
				local_c0.w = this->rotationQuat.w;
				local_c0.x = this->rotationQuat.x;
				local_c0.y = 0.0f;
				edF32Vector4NormalizeHard(&local_c0, &local_c0);
				local_c0.y = 1.5f;
				edF32Vector4NormalizeHard(&local_c0, &local_c0);
				edF32Vector4ScaleHard(600.0f, &eStack208, &local_c0);
				this->dynamic.speed = 0.0f;
				edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack240, &eStack208);
				v0 = this->dynamicExt.aImpulseVelocities;
				edF32Vector4AddHard(v0, v0, &eStack240);
				fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
				this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;
				SetState(0x17, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x17:
		StateFly(pBehaviour);
		break;
	case 0x18:
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if ((((this->pCollisionData)->flags_0x4 & 2) != 0) && (fabsf(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) < 1.0f)) {
				this->dynamic.speed = 0.0f;
				SetState(0x19, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x19:
		this->dynamic.speed = 0.0f;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(7, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1a:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (3.0f <= this->timeInAir) {
				SetState(0x1c, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1b:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (3.0f <= this->timeInAir) {
				TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
				SetState(0x1c, -1);
			}
			else {
				if ((this->field_0x37c->typeID == BOX) && (static_cast<CActorBox*>(this->field_0x37c)->actorState == 0xd)) {
					TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
					SetState(0x1c, -1);
				}
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1c:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(0x1d, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1d:
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		this->field_0x350 = this->field_0x350 & 0xfffffffe;
		if ((this->field_0x350 & 0x10) == 0) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(0x18, -1);
			}
		}
		else {
			uVar5 = TreatBoomyHit(pBehaviour);
			SetState(uVar5, -1);
		}
		break;
	case 0x1e:
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(0x1f, -1);
		}
	}

	CheckArea();

	uVar5 = this->field_0x350;
	if ((uVar5 & 4) == 0) {
		if ((uVar5 & 2) == 0) {
			this->field_0x350 = uVar5 & 0xfffffff7;
			this->field_0x354 = 0;
		}
	}
	else {
		this->field_0x350 = uVar5 | 8;
	}

	return;
}

uint CActorHedgehog::TreatBoomyHit(CBehaviourHedgehog* pBehaviour)
{
	uint uVar1;
	CLifeInterface* pCVar2;
	long lVar4;
	edF32VECTOR4* v0;
	float fVar5;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	this->field_0x350 = this->field_0x350 & 0xffffffef;

	lVar4 = pBehaviour->HasArmor();
	if (lVar4 == 0) {
		LifeDecrease(0x3f800000);

		pCVar2 = GetLifeInterface();
		fVar5 = pCVar2->GetValue();
		if (fVar5 <= 0.0f) {
			uVar1 = 0x1e;
		}
		else {
			local_10.z = (this->field_0x390).z;
			local_10.w = (this->field_0x390).w;
			local_10.x = (this->field_0x390).x;
			local_10.y = 0.0f;
			edF32Vector4NormalizeHard(&local_10, &local_10);
			local_10.y = 1.0f;
			edF32Vector4NormalizeHard(&local_10, &local_10);
			edF32Vector4ScaleHard(200.0f, &local_10, &local_10);
			this->dynamic.speed = 0.0f;
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack32, &local_10);
			v0 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(v0, v0, &eStack32);
			fVar5 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar5;
			uVar1 = 0x13;
		}
	}
	else {
		uVar1 = 10;
	}

	return uVar1;
}

void CActorHedgehog::StateGuardChase(CBehaviourHedgehog* pBehaviour)
{
	uint uVar1;
	long lVar2;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0f;
	if (this->field_0x354 != 0) {
		movParamsIn.rotSpeed = GetRunRotSpeed();
		movParamsIn.flags = movParamsIn.flags | 2;
		movParamsIn.acceleration = GetRunAcceleration();
		movParamsIn.speed = GetRunSpeed();
		movParamsIn.flags = movParamsIn.flags | 0x400;
		SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x354->currentLocation);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if ((this->field_0x350 & 8) == 0) {
		SetState(9, -1);
	}
	else {
		if ((1.5f < movParamsOut.moveVelocity) || (lVar2 = pBehaviour->HasArmor(), lVar2 != 0)) {
			if (movParamsOut.moveVelocity <= 0.2f) {
				SetState(8, -1);
			}
			else {
				if (((this->field_0x350 & 1) == 0) || (lVar2 = pBehaviour->HasArmor(), lVar2 == 0)) {
					if ((this->field_0x350 & 0x10) != 0) {
						uVar1 = TreatBoomyHit(pBehaviour);
						SetState(uVar1, -1);
					}
				}
				else {
					SetState(0x14, -1);
				}
			}
		}
		else {
			SetState(0x16, -1);
		}
	}

	return;
}

void CActorHedgehog::StateGuardChaseStand(CBehaviourHedgehog* pBehaviour)
{
	CActor* pCVar1;
	float fVar2;
	float fVar3;
	uint uVar4;
	long lVar5;

	this->dynamic.speed = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if ((this->field_0x350 & 8) == 0) {
		SetState(9, -1);
	}
	else {
		pCVar1 = this->field_0x354;
		if (((pCVar1 == (CActor*)0x0) ||
			(fVar2 = (pCVar1->currentLocation).x - this->currentLocation.x, fVar3 = (pCVar1->currentLocation).z - this->currentLocation.z,
				1.5f < sqrtf(fVar2 * fVar2 + 0.0f + fVar3 * fVar3))) || (lVar5 = pBehaviour->HasArmor(), lVar5 != 0)) {
			if ((this->field_0x354 == (CActor*)0x0) ||
				(fVar2 = (this->field_0x354->currentLocation).x - this->currentLocation.x, fVar3 = (this->field_0x354->currentLocation).z - this->currentLocation.z,
					sqrtf(fVar2 * fVar2 + 0.0f + fVar3 * fVar3) < 0.2f)) {
				if (((this->field_0x350 & 1) == 0) || (lVar5 = pBehaviour->HasArmor(), lVar5 == 0)) {
					if ((this->field_0x350 & 0x10) != 0) {
						uVar4 = TreatBoomyHit(pBehaviour);
						SetState(uVar4, -1);
					}
				}
				else {
					SetState(0x14, -1);
				}
			}
			else {
				SetState(7, -1);
			}
		}
		else {
			SetState(0x16, -1);
		}
	}

	return;
}

void CActorHedgehog::StateGuardComeBack(CBehaviourHedgehog* pBehaviour)
{
	edF32VECTOR4* pMoveToPosition;
	uint uVar1;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0f;
	movParamsIn.rotSpeed = GetWalkRotSpeed();
	movParamsIn.flags = movParamsIn.flags | 2;
	movParamsIn.acceleration = GetWalkAcceleration();
	movParamsIn.speed = GetWalkSpeed();
	movParamsIn.flags = movParamsIn.flags | 0x400;
	pMoveToPosition = pBehaviour->GetComeBackPosition();
	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, pMoveToPosition);

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if (movParamsOut.moveVelocity <= 0.5f) {
		this->dynamic.speed = 0.0f;
		SetState(6, -1);
	}
	else {
		uVar1 = this->field_0x350;
		if ((uVar1 & 8) == 0) {
			if ((uVar1 & 1) == 0) {
				if ((uVar1 & 0x10) != 0) {
					uVar1 = TreatBoomyHit(pBehaviour);
					SetState(uVar1, -1);
				}
			}
			else {
				SetState(0x14, -1);
			}
		}
		else {
			SetState(0x12, -1);
		}
	}

	return;
}

void CActorHedgehog::StateGuardSurprised(CBehaviourHedgehog* pBehaviour)
{
	bool bVar3;
	uint uVar4;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	this->dynamic.speed = 0.0f;
	if (this->field_0x354 != (CActor*)0x0) {
		movParamsIn.speed = 0.0f;
		movParamsOut.flags = 0;
		movParamsIn.pRotation = (edF32VECTOR4*)0x0;
		movParamsIn.flags = 0x10;
		movParamsIn.rotSpeed = GetWalkRotSpeed();
		movParamsIn.flags = movParamsIn.flags | 2;
		SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &this->field_0x354->currentLocation);
	}

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	uVar4 = this->field_0x350;
	if ((uVar4 & 0x10) == 0) {
		if ((uVar4 & 8) == 0) {
			SetState(9, -1);
		}
		else {
			if ((uVar4 & 1) == 0) {
				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					SetState(7, -1);
				}
			}
			else {
				SetState(0x14, -1);
			}
		}
	}
	else {
		uVar4 = TreatBoomyHit(pBehaviour);
		SetState(uVar4, -1);
	}

	return;
}

void CActorHedgehog::StateFly(CBehaviourHedgehog* pBehaviour)
{
	int iVar1;
	uint uVar2;
	edF32VECTOR4* peVar4;
	CActor* pReceiver;
	int iVar5;
	float fVar6;
	edF32VECTOR4 eStack624;
	edF32VECTOR4 eStack608;
	edF32VECTOR4 eStack592;
	edF32VECTOR4 eStack576;
	edF32VECTOR4 local_230;
	_msg_hit_param local_220;
	edF32VECTOR4 local_1a0;
	_msg_hit_param local_190;
	CActorsTable actorsTable;

	actorsTable.nbEntries = 0;
	ManageDyn(4.0f, 0x329, &actorsTable);

	this->field_0x37c = (CActor*)0x0;
	pReceiver = (CActor*)0x0;
	for (iVar5 = 0; iVar5 < actorsTable.nbEntries; iVar5 = iVar5 + 1) {
		if (actorsTable.aEntries[iVar5]->typeID == AMORTOS) {
			pReceiver = actorsTable.aEntries[iVar5];
		}

		iVar1 = actorsTable.aEntries[iVar5]->typeID;
		if ((iVar1 == BOX) || (iVar1 == MOVING_PLATFORM)) {
			this->field_0x37c = actorsTable.aEntries[iVar5];
		}
	}

	uVar2 = this->field_0x350;
	if ((uVar2 & 0x10) == 0) {
		if ((uVar2 & 1) == 0) {
			if (pReceiver == (CActor*)0x0) {
				if ((((this->pCollisionData)->flags_0x4 & 2) == 0) || (1.0f <= fabsf(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y))) {
					if (this->field_0x37c == (CActor*)0x0) {
						if (((this->pCollisionData)->flags_0x4 & 1) != 0) {
							this->dynamicExt.normalizedTranslation.x = 0.0f;
							this->dynamicExt.normalizedTranslation.y = 0.0f;
							this->dynamicExt.normalizedTranslation.z = 0.0f;
							this->dynamicExt.normalizedTranslation.w = 0.0f;
							this->dynamicExt.field_0x6c = 0.0f;
							this->dynamic.speed = 0.0f;

							SetState(0x1a, -1);
						}
					}
					else {
						this->dynamicExt.normalizedTranslation.x = 0.0f;
						this->dynamicExt.normalizedTranslation.y = 0.0f;
						this->dynamicExt.normalizedTranslation.z = 0.0f;
						this->dynamicExt.normalizedTranslation.w = 0.0f;
						this->dynamicExt.field_0x6c = 0.0f;
						this->dynamic.speed = 0.0f;

						TieToActor(this->field_0x37c, 0, 1, (edF32MATRIX4*)0x0);

						SetState(0x1b, -1);
					}
				}
				else {
					this->dynamic.speed = 0.0f;
					SetState(0x19, -1);
				}
			}
			else {
				local_220.projectileType = 0;
				DoMessage(pReceiver, MESSAGE_KICKED, &local_220);
				local_230.w = this->rotationQuat.w;
				local_230.y = 0.0f;
				local_230.x = -(float)*(undefined8*)&this->rotationQuat;
				local_230.z = -this->rotationQuat.z;
				edF32Vector4NormalizeHard(&local_230, &local_230);
				edF32Vector4ScaleHard(500.0f, &eStack592, &local_230);
				this->dynamic.speed = 0.0f;
				edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack624, &eStack592);
				peVar4 = this->dynamicExt.aImpulseVelocities;
				edF32Vector4AddHard(peVar4, peVar4, &eStack624);
				fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
				this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;
				SetState(0x17, -1);
				SetState(0x18, -1);
			}
		}
		else {
			this->field_0x350 = uVar2 & 0xfffffffe;
			this->dynamicExt.normalizedTranslation.x = 0.0f;
			this->dynamicExt.normalizedTranslation.y = 0.0f;
			this->dynamicExt.normalizedTranslation.z = 0.0f;
			this->dynamicExt.normalizedTranslation.w = 0.0f;
			this->dynamicExt.field_0x6c = 0.0f;
			this->dynamic.speed = 0.0f;
			local_190.flags = 1;
			local_190.projectileType = 0;
			local_190.damage = this->field_0x370;
			DoMessage(this->field_0x354, MESSAGE_KICKED, &local_190);
			local_1a0.w = this->rotationQuat.w;
			local_1a0.y = 0.0f;
			local_1a0.x = -(float)*(undefined8*)&this->rotationQuat;
			local_1a0.z = -this->rotationQuat.z;
			edF32Vector4NormalizeHard(&local_1a0, &local_1a0);
			edF32Vector4ScaleHard(200.0f, &eStack576, &local_1a0);
			this->dynamic.speed = 0.0f;
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack608, &eStack576);
			peVar4 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(peVar4, peVar4, &eStack608);
			fVar6 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar6;

			SetState(0x17, -1);
			SetState(0x18, -1);
		}
	}
	else {
		uVar2 = TreatBoomyHit(pBehaviour);
		SetState(uVar2, -1);
	}

	return;
}

void CActorHedgehog::StateGuardUpsideDown()
{
	long lVar1;
	CActorConeInfluence coneInfluence;
	uint local_4;
	CActorHero* pHero;

	this->dynamic.speed = 0.0f;
	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);
	pHero = CActorHero::_gThis;
	if (this->field_0x36c <= this->timeInAir) {
		SetState(0xc, -1);
	}
	else {
		coneInfluence.field_0x20.x = 0.0f;
		coneInfluence.field_0x20.y = 0.0f;
		coneInfluence.field_0x20.z = 0.0f;
		coneInfluence.field_0x20.w = 0.0f;
		coneInfluence.field_0x8 = 3.0f;
		coneInfluence.field_0x0 = 0.80000001f;
		coneInfluence.field_0x4 = 0.80000001f;
		coneInfluence.field_0xc = 1.0f;
		coneInfluence.field_0x10 = 0.0f;
		coneInfluence.field_0x14 = 1;
		SV_AttractActorInAConeAboveMe(CActorHero::_gThis, &coneInfluence);
		if (((pHero->dynamic.velocityDirectionEuler.y < 0.001f) &&
			(this->currentLocation.y + ((this->pCollisionData)->pObbPrim->position).y < ((pHero->pCollisionData)->highestVertex).y)) &&
			(lVar1 = SV_IsCylinderIntersect(0.15f, 0.0f, pHero), lVar1 != 0)) {
			local_4 = this->field_0x380;
			DoMessage(pHero, MESSAGE_ENTER_TRAMPO, (MSG_PARAM)local_4);
			SetState(0xd, -1);
		}
	}

	return;
}

void CActorHedgehog::State_0x10(CBehaviourHedgehogGuardArea* pBehaviour)
{
	edF32VECTOR4* peVar1;
	uint uVar2;
	edF32MATRIX4 eStack144;
	edF32VECTOR4 eStack80;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	CActor* pOther;

	movParamsOut.flags = 0;
	movParamsIn.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0f;
	pOther = this->pTiedActor;
	if (pOther == (CActor*)0x0) {
		peVar1 = pBehaviour->pathFollowReader.GetWayPoint();
	}
	else {
		pOther->SV_ComputeDiffMatrixFromInit(&eStack144);
		peVar1 = pBehaviour->pathFollowReader.GetWayPoint();
		edF32Matrix4MulF32Vector4Hard(&eStack80, &eStack144, peVar1);
		peVar1 = &eStack80;
	}

	movParamsIn.flags = movParamsIn.flags | 0x10;
	movParamsIn.rotSpeed = GetWalkRotSpeed();
	movParamsIn.flags = movParamsIn.flags | 2;
	movParamsIn.acceleration = GetWalkAcceleration();
	movParamsIn.speed = GetWalkSpeed();
	movParamsIn.flags = movParamsIn.flags | 0x400;
	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, peVar1);

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if ((this->field_0x350 & 0x10) == 0) {
		if ((this->field_0x350 & 8) == 0) {
			if (movParamsOut.moveVelocity < 0.5f) {
				SetState(0x11, -1);
			}
		}
		else {
			SetState(0x12, -1);
		}
	}
	else {
		uVar2 = TreatBoomyHit(pBehaviour);
		SetState(uVar2, -1);
	}

	return;
}

bool CActorHedgehog::SV_IsCylinderIntersect(float param_1, float param_2, CActor* pOtherActor)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	local_20 = (pOtherActor->currentLocation);
	edF32Vector4AddHard(&local_20, &local_20, &pOtherActor->pCollisionData->pObbPrim->position);
	local_30 = this->currentLocation;
	edF32Vector4AddHard(&local_30, &local_30, &(this->pCollisionData)->pObbPrim->position);
	edF32Vector4SubHard(&eStack16, &local_20, &local_30);
	fVar3 = eStack16.y;
	eStack16.y = 0.0f;
	fVar2 = edF32Vector4GetDistHard(&eStack16);
	if (fVar2 < (fabsf((pOtherActor->pCollisionData->pObbPrim->scale).x + ((this->pCollisionData)->pObbPrim->scale).x) - param_1) - 0.001f) {
		bVar1 = true;
		if ((fabsf((pOtherActor->pCollisionData->pObbPrim->scale).y + ((this->pCollisionData)->pObbPrim->scale).y) - param_2) - 0.001f <= fabsf(fVar3)) {
			bVar1 = false;
		}
	}
	else {
		bVar1 = false;
	}

	return bVar1;
}

int CActorHedgehog::CheckArea()
{
	uint uVar1;
	ed_zone_3d* pZone;
	int iVar2;
	CEventManager* pEventManager;
	CActorHero* pHero;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pHero = CActorHero::_gThis;
	uVar1 = this->field_0x374;
	if (uVar1 != 0xffffffff) {
		pZone = (ed_zone_3d*)0x0;
		if (uVar1 != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, uVar1);
		}

		iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pHero->currentLocation, 0);
		if (iVar2 == 1) {
			if (((this->field_0x350 & 4) == 0) && (this->field_0x354 == 0)) {
				this->field_0x354 = pHero;
			}

			this->field_0x350 = this->field_0x350 | 4;
		}
		else {
			this->field_0x350 = this->field_0x350 & 0xfffffffb;
		}
	}

	uVar1 = this->field_0x378;
	if (uVar1 != 0xffffffff) {
		pZone = (ed_zone_3d*)0x0;
		if (uVar1 != 0xffffffff) {
			pZone = edEventGetChunkZone(pEventManager->activeChunkId, uVar1);
		}

		iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pHero->currentLocation, 0);
		if (iVar2 == 1) {
			if (((this->field_0x350 & 2) == 0) && (this->field_0x354 == 0)) {
				this->field_0x354 = pHero;
			}

			this->field_0x350 = this->field_0x350 | 2;
		}
		else {
			this->field_0x350 = this->field_0x350 & 0xfffffffd;
		}
	}

	return;
}

void CBehaviourHedgehog::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHedgehog::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorHedgehog*>(pOwner);

	return;
}

void CBehaviourHedgehog::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		this->pOwner->SetState(6, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourHedgehog::InitState(int newState)
{
	ulong uVar2;
	edF32VECTOR4* v0;
	float fVar3;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CCollision* pCol;
	CActorHedgehog* pHedgehog;

	if (newState == 0x1e) {
		pHedgehog = this->pOwner;
		local_20.x = pHedgehog->currentLocation.x;
		local_20.z = pHedgehog->currentLocation.z;
		local_20.w = pHedgehog->currentLocation.w;
		local_20.y = pHedgehog->currentLocation.y + 0.5f;

		this->pOwner->addOnGenerator.Generate(&local_20);
		pCol = this->pOwner->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefff;
	}
	else {
		if (newState == 0x1d) {
			pHedgehog = this->pOwner;
			local_10.x = pHedgehog->rotationQuat.x;
			local_10.z = pHedgehog->rotationQuat.z;
			local_10.w = pHedgehog->rotationQuat.w;
			local_10.y = 0.0f;

			edF32Vector4NormalizeHard(&local_10, &local_10);

			local_10.y = 1.0f;
			local_10.x = -local_10.x;
			local_10.z = -local_10.z;
			edF32Vector4NormalizeHard(&local_10, &local_10);
			edF32Vector4ScaleHard(200.0f, &local_10, &local_10);
			this->pOwner->dynamic.speed = 0.0f;
			pHedgehog = this->pOwner;
			edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack48, &local_10);
			v0 = pHedgehog->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(v0, v0, &eStack48);
			fVar3 = edF32Vector4GetDistHard(pHedgehog->dynamicExt.aImpulseVelocities);
			pHedgehog->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar3;
		}
		else {
			if (newState != 0x17) {
				if (newState == 0x12) {
					uVar2 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
					CScene::_pinstance->field_0x38 = uVar2;
					this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper((static_cast<float>((uint)(uVar2 >> 0x10) & 0x7fff) * 0.4f) / 32767.0f + 0.8f, 0);
				}
				else {
					if (newState == 0x1f) {
						pCol = this->pOwner->pCollisionData;
						pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefff;
						pHedgehog = this->pOwner;
						pHedgehog->flags = pHedgehog->flags & 0xffffff7f;
						pHedgehog->flags = pHedgehog->flags | 0x20;
						pHedgehog->EvaluateDisplayState();
						pHedgehog = this->pOwner;
						pHedgehog->flags = pHedgehog->flags & 0xfffffffd;
						pHedgehog->flags = pHedgehog->flags | 1;
					}
					else {
						if (newState == 0xe) {
							pCol = this->pOwner->pCollisionData;
							pCol->flags_0x0 = pCol->flags_0x0 & 0xffffefff;
						}
						else {
							if ((newState == 0xb) || (newState == 10)) {
								pCol = this->pOwner->pCollisionData;
								pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7ffff;
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CBehaviourHedgehog::TermState(int oldState, int newState)
{
	CCollision* pCol;
	CActorHedgehog* pHedgehog;

	if (oldState != 0x17) {
		if (oldState == 0x12) {
			this->pOwner->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
		}
		else {
			if (oldState == 0x1f) {
				pCol = this->pOwner->pCollisionData;
				pCol->flags_0x0 = pCol->flags_0x0 | 0x1000;
				pHedgehog = this->pOwner;
				pHedgehog->flags = pHedgehog->flags & 0xffffff5f;
				pHedgehog->EvaluateDisplayState();
				this->pOwner->flags = this->pOwner->flags & 0xfffffffc;
			}
			else {
				if ((oldState == 0xe) || (oldState == 0x1e)) {
					pCol = this->pOwner->pCollisionData;
					pCol->flags_0x0 = pCol->flags_0x0 | 0x1000;
				}
				else {
					if ((oldState == 0xb) || (oldState == 10)) {
						pCol = this->pOwner->pCollisionData;
						pCol->flags_0x0 = pCol->flags_0x0 | 0x80000;
					}
				}
			}
		}
	}

	return;
}

int CBehaviourHedgehog::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int result;
	long lVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	_msg_hit_param hitParam;
	CActorHedgehog* pHedgehog;

	if (msg == 3) {
		lVar1 = HasArmor();
		if (lVar1 == 0) {
			this->pOwner->SetState(0x1e, -1);
		}
		else {
			pHedgehog = this->pOwner;
			if (pHedgehog->actorState != 0xb) {
				pHedgehog->SetState(10, -1);
			}
		}
		result = 1;
	}
	else {
		if (msg == MESSAGE_KICKED) {
			_msg_hit_param* pHitParam = (_msg_hit_param*)pMsgParam;
			if ((pHitParam->projectileType == 10) || (pHitParam->projectileType == 4)) {
				this->pOwner->field_0x350 = this->pOwner->field_0x350 | 0x10;
				pHedgehog = this->pOwner;
				pHedgehog->field_0x390 = pHitParam->field_0x20;

				return 1;
			}
		}
		else {
			if (msg == 0x1c) {
				pHedgehog = this->pOwner;

				if (pSender == pHedgehog->field_0x354) {
					pHedgehog->field_0x350 = pHedgehog->field_0x350 | 1;
				}

				if ((pSender->typeID == 0x29) && (this->pOwner->actorState == 0x17)) {
					hitParam.projectileType = 0;
					this->pOwner->DoMessage(pSender, MESSAGE_KICKED, &hitParam);
				}

				return 1;
			}
		}

		result = 0;
	}

	return result;
}

edF32VECTOR4* CBehaviourHedgehog::GetComeBackPosition()
{
	float fVar1;
	float fVar2;
	float fVar3;
	edF32MATRIX4 eStack64;
	CActorHedgehog* pHedgehog;
	CActor* pTied;

	pHedgehog = this->pOwner;
	pTied = pHedgehog->pTiedActor;
	if (pTied == (CActor*)0x0) {
		this->comeBackPosition = pHedgehog->baseLocation;
	}
	else {
		pTied->SV_ComputeDiffMatrixFromInit(&eStack64);
		edF32Matrix4MulF32Vector4Hard(&this->comeBackPosition, &eStack64, &this->pOwner->baseLocation);
	}

	return &this->comeBackPosition;
}

void CBehaviourHedgehogWatchDog::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHedgehogWatchDog::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorHedgehog*>(pOwner);
	return;
}

void CBehaviourHedgehogWatchDog::Manage()
{
	this->pOwner->BehaviourWatchDog_Manage(this);

	return;
}

void CBehaviourHedgehogWatchDog::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourHedgehog::Begin(pOwner, newState, newAnimationType);
	return;
}

bool CBehaviourHedgehogWatchDog::HasArmor()
{
	return false;
}

void CBehaviourHedgehogGuardArea::Create(ByteCode* pByteCode)
{
	this->pathFollowReader.Create(pByteCode);
	return;
}

void CBehaviourHedgehogGuardArea::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorHedgehog*>(pOwner);
	this->pathFollowReader.Init();
	this->pathFollowReader.Reset();
	return;
}

void CBehaviourHedgehogGuardArea::Manage()
{
	this->pOwner->BehaviourGuardArea_Manage(this);

	return;
}

void CBehaviourHedgehogGuardArea::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourHedgehog::Begin(pOwner, newState, newAnimationType);
	return;
}

bool CBehaviourHedgehogGuardArea::HasArmor()
{
	return false;
}

edF32VECTOR4* CBehaviourHedgehogGuardArea::GetComeBackPosition()
{
	return this->pathFollowReader.GetWayPoint();
}

void CBehaviourHedgehogWatchDogArmor::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHedgehogWatchDogArmor::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorHedgehog*>(pOwner);
	return;
}

void CBehaviourHedgehogWatchDogArmor::Manage()
{
	IMPLEMENTATION_GUARD();
	return;
}

void CBehaviourHedgehogWatchDogArmor::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourHedgehog::Begin(pOwner, newState, newAnimationType);
	return;
}

bool CBehaviourHedgehogWatchDogArmor::HasArmor()
{
	return true;
}

void CBehaviourHedgehogGuardAreaArmor::Create(ByteCode* pByteCode)
{
	this->pathFollowReader.Create(pByteCode);
	return;
}

void CBehaviourHedgehogGuardAreaArmor::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorHedgehog*>(pOwner);
	this->pathFollowReader.Init();
	this->pathFollowReader.Reset();
	return;
}

void CBehaviourHedgehogGuardAreaArmor::Manage()
{
	IMPLEMENTATION_GUARD();
	return;
}

void CBehaviourHedgehogGuardAreaArmor::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourHedgehog::Begin(pOwner, newState, newAnimationType);
	return;
}

bool CBehaviourHedgehogGuardAreaArmor::HasArmor()
{
	return true;
}
