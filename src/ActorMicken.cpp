#include "ActorMicken.h"
#include "MemoryStream.h"
#include "TimeController.h"
#include "EdenLib/edCollision/sources/edCollisions.h"
#include "ActorHero.h"
#include "ActorManager.h"
#include "ActorFactory.h"
#include "MathOps.h"
#include "ActorHeroPrivate.h"
#include "InputManager.h"

#define MICKEN_BEHAVIOUR_EAT 4

#define MICKEN_EAT_STATE_STAND			0x6
#define MICKEN_EAT_STATE_WALK_TO_FRUIT	0x8
#define MICKEN_EAT_STATE_EAT			0x9
#define MICKEN_EAT_STATE_CHEW			0xa

CActorMicken::CActorMicken()
{
	this->field_0x360 = 0;
	this->field_0x364 = 0;
	this->followPathReader = CPathFollowReader();
}

void CActorMicken::Create(ByteCode* pByteCode)
{
	CBehaviour* pCVar5;
	float fVar6;

	CActorAutonomous::Create(pByteCode);

	(this->streamRefWayPoint).index = pByteCode->GetS32();
	(this->streamRefActor).index = pByteCode->GetS32();

	this->field_0x358 = pByteCode->GetU32();
	this->field_0x35c = pByteCode->GetF32();
	if (this->field_0x35c < 0.0f) {
		this->field_0x35c = 1.0f;
	}

	this->field_0x368 = pByteCode->GetS32();
	this->field_0x3d0 = -1;

	pCVar5 = CActor::GetBehaviour(5);
	if (pCVar5 != (CBehaviour*)0x0) {
		this->field_0x3d0 = 5;
	}

	pCVar5 = CActor::GetBehaviour(7);
	if (pCVar5 != (CBehaviour*)0x0) {
		this->field_0x3d0 = 7;
	}

	this->field_0x3d4 = -1;

	pCVar5 = CActor::GetBehaviour(6);
	if (pCVar5 != (CBehaviour*)0x0) {
		this->field_0x3d4 = 6;
	}

	pCVar5 = CActor::GetBehaviour(8);
	if (pCVar5 != (CBehaviour*)0x0) {
		this->field_0x3d4 = 8;
	}

	this->field_0x3fc = (this->scale.z + this->scale.x + this->scale.y) / 3.0f;

	this->dynamic.weightB = this->field_0x3fc * this->dynamic.weightB;

	fVar6 = this->dynamicExt.field_0x8 / this->field_0x3fc;
	this->dynamicExt.field_0x8 = fVar6;
	if (0.95f < fVar6) {
		this->dynamicExt.field_0x8 = 0.95f;
	}

	fVar6 = this->dynamicExt.field_0x4 / this->field_0x3fc;
	this->dynamicExt.field_0x4 = fVar6;
	if (0.95f < fVar6) {
		this->dynamicExt.field_0x4 = 0.95f;
	}

	this->field_0x404 = 0;
	this->field_0x408 = 0;
	this->field_0x40c = 0;
	return;
}

void CActorMicken::Init()
{
	CShadow* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	CActorAutonomous::Init();
	streamRefWayPoint.Init();
	streamRefActor.Init();
	this->field_0x3d8 = 0;
	this->field_0x3dc = (CActorMovable*)0x0;
	this->field_0x3e0 = (CActorMovable*)0x0;
	this->pNearestFruit = (CActor*)0x0;
	this->flags_0x3e8 = this->flags_0x3e8 | 1;
	this->flags_0x3e8 = this->flags_0x3e8 & 0xfffffffd;
	this->field_0x370 = this->baseLocation;
	this->field_0x3f8 = 0.0f;
	this->field_0x400 = 0x11;
	this->field_0x410 = 0;

	pCVar1 = this->pShadow;
	if (pCVar1 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		fVar3 = this->scale.z;
		fVar2 = this->scale.x;

		if (fVar3 <= fVar2) {
			fVar2 = fVar3;
		}

		pCVar1->field_0x48 = fVar2 * 0.75f;
		pCVar1->field_0x50 = fVar2 * 0.75f;)
	}

	this->field_0xf0 = 3.0f;
}

void CActorMicken::Reset()
{
	int iVar1;
	CBehaviour* pBehaviourEat;
	float fVar3;
	float fVar4;
	float fVar5;

	CActorAutonomous::Reset();

	this->field_0x3d8 = 0;
	this->field_0x3dc = (CActorMovable*)0x0;
	this->field_0x3e0 = (CActorMovable*)0x0;
	this->pNearestFruit = (CActor*)0x0;
	this->flags_0x3e8 = this->flags_0x3e8 | 1;
	this->flags_0x3e8 = this->flags_0x3e8 & 0xfffffffd;

	this->field_0x370 = this->baseLocation;
	this->field_0x3f8 = 0.0f;
	this->field_0x400 = 0x11;
	this->field_0x410 = 0;

	pBehaviourEat = GetBehaviour(MICKEN_BEHAVIOUR_EAT);
	if (pBehaviourEat != (CBehaviour*)0x0) {
		IMPLEMENTATION_GUARD(
		if ((this->followPathReader).pPathFollow == (CPathFollow*)0x0) {
			*(uint*)(*(int*)&this->field_0x418 + 1000) = *(uint*)(*(int*)&this->field_0x418 + 1000) | 0x10;
		}
		else {
			CPathFollowReader::Reset(&this->followPathReader);
			*(uint*)(*(int*)&this->field_0x418 + 1000) = *(uint*)(*(int*)&this->field_0x418 + 1000) & 0xffffffef;
		}
		iVar1 = *(int*)&this->field_0x418;
		if ((*(uint*)&this->field_0x424 & 4) == 0) {
			*(uint*)(iVar1 + 1000) = *(uint*)(iVar1 + 1000) & 0xfffffffb;
		}
		else {
			*(uint*)(iVar1 + 1000) = *(uint*)(iVar1 + 1000) | 4;
		}
		iVar1 = *(int*)&this->field_0x418;
		if ((*(uint*)&this->field_0x424 & 8) == 0) {
			*(uint*)(iVar1 + 1000) = *(uint*)(iVar1 + 1000) & 0xfffffff7;
		}
		else {
			*(uint*)(iVar1 + 1000) = *(uint*)(iVar1 + 1000) | 8;
		})
	}

	return;
}

CBehaviour* CActorMicken::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 8) {
		pBehaviour = new CBehaviourMickenKicked;
	}
	else {
		if (behaviourType == 6) {
			pBehaviour = new CBehaviourMickenKicked;
		}
		else {
			if (behaviourType == 7) {
				pBehaviour = new CBehaviourMickenSquashed;
			}
			else {
				if (behaviourType == 5) {
					pBehaviour = new CBehaviourMickenSquashed;
				}
				else {
					if (behaviourType == MICKEN_BEHAVIOUR_EAT) {
						pBehaviour = &this->behaviourMickenEat;
					}
					else {
						pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorMicken::_gStateCfg_MCK[NUM_MCK_STATES] =
{
	StateConfig(0x0, 0x5900),
	StateConfig(0x0, 0x9000),
	StateConfig(0x7, 0xD900),
	StateConfig(0xD, 0x1900),
	StateConfig(0xE, 0x1900),
	StateConfig(0x7, 0xD900),
	StateConfig(0x10, 0x1),
	StateConfig(0x14, 0x1),
	StateConfig(0x0, 0x1),
	StateConfig(0x0, 0x110),
	StateConfig(0x10, 0x1900),
	StateConfig(0x7, 0xD900),
	StateConfig(0x7, 0xD900),
	StateConfig(0x0, 0x5900),
	StateConfig(0x13, 0x1900),
	StateConfig(0xC, 0x100),
	StateConfig(0x0, 0x100),
	StateConfig(0xF, 0x8000),
	StateConfig(0x0, 0x9000),
	StateConfig(0x10, 0x9000),
	StateConfig(0x0, 0x9800),
	StateConfig(0x0, 0xB100),
	StateConfig(0xF, 0x8100),
	StateConfig(0x9, 0x9900),
	StateConfig(0x0, 0x9900),
	StateConfig(0xB, 0x1900),
};

StateConfig* CActorMicken::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state - 6) < NUM_MCK_STATES);
		pStateConfig = _gStateCfg_MCK + state + -6;
	}

	return pStateConfig;
}

int CActorMicken::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	byte bVar1;
	float fVar2;
	float fVar3;
	bool bVar4;
	uint uVar5;
	StateConfig* pSVar6;
	Timer* pTVar7;
	CActorWindState* pCVar8;
	int iVar9;
	edF32VECTOR4* peVar10;
	float fVar11;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (msg == 1) {
		return 1;
	}
	if (msg == MESSAGE_SOCCER_START) {
		MessageSoccerParamsDetailed* pSoccerParams = reinterpret_cast<MessageSoccerParamsDetailed*>(pMsgParam);
		if (this->field_0x3d4 != -1) {
			iVar9 = this->actorState;
			if (iVar9 == -1) {
				uVar5 = 0;
			}
			else {
				pSVar6 = this->GetStateCfg(iVar9);
				uVar5 = pSVar6->flags_0x4;
			}

			if ((uVar5 & 0x1000) != 0) {		
				if (pSoccerParams->field_0x0 == 0) {
					this->rotationEuler.y = pSoccerParams->rotation;
					SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
					this->dynamic.rotationQuat = this->rotationQuat;

					this->dynamic.speed = pSoccerParams->speed;

					if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
						SetBehaviour(MICKEN_BEHAVIOUR_EAT, 7, -1);
					}
					else {
						if (this->currentAnimType == 10) {
							SetBehaviour(this->field_0x3d4, 0x1e, -1);
						}
						else {
							SetBehaviour(this->field_0x3d4, 0x1d, -1);
						}
					}
				}
				else {
					bVar1 = this->state_0x10;
					if (bVar1 != 0) {
						this->field_0x410 = (int)(char)bVar1;
						this->flags = this->flags | 2;
						this->flags = this->flags & 0xfffffffe;
						this->flags = this->flags | 0x80;
						this->flags = this->flags & 0xffffffdf;

						EvaluateDisplayState();
					}

					this->field_0x3e0 = reinterpret_cast<CActorMovable*>(pSender);
					SetBehaviour(this->field_0x3d4, 0x1b, -1);
				}
				return 1;
			}
		}

		if (((this->field_0x3d4 != -1) && (this->actorState == 0x1c)) && (pSoccerParams->field_0x0 == 0)) {
			pTVar7 = GetTimer();
			this->field_0x3f8 = this->field_0x3f8 + pTVar7->cutsceneDeltaTime;

			return 1;
		}

		return 0;
	}

	if (msg == 0x16) {
		IMPLEMENTATION_GUARD(
		edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x10), &eStack64, (edF32VECTOR4*)pMsgParam);
		peVar10 = this->dynamicExt.aImpulseVelocities + 2;
		edF32Vector4AddHard(peVar10, peVar10, &eStack64);
		fVar11 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities + 2);
		this->dynamicExt.aImpulseVelocityMagnitudes[2] = fVar11;
		pCVar8 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
		if (pCVar8 == (CActorWindState*)0x0) {
			fVar11 = 0.0;
		}
		else {
			pCVar8 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
			fVar11 = pCVar8->field_0x38;
		}
		if (fVar11 < 0.001) {
			fVar11 = *(float*)((int)pMsgParam + 0x10);
			pCVar8 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
			if (pCVar8 == (CActorWindState*)0x0) {
				bVar4 = false;
			}
			else {
				pCVar8 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
				iVar9 = pCVar8->field_0x0;
				pCVar8 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
				if (iVar9 == pCVar8->field_0x4) {
					bVar4 = true;
				}
				else {
					pCVar8 = (*(this->pVTable)->GetWindState)((CActorAutonomous*)this);
					if (pCVar8->field_0x4 == 0) {
						bVar4 = false;
					}
					else {
						bVar4 = true;
						if (edFCosinus[1820] <= ABS(*(float*)((int)pMsgParam + 4))) {
							bVar4 = false;
						}
					}
				}
			}
			if ((!bVar4) || (bVar4 = true, fVar11 <= 2.0)) {
				bVar4 = false;
			}
			if (!bVar4) {
				this->base.dynamic.speed = this->base.dynamic.speed * 0.4;
			}
		})

		SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0x10, -1);

		return 1;
	}

	if (msg == 2) {
		IMPLEMENTATION_GUARD(
		if (this->field_0x3d4 == -1) {
			iVar9 = this->actorState;
			if (iVar9 == -1) {
				uVar5 = 0;
			}
			else {
				pSVar6 = this->GetStateCfg(iVar9);
				uVar5 = pSVar6->flags_0x4;
			}
			if ((uVar5 & 1) == 0) goto LAB_00151118;
		}
		/* WARNING: Load size is inaccurate */
		iVar9 = *pMsgParam;
		if ((iVar9 == 8) || (iVar9 == 7)) {
			edF32Vector4NormalizeHard(&local_20, (edF32VECTOR4*)((int)pMsgParam + 0x20));
			local_30.y = local_20.y;
			local_30.z = local_20.z;
			local_30.w = local_20.w;
			local_20.y = 0.0;
			fVar11 = edF32Vector4NormalizeHard(&local_20, &local_20);
			if (0.001 < fVar11) {
				this->rotationQuat.x = local_20.x;
				this->rotationQuat.y = local_20.y;
				this->rotationQuat.z = local_20.z;
				this->rotationQuat.w = local_20.w;
				if (local_30.y < 0.5) {
					edF32Vector4ScaleHard(edFCosinus[683], &local_30, &local_20);
					local_30.y = 0.5;
				}
			}
			pTVar7 = GetTimer();
			edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x30) / pTVar7->cutsceneDeltaTime, &local_30, &local_30);
			pTVar7 = GetTimer();
			edF32Vector4ScaleHard(0.02 / pTVar7->cutsceneDeltaTime, &eStack96, &local_30);
			peVar10 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(peVar10, peVar10, &eStack96);
			fVar11 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar11;
			(*(this->pVTable)->SetBehaviour)((CActor*)this, this->field_0x3d4, 0x17, -1);
			return 1;
		}

		if (iVar9 == 10) {
			IMPLEMENTATION_GUARD(
			edF32Vector4ScaleHard(*(float*)((int)pMsgParam + 0x30), &local_10, (edF32VECTOR4*)((int)pMsgParam + 0x20));
			pTVar7 = GetTimer();
			edF32Vector4ScaleHard(0.02 / pTVar7->cutsceneDeltaTime, &eStack80, &local_10);
			peVar10 = this->dynamicExt.aImpulseVelocities;
			edF32Vector4AddHard(peVar10, peVar10, &eStack80);
			fVar11 = edF32Vector4GetDistHard(this->dynamicExt.aImpulseVelocities);
			this->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar11;
			local_10.y = 0.0;
			fVar11 = edF32Vector4SafeNormalize1Hard(&local_10, &local_10);
			if (0.001 < fVar11) {
				this->rotationQuat.x = local_10.x;
				this->rotationQuat.y = local_10.y;
				this->rotationQuat.z = local_10.z;
				this->rotationQuat.w = local_10.w;
			}
			(*(this->pVTable)->SetBehaviour)((CActor*)this, this->field_0x3d4, 0x17, -1);)
			return 1;
		})
	}
	else {
		if (msg == 0x22) {
			if (this->field_0x3d4 != -1) {
				iVar9 = this->actorState;
				if (iVar9 == -1) {
					uVar5 = 0;
				}
				else {
					pSVar6 = this->GetStateCfg(iVar9);
					uVar5 = pSVar6->flags_0x4;
				}

				if ((uVar5 & 0x1000) != 0) {
					this->field_0x3dc = static_cast<CActorMovable*>(pSender);
					SetBehaviour(this->field_0x3d4, 0x1c, -1);
					return 1;
				}
			}
			return 0;
		}
		if (msg == MESSAGE_GET_ACTION) {
			if (this->field_0x3d4 != -1) {
				iVar9 = this->actorState;

				if (iVar9 == -1) {
					uVar5 = 0;
				}
				else {
					pSVar6 = this->GetStateCfg(iVar9);
					uVar5 = pSVar6->flags_0x4;
				}

				if ((uVar5 & 0x1000) != 0) {
					iVar9 = this->actorState;
					uVar5 = 0;

					if (iVar9 != -1) {
						pSVar6 = this->GetStateCfg(iVar9);
						uVar5 = pSVar6->flags_0x4;
					}
					if ((uVar5 & 0x2000) == 0) {
						fVar11 = pSender->SV_GetCosAngle2D(&this->currentLocation);
						if (fVar11 <= 0.64265704f) {
							return 0;
						}

						fVar11 = (pSender->currentLocation).x - this->currentLocation.x;
						fVar2 = (pSender->currentLocation).y - this->currentLocation.y;
						fVar3 = (pSender->currentLocation).z - this->currentLocation.z;

						if (1.96f <= fVar11 * fVar11 + fVar2 * fVar2 + fVar3 * fVar3) {
							return 0;
						}
					}

					return 2;
				}
			}

			return 0;
		}

		if (msg == 0x1f) {
			if (pMsgParam == (void*)0x0) {
				this->flags_0x3e8 = this->flags_0x3e8 & 0xfffffffe;
			}
			else {
				this->flags_0x3e8 = this->flags_0x3e8 | 1;
			}

			return 1;
		}

		if (msg == 3) {
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xc, -1);
			return 1;
		}
	}
LAB_00151118:
	iVar9 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
	return iVar9;
}

int CActorMicken::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	uint uVar1;
	StateConfig* pSVar2;
	uint uVar3;
	int iVar4;

	uVar1 = param_5[1];
	if (*param_5 == 1) {
		if (param_3 == 2) {
			IMPLEMENTATION_GUARD(
			iVar4 = this->actorState;
			if (iVar4 == -1) {
				uVar3 = 0;
			}
			else {
				pSVar2 = this->GetStateCfg(iVar4);
				uVar3 = pSVar2->flags_0x4 & 1;
			}
			if (uVar3 == 0) {
				if (uVar1 == 0) {
					SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xc, -1);
				}
				else {
					SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xd, -1);
				}
			})
		}
		iVar4 = 1;
	}
	else {
		iVar4 = CActor::InterpretEvent(pEventMessage, param_3, param_4, param_5);
	}

	return iVar4;
}

void edF32Vector4ScaleV4Hard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar1 = v1->y;
	fVar2 = v1->z;
	fVar3 = v1->w;
	fVar4 = v2->y;
	fVar5 = v2->z;
	fVar6 = v2->w;
	v0->x = v2->x * v1->x;
	v0->y = fVar4 * fVar1;
	v0->z = fVar5 * fVar2;
	v0->w = fVar6 * fVar3;
	return;
}

void CActorMicken::StoreCollisionSphere()
{
	edColPRIM_OBJECT* peVar1;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	peVar1 = this->pCollisionData->pObbPrim;
	edF32Vector4ScaleV4Hard(&eStack32, &peVar1->field_0xb0, &scale);
	edF32Vector4ScaleV4Hard(&eStack16, &peVar1->field_0x90, &scale);

	ChangeCollisionSphere(0.0f, &eStack16, &eStack32);
	CActorAutonomous::StoreCollisionSphere();
	return;
}

struct MickenParams
{
	float distance;
	CActor* pInActor;
	CActor* pOutActor;
};

void CActorMicken::StateMickenStand(CBehaviourMickenEat* pBehaviour)
{
	CCollision* pCollisionRef;
	CActor* pTiedActorRef;
	float fVar3;
	float fVar4;
	bool bVar5;
	int nextState;
	Timer* pTimer;

	pCollisionRef = this->pCollisionData;

	bVar5 = PlayWaitingAnimation(0x40800000, ((float)rand() / 2.147484e+09) * 0.4f + 0.8f, this->field_0x400, -1, 1);
	if ((bVar5 != false) && (this->field_0x400 = this->field_0x400 + 1, this->field_0x400 == 0x13)) {
		this->field_0x400 = 0x11;
	}

	this->dynamic.speed = 0.0f;
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if ((pCollisionRef->flags_0x4 & 2) == 0) {
		if (0.2f < this->timeInAir) {
			this->SetState(7, -1);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	if (((this->flags_0x3e8 & 0x10) == 0) && (bVar5 = pBehaviour->pathFollowReader.AtGoal((pBehaviour->pathFollowReader).splinePointIndex, (pBehaviour->pathFollowReader).field_0xc), bVar5 == false)) {
		this->SetState(0x12, -1);
		return;
	}

	pTimer = GetTimer();
	if (pTimer->scaledTotalTime - this->field_0x3f0 <= 2.0f) {
		return;
	}

	pTiedActorRef = this->pTiedActor;
	if (pTiedActorRef != (CActor*)0x0) {
		fVar3 = (pTiedActorRef->currentLocation).x - this->currentLocation.x;
		fVar4 = (pTiedActorRef->currentLocation).z - this->currentLocation.z;
		this->field_0x3ec = sqrtf(fVar3 * fVar3 + 0.0f + fVar4 * fVar4);
		nextState = 0x11;

		if (0.3f < this->field_0x3ec) goto LAB_0014f808;
	}

	nextState = MICKEN_EAT_STATE_STAND;
LAB_0014f808:
	if (nextState != this->actorState) {
		this->SetState(nextState, -1);
	}

	return;
}

void CActorMicken::StateMickenRollOnGround(CBehaviourMickenKicked* pBehaviour, int param_3, int param_4)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;

	pCVar1 = this->pCollisionData;

	SV_MOV_UpdateSpeedIntensity(0.0, 8.0f);

	ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

	if ((pCVar1->flags_0x4 & 2) == 0) {
		if (0.2 < this->timeInAir) {
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, 7, -1);
			return;
		}
	}
	else {
		this->timeInAir = 0.0f;
	}

	if ((param_4 != 0) || (2.0f <= this->dynamic.linearAcceleration)) {
		if (param_4 == 0) {
			return;
		}

		pCVar2 = this->pAnimationController;
		peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
		bVar4 = false;

		if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
			if (peVar3->animPlayState == 0) {
				bVar4 = false;
			}
			else {
				bVar4 = (peVar3->field_0xcc & 2) != 0;
			}
		}

		if (!bVar4) {
			return;
		}
	}

	if (param_3 == -1) {
		if ((this->flags_0x3e8 & 4) == 0) {
			this->flags_0x3e8 = this->flags_0x3e8 | 0x10;
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, -1, -1);
		}
		else {
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xb, -1);
		}
	}
	else {
		SetState(param_3, -1);
	}
	return;
}

void CActorMicken::StateMickenKickImpactFly(CBehaviourMicken* pBehaviour, int param_3)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pAnimationController;
	ManageDyn(4.0f, 0x41920, (CActorsTable*)0x0);

	if (((pCVar1->flags_0x4 & 2) == 0) ||
		(1.0f <= fabs(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y))) {
		if (param_3 != 0) {
			peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
			if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
				bVar4 = false;
				if (peVar3->animPlayState != 0) {
					bVar4 = (peVar3->field_0xcc & 2) != 0;
				}
			}
			else {
				bVar4 = false;
			}

			if (bVar4) {
				if (4.0f < this->dynamic.horizontalLinearAcceleration) {
					SetState(0x19, -1);
				}
				else {
					SetState(0x18, -1);
				}
			}
		}
	}
	else {
		if ((this->flags_0x3e8 & 4) == 0) {
			this->flags_0x3e8 = this->flags_0x3e8 | 0x10;
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, -1, -1);
		}
		else {
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xb, -1);
		}
	}

	return;
}

void CActorMicken::StateMickenKickFly(CBehaviourMicken* pBehaviour, int param_3)
{
	CCollision* pCVar1;
	CAnimation* pCVar2;
	CActor* pReceiver;
	edAnmLayer* peVar3;
	bool bVar4;
	int iVar5;
	CActorsTable local_110;
	undefined4 local_4;

	pCVar1 = this->pCollisionData;
	pCVar2 = this->pAnimationController;
	local_110.entryCount = 0;
	ManageDyn(4.0f, 0x41920, &local_110);

	if ((pCVar1->flags_0x4 & 2) != 0) {
		if (param_3 == 0) {
			if (fabs(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y) < 1.0f) {
				for (iVar5 = 0; iVar5 < local_110.entryCount; iVar5 = iVar5 + 1) {
					pReceiver = local_110.aEntries[iVar5];

					if ((pReceiver->typeID == 0x29) && (2.0f < this->currentLocation.y - (pReceiver->currentLocation).y)) {
						local_4 = 0;
						DoMessage(pReceiver, (ACTOR_MESSAGE)0x42, 0);
					}
				}

				if ((this->flags_0x3e8 & 4) != 0) {
					SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xb, -1);
					return;
				}

				this->flags_0x3e8 = this->flags_0x3e8 | 0x10;
				SetBehaviour(MICKEN_BEHAVIOUR_EAT, -1, -1);
				return;
			}
		}
		else {
			this->flags_0x3e8 = this->flags_0x3e8 | 2;
		}
	}

	if ((this->flags_0x3e8 & 2) != 0) {
		peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;

		if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
			bVar4 = false;

			if (peVar3->animPlayState != 0) {
				bVar4 = (peVar3->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar4 = false;
		}

		if (bVar4) {
			this->flags_0x3e8 = this->flags_0x3e8 & 0xfffffffd;
			SetState(0x1a, -1);
		}
	}

	return;
}

void CActorMicken::BehaviourMickenKicked_Manage(CBehaviourMickenKicked* pBehaviour)
{
	Timer* pTVar1;
	float fVar2;

	switch (this->actorState) {
	case 0x17:
		StateMickenKickImpactFly(pBehaviour, 1);
		break;
	case 0x18:
		StateMickenKickImpactFly(pBehaviour, 0);
		break;
	case 0x19:
		StateMickenKickFly(pBehaviour, 1);
		break;
	case 0x1a:
		StateMickenKickFly(pBehaviour, 0);
		break;
	case 0x1b:
		StateAutSoccer(4.0f, 1, 9, this->field_0x3e0);
		break;
	case 0x1c:
		StateAutSoccer(0.0f, 0, 0xf, this->field_0x3dc);

		if (0.0f < this->field_0x3f8) {
			pTVar1 = GetTimer();
			fVar2 = this->field_0x3f8 + pTVar1->cutsceneDeltaTime;
			this->field_0x3f8 = fVar2;

			if (0.06f < fVar2) {
				SetState(0x17, -1);
			}
		}
		break;
	case 0x1d:
		StateMickenRollOnGround(pBehaviour, 0x1f, 0);
		break;
	case 0x1e:
		StateMickenRollOnGround(pBehaviour, -1, 0);
		break;
	case 0x1f:
		StateMickenRollOnGround(pBehaviour, -1, 1);
	}
	return;
}

void gActorMicken_GetNearestFruit(CActor* pActor, void* pParams)
{
	float fVar1;
	float fVar2;

	MickenParams* pMickenParams = reinterpret_cast<MickenParams*>(pParams);

	if ((((pActor->flags & 0x8000) == 0) && ((CActorFactory::gClassProperties[pActor->typeID].flags & 8) != 0)) &&
		(fVar2 = (pMickenParams->pInActor->currentLocation).x - (pActor->currentLocation).x,
			fVar1 = (pMickenParams->pInActor->currentLocation).z - (pActor->currentLocation).z, fVar2 = fVar2 * fVar2 + fVar1 * fVar1
			, fVar2 < pMickenParams->distance)) {
		pMickenParams->pOutActor = pActor;
		pMickenParams->distance = fVar2;
	}

	return;
}

void CActorMicken::BehaviourMickenEat_Manage(CBehaviourMickenEat* pBehaviour)
{
	CActor* pCVar1;
	CCollision* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	bool bVar5;
	edF32VECTOR4* peVar6;
	int iVar7;
	StateConfig* pStateConfig;
	uint uVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 eStack32;
	float local_10;
	CActorMicken* local_c;
	CActor* local_8;

	pCVar1 = this->pTiedActor;
	switch (this->actorState) {
	case MICKEN_EAT_STATE_STAND:
		StateMickenStand(pBehaviour);
		break;
	case 7:
		IMPLEMENTATION_GUARD(
		pCVar2 = this->pCollisionData;
		(*(this->pVTable)->ManageDyn)(4.0, (CActorHero*)this, 0x129, (CActorsTable*)0x0);
		if ((pCVar2->flags_0x4 & 2) == 0) break;
		pCVar1 = this->pTiedActor;
		if (pCVar1 == (CActor*)0x0) {
		LAB_0014f970:
			iVar7 = MICKEN_EAT_STATE_STAND;
		}
		else {
			fVar10 = (pCVar1->currentLocation).x - this->currentLocation.x;
			fVar11 = (pCVar1->currentLocation).z - this->currentLocation.z;
			this->field_0x3ec = SQRT(fVar10 * fVar10 + 0.0 + fVar11 * fVar11);
			iVar7 = 0x11;
			if (this->field_0x3ec <= 0.3) goto LAB_0014f970;
		}
		this->SetState(iVar7, -1);)
		break;
	case MICKEN_EAT_STATE_WALK_TO_FRUIT:
		IMPLEMENTATION_GUARD(
		iVar7 = WalkToPos(0.3f, pBehaviour, &this->pNearestFruit->currentLocation, 0);
		if (iVar7 != 0) {
			this->SetState(MICKEN_EAT_STATE_EAT, -1);
		})
		break;
	case MICKEN_EAT_STATE_EAT:
		IMPLEMENTATION_GUARD(
			StateMickenEat(this, (CBehaviourMicken*)pBehaviour);)
		break;
	case MICKEN_EAT_STATE_CHEW:
		IMPLEMENTATION_GUARD(
			StateMickenChew(this, pBehaviour);)
		break;
	case 0xb:
		IMPLEMENTATION_GUARD(
		iVar7 = WalkToPos(0.3, this, (CBehaviourMicken*)pBehaviour, (float*)&this->field_0x370, 0);
		if (iVar7 != 0) {
			this->SetState(MICKEN_EAT_STATE_STAND, -1);
		})
		break;
	case 0xc:
		IMPLEMENTATION_GUARD(
		this->base.dynamic.speed = 0.0;
		(*(this->pVTable)->ManageDyn)(4.0, (CActorHero*)this, 0x129, (CActorsTable*)0x0);
		pCVar3 = this->pAnimationController;
		peVar4 = (pCVar3->anmBinMetaAnimator).base.aAnimData;
		if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
			bVar5 = false;
			if (peVar4->animPlayState != 0) {
				bVar5 = (peVar4->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar5 = false;
		}
		if (bVar5) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			CActor::EvaluateDisplayState((CActor*)this);
			this->dynamicExt.normalizedTranslation.x = 0.0;
			this->dynamicExt.normalizedTranslation.y = 0.0;
			this->dynamicExt.normalizedTranslation.z = 0.0;
			this->dynamicExt.normalizedTranslation.w = 0.0;
			this->dynamicExt.field_0x6c = 0.0;
			this->base.dynamic.speed = 0.0;
			this->SetState(0xe, -1);
		})
		break;
	case 0xd:
		IMPLEMENTATION_GUARD(
		this->base.dynamic.speed = 0.0;
		(*(this->pVTable)->ManageDyn)(4.0, (CActorHero*)this, 0x129, (CActorsTable*)0x0);
		pCVar3 = this->pAnimationController;
		peVar4 = (pCVar3->anmBinMetaAnimator).base.aAnimData;
		if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
			bVar5 = false;
			if (peVar4->animPlayState != 0) {
				bVar5 = (peVar4->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar5 = false;
		}
		if (bVar5) {
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			CActor::EvaluateDisplayState((CActor*)this);
			this->dynamicExt.normalizedTranslation.x = 0.0;
			this->dynamicExt.normalizedTranslation.y = 0.0;
			this->dynamicExt.normalizedTranslation.z = 0.0;
			this->dynamicExt.normalizedTranslation.w = 0.0;
			this->dynamicExt.field_0x6c = 0.0;
			this->base.dynamic.speed = 0.0;
			this->SetState(0xe, -1);
		})
		break;
	case 0xe:
		IMPLEMENTATION_GUARD(
		if (this->field_0x35c < this->timeInAir) {
			if ((this->streamRefWayPoint).pWayPoint == (CWayPoint*)0x0) {
				this->SetState(3, -1);
				this->flags = this->flags & 0xffffff7f;
				this->flags = this->flags | 0x20;
				CActor::EvaluateDisplayState((CActor*)this);
				this->flags = this->flags & 0xfffffffd;
				this->flags = this->flags | 1;
			}
			else {
				this->SetState(0xf, -1);
				this->flags = this->flags & 0xfffffffc;
				this->flags = this->flags & 0xffffff5f;
				CActor::EvaluateDisplayState((CActor*)this);
				this->field_0x410 = 0;
			}
		})
		break;
	case 0xf:
		IMPLEMENTATION_GUARD(
		ActorFunc_0014edc0((Actor*)this);)
		break;
	case 0x10:
		IMPLEMENTATION_GUARD(
		StateMickenRollInTheWind(this);)
		break;
	case 0x11:
		IMPLEMENTATION_GUARD(
		if (pCVar1 == (CActor*)0x0) {
			(*(this->pVTable)->ManageDyn)(4.0, (CActorHero*)this, 0x1002023b, (CActorsTable*)0x0);
			this->SetState(MICKEN_EAT_STATE_STAND, -1);
		}
		else {
			peVar6 = CActor::GetBottomPosition(pCVar1);
			iVar7 = WalkToPos(0.3, this, (CBehaviourMicken*)pBehaviour, (float*)peVar6, 1);
			if (iVar7 != 0) {
				this->SetState(MICKEN_EAT_STATE_STAND, -1);
			}
		})
		break;
	case 0x12:
		IMPLEMENTATION_GUARD(
		if (pCVar1 == (CActor*)0x0) {
			peVar6 = CPathFollowReader::GetWayPoint(&pBehaviour->pathFollowReader);
		}
		else {
			CActor::SV_ComputeDiffMatrixFromInit(pCVar1, &eStack96);
			peVar6 = CPathFollowReader::GetWayPoint(&pBehaviour->pathFollowReader);
			edF32Matrix4MulF32Vector4Hard(&eStack32, &eStack96, peVar6);
			peVar6 = &eStack32;
		}
		iVar7 = WalkToPos(0.3, this, (CBehaviourMicken*)pBehaviour, (float*)peVar6, 0);
		if (iVar7 != 0) {
			fVar10 = CAnimation::edAnmMetaAnimator::GetLayerAnimTime
			((edAnmMetaAnimator*)this->pAnimationController, 0, 0);
			if (fVar10 < 0.17) {
				CPathFollowReader::GetDelay(&pBehaviour->pathFollowReader);
				if (0.0 < fVar10) {
					this->SetState(0x13, -1);
				}
				else {
					bVar5 = CPathFollowReader::AtGoal
					(&pBehaviour->pathFollowReader, (pBehaviour->pathFollowReader).splinePointIndex,
						(pBehaviour->pathFollowReader).field_0xc);
					if (bVar5 == false) {
						CPathFollowReader::NextWayPoint(&pBehaviour->pathFollowReader);
					}
					else {
						this->SetState(MICKEN_EAT_STATE_STAND, -1);
					}
				}
			}
		})
		break;
	case 0x13:
		IMPLEMENTATION_GUARD(
		this->base.dynamic.speed = 0.0;
		this->dynamicExt.normalizedTranslation.x = 0.0;
		this->dynamicExt.normalizedTranslation.y = 0.0;
		this->dynamicExt.normalizedTranslation.z = 0.0;
		this->dynamicExt.normalizedTranslation.w = 0.0;
		this->dynamicExt.field_0x6c = 0.0;
		fVar10 = (float)(*(this->pVTable)->ManageDyn)
			(4.0, (CActorHero*)this, 0x100a023b, (CActorsTable*)0x0);
		fVar11 = this->timeInAir;
		CPathFollowReader::GetDelay(&pBehaviour->pathFollowReader);
		if (fVar10 < fVar11) {
			bVar5 = CPathFollowReader::AtGoal
			(&pBehaviour->pathFollowReader, (pBehaviour->pathFollowReader).splinePointIndex,
				(pBehaviour->pathFollowReader).field_0xc);
			if (bVar5 == false) {
				CPathFollowReader::NextWayPoint(&pBehaviour->pathFollowReader);
				this->SetState(0x12, -1);
			}
			else {
				this->SetState(MICKEN_EAT_STATE_STAND, -1);
			}
		})
		break;
	case 0x14:
		IMPLEMENTATION_GUARD(
		pCVar2 = this->pCollisionData;
		this->base.dynamic.speed = 0.0;
		this->dynamicExt.normalizedTranslation.x = 0.0;
		this->dynamicExt.normalizedTranslation.y = 0.0;
		this->dynamicExt.normalizedTranslation.z = 0.0;
		this->dynamicExt.normalizedTranslation.w = 0.0;
		this->dynamicExt.field_0x6c = 0.0;
		(*(this->pVTable)->ManageDyn)(4.0, (CActorHero*)this, 0x100a023b, (CActorsTable*)0x0);
		if ((pCVar2->flags_0x4 & 2) == 0) {
			if (0.2 < this->timeInAir) {
				this->SetState(7, -1);
				break;
			}
		}
		else {
			this->timeInAir = 0.0;
		}
		pCVar3 = this->pAnimationController;
		peVar4 = (pCVar3->anmBinMetaAnimator).base.aAnimData;
		if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
			bVar5 = false;
			if (peVar4->animPlayState != 0) {
				bVar5 = (peVar4->field_0xcc & 2) != 0;
			}
		}
		else {
			bVar5 = false;
		}
		if (bVar5) {
			this->SetState(MICKEN_EAT_STATE_STAND, -1);
		})
	}

	if ((pBehaviour->field_0x10 & 2) != 0) {
		iVar7 = this->actorState;
		uVar9 = 0;
		if (iVar7 != -1) {
			pStateConfig = GetStateCfg(iVar7);
			uVar9 = pStateConfig->flags_0x4;
		}

		if ((uVar9 & 0x4000) != 0) {
			local_70.w = pBehaviour->field_0x8;
			pCVar1 = this->pNearestFruit;

			MickenParams local_10;
			local_10.distance = local_70.w * local_70.w;
			local_10.pOutActor = (CActor*)0x0;
			local_70.xyz = this->currentLocation.xyz;
			local_10.pInActor = this;
			(CScene::ptable.g_ActorManager_004516a4)->cluster.ApplyCallbackToActorsIntersectingSphere(&local_70, gActorMicken_GetNearestFruit, &local_10);
			this->pNearestFruit = local_10.pOutActor;
			if (this->pNearestFruit == (CActor*)0x0) {
				if (pCVar1 != (CActor*)0x0) {
					this->SetState(0xb, -1);
				}
			}
			else {
				if (this->actorState != MICKEN_EAT_STATE_WALK_TO_FRUIT) {
					fVar10 = (this->pNearestFruit->currentLocation).x - this->currentLocation.x;
					fVar11 = (this->pNearestFruit->currentLocation).z - this->currentLocation.z;
					this->field_0x3ec = sqrtf(fVar10 * fVar10 + 0.0f + fVar11 * fVar11);
					this->SetState(MICKEN_EAT_STATE_WALK_TO_FRUIT, -1);
				}
			}
		}
	}
	return;
}

void CActorMicken::BehaviourMickenSquashed_Manage(CBehaviourMickenSquashed* pBehaviour)
{
	int* piVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	CPlayerInput* pCVar5;
	float fVar6;
	float puVar7;
	BounceParams local_30;

	this->dynamic.speed = 0.0f;
	this->dynamicExt.normalizedTranslation.x = 0.0f;
	this->dynamicExt.normalizedTranslation.y = 0.0f;
	this->dynamicExt.normalizedTranslation.z = 0.0f;
	this->dynamicExt.normalizedTranslation.w = 0.0f;
	this->dynamicExt.field_0x6c = 0.0f;

	ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

	if ((pBehaviour->field_0xc == 0) && (0.2f < this->timeInAir)) {
		puVar7 = 0.8f;
		fVar6 = pBehaviour->field_0x8;
		pCVar5 = this->field_0x3d8->GetInputManager(0, 0);

		if ((pCVar5 == (CPlayerInput*)0x0) || (0.4f <= pCVar5->aButtons[4].pressedDuration)) {
			fVar6 = fVar6 - 600.0f;
		}
		else {
			fVar6 = fVar6 + 600.0f;
			puVar7 = 1.0;
			if (this->scale.y * 454.5f < pBehaviour->field_0x8) {
				puVar7 = 1.2f;
			}
		}

		if (1650.0f < fVar6) {
			local_30.field_0x10 = 1650.0f;
		}
		else {
			local_30.field_0x10 = 450.0f;

			if (450.0f <= fVar6) {
				local_30.field_0x10 = fVar6;
			}
		}

		piVar1 = (int*)this->field_0x364;
		if (((piVar1 == (int*)0x0) || (this->field_0x360 == 0)) || (bVar4 = true, this->field_0x360 != piVar1[6])) {
			bVar4 = false;
		}

		if (bVar4) {
			if (((piVar1 != (int*)0x0) && (this->field_0x360 != 0)) && (this->field_0x360 == piVar1[6])) {
				IMPLEMENTATION_GUARD(
				(**(code**)(*piVar1 + 0xc))();)
			}
			this->field_0x364 = 0;
			this->field_0x360 = 0;
		}

		IMPLEMENTATION_GUARD_LOG(
		CFxHandle::SV_FX_Start((CFxHandle*)&this->field_0x360);

		piVar1 = (int*)this->field_0x364;
		if (((piVar1 != (int*)0x0) && (this->field_0x360 != 0)) && (this->field_0x360 == piVar1[6])) {
			(**(code**)(*piVar1 + 0x10))(0, 0);
		}

		if (((int*)this->field_0x364 != (int*)0x0) && (this->field_0x360 != 0)) {
			(**(code**)(*(int*)this->field_0x364 + 0x40))(puVar7);
		})

		local_30.field_0x0.y = 1.0f;
		local_30.field_0x0.x = 0.0f;
		local_30.field_0x0.z = 0.0f;
		local_30.field_0x0.w = 0.0f;
		DoMessage(this->field_0x3d8, MESSAGE_BOUNCE, &local_30);
		pBehaviour->field_0xc = 1;
	}

	pCVar2 = this->pAnimationController;
	peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
	bVar4 = false;

	if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
		if (peVar3->animPlayState == 0) {
			bVar4 = false;
		}
		else {
			bVar4 = (peVar3->field_0xcc & 2) != 0;
		}
	}

	if (bVar4) {
		this->SetState(0x16, -1);
	}

	return;
}

void gMickenCallback(CActor* pActor, void* pParams)
{
	float fVar1;
	float fVar2;

	MickenParams* pMickenParams = reinterpret_cast<MickenParams*>(pParams);

	if (((pActor->typeID == 0x24) &&
		(fVar1 = (pMickenParams->pInActor->currentLocation).x - (pActor->currentLocation).x,
			fVar2 = (pMickenParams->pInActor->currentLocation).z - (pActor->currentLocation).z,
			fVar1 * fVar1 + fVar2 * fVar2 < pMickenParams->distance)) &&
		(1.0f < (pActor->currentLocation).y - (pMickenParams->pInActor->currentLocation).y)) {
		pMickenParams->pOutActor = pActor;
	}
	return;
}

void CActorMicken::BehaviourMicken_Manage(int state, CBehaviourMicken* pBehaviourMicken)
{
	bool bVar1;
	uint uVar2;
	int iVar3;
	StateConfig* pStateConfig;
	float fVar4;
	edF32VECTOR4 local_50;
	CActorConeInfluence actorConeInfluence;
	MickenParams mickenCallbackParams;
	CEventManager* pEventManager;
	CActorHero* pHero;
	CActorMicken* pMicken;

	if ((this->flags & 0x2000001) == 0) {
		iVar3 = this->actorState;
		if (iVar3 == -1) {
			uVar2 = 0;
		}
		else {
			pStateConfig = this->GetStateCfg(iVar3);
			uVar2 = pStateConfig->flags_0x4 & 1;
		}

		pEventManager = CScene::ptable.g_EventManager_006f5080;

		if (((uVar2 == 0) && (this->field_0x410 != 0)) && (this->state_0x10 == 0)) {
			IMPLEMENTATION_GUARD(
			iVar3 = this->field_0x404;
			pMicken = this;
			while (iVar3 != 0) {
				uVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, (ed_zone_3d*)pMicken->field_0x404,
					&this->currentLocation, 0);
				if ((uVar2 & 1) != 0) {
					bVar1 = true;
					goto LAB_0014d1a8;
				}
				iVar3 = pMicken->field_0x408;
				pMicken = (CActorMicken*)&(pMicken->base).base.base.objectId;
			}
			bVar1 = false;)
		LAB_0014d1a8:
			if (bVar1) {
				this->flags = this->flags & 0xfffffffc;
				this->flags = this->flags & 0xffffff5f;
				EvaluateDisplayState();
			}
			else {
				SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xd, -1);
			}
		}
	}

	if (this->field_0x3d0 != -1) {
		iVar3 = this->actorState;

		if (iVar3 == -1) {
			uVar2 = 0;
		}
		else {
			pStateConfig = this->GetStateCfg(iVar3);
			uVar2 = pStateConfig->flags_0x4;
		}

		pHero = CActorHero::_gThis;
		if ((uVar2 & 0x800) != 0) {
			actorConeInfluence.field_0x8 = this->scale.y * 3.0f;
			actorConeInfluence.field_0x0 = this->scale.y * 0.4f;
			actorConeInfluence.field_0x4 = this->scale.y * 1.2f;
			actorConeInfluence.field_0xc = 1.0f;
			actorConeInfluence.field_0x10 = 0.0f;
			actorConeInfluence.field_0x20.x = 0.0f;
			actorConeInfluence.field_0x20.y = -0.2f;
			actorConeInfluence.field_0x20.z = 0.0f;
			actorConeInfluence.field_0x20.w = 0.0f;
			actorConeInfluence.field_0x14 = (this->flags_0x3e8 & 1) != 0;

			fVar4 = SV_AttractActorInAConeAboveMe(CActorHero::_gThis, &actorConeInfluence);
			if ((fVar4 < 0.3f) && (iVar3 = DoMessage(pHero, MESSAGE_ENTER_TRAMPO, (MSG_PARAM)this->field_0x358), iVar3 != 0)) {
				this->field_0x3d8 = reinterpret_cast<CActorHeroPrivate*>(pHero);
				SetBehaviour(this->field_0x3d0, -1, -1);
			}
		}
	}

	iVar3 = this->actorState;
	if (iVar3 == -1) {
		uVar2 = 0;
	}
	else {
		pStateConfig = this->GetStateCfg(iVar3);
		uVar2 = pStateConfig->flags_0x4 & 0x8000;
	}

	if (uVar2 != 0) {
		mickenCallbackParams.distance = 5.76f;
		mickenCallbackParams.pOutActor = (CActor*)0x0;
		local_50.xyz = this->currentLocation.xyz;
		local_50.w = 2.4f;
		mickenCallbackParams.pInActor = (CActor*)this;
		(CScene::ptable.g_ActorManager_004516a4)->cluster.ApplyCallbackToActorsIntersectingSphere(&local_50, gMickenCallback, &mickenCallbackParams);
		if (mickenCallbackParams.pOutActor != (CActor*)0x0) {
			SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xd, -1);
		}
	}

	const int behaviourId = this->curBehaviourId;
	if (behaviourId == 6) {
		BehaviourMickenKicked_Manage(static_cast<CBehaviourMickenKicked*>(pBehaviourMicken));
	}
	else {
		if (behaviourId == 5) {
			iVar3 = this->actorState;
			if (iVar3 == 0x16) {
				// StateMickenWaitDuration (inlined?)
				fVar4 = this->scale.y;
				this->dynamic.speed = 0.0f;
				this->dynamicExt.normalizedTranslation.x = 0.0f;
				this->dynamicExt.normalizedTranslation.y = 0.0f;
				this->dynamicExt.normalizedTranslation.z = 0.0f;
				this->dynamicExt.normalizedTranslation.w = 0.0f;
				this->dynamicExt.field_0x6c = 0.0f;

				ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

				if (fVar4 * 0.15f < this->timeInAir) {
					if ((this->flags_0x3e8 & 8) == 0) {
						this->flags_0x3e8 = this->flags_0x3e8 | 0x10;
						SetBehaviour(MICKEN_BEHAVIOUR_EAT, -1, -1);
					}
					else {
						SetBehaviour(MICKEN_BEHAVIOUR_EAT, 0xb, -1);
					}
				}
			}
			else {
				if (iVar3 == 0x15) {
					BehaviourMickenSquashed_Manage(static_cast<CBehaviourMickenSquashed*>(pBehaviourMicken));
				}
			}
		}
		else {
			if (behaviourId == MICKEN_BEHAVIOUR_EAT) {
				BehaviourMickenEat_Manage(static_cast<CBehaviourMickenEat*>(pBehaviourMicken));
			}
		}
	}

	return;
}

void CBehaviourMicken::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorMicken*>(pOwner);
	return;
}

void CBehaviourMicken::InitState(int newState)
{
	CActorMicken* pCVar1;
	Timer* pTVar2;

	pCVar1 = this->pOwner;
	pTVar2 = GetTimer();
	pCVar1->field_0x3f0 = pTVar2->scaledTotalTime;
	return;
}

void CBehaviourMickenKicked::Manage()
{
	this->pOwner->BehaviourMicken_Manage(6, this);
	return;
}

void CBehaviourMickenKicked::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorMicken* pMicken;

	if (newState == -1) {
		pMicken = this->pOwner;
		pMicken->SetState(0x17, -1);
	}

	else {
		pMicken = this->pOwner;
		pMicken->SetState(newState, newAnimationType);
	}

	(this->pOwner)->field_0x3f8 = 0.0f;
	return;
}

void CBehaviourMickenKicked::InitState(int newState)
{
	byte bVar1;
	CActorMicken* pCVar2;
	Timer* pTVar3;

	pCVar2 = this->pOwner;
	pTVar3 = GetTimer();
	pCVar2->field_0x3f0 = pTVar3->scaledTotalTime;
	pCVar2 = this->pOwner;

	if (newState == 0x19) {
		pCVar2->dynamic.speed = 0.0f;
		pCVar2->flags_0x3e8 = pCVar2->flags_0x3e8 & 0xfffffffd;
	}
	else {
		if (newState == 0x17) {
			pCVar2->dynamic.speed = 0.0f;
			bVar1 = pCVar2->state_0x10;
			if (bVar1 != 0) {
				pCVar2->field_0x410 = bVar1;
				pCVar2->flags = pCVar2->flags | 2;
				pCVar2->flags = pCVar2->flags & 0xfffffffe;
				pCVar2->flags = pCVar2->flags | 0x80;
				pCVar2->flags = pCVar2->flags & 0xffffffdf;
				pCVar2->EvaluateDisplayState();
			}
		}
	}
	return;
}

void CBehaviourMickenKicked::TermState(int newState, int)
{
	if (newState == 0x17) {
		this->pOwner->field_0x3f8 = 0.0f;
	}

	return;
}

int CBehaviourMickenKicked::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorMicken* pMicken;
	Timer* pTVar2;
	int iVar3;
	edF32VECTOR4* v0;
	float fVar4;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	MessageKickedParams* pParam = reinterpret_cast<MessageKickedParams*>(pMsgParam);

	/* WARNING: Load size is inaccurate */
	if (((msg == 2) && (pParam->field_0x0)) && (pMicken = this->pOwner, pMicken->actorState == 0x1c)) {
		pMicken->SetState(0x17, -1);

		local_20.x = this->pOwner->currentLocation.x;
		local_20.z = this->pOwner->currentLocation.z;
		local_20.w = this->pOwner->currentLocation.w;
		local_20.y = pParam->field_0x44;

		this->pOwner->UpdatePosition(&local_20, true);

		edF32Vector4ScaleHard(pParam->field_0x30 / this->pOwner->scale.y, &eStack16, &pParam->impulseDirection);

		pMicken = this->pOwner;
		pTVar2 = GetTimer();
		edF32Vector4ScaleHard(0.02f / pTVar2->cutsceneDeltaTime, &eStack48, &eStack16);

		v0 = pMicken->dynamicExt.aImpulseVelocities;
		edF32Vector4AddHard(v0, v0, &eStack48);
		fVar4 = edF32Vector4GetDistHard(pMicken->dynamicExt.aImpulseVelocities);
		pMicken->dynamicExt.aImpulseVelocityMagnitudes[0] = fVar4;
		iVar3 = 1;
	}
	else {
		iVar3 = 0;
	}

	return iVar3;
}

void CBehaviourMickenSquashed::Manage()
{
	this->pOwner->BehaviourMicken_Manage(5, this);
	return;
}

void CBehaviourMickenSquashed::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorMicken* pMicken;

	if (newState == -1) {
		pMicken = this->pOwner;
		pMicken->SetState(0x15, -1);
	}
	else {
		pMicken = this->pOwner;
		pMicken->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMickenSquashed::InitState(int newState)
{
	CActorMicken* pCVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	pCVar1 = this->pOwner;
	pTVar2 = GetTimer();
	pCVar1->field_0x3f0 = pTVar2->scaledTotalTime;
	pCVar1 = this->pOwner;

	if (newState == 0x15) {
		fVar5 = fabs(pCVar1->field_0x3d8->dynamic.linearAcceleration * pCVar1->field_0x3d8->dynamic.velocityDirectionEuler.y);
		fVar3 = pCVar1->scale.y;
		fVar4 = pCVar1->scale.y * 450.0f;
		if (fVar3 * 10.0f < fVar5) {
			fVar4 = fVar4 + fVar3 * 600.0f;
		}

		fVar3 = pCVar1->scale.y;
		if (fVar3 * 15.5 < fVar5) {
			fVar4 = fVar4 + fVar3 * 600.0f;
		}

		this->field_0x8 = fVar4;
		this->field_0xc = 0;

		pCVar1->pCollisionData->actorFieldA = pCVar1->field_0x3d8;
	}

	return;
}

void CBehaviourMickenSquashed::TermState(int newState, int)
{
	if (newState == 0x15) {
		this->pOwner->pCollisionData->actorFieldA = (CActor*)0x0;
	}
}

void CBehaviourMickenEat::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetU32();
	this->pathFollowReader.Create(pByteCode);
}

void CBehaviourMickenEat::Init(CActor* pOwner)
{
	CActorMicken* pMicken;

	this->pOwner = static_cast<CActorMicken*>(pOwner);
	this->pathFollowReader.Init();

	if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
		pMicken = this->pOwner;
		pMicken->flags_0x3e8 = pMicken->flags_0x3e8 | 0x10;
	}
	else {
		this->pathFollowReader.Reset();
		pMicken = this->pOwner;
		pMicken->flags_0x3e8 = pMicken->flags_0x3e8 & 0xffffffef;
	}

	pMicken = this->pOwner;
	if ((this->field_0x10 & 4) == 0) {
		pMicken->flags_0x3e8 = pMicken->flags_0x3e8 & 0xfffffffb;
	}
	else {
		pMicken->flags_0x3e8 = pMicken->flags_0x3e8 | 4;
	}

	pMicken = this->pOwner;
	if ((this->field_0x10 & 8) == 0) {
		pMicken->flags_0x3e8 = pMicken->flags_0x3e8 & 0xfffffff7;
	}
	else {
		pMicken->flags_0x3e8 = pMicken->flags_0x3e8 | 8;
	}

	return;
}

void CBehaviourMickenEat::Manage()
{
	this->pOwner->BehaviourMicken_Manage(4, this);
	return;
}

void CBehaviourMickenEat::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorMicken* pMicken;
	CActor* pTiedActor;
	float fVar3;
	float fVar4;
	int iVar5;

	if (newState == -1) {
		pMicken = this->pOwner;
		if (pMicken->prevBehaviourId == 2) {
			pMicken->SetState(7, -1);
		}
		else {
			pTiedActor = pMicken->pTiedActor;
			if ((pTiedActor == (CActor*)0x0) ||
				(fVar3 = (pTiedActor->currentLocation).x - pMicken->currentLocation.x,
					fVar4 = (pTiedActor->currentLocation).z - pMicken->currentLocation.z,
					pMicken->field_0x3ec = sqrtf(fVar3 * fVar3 + 0.0f + fVar4 * fVar4),
					pMicken->field_0x3ec <= 0.3f)) {
				iVar5 = MICKEN_EAT_STATE_STAND;
			}
			else {
				iVar5 = 0x11;
			}

			pMicken->SetState(iVar5, -1);
		}
	}
	else {
		pMicken = this->pOwner;
		pMicken->SetState(newState, newAnimationType);
	}
	return;
}

void CBehaviourMickenEat::End(int newBehaviourId)
{
	this->pOwner->pNearestFruit = (CActor*)0x0;
	return;
}

void CBehaviourMickenEat::InitState(int newState)
{
	Timer* pTimer;
	CActorMicken* pMicken;

	pMicken = this->pOwner;
	pTimer = GetTimer();
	pMicken->field_0x3f0 = pTimer->scaledTotalTime;

	pMicken = this->pOwner;
	if (newState == MICKEN_EAT_STATE_CHEW) {
		pTimer = GetTimer();
		pMicken->field_0x3f4 = pTimer->scaledTotalTime;
	}
	else {
		if (newState == MICKEN_EAT_STATE_EAT) {
			pMicken->flags = pMicken->flags | 0x8000;
			pTimer = GetTimer();
			pMicken->field_0x3f4 = pTimer->scaledTotalTime;
		}
	}
	return;
}

void CBehaviourMickenEat::TermState(int newState, int oldState)
{
	CActorMicken* pMicken;

	pMicken = this->pOwner;
	if (newState == MICKEN_EAT_STATE_CHEW) {
		if (pMicken->pNearestFruit != (CActor*)0x0) {
			pMicken->DoMessage(pMicken->pNearestFruit, (ACTOR_MESSAGE)0x21, 0);
			pMicken->pNearestFruit = (CActor*)0x0;
		}
	}
	else {
		if (newState == MICKEN_EAT_STATE_EAT) {
			if (((pMicken->pNearestFruit != (CActor*)0x0) && (oldState != MICKEN_EAT_STATE_CHEW)) && ((pMicken->flags & 0x8000) == 0)) {
				pMicken->DoMessage(pMicken->pNearestFruit, (ACTOR_MESSAGE)0x21, 0);
				pMicken->pNearestFruit = (CActor*)0x0;
			}

			pMicken->flags = pMicken->flags & 0xffff7fff;
		}
	}

	return;
}

int CBehaviourMickenEat::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	CActorMicken* pMicken;

	int* pParam = (int*)pMsgParam;

	/* WARNING: Load size is inaccurate */
	if ((msg == 2) && (*pParam == 4)) {
		pMicken = this->pOwner;
		pMicken->SetState(0x14, -1);
		iVar1 = 1;
	}
	else {
		iVar1 = 0;
	}
	return iVar1;
}
