#include "ActorAmortos.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "MathOps.h"
#include "CollisionManager.h"
#include "ActorHero.h"
#include "TimeController.h"
#include "ActorManager.h"
#include "ActorFactory.h"

void CActorAmortos::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pAVar1;
	int* piVar2;
	char* pcVar3;
	int iVar4;
	uint uVar5;
	float fVar6;

	CActor::Create(pByteCode);

	pAVar1 = (S_ACTOR_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pAVar1->entryCount != 0x0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pAVar1->entryCount * sizeof(int);
	}
	this->pCheckpoint_0x184 = pAVar1;

	pAVar1 = (S_ACTOR_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pAVar1->entryCount != 0x0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pAVar1->entryCount * sizeof(int);
	}
	this->field_0x188 = pAVar1;

	pAVar1 = (S_ACTOR_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pAVar1->entryCount != 0x0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pAVar1->entryCount * sizeof(int);
	}
	this->field_0x18c = pAVar1;

	this->field_0x190.index = pByteCode->GetS32();

	this->field_0x164 = pByteCode->GetF32();
	this->field_0x168 = pByteCode->GetF32();
	this->field_0x160 = pByteCode->GetF32();
	this->field_0x16c = pByteCode->GetF32();
	this->field_0x170 = pByteCode->GetF32();
	this->field_0x174 = pByteCode->GetF32();
	this->field_0x178 = pByteCode->GetF32();
	this->field_0x17c = pByteCode->GetF32();
	this->field_0x180 = pByteCode->GetF32();
	this->addOnGenerator.Create(this, pByteCode);
	this->field_0x1c8 = pByteCode->GetU32();

	S_TARGET_STREAM_REF* pTargetStreamRef = (S_TARGET_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}
	this->field_0x1cc = pTargetStreamRef;

	pcVar3 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar3 + sizeof(S_STREAM_EVENT_CAMERA);
	this->field_0x1d0 = (S_STREAM_EVENT_CAMERA*)pcVar3;

	pTargetStreamRef = (S_TARGET_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}
	this->field_0x1d4 = pTargetStreamRef;

	pcVar3 = pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pcVar3 + sizeof(S_STREAM_EVENT_CAMERA);
	this->field_0x1d8 = (S_STREAM_EVENT_CAMERA*)pcVar3;

	return;
}

void CActorAmortos::Init()
{
	S_TARGET_STREAM_REF* pSVar1;
	S_STREAM_REF<CActor>* pStreamRefActor;
	int iVar3;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	pStreamRefActor = this->pCheckpoint_0x184->aEntries;
	for (iVar3 = this->pCheckpoint_0x184->entryCount; iVar3 != 0; iVar3 = iVar3 + -1) {
		pStreamRefActor->Init();
		pStreamRefActor = pStreamRefActor + 1;
	}

	this->addOnGenerator.Init(0);

	CActor::Init();

	pStreamRefActor = this->field_0x18c->aEntries;
	for (iVar3 = this->field_0x18c->entryCount; iVar3 != 0; iVar3 = iVar3 + -1) {
		pStreamRefActor->Init();
		pStreamRefActor = pStreamRefActor + 1;
	}

	pStreamRefActor = this->field_0x188->aEntries;
	for (iVar3 = this->field_0x188->entryCount; iVar3 != 0; iVar3 = iVar3 + -1) {
		pStreamRefActor->Init();
		pStreamRefActor = pStreamRefActor + 1;
	}

	field_0x190.Init();

	pSVar1 = this->field_0x1cc;
	iVar3 = 0;
	if (0 < pSVar1->entryCount) {
		do {
			pSVar1->aEntries[iVar3].Init();
			pSVar1 = this->field_0x1cc;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pSVar1->entryCount);
	}

	this->field_0x1d0->Init();

	pSVar1 = this->field_0x1d4;
	iVar3 = 0;
	if (0 < pSVar1->entryCount) {
		do {
			pSVar1->aEntries[iVar3].Init();
			pSVar1 = this->field_0x1d4;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pSVar1->entryCount);
	}

	this->field_0x1d8->Init();

	SV_GetBoneDefaultWorldPosition(this->field_0x1c8, &local_10);
	edF32Vector4SubHard(&local_20, &local_10, &this->currentLocation);
	edF32Vector4NormalizeHard(&local_20, &local_20);
	edF32Vector4ScaleHard(-0.5f, &eStack48, &local_20);
	edF32Vector4AddHard(&local_10, &local_10, &eStack48);

	(this->cylinderDetection).field_0x0 = this->field_0x17c;
	(this->cylinderDetection).field_0x4 = this->field_0x174;
	(this->cylinderDetection).field_0x8 = this->field_0x178;
	(this->cylinderDetection).field_0xc = this->field_0x180;
	(this->cylinderDetection).field_0x20 = local_10;
	(this->cylinderDetection).field_0x30 = local_20;

	pSVar1 = this->field_0x1cc;
	iVar3 = 0;
	if (0 < pSVar1->entryCount) {
		do {
			pSVar1->aEntries[iVar3].Reset();
			pSVar1 = this->field_0x1cc;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pSVar1->entryCount);
	}

	this->field_0x1d0->Reset(this);

	pSVar1 = this->field_0x1d4;
	iVar3 = 0;
	if (0 < pSVar1->entryCount) {
		do {
			pSVar1->aEntries[iVar3].Reset();
			pSVar1 = this->field_0x1d4;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pSVar1->entryCount);
	}

	this->field_0x1d8->Reset(this);

	this->field_0x2a0 = 1.08f;
	this->field_0x2b4 = 1.65f;
	this->field_0x2b0 = 0.0f;
	this->field_0x2ac = 0;
	this->field_0x2a8 = this->field_0x160;

	return;
}

void CActorAmortos::Term()
{
	IMPLEMENTATION_GUARD();
}

void CActorAmortos::Reset()
{
	CCollision* pCVar1;
	S_TARGET_STREAM_REF* pSVar2;
	int iVar3;
	int iVar4;

	for (int i = 0; i < this->field_0x1cc->entryCount; i++) {
		this->field_0x1cc->aEntries[i].Reset();
	}

	this->field_0x1d0->Reset(this);

	for (int i = 0; i < this->field_0x1d4->entryCount; i++) {
		this->field_0x1d4->aEntries[i].Reset();
	}

	this->field_0x1d8->Reset(this);

	this->field_0x2a0 = 1.08f;
	this->field_0x2b4 = 1.65f;
	this->field_0x2b0 = 0.0f;
	this->field_0x2ac = 0;
	this->field_0x2a8 = this->field_0x160;

	CActor::Reset();

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;

	return;
}

CBehaviour* CActorAmortos::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

CActorAmortos::CActorAmortos()
{
	//*(undefined4*)&this->field_0x1e4 = 0;
	//*(undefined4*)&this->field_0x244 = 0;

	//*(undefined4*)&this->field_0x304 = 0;
	this->field_0x408.entryCount = 0;
}

StateConfig CActorAmortos::_gStateCfg_AMO[5] = {
	StateConfig(0x6, 0xc),
	StateConfig(0x9, 0x8),
	StateConfig(0xa, 0x8),
	StateConfig(0x7, 0x8),
	StateConfig(0x8, 0xc),
};

StateConfig* CActorAmortos::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_AMO + state + -5;
	}

	return pStateConfig;
}

void CActorAmortos::ChangeManageState(int state)
{
	CActor::ChangeManageState(state);

	if (state == 0) {
		if (this->actorState == 6) {
			this->pAnimationController->UnRegisterBone(this->field_0x1c8);
		}
	}
	else {
		SetState(-1, -1);
		SetState(5, -1);
	}

	return;
}

CVision* CActorAmortos::GetVision()
{
	IMPLEMENTATION_GUARD();
}

int CActorAmortos::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	long lVar2;
	edF32VECTOR4 local_10;

	if (msg == 7) {
		IMPLEMENTATION_GUARD(
		edF32Vector4ScaleHard(2.0, &local_10, (edF32VECTOR4*)&((this->pMeshTransform)->base).transformA.ba);
		iVar1 = 1;
		*(float*)((int)pMsgParam + 0x20) = local_10.x;
		*(float*)((int)pMsgParam + 0x24) = local_10.y;
		*(float*)((int)pMsgParam + 0x28) = local_10.z;
		*(float*)((int)pMsgParam + 0x2c) = local_10.w;)
	}
	else {
		if ((msg == 0xf) || (msg == 0x43)) {
			SetState(9, -1);
			iVar1 = 1;
		}
		else {
			if ((msg == 0x10) || (msg == 0x42)) {
				SetState(7, -1);
				iVar1 = 1;
			}
			else {
				if (msg == 2) {
					lVar2 = IsLockable();
					if (lVar2 != 0) {
						/* WARNING: Load size is inaccurate */
						IMPLEMENTATION_GUARD(
						iVar1 = *pMsgParam;
						if (iVar1 == 5) {
							if (0.0 < *(float*)((int)pMsgParam + 0xc)) {
								SetState(7, -1);
								return 1;
							}
						}
						else {
							if (iVar1 == 10) {
								if (0.0 < *(float*)((int)pMsgParam + 0xc)) {
									SetState(7, -1);
									return 1;
								}
							}
							else {
								if ((((iVar1 == 9) || (iVar1 == 3)) || (iVar1 == 1)) || ((iVar1 == 4 || (iVar1 == 0)))) {
									SetState(7, -1);
									return 1;
								}
							}
						})
					}

					iVar1 = 0;
				}
				else {
					iVar1 = CActor::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}

	return iVar1;
}

int CActorAmortos::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	IMPLEMENTATION_GUARD();
}

void CActorAmortos::BehaviourAmortosStand_Manage()
{
	int iVar1;
	CCollision* pCVar2;
	CAnimation* pCVar3;
	edAnmLayer* peVar4;
	CActor* pReceiver;
	bool bVar5;
	undefined8 uVar6;
	long lVar7;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	BounceParams local_30;
	edF32VECTOR4 local_20;
	undefined4 local_8;
	float* local_4;

	this->field_0x1d0->Manage(this);
	this->field_0x1d8->Manage(this);

	iVar1 = this->actorState;

	if (iVar1 == 9) {
		this->field_0x2a0 = this->timeInAir / 2.0f;

		lVar7 = DetectActorWithCylinder();
		if (lVar7 != 0) {
			SetState(6, -1);
		}

		pCVar3 = this->pAnimationController;
		peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
		bVar5 = false;
		if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
			if (peVar4->animPlayState == 0) {
				bVar5 = false;
			}
			else {
				bVar5 = (peVar4->field_0xcc & 2) != 0;
			}
		}

		if (bVar5) {
			this->field_0x2a0 = 1.08f;
			SetState(5, -1);
		}
	}
	else {
		if (iVar1 == 8) {
			StateAmortosWaitReSwell();
		}
		else {
			if (iVar1 == 7) {
				StateAmortosExplode();
			}
			else {
				if (iVar1 == 6) {
					pCVar3 = this->pAnimationController;
					peVar4 = (pCVar3->anmBinMetaAnimator).aAnimData;
					bVar5 = false;
					if ((peVar4->currentAnimDesc).animType == pCVar3->currentAnimType_0x30) {
						if (peVar4->animPlayState == 0) {
							bVar5 = false;
						}
						else {
							bVar5 = (peVar4->field_0xcc & 2) != 0;
						}
					}

					if (bVar5) {
						local_30.field_0x10 = 650.0f;
						SV_GetBoneWorldPosition(this->field_0x1c8, &eStack96);
						edF32Vector4SubHard(&local_50, &eStack96, &this->currentLocation);
						edF32Vector4NormalizeHard(&local_50, &local_50);
						local_30.field_0x0 = local_50;
						DoMessage(this->pLaunchedActor, MESSAGE_IMPULSE, &local_30);

						pReceiver = (this->field_0x190).Get();
						if (pReceiver == (CActor*)0x0) {
							SetState(7, -1);
						}
						else {
							local_8 = 0;
							DoMessage(pReceiver, (ACTOR_MESSAGE)0x41, 0);
						}
					}
				}
				else {
					if (iVar1 == 5) {
						this->pLaunchedActor = CActorHero::_gThis;
						lVar7 = DetectActorWithCylinder();
						if (lVar7 != 0) {
							pCVar2 = this->pCollisionData;
							pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffefff;
							pCVar2 = this->pCollisionData;
							pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7ffff;
							SetState(6, -1);
						}
					}
				}
			}
		}
	}

	if (this->pTiedActor != (CActor*)0x0) {
		local_20 = this->baseLocation;
		SV_UpdatePosition_Rel(&local_20, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CActorAmortos::BehaviourAmortosStand_TermState()
{
	CCollision* pCVar1;
	S_ACTOR_STREAM_REF* pSVar2;
	edF32VECTOR4* v2;
	StateConfig* pSVar3;
	uint uVar4;
	int iVar5;
	int iVar7;
	edF32VECTOR4 eStack16;
	CActor* pLinked;

	if (this->field_0x2ac == 0) {
		pCVar1 = this->pCollisionData;
		v2 = GetBottomPosition();
		edF32Vector4AddHard(&eStack16, &pCVar1->highestVertex, v2);
		edF32Vector4ScaleHard(0.5f, &eStack16, &eStack16);
		IMPLEMENTATION_GUARD_LOG(
		ActorComponent34Func_00397180(&this->addOnGenerator, &eStack16);)
		iVar7 = 0;
		while (true) {
			pSVar2 = this->pCheckpoint_0x184;
			iVar5 = 0;
			if (pSVar2 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar5 = pSVar2->entryCount;
			}

			if (iVar5 <= iVar7) break;

			pLinked = pSVar2->aEntries[iVar7].Get();
			if (pLinked != (CActor*)0x0) {
				if (pLinked->actorState == -1) {
					uVar4 = 0;
				}
				else {
					pSVar3 = pLinked->GetStateCfg(pLinked->actorState);
					uVar4 = pSVar3->flags_0x4 & 1;
				}

				if (uVar4 == 0) {
					pCVar1 = pLinked->pCollisionData;
					if (pCVar1 != (CCollision*)0x0) {
						pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
					}
					pLinked->flags = pLinked->flags | 0x80;
					pLinked->flags = pLinked->flags & 0xffffffdf;
					pLinked->EvaluateDisplayState();
					pLinked->flags = pLinked->flags & 0xfffffffc;
				}
			}

			iVar7 = iVar7 + 1;
		}
	}

	this->field_0x2ac = 1;
	this->field_0x2b4 = 1.65f;

	return;
}

bool CActorAmortos::DetectActorWithCylinder()
{
	CActorMovable* pCVar1;
	S_ACTOR_STREAM_REF* pSVar2;
	bool bVar3;
	uint uVar4;
	StateConfig* pSVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	uint local_8;
	uint local_4;

	SV_GetBoneDefaultWorldPosition(this->field_0x1c8, &local_20);
	edF32Vector4SubHard(&local_30, &local_20, &this->currentLocation);
	edF32Vector4NormalizeHard(&local_30, &local_30);
	edF32Vector4ScaleHard(-0.5f, &eStack64, &local_30);
	edF32Vector4AddHard(&local_20, &local_20, &eStack64);
	(this->cylinderDetection).field_0x20 = local_20;
	(this->cylinderDetection).field_0x30 = local_30;

	pCVar1 = CActorHero::_gThis;
	edF32Vector4SubHard(&eStack80, &(this->cylinderDetection).field_0x20, &this->currentLocation);
	if (pCVar1 != (CActorHero*)0x0) {
		fVar9 = edF32Vector4DotProductHard(&eStack80, &pCVar1->dynamic.velocityDirectionEuler);
		bVar3 = this->cylinderDetection.DetectAttract(pCVar1);
		if (bVar3 != false) {
			iVar7 = pCVar1->actorState;
			if (iVar7 == -1) {
				uVar4 = 0;
			}
			else {
				pSVar5 = pCVar1->GetStateCfg(iVar7);
				uVar4 = pSVar5->flags_0x4 & 0x100;
			}

			if ((uVar4 == 0) && (fVar9 <= 0.0f)) {
				local_4 = this->field_0x1c8;
				iVar7 = DoMessage(pCVar1, (ACTOR_MESSAGE)0x1d, (MSG_PARAM)local_4);
				if (iVar7 != 0) {
					this->pLaunchedActor = pCVar1;
					return true;
				}
			}
		}
	}

	iVar8 = 0;
	while (true) {
		pSVar2 = this->field_0x188;
		iVar6 = 0;

		if (pSVar2 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = pSVar2->entryCount;
		}

		if (iVar6 <= iVar8) break;

		pCVar1 = reinterpret_cast<CActorMovable*>(pSVar2->aEntries[iVar8].Get());
		bVar3 = this->cylinderDetection.DetectAttract(pCVar1);
		if (bVar3 != false) {
			iVar6 = pCVar1->actorState;
			if (iVar6 == -1) {
				uVar4 = 0;
			}
			else {
				pSVar5 = pCVar1->GetStateCfg(iVar6);
				uVar4 = pSVar5->flags_0x4 & 0x100;
			}

			if ((uVar4 == 0) && (fVar9 = edF32Vector4DotProductHard(&eStack80, &pCVar1->dynamic.velocityDirectionEuler), fVar9 < 0.0f)) {
				local_8 = this->field_0x1c8;
				iVar6 = DoMessage(pCVar1, (ACTOR_MESSAGE)0x1d, (MSG_PARAM)local_8);
				if (iVar6 != 0) {
					this->pLaunchedActor = pCVar1;
					return true;
				}
			}
		}

		iVar8 = iVar8 + 1;
	}

	return false;
}

bool gAmoReswellCallback(CActor* pActor, void* pParams)
{
	S_ACTOR_STREAM_REF* pSVar1;
	bool bVar2;
	int iVar3;
	int iVar5;

	CActorAmortos* pAmortos = reinterpret_cast<CActorAmortos*>(pParams);

	if ((((CActorFactory::gClassProperties[pActor->typeID].flags & 0x200) == 0) && (bVar2 = pActor->IsKindOfObject(2), bVar2 != false)) && (pAmortos != (CActorAmortos*)pActor))
	{
		pSVar1 = pAmortos->field_0x18c;
		iVar5 = 0;
		while (true) {
			iVar3 = 0;
			if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar3 = pSVar1->entryCount;
			}

			if (iVar3 <= iVar5) break;

			if (pSVar1->aEntries[iVar5].Get() == pActor) {
				return false;
			}

			iVar5 = iVar5 + 1;
		}

		bVar2 = true;
	}
	else {
		bVar2 = false;
	}

	return bVar2;
}

void CActorAmortos::StateAmortosWaitReSwell()
{
	float fVar1;
	float fVar2;
	CActorManager* pCVar3;
	Timer* pTVar4;
	float fVar5;
	edF32VECTOR4 intersectSphere;
	CActorsTable local_110;

	pCVar3 = CScene::ptable.g_ActorManager_004516a4;
	pTVar4 = GetTimer();
	fVar5 = this->field_0x2a8 - pTVar4->cutsceneDeltaTime;
	this->field_0x2a8 = fVar5;

	if ((fVar5 < 0.0f) && ((this->field_0x190).Get() == (CActor*)0x0)) {
		if ((this->actorFieldS & 8) == 0) {
			local_110.entryCount = 0;
			intersectSphere.w = 2.3f;
			intersectSphere.xyz = this->currentLocation.xyz;
			pCVar3->cluster.GetActorsIntersectingSphereWithCriterion(&local_110, &intersectSphere, gAmoReswellCallback, this);
			if (local_110.entryCount == 0) {
				SetState(9, -1);
			}
		}
		else {
			fVar5 = CActorHero::_gThis->currentLocation.x - this->currentLocation.x;
			fVar1 = CActorHero::_gThis->currentLocation.y - this->currentLocation.y;
			fVar2 = CActorHero::_gThis->currentLocation.z - this->currentLocation.z;
			if (3.3f < sqrtf(fVar5 * fVar5 + fVar1 * fVar1 + fVar2 * fVar2)) {
				SetState(9, -1);
			}
		}
	}

	return;
}

bool gAmoExplodeCallbackDeluxe(CActor* pActor, void* pParams)
{
	int iVar1;
	int iVar3;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;
	CActorMovable* pCVar1;

	CActorAmortos* pAmortos = reinterpret_cast<CActorAmortos*>(pParams);

	pCVar1 = reinterpret_cast<CActorMovable*>(pAmortos->field_0x190.Get());

	if ((pCVar1 != (CActorMovable*)0x0) && (fVar4 = (pCVar1->dynamic).rotationQuat.y, fVar4 != 0.0f)) {
		IMPLEMENTATION_GUARD(
		edF32Vector4SubHard(&eStack16, (edF32VECTOR4*)((int)fVar4 + 0x30), &pAmortos->currentLocation);
		edProjectVectorOnPlane
		(0.0, &eStack32, &eStack16, (edF32VECTOR4*)&(((pAmortos->base).pMeshTransform)->base).transformA.ba, 0);
		fVar4 = edF32Vector4GetDistHard(&eStack32);
		if (fVar4 < 2.0) {
			return false;
		})
	}

	if (((CActorFactory::gClassProperties[pActor->typeID].flags & 0x10) != 0) && (pActor != pAmortos)) {
		edF32Vector4SubHard(&eStack16, &pActor->currentLocation, &pAmortos->currentLocation);
		edProjectVectorOnPlane(0.0f, &eStack32, &eStack16, &pAmortos->pMeshTransform->base.transformA.rowY, 0);
		fVar4 = edF32Vector4GetDistHard(&eStack32);

		if ((pAmortos->field_0x2b0 < fVar4) && (fVar4 < pAmortos->field_0x2b4)) {
			edF32Vector4SubHard(&eStack16, &eStack16, &eStack32);
			fVar4 = edF32Vector4GetDistHard(&eStack16);
			if (fVar4 < 2.0f) {
				iVar1 = (pAmortos->field_0x408).entryCount;
				iVar3 = 0;
				if (0 < iVar1) {
					do {
						if (pActor == pAmortos->field_0x408.aEntries[iVar3]) {
							return false;
						}
						iVar3 = iVar3 + 1;
					} while (iVar3 < iVar1);
				}

				return true;
			}
		}
	}

	return false;
}

void CActorAmortos::StateAmortosExplode()
{
	CActor* pNewEntry;
	float fVar1;
	float fVar2;
	Timer* pTVar3;
	int iVar4;
	undefined4 local_1b0[3];
	undefined4 local_1a4;
	edF32VECTOR4 eStack400;
	float local_180;
	CActorsTable table;
	edF32VECTOR4 intersectSphere;
	undefined4* local_4;

	pTVar3 = GetTimer();
	this->field_0x2b4 = this->field_0x2b4 + (this->field_0x164 * pTVar3->cutsceneDeltaTime) / this->field_0x168;

	intersectSphere.xyz = this->currentLocation.xyz;
	intersectSphere.w = this->field_0x2b4;

	table.entryCount = 0;
	(CScene::ptable.g_ActorManager_004516a4)->cluster.GetActorsIntersectingSphereWithCriterion(&table, &intersectSphere, gAmoExplodeCallbackDeluxe, this);

	for (iVar4 = 0; iVar4 < table.entryCount; iVar4 = iVar4 + 1) {
		pNewEntry = table.aEntries[iVar4];
		this->field_0x408.Add(pNewEntry);
		local_1b0[0] = 10;
		edF32Vector4SubHard(&eStack400, &pNewEntry->currentLocation, &this->currentLocation);
		edF32Vector4NormalizeHard(&eStack400, &eStack400);
		eStack400.y = 0.5f;
		edF32Vector4NormalizeHard(&eStack400, &eStack400);
		fVar1 = (pNewEntry->currentLocation).x - this->currentLocation.x;
		fVar2 = (pNewEntry->currentLocation).z - this->currentLocation.z;
		local_4 = local_1b0;
		local_1a4 = 0;
		local_180 = this->field_0x16c - (sqrtf(fVar1 * fVar1 + 0.0f + fVar2 * fVar2) * (this->field_0x16c - this->field_0x170)) / this->field_0x164;
		DoMessage(pNewEntry, (ACTOR_MESSAGE)2, (MSG_PARAM)local_4);
	}

	if (this->field_0x164 <= this->field_0x2b4) {
		this->field_0x408.entryCount = 0;
		SetState(8, -1);
	}
	return;
}

void CBehaviourAmortos::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = reinterpret_cast<CActorAmortos*>(pOwner);

	return;
}

void CBehaviourAmortosStand::Manage()
{
	this->pOwner->BehaviourAmortosStand_Manage();

	return;
}

void CBehaviourAmortosStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = reinterpret_cast<CActorAmortos*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourAmortosStand::InitState(int newState)
{
	CActorAmortos* pAmortos;
	CCollision* pCVar1;
	S_ACTOR_STREAM_REF* pSVar2;
	int iVar3;
	int iVar5;
	CActor* pLinked;

	pAmortos = this->pOwner;
	iVar5 = pAmortos->actorState;
	if (iVar5 == 6) {
		pAmortos->pAnimationController->RegisterBone(pAmortos->field_0x1c8);
	}
	else {
		if (iVar5 == 8) {
			pAmortos->field_0x2a8 = pAmortos->field_0x160;
		}
		else {
			if (iVar5 == 9) {
				pAmortos->pAnimationController->RegisterBone(pAmortos->field_0x1c8);
				pCVar1 = pAmortos->pCollisionData;
				pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
				pCVar1 = pAmortos->pCollisionData;
				pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;

				for (int i = 0; i < pAmortos->field_0x1d4->entryCount; i++) {
					pAmortos->field_0x1d4->aEntries[i].Switch(pAmortos);
				}

				pAmortos->field_0x1d8->SwitchOn(pAmortos);
			}
			else {
				if (iVar5 == 7) {
					for (int i = 0; i < pAmortos->field_0x1cc->entryCount; i++) {
						pAmortos->field_0x1cc->aEntries[i].Switch(pAmortos);
					}

					pAmortos->field_0x1d0->SwitchOn(pAmortos);

					pCVar1 = pAmortos->pCollisionData;
					pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffefff;
					pCVar1 = pAmortos->pCollisionData;
					pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xfff7ffff;
					pAmortos->field_0x2b0 = 1.65f;
					pAmortos->field_0x2b4 = 1.65f;
				}
				else {
					if (iVar5 == 5) {
						pCVar1 = pAmortos->pCollisionData;
						pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
						pCVar1 = pAmortos->pCollisionData;
						pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;
						iVar5 = 0;
						if (pAmortos->field_0x2ac == 0) {
							while (true) {
								pSVar2 = pAmortos->pCheckpoint_0x184;
								iVar3 = 0;

								if (pSVar2 != (S_ACTOR_STREAM_REF*)0x0) {
									iVar3 = pSVar2->entryCount;
								}
								if (iVar3 <= iVar5) break;

								pLinked = pSVar2->aEntries[iVar5].Get();

								if (pLinked != (CActor*)0x0) {
									pLinked->flags = pLinked->flags & 0xffffff7f;
									pLinked->flags = pLinked->flags | 0x20;
									pLinked->EvaluateDisplayState();
									pLinked->flags = pLinked->flags & 0xfffffffd;
									pLinked->flags = pLinked->flags | 1;
								}
								iVar5 = iVar5 + 1;
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CBehaviourAmortosStand::TermState(int oldState, int newState)
{
	CActorAmortos* pAmortos;
	int state;

	pAmortos = this->pOwner;
	state = pAmortos->actorState;
	if (state == 6) {
		pAmortos->pAnimationController->UnRegisterBone(pAmortos->field_0x1c8);
	}
	else {
		if (state == 9) {
			pAmortos->pAnimationController->UnRegisterBone(pAmortos->field_0x1c8);
		}
		else {
			if (state == 7) {
				pAmortos->BehaviourAmortosStand_TermState();
			}
		}
	}

	return;
}

CCylinderDetection::CCylinderDetection()
{
	(this->field_0x20).x = 0.0f;
	(this->field_0x20).y = 0.0f;
	(this->field_0x20).z = 0.0f;
	(this->field_0x20).w = 1.0f;

	(this->field_0x30).x = 0.0f;
	(this->field_0x30).y = 0.0f;
	(this->field_0x30).z = 0.0f;
	(this->field_0x30).w = 0.0f;

	this->field_0x0 = 0.0f;
	this->field_0x4 = 0.0f;
	this->field_0x8 = 0.0f;
	this->field_0xc = 0.0f;

	this->field_0x10 = 15.0f;

	return;
}

bool CCylinderDetection::DetectAttract(CActor* pActor)
{
	bool bVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	float t;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if ((pActor != (CActor*)0x0) && (bVar1 = pActor->IsKindOfObject(2), bVar1 != false)) {
		bVar1 = false;
		if ((((this->field_0x20).w == 1.0f) && ((((bVar1 = false, (this->field_0x30).w == 0.0f && (bVar1 = false, 0.0f <= this->field_0x0)) &&
				(bVar1 = false, 0.0f <= this->field_0x4)) && ((bVar1 = false, 0.0f <= this->field_0x8 && (bVar1 = false, 0.0f <= this->field_0xc)))))) &&
			(bVar1 = false, 0.0f < this->field_0x10)) {
			bVar1 = true;
		}

		if (bVar1) {
			CActorMovable* pMovable = reinterpret_cast<CActorMovable*>(pActor);
			edF32Vector4SubHard(&eStack16, &this->field_0x20, &pMovable->currentLocation);
			fVar3 = edF32Vector4DotProductHard(&eStack16, &this->field_0x30);
			fVar4 = -fVar3;
			if ((fVar4 < this->field_0x4) && (0.0f < fVar4)) {
				edF32Vector4ScaleHard(fVar4, &eStack32, &this->field_0x30);
				edF32Vector4AddHard(&eStack48, &eStack32, &eStack16);
				fVar4 = edF32Vector4GetDistHard(&eStack48);
				if (0.001f < fVar4) {
					edF32Vector4NormalizeHard(&eStack48, &eStack48);
				}

				if (fVar4 < this->field_0x0) {
					t = (this->field_0xc * (pMovable->dynamic).linearAcceleration) / this->field_0x10;

					pTVar2 = GetTimer();
					if (fVar4 < t * pTVar2->cutsceneDeltaTime) {
						pTVar2 = GetTimer();
						t = fVar4 / pTVar2->cutsceneDeltaTime;
					}

					edF32Vector4ScaleHard(t, &eStack48, &eStack48);

					pMovable->SV_MOV_AddSpeedVector(&eStack48);

					if (fabs(fVar3) < this->field_0x8) {
						return true;
					}
				}
			}
		}
	}

	return false;
}
