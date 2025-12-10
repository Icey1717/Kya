#include "ActorDCA.h"
#include "MemoryStream.h"
#include "CollisionManager.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include "ActorProjectile.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "InputManager.h"

void CActorDCA::Create(ByteCode* pByteCode)
{
	CCollision* pCVar1;
	float fVar2;
	int iVar3;
	uint uVar4;
	CActorSound* pCVar5;
	float fVar6;

	CActor::Create(pByteCode);

	this->pCamera = (CCameraDCA*)0x0;

	this->field_0x504.index = pByteCode->GetS32();
	this->field_0x500.index = pByteCode->GetS32();

	this->field_0x464 = pByteCode->GetF32();

	this->field_0x508.index = pByteCode->GetS32();
	this->field_0x50c.index = pByteCode->GetS32();

	this->aimAdjustSpeed = pByteCode->GetF32() * 0.01745329f;

	this->fireShot.Create(pByteCode);

	this->field_0x450 = pByteCode->GetU32();
	this->field_0x454 = pByteCode->GetU32();

	pCVar1 = this->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 & 0xffffffc4;
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x800480;
	}

	this->field_0x4fc = CreateActorSound(4);

	this->aimInputDirection = gF32Vertex4Zero.xyz;
	this->aimDirection = gF32Vertex4Zero.xyz;
	this->pControlledByActor = (CActor*)0x0;
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
	this->pCamera = static_cast<CCameraDCA*>(CCameraManager::_gThis->GetDefGameCamera(CT_DCA));

	if (this->pCamera == (CCamera*)0x0) {
		ByteCode BStack16;
		this->pCamera = static_cast<CCameraDCA*>(pCameraManager->AddCamera(CT_DCA, &BStack16, "DCA"));
		this->pCamera->Init();
	}

	this->pAnimationController->RegisterBone(this->field_0x450);

	CActor::Init();

	this->fireShot.Init();

	this->field_0x500.Init();
	this->field_0x504.Init();
	this->field_0x508.Init();
	this->field_0x50c.Init();

	this->aimInputDirection.x = 0.0f;
	this->aimInputDirection.y = 0.0f;
	this->aimInputDirection.z = 0.0f;

	(this->aimDirection).x = 0.0f;
	(this->aimDirection).y = 0.0f;
	(this->aimDirection).z = 0.0f;

	this->pControlledByActor = (CActor*)0x0;
	this->field_0x460 = 0.0f;
	this->field_0x45c = 0.0f;

	this->projectAimLocation = gF32Vertex4Zero;
	this->projectAimDirection = gF32Vertex4Zero;
	this->aimDirection = gF32Vertex4Zero.xyz;

	this->field_0x459 = 0;

	this->fireShot.Reset();
	(this->fireShot).field_0x290 = 1;

	return;
}

void CActorDCA::Term()
{
	this->pAnimationController->UnRegisterBone(this->field_0x450);
	CActor::Term();

	return;
}

void CActorDCA::Reset()
{
	CActor::Reset();
	(this->aimInputDirection).x = 0.0f;
	(this->aimInputDirection).y = 0.0f;
	(this->aimInputDirection).z = 0.0f;

	(this->aimDirection).x = 0.0f;
	(this->aimDirection).y = 0.0f;
	(this->aimDirection).z = 0.0f;

	this->pControlledByActor = (CActor*)0x0;
	this->field_0x460 = 0.0f;
	this->field_0x45c = 0.0f;
	
	this->projectAimLocation = gF32Vertex4Zero;
	this->projectAimDirection = gF32Vertex4Zero;
	this->aimDirection = gF32Vertex4Zero.xyz;

	this->field_0x459 = 0;
	this->fireShot.Reset();
	(this->fireShot).field_0x290 = 1;

	return;
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

void CActorDCA::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edANM_HDR* peVar1;
	float fVar3;
	float fVar4;
	float fVar5;

	if (layerId == 8) {
		if (newAnim == 9) {
			char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
			AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

			fVar3 = 0.0f;
			if (this->field_0x460 != 0.0f) {
				fVar3 = (this->behaviourControlled).controlledParams.field_0x0;
				fVar3 = ((fVar3 + (GetTimer()->scaledTotalTime - this->field_0x460) * (this->behaviourControlled).controlledParams.field_0x8) - fVar3) / ((this->behaviourControlled).controlledParams.field_0x4 - fVar3);
			}

			fVar4 = 1.0f;
			if (fVar3 <= 1.0f) {
				fVar4 = fVar3;
			}

			if (fVar4 < 0.0f) {
				fVar4 = 0.0f;
			}

			pValue->field_0xc = 1.0f - fVar4;
			pValue->field_0x10 = fVar4;
		}
	}
	else {
		if ((layerId == 2) || (layerId == 1)) {
			char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
			AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

			fVar4 = ((this->aimDirection).x * 0.5f) / 3.141593f + 0.5f;
			fVar3 = ((this->aimDirection).y * 0.5f) / 3.141593f + 0.5f;

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

void CActorDCA::UpdateAimAngle()
{
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	fVar4 = (this->aimInputDirection).x - (this->aimDirection).x;
	fVar5 = this->aimAdjustSpeed * Timer::GetTimer()->cutsceneDeltaTime;
	fVar3 = -fVar5;
	if (fVar4 != 0.0f) {
		fVar2 = fVar3;

		if ((fVar3 <= fVar4) && (fVar2 = fVar5, fVar4 <= fVar5)) {
			fVar2 = fVar4;
		}

		fVar2 = (this->aimDirection).x + fVar2;
		(this->aimDirection).x = fVar2;

		if (1.047198f <= fVar2) {
			(this->aimDirection).x = 1.0472f;
		}
		else {
			if (fVar2 < -1.047198f) {
				(this->aimDirection).x = -1.047198f;
			}
		}
	}

	fVar4 = (this->aimInputDirection).y - (this->aimDirection).y;

	if ((fVar3 <= fVar4) && (fVar3 = fVar5, fVar4 <= fVar5)) {
		fVar3 = fVar4;
	}

	if (fVar3 != 0.0f) {
		fVar3 = (this->aimDirection).y + fVar3;
		(this->aimDirection).y = fVar3;

		if (1.919862f <= fVar3) {
			(this->aimDirection).y = 1.919862f;
		}
		else {
			if (fVar3 < -1.919862f) {
				(this->aimDirection).y = -1.919862f;
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
	edF32VECTOR4 aimDirection;
	edF32VECTOR4 aimLocation;
	edF32MATRIX4 auStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR3 adjustedAimDirection;

	bVar5 = pBehaviour->GetShouldDoSomething();
	if (bVar5 != false) {
		UpdateAimAngle();

		if (this->bIsFiring != 0) {
			adjustedAimDirection.y = (this->aimDirection).y;
			adjustedAimDirection.z = (this->aimDirection).z;
			adjustedAimDirection.x = (this->aimDirection).x * -0.5f;

			edF32Matrix4FromEulerSoft(&auStack96, &adjustedAimDirection, "XYZ");
			edF32Matrix4MulF32Matrix4Hard(&auStack96, &auStack96, &this->pMeshTransform->base.transformA);
			edF32Matrix4MulF32Vector4Hard(&aimLocation, &auStack96, &edF32VECTOR4_00425700);
			edF32Vector4NormalizeHard(&aimDirection, &auStack96.rowZ);

			if (&this->projectAimLocation != (edF32VECTOR4*)0x0) {
				this->projectAimLocation = aimLocation;
			}

			if (&this->projectAimDirection != (edF32VECTOR4*)0x0) {
				this->projectAimDirection = aimDirection;
			}

			if (this->pControlledByActor == (CActor*)0x0) {
				bVar5 = false;
			}
			else {
				bVar5 = this->pControlledByActor->typeID == ACTOR_HERO_PRIVATE;
			}

			if (bVar5) {
				uVar6 = this->fireShot.Project(this->field_0x45c, &this->projectAimLocation, &this->projectAimDirection, this);
			}
			else {
				uVar6 = this->fireShot.ProjectDirected(this->field_0x4f0, &this->projectAimLocation, pBehaviour->GetLocation(), this);
			}

			if (uVar6 != false) {
				SetState(7, -1);
			}

			this->bIsFiring = 0;
			this->field_0x45c = 500.0f;
		}

		iVar1 = this->actorState;
		if (iVar1 == 7) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				SetState(6, -1);
			}
		}
		else {
			if (iVar1 == 6) {
				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
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

void CActorDCA::BehaviourControlled_Manage(CBhvControlled* pBehaviour, CONTROLLED_PARAMS* pParams)
{
	CCameraDCA* pCVar1;
	CPlayerInput* pPlayerInput;
	edF32MATRIX4* peVar4;
	float lx;
	float delta;
	float ly;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32MATRIX4 auStack160;
	edF32MATRIX4 local_60;
	edF32VECTOR4 local_20;
	edF32VECTOR3 local_10;
	bool bControlledByHero;
	CCameraManager* pCameraManager;

	BehaviourDefault_Manage(pBehaviour);

	if (this->pControlledByActor == (CActor*)0x0) {
		bControlledByHero = false;
	}
	else {
		bControlledByHero = this->pControlledByActor->typeID == ACTOR_HERO_PRIVATE;
	}

	if ((bControlledByHero) && (pPlayerInput = this->pControlledByActor->GetInputManager(0, 0), pPlayerInput != (CPlayerInput*)0x0)) {
		ToggleMeshAlpha();
		SetBFCulling(0);
		ly = pPlayerInput->aAnalogSticks[0].y;
		lx = pPlayerInput->aAnalogSticks[0].x;

		delta = GetTimer()->cutsceneDeltaTime;

		if ((0.001f < fabs(lx)) || (0.001f < fabs(ly))) {
			if (this->field_0x45a == 0) {
				IMPLEMENTATION_GUARD_AUDIO(
				CActorSound::SoundStart(this->field_0x4fc, 3, this->field_0x50c.pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);)
				this->field_0x45a = 1;
			}
		}
		else {
			if (this->field_0x45a != 0) {
				IMPLEMENTATION_GUARD_AUDIO(
				CActorSound::FadeTo(0.0f, -2.0f, 0.1f, this->field_0x4fc, 3);)
				this->field_0x45a = 0;
			}
		}

		(this->aimDirection).x = (this->aimDirection).x + ly * this->aimAdjustSpeed * delta;
		if (1.047198f <= (this->aimDirection).x) {
			(this->aimDirection).x = 1.047198f;
		}
		else {
			if ((this->aimDirection).x < -1.047198f) {
				(this->aimDirection).x = -1.047198f;
			}
		}

		(this->aimDirection).y = (this->aimDirection).y - lx * this->aimAdjustSpeed * delta;
		if (1.919862f <= (this->aimDirection).y) {
			(this->aimDirection).y = 1.919862f;
		}
		else {
			if ((this->aimDirection).y < -1.919862f) {
				(this->aimDirection).y = -1.919862f;
			}
		}

		this->aimInputDirection = this->aimDirection;
		local_10.y = (this->aimDirection).y;
		local_10.z = (this->aimDirection).z;
		local_10.x = (this->aimDirection).x * -0.5f;

		edF32Matrix4FromEulerSoft(&auStack160, &local_10, "XYZ");
		edF32Matrix4MulF32Matrix4Hard(&auStack160, &auStack160, &this->pMeshTransform->base.transformA);
		edF32Matrix4MulF32Vector4Hard(&local_b0, &auStack160, &edF32VECTOR4_00425700);
		edF32Vector4NormalizeHard(&local_c0, &auStack160.rowZ);
		if (&this->projectAimLocation != (edF32VECTOR4*)0x0) {
			this->projectAimLocation = local_b0;
		}

		if (&this->projectAimDirection != (edF32VECTOR4*)0x0) {
			this->projectAimDirection = local_c0;
		}

		local_60 = gF32Matrix4Unit;
		peVar4 = this->pAnimationController->GetCurBoneMatrix(this->field_0x450);
		edF32Matrix4MulF32Vector4Hard(&local_20, &this->pMeshTransform->base.transformA, &peVar4->rowT);
		edF32Vector4CrossProductHard(&local_60.rowX, &gF32Vector4UnitY, &peVar4->rowZ);
		edF32Vector4NormalizeHard(&local_60.rowX, &local_60.rowX);
		edF32Vector4CrossProductHard(&local_60.rowY, &peVar4->rowZ, &local_60.rowX);
		edF32Vector4NormalizeHard(&local_60.rowY, &local_60.rowY);
		local_60.rowZ = peVar4->rowZ;
		edF32Vector4NormalizeHard(&local_60.rowZ, &local_60.rowZ);
		edF32Matrix4MulF32Matrix4Hard(&local_60, &local_60, &this->pMeshTransform->base.transformA);
		edF32Vector4NormalizeHard(&local_60.rowX, &local_60.rowX);
		edF32Vector4NormalizeHard(&local_60.rowY, &local_60.rowY);
		edF32Vector4NormalizeHard(&local_60.rowZ, &local_60.rowZ);
		local_60.rowT = local_20;
		edF32Matrix4MulF32Vector4Hard(&local_60.rowT, &local_60, &pParams->field_0x10);
		pCVar1 = this->pCamera;
		pCVar1->field_0xb0 = local_60.rowT;
		edF32Matrix4MulF32Vector4Hard(&local_60.rowT, &local_60, &pParams->field_0x20);
		pCVar1 = this->pCamera;
		pCVar1->lookAt = local_60.rowT;
		pCameraManager = CCameraManager::_gThis;
		if (this->field_0x4f8 == 0) {
			this->pCamera->fov = pParams->fov;
			this->pCamera->SetOtherTarget(this->pTiedActor);
			pCameraManager->PushCamera(this->pCamera, 1);
			this->field_0x4f8 = 1;
		}
		else {
			if (this->field_0x45b == 0) {
				if (pPlayerInput->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue == 0.0f) {
					this->field_0x45b = 1;
				}
			}
			else {
				if (pPlayerInput->aButtons[INPUT_BUTTON_INDEX_SQUARE].clickValue == 0.0f) {
					if ((this->field_0x459 != 0) && (this->actorState != 6)) {
						this->field_0x459 = 0;
						lx = GetTimer()->scaledTotalTime - this->field_0x460;
						this->field_0x460 = lx;
						lx = pParams->field_0x0 + lx * pParams->field_0x8;
						this->field_0x45c = lx;

						if (pParams->field_0x4 < lx) {
							this->field_0x45c = pParams->field_0x4;
						}

						this->bIsFiring = 1;
						this->field_0x460 = 0.0;
						IMPLEMENTATION_GUARD_AUDIO(
						CActorSound::FadeTo(1.0f, 1.0f, 0.0f, this->field_0x4fc, 0);)
					}
				}
				else {
					if (this->field_0x459 == 0) {
						this->field_0x459 = 1;
						if (this->field_0x460 == 0.0f) {
							this->field_0x460 = GetTimer()->scaledTotalTime;
						}
						if ((0.0f < pParams->field_0x8) && (CScene::ptable.g_AudioManager_00451698 != (CAudioManager*)0x0)) {
							IMPLEMENTATION_GUARD_AUDIO(
							CActorSound::FadeTo(1.0f, this->field_0x464, (pParams->field_0x4 - pParams->field_0x0) / pParams->field_0x8, this->field_0x4fc, 0);)
						}
					}
				}
			}
		}
	}

	return;
}

void CActorDCA::CBhvDefault::Manage()
{
	this->pOwner->BehaviourDefault_Manage(this);

	return;
}

void CActorDCA::CBhvDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorDCA* pDCA;

	this->pOwner = reinterpret_cast<CActorDCA*>(pOwner);

	if (newState == -1) {
		pDCA = this->pOwner;
		pDCA->bIsFiring = 0;
		pDCA->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorDCA.h
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
	CActorDCA* pDCA;
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
		this->pOwner->bIsFiring = 1;
	}
	else {
		if (msg == 0x28) {
			pDCA = this->pOwner;
			if (pMsgParam == (void*)0x0) {
				(pDCA->aimInputDirection).y = 0.0f;
				(pDCA->aimInputDirection).x = 0.0f;
			}
			else {
				IMPLEMENTATION_GUARD(
				peVar2 = (pDCA->base).pMeshTransform;
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
				peVar4 = CAnimation::GetCurBoneMatrix((pDCA->base).pAnimationController, pDCA->field_0x450);
				edF32Matrix4MulF32Vector4Hard
				(&local_60, (edF32MATRIX4*)(pDCA->base).pMeshTransform, (edF32VECTOR4*)&peVar4->da);
				local_40.da = local_60.x;
				local_40.db = local_60.y;
				local_40.dc = local_60.z;
				local_40.dd = local_60.w;
				FUN_00265430((long)(int)&local_40);
				edF32Matrix4MulF32Vector4Hard(&local_50, &local_40, (edF32VECTOR4*)pMsgParam);
				edF32Vector4NormalizeHard(&local_50, &local_50);
				fVar5 = atan2f(-local_50.x, local_50.z);
				(pDCA->aimInputDirection).y = -fVar5;
				fVar5 = atan2f(local_50.y, SQRT(local_50.z * local_50.z + local_50.x * local_50.x));
				(pDCA->aimInputDirection).x = fVar5;)
			}

			result = 1;
		}
		else {
			if (msg == 0x14) {
				const int bState = static_cast<int>(reinterpret_cast<uintptr_t>(pMsgParam));
				if (bState == 0) {
					pDCA = this->pOwner;
					if (pSender == (CActor*)0x0) {
						pDCA->SetBehaviour(DCA_BEHAVIOUR_DEFAULT, -1, -1);
						pDCA->pControlledByActor = (CActor*)0x0;
					}
					else {
						pDCA->pControlledByActor = pSender;
						pDCA->SetBehaviour(DCA_BEHAVIOUR_CONTROLLED, -1, -1);
					}
				}
				else {
					pDCA = this->pOwner;
					if (pDCA->pControlledByActor == pSender) {
						pDCA->SetBehaviour(DCA_BEHAVIOUR_DEFAULT, -1, -1);
						pDCA->pControlledByActor = (CActor*)0x0;
					}
				}

				result = 1;
			}
			else {
				if (msg == MESSAGE_GET_ACTION) {
					if ((this->pOwner == pSender->pTiedActor) && (this->pOwner->pControlledByActor == (CActor*)0x0)) {
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
	this->controlledParams.field_0x0 = pByteCode->GetF32();
	this->controlledParams.field_0x4 = pByteCode->GetF32();
	this->controlledParams.field_0x8 = pByteCode->GetF32();
	this->controlledParams.fov = pByteCode->GetF32() * 0.01745329f;

	this->controlledParams.field_0x10.x = pByteCode->GetF32();
	this->controlledParams.field_0x10.y = pByteCode->GetF32();
	this->controlledParams.field_0x10.z = pByteCode->GetF32();
	this->controlledParams.field_0x10.w = 1.0f;

	this->controlledParams.field_0x20.x = pByteCode->GetF32();
	this->controlledParams.field_0x20.y = pByteCode->GetF32();
	this->controlledParams.field_0x20.z = pByteCode->GetF32();
	this->controlledParams.field_0x20.w = 1.0f;

	return;
}

void CActorDCA::CBhvControlled::Manage()
{
	this->pOwner->BehaviourControlled_Manage(this, &this->controlledParams);

	return;
}

void CActorDCA::CBhvControlled::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorDCA* pDCA;
	CActor* pActor;
	CAnimation* pAnimationController;
	bool bControlledByHero;
	int mode;
	int layerIndex;

	this->pOwner = static_cast<CActorDCA*>(pOwner);

	if (newState == -1) {
		pDCA = this->pOwner;
		pActor = pDCA->pControlledByActor;
		if (pActor == (CActor*)0x0) {
			bControlledByHero = false;
		}
		else {
			bControlledByHero = pActor->typeID == ACTOR_HERO_PRIVATE;
		}

		if (bControlledByHero) {
			pDCA->field_0x4f8 = 0;

			IMPLEMENTATION_GUARD_AUDIO(
			CActorSound::SoundStart
			((CActorSound*)pDCA->field_0x4fc, (CActor*)pDCA, 0, (CSound*)pDCA->field_0x500, 1, 0,
				(SOUND_SPATIALIZATION_PARAM*)0x0);)
		}

		pDCA->bIsFiring = 0;
		pDCA->field_0x460 = 0.0f;
		pDCA->field_0x45a = 0;

		pDCA->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	bool bIsLayerActive;

	pDCA = this->pOwner;
	pAnimationController = pDCA->pAnimationController;
	if (((pAnimationController != (CAnimation*)0x0) &&
		(bIsLayerActive = pAnimationController->IsLayerActive(8), bIsLayerActive != false)) &&
		(mode = pDCA->GetIdMacroAnim(9), mode != -1)) {
		layerIndex = pAnimationController->PhysicalLayerFromLayerId(8);
		pAnimationController->anmBinMetaAnimator.SetLayerBlendingOp(layerIndex, 3);
		(pAnimationController->anmBinMetaAnimator).aAnimData[layerIndex].field_0x4 = 1.0f;
		pAnimationController->anmBinMetaAnimator.SetAnimOnLayer(mode, layerIndex, 9);
	}

	return;
}

void CActorDCA::CBhvControlled::End(int newBehaviourId)
{
	CAnimation* pAnimationController;
	CActorDCA* pCVar1;
	bool bControlledByHero;
	int layerIndex;
	float fVar3;
	float fVar4;

	pAnimationController = this->pOwner->pAnimationController;
	if ((pAnimationController != (CAnimation*)0x0) &&
		(bControlledByHero = pAnimationController->IsLayerActive(8), bControlledByHero != false)) {
		layerIndex = pAnimationController->PhysicalLayerFromLayerId(8);
		pAnimationController->anmBinMetaAnimator.SetAnimOnLayer(-1, layerIndex, 0xffffffff);
	}

	pCVar1 = this->pOwner;
	if (pCVar1->pControlledByActor == (CActor*)0x0) {
		bControlledByHero = false;
	}
	else {
		bControlledByHero = pCVar1->pControlledByActor->typeID == ACTOR_HERO_PRIVATE;
	}

	if (bControlledByHero) {
		if (pCVar1->field_0x4f8 != 0) {
			CCameraManager::_gThis->PopCamera(pCVar1->pCamera);
		}

		pCVar1->field_0x4fc->SoundStop(0);
		pCVar1->field_0x4fc->SoundStop(3);
	}

	pCVar1->bIsFiring = 0;
	pCVar1->aimInputDirection.y = 0.0f;
	pCVar1->aimInputDirection.x = 0.0f;

	pCVar1->aimDirection = pCVar1->aimInputDirection;

	return;
}

int CActorDCA::CBhvControlled::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorDCA* pDCA;
	CActor* pCVar2;
	ed_3d_hierarchy_node* peVar3;
	bool bControlledByHero;
	undefined8 uVar5;
	edF32MATRIX4* peVar6;
	uint uVar7;
	float fVar8;
	edF32VECTOR4 local_120;
	edF32VECTOR4 local_110;
	edF32MATRIX4 local_100;
	edF32VECTOR4 local_c0;
	edF32VECTOR4 local_b0;
	edF32MATRIX4 auStack160;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR3 local_10;

	if (msg == 0xf) {
		pCVar2 = this->pOwner->pControlledByActor;
		if (pCVar2 == (CActor*)0x0) {
			bControlledByHero = false;
		}
		else {
			bControlledByHero = pCVar2->typeID == ACTOR_HERO_PRIVATE;
		}

		if (!bControlledByHero) {
			this->pOwner->bIsFiring = 1;
		}

		uVar7 = 1;
	}
	else {
		if (msg == 0x28) {
			pCVar2 = this->pOwner->pControlledByActor;
			uVar7 = 1;
			if (pCVar2 == (CActor*)0x0) {
				bControlledByHero = false;
			}
			else {
				bControlledByHero = pCVar2->typeID == ACTOR_HERO_PRIVATE;
			}

			if (!bControlledByHero) {
				pDCA = this->pOwner;
				if (pMsgParam == (edF32VECTOR4*)0x0) {
					(pDCA->aimInputDirection).y = 0.0f;
					(pDCA->aimInputDirection).x = 0.0f;
					fVar8 = fabs((pDCA->aimDirection).x - (pDCA->aimInputDirection).x) + fabs((pDCA->aimDirection).y - (pDCA->aimInputDirection).y);
				}
				else {
					local_10.y = (pDCA->aimDirection).y;
					local_10.z = (pDCA->aimDirection).z;
					local_10.x = (pDCA->aimDirection).x * -0.5f;
					edF32Matrix4FromEulerSoft(&auStack160, &local_10, "XYZ");
					edF32Matrix4MulF32Matrix4Hard(&auStack160, &auStack160, &pDCA->pMeshTransform->base.transformA);

					edF32Matrix4MulF32Vector4Hard(&local_b0, &auStack160, &edF32VECTOR4_00425700);
					edF32Vector4NormalizeHard(&local_c0, &auStack160.rowZ);
					local_20 = local_b0;
					local_30 = local_c0;
					edF32Vector4SubHard(&eStack64, reinterpret_cast<edF32VECTOR4*>(pMsgParam), &local_20);
					pDCA->vectorDyn.BuildFromAccelDistAmplitude(1.0f, &CDynamicExt::gForceGravity, &eStack64, 1);
					pDCA->vectorDyn.Integrate(0.05f);
					edF32Vector4NormalizeHard(&eStack80, &(pDCA->vectorDyn).instanceIndex);
					fVar8 = edF32Vector4GetDistHard(&eStack64);
					edF32Vector4ScaleHard(fVar8, &eStack80, &eStack80);
					edF32Vector4AddHard(&eStack96, &local_20, &eStack80);
					if (pMsgParam == (edF32VECTOR4*)0x0) {
						(pDCA->aimInputDirection).y = 0.0;
						(pDCA->aimInputDirection).x = 0.0;
					}
					else {
						local_100 = pDCA->pMeshTransform->base.transformA;
						peVar6 = pDCA->pAnimationController->GetCurBoneMatrix(pDCA->field_0x450);
						edF32Matrix4MulF32Vector4Hard(&local_120, &pDCA->pMeshTransform->base.transformA, &peVar6->rowT);
						local_100.rowT = local_120;
						edF32Matrix4InverseOrthoSoft(&local_100);
						edF32Matrix4MulF32Vector4Hard(&local_110, &local_100, &eStack96);
						edF32Vector4NormalizeHard(&local_110, &local_110);
						fVar8 = atan2f(-local_110.x, local_110.z);
						(pDCA->aimInputDirection).y = -fVar8;
						fVar8 = atan2f(local_110.y, sqrtf(local_110.z * local_110.z + local_110.x * local_110.x));
						(pDCA->aimInputDirection).x = fVar8;
					}

					fVar8 = fabs((pDCA->aimDirection).x - (pDCA->aimInputDirection).x) +
						fabs((pDCA->aimDirection).y - (pDCA->aimInputDirection).y);
				}

				uVar7 = (uint)(fVar8 < 0.01);
			}
		}
		else {
			if (msg == 0x14) {
				if (pMsgParam != (void*)0x0) {
					this->pOwner->field_0x459 = 0;
					pDCA = this->pOwner;
					pDCA->SetBehaviour(DCA_BEHAVIOUR_DEFAULT, -1, -1);
					pDCA->pControlledByActor = (CActor*)0x0;
				}
			}
			else {
				if (msg == 0x12) {
					if (this->pOwner->pControlledByActor != (CActor*)0x0) {
						return 0;
					}

					return 1;
				}
			}

			uVar7 = 0;
		}
	}

	return uVar7;
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
	if (GetShouldDoSomething() != 0) {
		pfVar6 = GetLocation();
		local_20 = *pfVar6;

		if (((this->field_0x8 & 2) != 0) && (pTiedActor = pDCA->pTiedActor, pTiedActor != (CActor*)0x0)) {
			pTiedActor->SV_ComputeDiffMatrixFromInit(&eStack96);
			edF32Matrix4MulF32Vector4Hard(&local_20, &eStack96, &local_20);
		}

		if (false) { // (undefined*)register0x000001d0 == (undefined*)0x20) {
			(pDCA->aimInputDirection).y = 0.0f;
			(pDCA->aimInputDirection).x = 0.0f;
		}
		else {
			local_10.y = (pDCA->aimDirection).y;
			local_10.z = (pDCA->aimDirection).z;
			local_10.x = (pDCA->aimDirection).x * -0.5f;
			edF32Matrix4FromEulerSoft(&auStack240, &local_10, "XYZ");
			edF32Matrix4MulF32Matrix4Hard(&auStack240, &auStack240, &pDCA->pMeshTransform->base.transformA);
			edF32Matrix4MulF32Vector4Hard(&local_100, &auStack240, &edF32VECTOR4_00425700);
			edF32Vector4NormalizeHard(&local_110, &auStack240.rowZ);

			local_70 = local_100;
			local_80 = local_110;

			edF32Vector4SubHard(&eStack144, &local_20, &local_70);

			pDCA->vectorDyn.BuildFromAccelDistAmplitude(1.0f, &CDynamicExt::gForceGravity, &eStack144, 1);
			pDCA->vectorDyn.Integrate(0.05f);
			edF32Vector4NormalizeHard(&eStack160, &pDCA->vectorDyn.instanceIndex);
			fVar10 = edF32Vector4GetDistHard(&eStack144);
			edF32Vector4ScaleHard(fVar10, &eStack160, &eStack160);
			edF32Vector4AddHard(&eStack176, &local_70, &eStack160);
			if (false) { // ((undefined*)register0x000001d0 == (undefined*)0xb0) {
				(pDCA->aimInputDirection).y = 0.0f;
				(pDCA->aimInputDirection).x = 0.0f;
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
				(pDCA->aimInputDirection).y = -fVar10;
				fVar10 = atan2f(local_160.y, sqrtf(local_160.z * local_160.z + local_160.x * local_160.x));
				(pDCA->aimInputDirection).x = fVar10;
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
		pDCA->bIsFiring = 0;
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