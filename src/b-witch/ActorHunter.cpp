#include "ActorHunter.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "EventManager.h"
#include "ActorHero.h"

void CActorHunter::Create(ByteCode* pByteCode)
{
	CActorAutonomous::Create(pByteCode);

	this->boneId = pByteCode->GetU32();
	this->field_0x354 = pByteCode->GetU32();
	(this->actorRef).index = pByteCode->GetS32();
	this->field_0x3a4 = pByteCode->GetF32();
	this->field_0x350 = pByteCode->GetF32();

	return;
}

void CActorHunter::Init()
{
	CActorAutonomous::Init();

	this->actorRef.Init();
	this->pAnimationController->RegisterBone(this->boneId);
	this->animLength = this->pAnimationController->GetAnimLength(0x11, 2);
	GetLifeInterfaceOther()->field_0x10 = this;
	this->field_0x370 = 0;
	this->field_0x35c = 0;

	return;
}

void CActorHunter::Reset()
{
	CActorAutonomous::Reset();

	this->field_0x370 = 0;
	this->field_0x35c = 0;

	return;
}

CBehaviour* CActorHunter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == HUNTER_BEHAVIOUR_WATCH) {
		pBehaviour = &this->behaviourWatch;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorHunter::_gStateCfg_HTR[5] = {
	StateConfig(0xd, 0x140),
	StateConfig(0xd, 0x40),
	StateConfig(0xc, 0x40),
	StateConfig(0xd, 0x40),
	StateConfig(0xf, 0x40), 
};

StateConfig* CActorHunter::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActorMovable::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 5);
		pStateConfig = _gStateCfg_HTR + state + -5;
	}

	return pStateConfig;
}

int CActorHunter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CLifeInterface* pCVar1;
	int result;
	float fVar2;

	if (msg == 0x36) {
		if (pMsgParam == static_cast<void*>(0x0)) {
			this->field_0x35c = this->field_0x35c & 0xfffffffe;
		}
		else {
			this->field_0x35c = this->field_0x35c | 1;
		}

		result = 1;
	}
	else {
		if (msg == 0x3a) {
			result = this->actorState;
			if ((result == 8) || (result == 10)) {
				SetState(9, -1);
			}

			result = 1;
		}
		else {
			if (msg == 2) {
				result = this->actorState;
				if (result == 8) {
					SetState(10, -1);
				}
				else {
					if (result == 10) {
						RestartCurAnim();
						LifeDecrease(2.0f);

						fVar2 = GetLifeInterface()->GetValue();
						if (fVar2 <= 0.0f) {
							DoMessage(this->field_0x364, (ACTOR_MESSAGE)3, 0);
						}
					}
				}

				result = 1;
			}
			else {
				if (msg == 0x44) {
					SetState(8, -1);
					result = 1;
				}
				else {
					result = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}

	return result;
}


void CActorHunter::SetupCollisionMatrix()
{
	edF32MATRIX4 matrix = gF32Matrix4Unit;
	pCollisionData->flags_0x0 &= 0xfffffff4;
	SV_GetBoneWorldPosition(boneId, &matrix.rowT);
	pCollisionData->CheckCollisions_UpdateCollisionMatrix(this, &matrix, (CActorsTable*)0x0, (CActor*)0x0, 1);

	return;
}

void CBehaviourHunter::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHunter::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorHunter*>(pOwner);

	return;
}

int CBehaviourHunter::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourHunter::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourHunterWatch::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourHunterWatch::Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	CActorFighter* pCVar4;
	ed_zone_3d* pZone;
	int iVar5;
	edF32MATRIX4 local_150;
	edF32MATRIX4 local_110;
	edF32MATRIX4 local_d0;
	edF32MATRIX4 local_90;
	edF32MATRIX4 local_50;
	CCollision* pCol;
	CEventManager* pEventManager;
	CActorHunter* pHunter;

	pHunter = this->pOwner;
	iVar5 = pHunter->actorState;
	if (iVar5 == 10) {
		pHunter->SetupCollisionMatrix();

		if (pHunter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pHunter->SetState(8, -1);
		}
	}
	else {
		if (iVar5 == 7) {
			pHunter->SetupCollisionMatrix();

			if (pHunter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				pHunter->DoMessage(pHunter->actorRef.Get(), MESSAGE_ACTIVATE, 0);
				pHunter->DoMessage(pHunter->field_0x364, MESSAGE_DEACTIVATE, 0);
				pHunter->SetState(6, -1);
			}
		}
		else {
			if (iVar5 == 9) {
				pHunter->SetupCollisionMatrix();

				if (pHunter->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					pHunter->SetState(6, -1);
				}
			}
			else {
				if (iVar5 == 8) {
					pHunter->SetupCollisionMatrix();

					if (pHunter->field_0x3a4 < pHunter->timeInAir) {
						pHunter->SetState(9, -1);
					}
				}
				else {
					if (iVar5 == 6) {
						pHunter->SetupCollisionMatrix();

						pEventManager = CScene::ptable.g_EventManager_006f5080;
						if ((pHunter->field_0x364 != static_cast<CActor*>(0x0)) && (pHunter->field_0x364[2].rotationQuat.z == 1.401298e-45f)) {
							pZone = static_cast<ed_zone_3d*>(0x0);
							pCVar4 = CActorHero::_gThis;
							if (pHunter->field_0x354 != 0xffffffff) {
								pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pHunter->field_0x354);
							}

							iVar5 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pCVar4->currentLocation, 0);
							if (iVar5 != 2) {
								pHunter->DoMessage((pHunter->actorRef).Get(), (ACTOR_MESSAGE)0x28, &CActorHero::_gThis->currentLocation);
								if (pHunter->field_0x370 == '\0') {
									pHunter->field_0x370 = 1;
									pHunter->SetState(7, -1);
								}
								else {
									if ((pHunter->field_0x350 - pHunter->animLength < pHunter->timeInAir) && ((pHunter->field_0x35c & 1) == 0)) {
										pHunter->field_0x370 = 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return;
}

void CBehaviourHunterWatch::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourHunter::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(6, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourHunterWatch::InitState(int newState)
{
	CCollision* pCVar1;
	CLifeInterface* pCVar2;
	float fVar3;
	CActorHunter* pHunter;

	pHunter = this->pOwner;
	if (newState == 9) {
		pHunter->DoMessage(pHunter->field_0x364, (ACTOR_MESSAGE)0x27, 0);
	}
	else {
		if (newState == 10) {
			pHunter->LifeDecrease(2.0f);
			fVar3 = pHunter->GetLifeInterface()->GetValue();
			if (fVar3 <= 0.0f) {
				pHunter->DoMessage(pHunter->field_0x364, (ACTOR_MESSAGE)3, 0);
			}
		}
		else {
			if (newState == 8) {
				pCVar1 = pHunter->pCollisionData;
				pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
			}
			else {
				if (newState == 7) {
					pHunter->DoMessage(pHunter->field_0x364, MESSAGE_ACTIVATE, 0);
				}
				else {
					if (newState == 6) {
						pCVar1 = pHunter->pCollisionData;
						pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffefff;
					}
				}
			}
		}
	}

	return;
}

void CBehaviourHunterWatch::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourHunterWatch::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}