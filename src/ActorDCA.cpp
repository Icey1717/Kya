#include "ActorDCA.h"
#include "MemoryStream.h"
#include "CollisionManager.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include "ActorProjectile.h"
#include "TimeController.h"
#include "ActorHero.h"

void CActorDCA::Create(ByteCode* pByteCode)
{
	CCollision* pCVar1;
	float fVar2;
	int iVar3;
	uint uVar4;
	CActorSound* pCVar5;
	float fVar6;

	CActor::Create(pByteCode);

	this->pCamera = (CCamera*)0x0;

	this->field_0x504.index = pByteCode->GetS32();
	this->field_0x500.index = pByteCode->GetS32();

	this->field_0x464 = pByteCode->GetF32();

	this->field_0x508.index = pByteCode->GetS32();
	this->field_0x50c.index = pByteCode->GetS32();

	this->field_0x160 = pByteCode->GetF32() * 0.01745329f;

	this->fireShot.Create(pByteCode);

	this->field_0x450 = pByteCode->GetU32();
	this->field_0x454 = pByteCode->GetU32();

	pCVar1 = this->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffffc4;
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x800480;
	}

	this->field_0x4fc = CreateActorSound(4);

	this->field_0x164 = gF32Vertex4Zero.xyz;
	this->field_0x170 = gF32Vertex4Zero.xyz;
	this->field_0x1a0 = (CActor*)0x0;
	this->field_0x4f0 = 3.0f;

	return;
}

void CActorDCA::Init()
{
	float fVar1;
	float fVar2;
	float fVar3;
	CCamera* pCVar4;
	CCameraManager* pCameraManager;

	pCameraManager = CCameraManager::_gThis;
	this->pCamera = CCameraManager::_gThis->GetDefGameCamera(CT_DCA);

	if (this->pCamera == (CCamera*)0x0) {
		ByteCode BStack16;
		this->pCamera = pCameraManager->AddCamera(CT_DCA, &BStack16, "DCA");
		this->pCamera->Init();
	}

	this->pAnimationController->RegisterBone(this->field_0x450);

	CActor::Init();

	this->fireShot.Init();

	this->field_0x500.Init();
	this->field_0x504.Init();
	this->field_0x508.Init();
	this->field_0x50c.Init();

	this->field_0x164.x = 0.0f;
	this->field_0x164.y = 0.0f;
	this->field_0x164.z = 0.0f;

	(this->field_0x170).x = 0.0f;
	(this->field_0x170).y = 0.0f;
	(this->field_0x170).z = 0.0f;

	this->field_0x1a0 = (CActor*)0x0;
	this->field_0x460 = 0;
	this->field_0x45c = 0;

	this->field_0x180 = gF32Vertex4Zero;
	this->field_0x190 = gF32Vertex4Zero;
	this->field_0x170 = gF32Vertex4Zero.xyz;

	this->field_0x459 = 0;

	this->fireShot.Reset();
	this->field_0x440 = 1;

	return;
}

void CActorDCA::Term()
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::Reset()
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CheckpointReset()
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorDCA::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == DCA_BEHAVIOUR_TRAJECTORY) {
		pBehaviour = new CBhvTrajectory;
	}
	else {
		if (behaviourType == DCA_BEHAVIOUR_AI_CONTROLLED) {
			pBehaviour = &this->behaviourAiControlled;
		}
		else {
			if (behaviourType == DCA_BEHAVIOUR_CONTROLLED) {
				pBehaviour = &this->behaviourControlled;
			}
			else {
				if (behaviourType == DCA_BEHAVIOUR_DEFAULT) {
					pBehaviour = &this->behaviourDefault;
				}
				else {
					pBehaviour = CActor::BuildBehaviour(behaviourType);
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorDCA::_gStateCfg_DCA[3] = {
	StateConfig(0x8, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x7, 0x0),
};

StateConfig* CActorDCA::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 3); // "CActorDCA::GetStateCfg: state out of range, game bug?!!");
		pStateConfig = _gStateCfg_DCA + state + -5;
	}

	return pStateConfig;
}

void CActorDCA::AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edANM_HDR* peVar1;
	Timer* pTVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	if (param_2 == 8) {
		if (newAnim == 9) {
			IMPLEMENTATION_GUARD(
			fVar3 = 0.0;
			peVar1 = pAnimator->pAnimKeyTableEntry;
			if ((float)this->field_0x460 != 0.0) {
				pTVar2 = GetTimer();
				fVar3 = *(float*)&this->field_0x530;
				fVar3 = ((fVar3 + (pTVar2->scaledTotalTime - (float)this->field_0x460) * *(float*)&this->field_0x538) - fVar3)
					/ (*(float*)&this->field_0x534 - fVar3);
			}
			fVar4 = 1.0;
			if (fVar3 <= 1.0) {
				fVar4 = fVar3;
			}
			if (fVar4 < 0.0) {
				fVar4 = 0.0;
			}
			(&peVar1[1].count_0x0)[peVar1->keyIndex_0x8] = (int)(1.0 - fVar4);
			(&peVar1[1].field_0x4)[peVar1->keyIndex_0x8] = (int)fVar4;)
		}
	}
	else {
		if ((param_2 == 2) || (param_2 == 1)) {
			char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
			AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

			fVar4 = ((this->field_0x170).x * 0.5f) / 3.141593f + 0.5f;
			fVar3 = ((this->field_0x170).y * 0.5f) / 3.141593f + 0.5f;

			if (fVar4 < 0.0f) {
				fVar5 = 0.0f;
			}
			else {
				fVar5 = 1.0f;
				if (fVar4 < 1.0f) {
					fVar5 = fVar4;
				}
			}

			if (fVar3 < 0.0f) {
				fVar4 = 0.0f;
			}
			else {
				fVar4 = 1.0f;
				if (fVar3 < 1.0f) {
					fVar4 = fVar3;
				}
			}

			pValue->field_0xc = fVar5;
			pValue->field_0x10 = fVar4;
		}
	}

	return;
}

void CActorDCA::FUN_002e6600()
{
	Timer* pTVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	pTVar1 = Timer::GetTimer();
	fVar4 = (this->field_0x164).x - (this->field_0x170).x;
	fVar5 = this->field_0x160 * pTVar1->cutsceneDeltaTime;
	fVar3 = -fVar5;
	if (fVar4 != 0.0f) {
		fVar2 = fVar3;

		if ((fVar3 <= fVar4) && (fVar2 = fVar5, fVar4 <= fVar5)) {
			fVar2 = fVar4;
		}

		fVar2 = (this->field_0x170).x + fVar2;
		(this->field_0x170).x = fVar2;

		if (1.047198f <= fVar2) {
			(this->field_0x170).x = 1.0472f;
		}
		else {
			if (fVar2 < -1.047198f) {
				(this->field_0x170).x = -1.047198f;
			}
		}
	}

	fVar4 = (this->field_0x164).y - (this->field_0x170).y;

	if ((fVar3 <= fVar4) && (fVar3 = fVar5, fVar4 <= fVar5)) {
		fVar3 = fVar4;
	}

	if (fVar3 != 0.0f) {
		fVar3 = (this->field_0x170).y + fVar3;
		(this->field_0x170).y = fVar3;

		if (1.919862f <= fVar3) {
			(this->field_0x170).y = 1.919862f;
		}
		else {
			if (fVar3 < -1.919862f) {
				(this->field_0x170).y = -1.919862f;
			}
		}
	}

	return;
}

void CActorDCA::BehaviourDefault_InitState(int state)
{
	CAnimation* pAnimationController;
	bool bVar1;
	int layerIndex;
	int iVar2;

	pAnimationController = this->pAnimationController;
	if ((pAnimationController != (CAnimation*)0x0) &&
		(bVar1 = pAnimationController->IsLayerActive(2), bVar1 != false)) {
		layerIndex = pAnimationController->PhysicalLayerFromLayerId(2);
		if (state == 6) {
			pAnimationController->anmBinMetaAnimator.SetLayerBlendingOp(layerIndex, 1);
			iVar2 = GetIdMacroAnim(8);
			pAnimationController->anmBinMetaAnimator.SetAnimOnLayer(iVar2, layerIndex, 0xffffffff);

			IMPLEMENTATION_GUARD_AUDIO(
			CActorSound::SoundStart
			((CActorSound*)this->field_0x4fc, (CActor*)this, 2, (CSound*)this->field_0x508, 1, 0,
				(SOUND_SPATIALIZATION_PARAM*)0x0);)
		}
		else {
			if (state == 7) {
				pAnimationController->anmBinMetaAnimator.SetLayerBlendingOp(layerIndex, 1);
				iVar2 = GetIdMacroAnim(8);
				pAnimationController->anmBinMetaAnimator.SetAnimOnLayer(iVar2, layerIndex, 0xffffffff);

				IMPLEMENTATION_GUARD_AUDIO(
				CActorSound::SoundStart
				((CActorSound*)this->field_0x4fc, (CActor*)this, 1, (CSound*)this->field_0x504, 1, 0,
					(SOUND_SPATIALIZATION_PARAM*)0x0);)
			}
			else {
				if (state == 5) {
					pAnimationController->anmBinMetaAnimator.SetLayerBlendingOp(layerIndex, 1);
					(pAnimationController->anmBinMetaAnimator).aAnimData[layerIndex].animPlayState = 0;
				}
			}
		}
	}

	return;
}

edF32VECTOR4 edF32VECTOR4_00425700 =
{
	0.0f, 1.6f, 1.92f, 1.0f
};

void CActorDCA::BehaviourDefault_Manage(CBhvDefault* pBehaviour)
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	undefined8 uVar4;
	bool bVar5;
	bool uVar6;
	edF32VECTOR4* peVar7;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32MATRIX4 auStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR3 local_10;

	bVar5 = pBehaviour->GetShouldDoSomething();
	if (bVar5 != false) {
		FUN_002e6600();

		if (this->field_0x458 != 0) {
			local_10.y = (this->field_0x170).y;
			local_10.z = (this->field_0x170).z;
			local_10.x = (this->field_0x170).x * -0.5f;
			edF32Matrix4FromEulerSoft(&auStack96, &local_10, "XYZ");
			edF32Matrix4MulF32Matrix4Hard(&auStack96, &auStack96, &this->pMeshTransform->base.transformA);
			edF32Matrix4MulF32Vector4Hard(&local_70, &auStack96, &edF32VECTOR4_00425700);
			edF32Vector4NormalizeHard(&local_80, &auStack96.rowY);

			if (&this->field_0x180 != (edF32VECTOR4*)0x0) {
				this->field_0x180 = local_70;
			}

			if (&this->field_0x190 != (edF32VECTOR4*)0x0) {
				this->field_0x190 = local_80;
			}

			if (this->field_0x1a0 == (CActor*)0x0) {
				bVar5 = false;
			}
			else {
				bVar5 = this->field_0x1a0->typeID == 6;
			}

			if (bVar5) {
				IMPLEMENTATION_GUARD(
				uVar6 = CFireShot::FUN_003377b0
				(this->field_0x45c, &this->fireShot, &this->field_0x180, &this->field_0x190, (long)(int)this);)
			}
			else {
				uVar6 = this->fireShot.ProjectDirected(this->field_0x4f0, &this->field_0x180, pBehaviour->GetLocation(), this);
			}

			if (uVar6 != false) {
				SetState(7, -1);
			}

			this->field_0x458 = 0;
			this->field_0x45c = 500.0f;
		}

		iVar1 = this->actorState;
		if (iVar1 == 7) {
			pCVar2 = this->pAnimationController;
			peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
			bVar5 = false;
			if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
				if (peVar3->animPlayState == 0) {
					bVar5 = false;
				}
				else {
					bVar5 = (peVar3->field_0xcc & 2) != 0;
				}
			}

			if (bVar5) {
				SetState(6, -1);
			}
		}
		else {
			if (iVar1 == 6) {
				pCVar2 = this->pAnimationController;
				peVar3 = (pCVar2->anmBinMetaAnimator).aAnimData;
				bVar5 = false;
				if ((peVar3->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
					if (peVar3->animPlayState == 0) {
						bVar5 = false;
					}
					else {
						bVar5 = (peVar3->field_0xcc & 2) != 0;
					}
				}

				if (bVar5) {
					SetState(5, -1);
				}
			}
		}
	}

	if (this->pTiedActor != (CActor*)0x0) {
		local_20 = this->baseLocation;
		CActor::SV_UpdatePosition_Rel(&local_20, 0, 1, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	this->fireShot.ManageShots();

	return;
}

CFireShot::CFireShot()
{
	this->field_0x290 = 0;
	this->field_0x294 = 0;

	return;
}

void CFireShot::Create(ByteCode* pByteCode)
{
	S_ACTOR_STREAM_REF* pSVar1;

	pSVar1 = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * sizeof(S_STREAM_REF<CActor>);
	}

	this->pActorStreamRef = pSVar1;

	return;
}

void CFireShot::Init()
{
	for (int i = 0; i < this->pActorStreamRef->entryCount; i++) {
		this->pActorStreamRef->aEntries[i].Init();
	}

	return;
}

void CFireShot::Reset()
{
	bool bVar1;
	int iVar2;
	int iVar3;
	CActorProjectile* pProj;

	this->field_0x298 = 0;
	this->field_0x29c = 0;

	if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->pActorStreamRef->entryCount;
	}

	iVar3 = iVar2 + -1;

	if (iVar2 != 0) {
		iVar2 = iVar3 * 4;
		do {
			pProj = reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[iVar3].Get());
			if (pProj != (CActorProjectile*)0x0) {
				pProj->GoToSleep();
			}
			iVar2 = iVar2 + -4;
			bVar1 = iVar3 != 0;
			iVar3 = iVar3 + -1;
		} while (bVar1);
	}
	return;
}

void CFireShot::ManageShots()
{
	bool bVar1;
	Timer* pTVar2;
	int iVar3;
	int iVar4;
	uint uVar5;
	uint uVar6;
	S_SHOT_DATA* pSVar7;
	float fVar7;

	pSVar7 = this->aShots;
	uVar6 = 0;
	uVar5 = this->field_0x298;
	iVar4 = 0;
	while (true) {
		if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = 0;
		}
		else {
			iVar3 = this->pActorStreamRef->entryCount;
		}

		if (iVar3 <= (int)uVar6) break;

		if ((uVar5 & 1) != 0) {
			pTVar2 = Timer::GetTimer();
			pSVar7->field_0x18 = pSVar7->field_0x18 + pTVar2->cutsceneDeltaTime;
			bVar1 = false;

			if (this->pActorStreamRef->aEntries[uVar6].Get()->actorState == 6) {
				this->field_0x298 = this->field_0x298 & ~(1 << (uVar6 & 0x1f));
			}
			else {
				if (0.0f < pSVar7->timeToExplode) {
					pTVar2 = Timer::GetTimer();
					fVar7 = pSVar7->timeToExplode - pTVar2->cutsceneDeltaTime;
					pSVar7->timeToExplode = fVar7;

					if ((fVar7 <= 0.0f) && (this->pActorStreamRef->aEntries[uVar6].Get()->actorState != 0xe)) {
						bVar1 = true;
					}
				}

				if (bVar1) {
					reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[uVar6].Get())->Die();
				}
			}
		}

		uVar5 = uVar5 >> 1;
		iVar4 = iVar4 + 4;
		uVar6 = uVar6 + 1;
		pSVar7 = pSVar7 + 1;
	}

	return;
}

bool CFireShot::ProjectDirected(float velocity, edF32VECTOR4* pSource, edF32VECTOR4* pTarget, CActor* pFiringActor)
{
	CActorProjectile* pProjectile;
	edF32VECTOR4 direction;
	S_SHOT_DATA* pShotData;

	edF32Vector4SubHard(&direction, pTarget, pSource);

	pProjectile = reinterpret_cast<CActorProjectile*>(_ComputeNewShotNoRelease(0.1f, pSource, &direction, &pShotData, pFiringActor, false));
	if (pProjectile != (CActorProjectile*)0x0) {
		pProjectile->ProjectDirected(velocity, pSource, pTarget, 1, (CActor*)pFiringActor);
	}

	return pProjectile != (CActorProjectile*)0x0;
}

uint CFireShot::GetProjectileIndex(edF32VECTOR4* pLocation, bool param_3)
{
	S_ACTOR_STREAM_REF* pSVar1;
	bool bVar2;
	uint uVar3;
	uint uVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 eStack16;
	uint puVar3;

	if (this->field_0x290 == 0) {
		pSVar1 = this->pActorStreamRef;
		uVar3 = 0;

		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			uVar3 = pSVar1->entryCount;
		}

		uVar4 = this->field_0x298;

		if (uVar4 != (1 << (uVar3 & 0x1f)) - 1U) {
			uVar3 = 0;
			while (true) {
				if (pSVar1 == (S_ACTOR_STREAM_REF*)0x0) {
					bVar2 = (int)uVar3 < 0;
				}
				else {
					bVar2 = (int)uVar3 < pSVar1->entryCount;
				}

				if ((!bVar2) || ((uVar4 & 1) == 0)) break;

				uVar4 = uVar4 >> 1;
				uVar3 = uVar3 + 1;
			}

			this->field_0x298 = this->field_0x298 | 1 << (uVar3 & 0x1f);
			return uVar3;
		}

		if (param_3 != false) {
			iVar6 = 0;
			if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar6 = pSVar1->entryCount;
			}

			iVar5 = iVar6 + -1;

			if (iVar6 != 0) {
				iVar6 = iVar5 * 4;
				fVar8 = 0.0;
				do {
					edF32Vector4SubHard(&eStack16, &this->pActorStreamRef->aEntries[iVar5].Get()->currentLocation, pLocation);

					fVar7 = edF32Vector4GetLengthSoft(&eStack16);
					if (fVar7 < fVar8) {
						fVar7 = fVar8;
					}

					iVar6 = iVar6 + -4;
					bVar2 = iVar5 != 0;
					iVar5 = iVar5 + -1;
					fVar8 = fVar7;
				} while (bVar2);
			}
		}
	}
	else {
		uVar3 = 1 << (this->field_0x29c & 0x1f);

		if ((this->field_0x298 & uVar3) == 0) {
			this->field_0x298 = this->field_0x298 | uVar3;

			uVar3 = this->field_0x29c;
			this->field_0x29c = uVar3 + 1;

			if (this->pActorStreamRef == (S_ACTOR_STREAM_REF*)0x0) {
				iVar6 = 0;
			}
			else {
				iVar6 = this->pActorStreamRef->entryCount;
			}

			if (iVar6 == 0) {
				trap(7);
			}

			this->field_0x29c = (int)this->field_0x29c % iVar6;
			return uVar3;
		}
	}

	return 0xffffffff;
}

CActor* CFireShot::_ComputeNewShotNoRelease(float velocity, edF32VECTOR4* pLocation, edF32VECTOR4* pDirection, S_SHOT_DATA** pShotData, CActor* pFiringActor, bool param_7)
{
	uint projectileIndex;
	CActorProjectile* pRef;

	projectileIndex = GetProjectileIndex(pLocation, param_7);

	if (projectileIndex == 0xffffffff) {
		pRef = (CActorProjectile*)0x0;
	}
	else {
		pRef = reinterpret_cast<CActorProjectile*>(this->pActorStreamRef->aEntries[projectileIndex].Get());
		*pShotData = this->aShots + projectileIndex;
		edF32Vector4SubHard(&(*pShotData)->field_0x0, pDirection, pLocation);
		edF32Vector4NormalizeHard(&(*pShotData)->field_0x0, &(*pShotData)->field_0x0);

		pRef->pCollisionData->Reset();

		if (pFiringActor != (CActorDCA*)0x0) {
			pRef->pCollisionData->actorFieldA = pFiringActor;
			pFiringActor->pCollisionData->actorFieldA = pRef;
		}

		pRef->UpdatePosition(pLocation, true);

		(*pShotData)->field_0x10 = velocity;
		(*pShotData)->timeToExplode = pRef->field_0x350->timeToExplode;
		(*pShotData)->field_0x18 = 0.0f;
	}

	return pRef;
}

void CActorDCA::CBhvDefault::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CBhvDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorDCA* pDCA;

	this->pOwner = reinterpret_cast<CActorDCA*>(pOwner);

	if (newState == -1) {
		pDCA = this->pOwner;
		pDCA->field_0x458 = 0;
		pDCA->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CActorDCA::CBhvDefault::InitState(int newState)
{
	this->pOwner->BehaviourDefault_InitState(newState);

	return;
}

void CActorDCA::CBhvDefault::TermState(int oldState, int newState)
{
	if ((oldState == 5) && (newState == -1)) {
		this->pOwner->field_0x45b = 0;
	}

	return;
}

int CActorDCA::CBhvDefault::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorDCA* pCVar1;
	ed_3d_hierarchy_node* peVar2;
	undefined8 uVar3;
	int result;
	edF32MATRIX4* peVar4;
	float fVar5;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 local_40;

	if (msg == 0xf) {
		result = 1;
		this->pOwner->field_0x458 = 1;
	}
	else {
		if (msg == 0x28) {
			pCVar1 = this->pOwner;
			if (pMsgParam == (void*)0x0) {
				(pCVar1->field_0x164).y = 0.0f;
				(pCVar1->field_0x164).x = 0.0f;
			}
			else {
				IMPLEMENTATION_GUARD(
				peVar2 = (pCVar1->base).pMeshTransform;
				uVar3 = *(undefined8*)&(peVar2->base).transformA;
				local_40.ac = (peVar2->base).transformA.ac;
				local_40.ad = (peVar2->base).transformA.ad;
				local_40.ba = (peVar2->base).transformA.ba;
				local_40.bb = (peVar2->base).transformA.bb;
				local_40.bc = (peVar2->base).transformA.bc;
				local_40.bd = (peVar2->base).transformA.bd;
				local_40.ca = (peVar2->base).transformA.ca;
				local_40.cb = (peVar2->base).transformA.cb;
				local_40.cc = (peVar2->base).transformA.cc;
				local_40.cd = (peVar2->base).transformA.cd;
				local_40.da = (peVar2->base).transformA.da;
				local_40.db = (peVar2->base).transformA.db;
				local_40.dc = (peVar2->base).transformA.dc;
				local_40.dd = (peVar2->base).transformA.dd;
				local_40.aa = (float)uVar3;
				local_40.ab = (float)((ulong)uVar3 >> 0x20);
				peVar4 = CAnimation::GetCurBoneMatrix((pCVar1->base).pAnimationController, pCVar1->field_0x450);
				edF32Matrix4MulF32Vector4Hard
				(&local_60, (edF32MATRIX4*)(pCVar1->base).pMeshTransform, (edF32VECTOR4*)&peVar4->da);
				local_40.da = local_60.x;
				local_40.db = local_60.y;
				local_40.dc = local_60.z;
				local_40.dd = local_60.w;
				FUN_00265430((long)(int)&local_40);
				edF32Matrix4MulF32Vector4Hard(&local_50, &local_40, (edF32VECTOR4*)pMsgParam);
				edF32Vector4NormalizeHard(&local_50, &local_50);
				fVar5 = atan2f(-local_50.x, local_50.z);
				(pCVar1->field_0x164).y = -fVar5;
				fVar5 = atan2f(local_50.y, SQRT(local_50.z * local_50.z + local_50.x * local_50.x));
				(pCVar1->field_0x164).x = fVar5;)
			}

			result = 1;
		}
		else {
			if (msg == 0x14) {
				if (pMsgParam == (void*)0x0) {
					pCVar1 = this->pOwner;
					if (pSender == (CActor*)0x0) {
						pCVar1->SetBehaviour(2, -1, -1);
						pCVar1->field_0x1a0 = (CActor*)0x0;
					}
					else {
						pCVar1->field_0x1a0 = pSender;
						pCVar1->SetBehaviour(3, -1, -1);
					}
				}
				else {
					pCVar1 = this->pOwner;
					if (pCVar1->field_0x1a0 == pSender) {
						pCVar1->SetBehaviour(2, -1, -1);
						pCVar1->field_0x1a0 = (CActor*)0x0;
					}
				}

				result = 1;
			}
			else {
				if (msg == 0x12) {
					if ((this->pOwner == (CActorDCA*)pSender->pTiedActor) && (this->pOwner->field_0x1a0 == (CActor*)0x0)) {
						result = 1;
					}
					else {
						result = 0;
					}
				}
				else {
					result = 0;
				}
			}
		}
	}

	return result;
}

edF32VECTOR4* CActorDCA::CBhvDefault::GetLocation()
{
	return &CActorHero::_gThis->currentLocation;
}

bool CActorDCA::CBhvDefault::GetShouldDoSomething()
{
	return true;
}

void CActorDCA::CBhvControlled::Create(ByteCode* pByteCode)
{
	this->field_0x10 = pByteCode->GetF32();
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x18 = pByteCode->GetF32();
	this->field_0x1c = pByteCode->GetF32() * 0.01745329f;
	this->field_0x20 = pByteCode->GetF32();
	this->field_0x24 = pByteCode->GetF32();
	this->field_0x28 = pByteCode->GetF32();
	this->field_0x2c = 1.0f;
	this->field_0x30 = pByteCode->GetF32();
	this->field_0x34 = pByteCode->GetF32();
	this->field_0x38 = pByteCode->GetF32();
	this->field_0x3c = 1.0f;

	return;
}

void CActorDCA::CBhvControlled::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CBhvControlled::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CBhvControlled::End(int newBehaviourId)
{
	IMPLEMENTATION_GUARD();
}

int CActorDCA::CBhvControlled::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CBhvIAControlled::Create(ByteCode* pByteCode)
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CBhvIAControlled::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CActorDCA::CBhvTrajectory::Create(ByteCode* pByteCode)
{
	(this->actorRef).index = pByteCode->GetS32();
	this->field_0x18 = pByteCode->GetF32();
	this->field_0x8 = pByteCode->GetU32();
	this->field_0xc = 0;

	return;
}

void CActorDCA::CBhvTrajectory::Init(CActor* pOwner)
{
	this->actorRef.Init();

	return;
}

void CActorDCA::CBhvTrajectory::Manage()
{
	CActorDCA* pDCA;
	CActor* pTiedActor;
	ed_3d_hierarchy_node* peVar1;
	int iVar2;
	undefined8 uVar3;
	bool bVar4;
	bool bVar5;
	edF32VECTOR4* pfVar6;
	edF32MATRIX4* peVar7;
	long lVar9;
	float fVar10;
	edF32VECTOR4 local_170;
	edF32VECTOR4 local_160;
	edF32MATRIX4 local_150;
	edF32VECTOR4 local_110;
	edF32VECTOR4 local_100;
	edF32MATRIX4 auStack240;
	edF32VECTOR4 eStack176;
	edF32VECTOR4 eStack160;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR3 local_10;
	undefined4 local_4;

	pDCA = this->pOwner;
	lVar9 = GetShouldDoSomething();
	if (lVar9 != 0) {
		pfVar6 = GetLocation();
		local_20 = *pfVar6;

		if (((this->field_0x8 & 2) != 0) && (pTiedActor = pDCA->pTiedActor, pTiedActor != (CActor*)0x0)) {
			pTiedActor->SV_ComputeDiffMatrixFromInit(&eStack96);
			edF32Matrix4MulF32Vector4Hard(&local_20, &eStack96, &local_20);
		}

		if (false) { // (undefined*)register0x000001d0 == (undefined*)0x20) {
			(pDCA->field_0x164).y = 0.0f;
			(pDCA->field_0x164).x = 0.0f;
		}
		else {
			local_10.y = (pDCA->field_0x170).y;
			local_10.z = (pDCA->field_0x170).z;
			local_10.x = (pDCA->field_0x170).x * -0.5f;
			edF32Matrix4FromEulerSoft(&auStack240, &local_10, "XYZ");
			edF32Matrix4MulF32Matrix4Hard(&auStack240, &auStack240, &pDCA->pMeshTransform->base.transformA);
			edF32Matrix4MulF32Vector4Hard(&local_100, &auStack240, &edF32VECTOR4_00425700);
			edF32Vector4NormalizeHard(&local_110, &auStack240.rowZ);

			local_70 = local_100;
			local_80 = local_110;

			edF32Vector4SubHard(&eStack144, &local_20, &local_70);

			pDCA->vectorDyn.BuildFromAccelDistAmplitude(1.0f, &CDynamicExt::gForceGravity, &eStack144, 1);
			pDCA->vectorDyn.Integrate(0.05f);
			edF32Vector4NormalizeHard(&eStack160, &pDCA->vectorDyn.field_0x60);
			fVar10 = edF32Vector4GetDistHard(&eStack144);
			edF32Vector4ScaleHard(fVar10, &eStack160, &eStack160);
			edF32Vector4AddHard(&eStack176, &local_70, &eStack160);
			if (false) { // ((undefined*)register0x000001d0 == (undefined*)0xb0) {
				(pDCA->field_0x164).y = 0.0f;
				(pDCA->field_0x164).x = 0.0f;
			}
			else {
				local_150 = pDCA->pMeshTransform->base.transformA;

				peVar7 = pDCA->pAnimationController->GetCurBoneMatrix(pDCA->field_0x450);
				edF32Matrix4MulF32Vector4Hard(&local_170, &pDCA->pMeshTransform->base.transformA, &peVar7->rowT);
				local_150.rowT = local_170;
				edF32Matrix4InverseOrthoSoft(&local_150);
				edF32Matrix4MulF32Vector4Hard(&local_160, &local_150, &eStack176);
				edF32Vector4NormalizeHard(&local_160, &local_160);
				fVar10 = atan2f(-local_160.x, local_160.z);
				(pDCA->field_0x164).y = -fVar10;
				fVar10 = atan2f(local_160.y, sqrtf(local_160.z * local_160.z + local_160.x * local_160.x));
				(pDCA->field_0x164).x = fVar10;
			}
		}

		fVar10 = Timer::GetTimer()->cutsceneDeltaTime;
		iVar2 = this->pOwner->actorState;
		fVar10 = (Timer::GetTimer()->scaledTotalTime - fVar10) - this->field_0x10;

		if ((iVar2 == 6) || (iVar2 == 7)) {
			bVar5 = false;
		}
		else {
			if (fVar10 <= Timer::GetTimer()->cutsceneDeltaTime) {
				bVar5 = true;
			}
			else {
				bVar5 = false;

				if (this->pOwner->actorState == 5) {
					bVar4 = true;

					if ((0.0f < fVar10) && ((int)(fVar10 / this->field_0x18) ==
							(int)((Timer::GetTimer()->scaledTotalTime - this->field_0x10) / this->field_0x18))) {
						bVar4 = false;
					}

					if (bVar4) {
						bVar5 = true;
					}
				}
			}
		}

		if (bVar5) {
			local_4 = 0;
			pDCA->DoMessage(pDCA, (ACTOR_MESSAGE)0xf, 0);
			this->field_0x1c = Timer::GetTimer()->scaledTotalTime;
		}
	}

	pDCA->BehaviourDefault_Manage(this);

	return;
}

void CActorDCA::CBhvTrajectory::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	Timer* pTimer;
	CActorDCA* pDCA;

	this->pOwner = reinterpret_cast<CActorDCA*>(pOwner);

	if (newState == -1) {
		pDCA = this->pOwner;
		pDCA->field_0x458 = 0;
		pDCA->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	pTimer = Timer::GetTimer();
	this->field_0x1c = pTimer->scaledTotalTime - this->field_0x18;

	if ((this->field_0x8 & 1) != 0) {
		this->field_0xc = 1;
		pTimer = Timer::GetTimer();
		this->field_0x10 = pTimer->scaledTotalTime;
		this->pOwner->flags = this->pOwner->flags | 0x800;
	}

	return;
}

void CActorDCA::CBhvTrajectory::End(int newBehaviourId)
{
	this->field_0xc = 0;
	this->pOwner->flags = this->pOwner->flags & 0xfffff7ff;

	return;
}

int CActorDCA::CBhvTrajectory::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	Timer* pTimer;
	int result;

	if (msg == 0x10) {
		if (this->field_0xc == 0) {
			CBhvDefault::InterpretMessage(pSender, 0x10, pMsgParam);
		}
		else {
			this->field_0xc = 0;
			this->pOwner->flags = this->pOwner->flags & 0xfffff7ff;
		}

		result = 1;
	}
	else {
		if (msg == 0xf) {
			if (this->field_0xc == 0) {
				this->field_0xc = 1;
				pTimer = Timer::GetTimer();
				this->field_0x10 = pTimer->scaledTotalTime;
				this->pOwner->flags = this->pOwner->flags | 0x800;
			}
			else {
				CBhvDefault::InterpretMessage(pSender, 0xf, pMsgParam);
			}

			result = 1;
		}
		else {
			result = CBhvDefault::InterpretMessage(pSender, msg, pMsgParam);
		}
	}

	return result;
}

edF32VECTOR4* CActorDCA::CBhvTrajectory::GetLocation()
{
	CActor* pCVar1;
	edF32VECTOR4* peVar2;

	pCVar1 = (this->actorRef).Get();

	if (pCVar1 == (CActor*)0x0) {
		peVar2 = &gF32Vertex4Zero;
	}
	else {
		peVar2 = &pCVar1->currentLocation;
	}

	return peVar2;
}

bool CActorDCA::CBhvTrajectory::GetShouldDoSomething()
{
	return this->field_0xc;
}
