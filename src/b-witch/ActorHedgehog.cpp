#include "ActorHedgehog.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"

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
