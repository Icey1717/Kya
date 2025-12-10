#include "ActorBoomy.h"
#include "MemoryStream.h"
#include "CameraGame.h"
#include "edMathTCBSpline.h"
#include "CollisionManager.h"
#include "LevelScheduler.h"
#include "MathOps.h"
#include "ActorHero.h"
#include "TimeController.h"

CActorBoomy* CActorBoomy::_gThis;

void CActorBoomy::Create(ByteCode* pByteCode)
{
	CCollision* pCVar2;
	CSound* pCVar3;
	uint uVar4;
	uint count;
	int iVar5;
	uint uVar6;
	CCamera* pAVar5;
	CActorSound* pCVar7;
	int iVar8;
	CedMathTCBSpline* this_00;
	BoomySubObj_0x1c* pCVar9;
	int iVar10;
	float fVar11;

	_gThis = this;

	this->pHero = (CActorHero*)0x0;

	CActorMovable::Create(pByteCode);

	this->vision.Create(this, pByteCode);


	this->field_0x2b8.index = pByteCode->GetS32();
	this->field_0x2bc.index = pByteCode->GetS32();

	this->field_0x744 = pByteCode->GetF32();
	uVar4 = pByteCode->GetU32();
	this->field_0x740 = pByteCode->GetF32();
	count = pByteCode->GetU32();

	this->particleID_0x3d0 = pByteCode->GetS32();
	this->particleID_0x3d4 = pByteCode->GetS32();
	this->particleID_0x3dc = pByteCode->GetS32();
	this->particleID_0x3d8 = this->particleID_0x3dc;

	this->field_0x3cc = pByteCode->GetU32();

	(this->staticMeshComponent).textureIndex = pByteCode->GetS32();
	(this->staticMeshComponent).meshIndex = pByteCode->GetS32();

	this->staticMeshComponent.Reset();

	/* Magic Boomy */
	this->pCamera = static_cast<CCameraGame*>(CScene::ptable.g_CameraManager_0045167c->AddCamera(CT_MainCamera, pByteCode, "Magic Boomy"));
	(this->pCamera->cameraConfig).field_0x10 = 0.25f;
	(this->pCamera->cameraConfig).field_0x14 = 0.1f;

	this->pCamera->field_0x94 = SWITCH_MODE_B;
	this->pCamera->field_0x9c = 0.0f;

	CActor::SV_InstallMaterialId(this->particleID_0x3d0);
	CActor::SV_InstallMaterialId(this->particleID_0x3d4);
	CActor::SV_InstallMaterialId(this->particleID_0x3dc);

	this->fxTail.Create(0.5f, count, 4, this->particleID_0x3d0);
	this->fxTail.field_0xb0 = uVar4;
	this->fxTail.ChangeOrder("ZXY");

	pCVar2 = this->pCollisionData;
	pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffaff7;
	pCVar2 = this->pCollisionData;
	pCVar2->flags_0x0 = pCVar2->flags_0x0 | 4;

	this->field_0x2b4 = CreateActorSound(1);

	iVar5 = pByteCode->GetS32();
	assert(iVar5 < 4);
	iVar10 = 0;
	pCVar9 = this->aBoomyTypeInfo + 1;
	if (0 < iVar5) {
		do {
			pCVar9->flags = 0;
			pCVar9->field_0x4 = pByteCode->GetF32();
			pCVar9->visionRange = pByteCode->GetF32();
			pCVar9->hitDamage = pByteCode->GetS32();
			pCVar9->fxColorA = pByteCode->GetU32();
			iVar10 = iVar10 + 1;
			pCVar9->hitProjectileType = 4;
			pCVar9 = pCVar9 + 1;
		} while (iVar10 < iVar5);
	}

	this->aBoomyTypeInfo[0].flags = 0;

	this->curBoomyTypeId = CLevelScheduler::ScenVar_Get(SCN_ABILITY_BOOMY_TYPE);

	UpdateTypeInfo();

	this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
	(this->vision).visionRange_0x34 = this->aBoomyTypeInfo[this->curBoomyTypeId].visionRange;

	this->pSpline = new CedMathTCBSpline(5, 0);

	return;
}

edF32VECTOR4 edF32VECTOR4_0040e200 = { 0.2617994f, 0.2617994f, 0.0f, 0.0f };
edF32VECTOR4 edF32VECTOR4_0040e210 = { 0.6981317f, 0.6981317f, 0.0f, 0.0f };

void CActorBoomy::Init()
{
	CShadow* pCVar1;
	int iVar2;
	bool bVar3;
	ulong uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	ByteCode BStack32;
	uint local_4;

	CActor::Init();

	ClearLocalData();

	pCVar1 = this->pShadow;
	if (pCVar1 != (CShadow*)0x0) {
		IMPLEMENTATION_GUARD(
		this->particleID_0x3d8 = pCVar1->field_0x54;)
	}

	this->speedDyn.Init(0.0f, 12.0f);

	this->vision.location = this->currentLocation;
	this->vision.rotationQuat = this->rotationQuat;
	this->vision.flags = 0x3b;

	this->field_0x2b8.Init();
	this->field_0x2bc.Init();

	this->fxTail.Init(16.0f, this->sectorId);

	local_4 = this->aBoomyTypeInfo[0].fxColorA;
	iVar2 = (this->fxTail).count_0x34;
	if (iVar2 == 0) {
		trap(7);
	}

	(this->fxTail).field_0x30 = (float)((int)((local_4 >> 0x18) * 6) / iVar2);

	this->fxLightEmitterA.Create(0.4f, 0x80, 0x10);
	this->fxLightEmitterA.ChangeMaterialId(this->particleID_0x3d4, 0);
	this->fxLightEmitterA.ChangeRadius(0.1f, 0.2f);
	this->fxLightEmitterA.ChangeLengthAndWidth(1.0f, 0.2f, 0.2f, 0.2f);
	this->fxLightEmitterA.ChangeSpeedAndAccel(3.0f, 4.0f, 5.0f, -8.0f);
	this->fxLightEmitterA.field_0x20 = edF32VECTOR4_0040e200;
	this->fxLightEmitterA.ChangeColors(0x20406040);
	this->fxLightEmitterA.ChangeStallTime(this->fxLightEmitterA.instanceIndex);

	this->fxLightEmitterB.Create(0.54f, 0x80, 0x18);
	this->fxLightEmitterB.ChangeMaterialId(this->particleID_0x3d4, 1);
	this->fxLightEmitterB.ChangeRadius(0.1f, 0.2f);
	this->fxLightEmitterB.ChangeLengthAndWidth(1.0f, 0.05f, 0.15f, 0.0f);
	this->fxLightEmitterB.ChangeSpeedAndAccel(4.0f, 5.0f, 6.0f, -9.0f);
	this->fxLightEmitterB.field_0x20 = edF32VECTOR4_0040e210;
	this->fxLightEmitterB.ChangeColors(0x80808080);
	this->fxLightEmitterB.ChangeStallTime(this->fxLightEmitterB.instanceIndex);

	bVar3 = (this->staticMeshComponent).textureIndex != -1;
	if (bVar3) {
		bVar3 = (this->staticMeshComponent).meshIndex != -1;
	}

	if (bVar3) {
		this->staticMeshComponent.Init(CScene::_scene_handleB, (ed_g3d_manager*)0x0, (ed_3d_hierarchy_setup*)0x0, (char*)0x0);
		this->staticMeshComponent.SetVisible((ed_3D_Scene*)0x0);
		ed3DHierarchyNodeSetFlag(this->staticMeshComponent.pMeshTransformParent, 0x480);
	}

	return;
}

void CActorBoomy::Term()
{
	bool bVar1;

	CActor::Term();

	bVar1 = (this->staticMeshComponent).textureIndex != -1;
	if (bVar1) {
		bVar1 = (this->staticMeshComponent).meshIndex != -1;
	}

	if (bVar1) {
		this->staticMeshComponent.Term(CScene::_scene_handleB);
	}

	return;
}

void CActorBoomy::Draw()
{
	this->vision.Draw();

	if (6 < this->actorState) {
		CActor::Draw();

		IMPLEMENTATION_GUARD_FX(
		this->fxLightEmitterB.Draw(0xffffffff, 0, 0);

		if (this->field_0x2c4 == 2) {
			this->fxLightEmitterA.Draw(0xffffffff, 0, 0);
		})
	}

	return;
}

void CActorBoomy::Reset()
{
	CActor::Reset();

	ClearLocalData();

	return;
}

void CActorBoomy::CheckpointReset()
{
	ClearLocalData();

	return;
}

CBehaviour* CActorBoomy::BuildBehaviour(int behaviourType)
{
	CBehaviour* pCVar1;

	if (behaviourType == 2) {
		pCVar1 = &this->launchBehaviour;
	}
	else {
		pCVar1 = CActor::BuildBehaviour(behaviourType);
	}

	return pCVar1;
}

StateConfig CActorBoomy::_gStateCfg_BMY[7] = {
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x6, 0x0),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorBoomy::GetStateCfg(int state)
{
	StateConfig* pSVar1;

	if (state < 5) {
		pSVar1 = CActorMovable::GetStateCfg(state);
	}
	else {
		pSVar1 = _gStateCfg_BMY + state + -5;
	}
	return pSVar1;
}

// Should be in: D:/Projects/b-witch/ActorBoomy.h
CVision* CActorBoomy::GetVision()
{
	return &this->vision;
}

int CActorBoomy::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CShadow* pCVar1;
	CVision* pCVar2;
	int iVar3;
	ulong newHash;
	ulong uVar4;
	float fVar5;

	uVar4 = msg;
	if (uVar4 == MESSAGE_BOOMY_CHANGED) {
		if (pMsgParam == (void*)3) {
			newHash = 1;
		}
		else {
			if (pMsgParam == (void*)2) {
				newHash = 1;
			}
			else {
				newHash = 1;

				if (pMsgParam != (void*)1) {
					newHash = uVar4;
				}
			}
		}

		SV_PatchMaterial(1, newHash, (ed_g2d_manager*)0x0);

		this->curBoomyTypeId = (int)pMsgParam;
		GetVision()->visionRange_0x34 = this->aBoomyTypeInfo[this->curBoomyTypeId].visionRange;
	}
	else {
		if (uVar4 == 6) {
			IMPLEMENTATION_GUARD(
			(this->base).dynamic.speed = 0.0;
			if (this->field_0x2c4 == 2) {
				SetState(0xb, -1);
			}
			else {
				SetState(9, -1);
			}
			return 1;)
		}

		if (uVar4 == MESSAGE_GET_RUN_SPEED) {
			float* pRunSpeedParam = reinterpret_cast<float*>(pMsgParam);
			*pRunSpeedParam = this->field_0x2c0;
			return 1;
		}

		if (uVar4 == 4) {
			this->flags = this->flags | 2;
			this->flags = this->flags & 0xfffffffe;
			this->flags = this->flags | 0x80;
			this->flags = this->flags & 0xffffffdf;

			EvaluateDisplayState();

			this->flags = this->flags | 0x400;

			this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 4;
			this->field_0x2c4 = (byte)pMsgParam;

			if (this->field_0x2c4 == 2) {
				this->field_0xf0 = 5.0f;
			
				pCVar1 = this->pShadow;
				if (pCVar1 != (CShadow*)0x0) {
					IMPLEMENTATION_GUARD(
					CShadow::SetMaterialId(pCVar1, this->particleID_0x3dc);
					((this->pShadow)->base).field_0x30 = 0x5060a0a0;
					((this->pShadow)->base).field_0x48 = 0.75;
					(this->pShadow)->field_0x50 = 0.75;)
				}

				IMPLEMENTATION_GUARD_FX(
				CFxLightEmitter::InitRays(&this->fxLightEmitterA, (this->fxLightEmitterA).field_0x4c);
				CFxLightEmitter::InitRays(&this->fxLightEmitterB, (this->fxLightEmitterB).field_0x4c);
				CFxLightEmitter::ChangeGenAtHand(&this->fxLightEmitterB, 0);)
				this->pTargetActor = (CActor*)0x0;
			}
			else {
				this->field_0xf0 = 5.0f;

				pCVar1 = this->pShadow;
				if (pCVar1 != (CShadow*)0x0) {
					IMPLEMENTATION_GUARD(
					CShadow::SetMaterialId(pCVar1, this->particleID_0x3d8);
					((this->pShadow)->base).field_0x30 = 0x50808080;
					((this->pShadow)->base).field_0x48 = 0.3;
					(this->pShadow)->field_0x50 = 0.3;)
				}

				IMPLEMENTATION_GUARD_FX(
				CFxLightEmitter::ChangeGenAtHand(&this->fxLightEmitterB, 1);)
			}

			this->hitActorsTable.nbEntries = 0;
			SetState(BOOMY_STATE_GET_TARGET, -1);

			return 1;
		}
	}

	return CActorMovable::InterpretMessage(pSender, msg, pMsgParam);
}

void CActorBoomy::ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable)
{
	edF32MATRIX4 eStack128;
	edF32MATRIX4 local_40;

	CActorMovable::ManageDyn(param_1, flags, pActorsTable);

	local_40 = gF32Matrix4Unit;

	local_40.aa = this->scale.x;
	local_40.bb = this->scale.y;
	local_40.cc = this->scale.z;

	if (fabs(this->rotationEuler.z) <= 0.0001f) {
		edF32Matrix4RotateYHard(this->rotationEuler.y, &local_40, &local_40);
	}
	else {
		edF32Matrix4FromEulerSoft(&eStack128, &this->rotationEuler.xyz, "ZXY");
		edF32Matrix4MulF32Matrix4Hard(&local_40, &local_40, &eStack128);
	}

	local_40.rowT = this->currentLocation;

	edF32Matrix4CopyHard(&this->pMeshTransform->base.transformA, &local_40);

	return;
}

void CActorBoomy::BehaviourBoomyLaunch_Manage()
{
	bool bVar1;
	bool bVar2;
	int iVar3;

	IMPLEMENTATION_GUARD_FX(
	if (this->field_0x2c4 == 2) {
		CFxLightEmitter::ChangeColors(&this->fxLightEmitterB, 0x80808080);
	})

	switch (this->actorState) {
	case BOOMY_STATE_GET_TARGET:
		StateBoomyGetTarget();
		break;
	case BOOMY_STATE_GOTO_TARGET:
		StateBoomyGotoTarget();
		break;
	case BOOMY_STATE_GOTO_LAUNCHER:
		StateBoomyGotoLauncher();
		break;
	case 9:
		this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
		this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffe;
		this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffb;
		this->field_0x294 = 1;

		bVar1 = this->fxLightEmitterB.Manage(0, 0);
		iVar3 = this->fxTail.Manage(0, 0.0f, 1);
		bVar2 = this->fxLightEmitterA.Manage(0, 0);

		if ((bVar1 != false && iVar3 != 0) && bVar2 != false) {
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->flags = this->flags & 0xfffffbff;

			this->fxTail.Reset();
			this->fxTail.SetPatchActive(0);
		}
		break;
	case BOOMY_STATE_CONTROL:
		IMPLEMENTATION_GUARD(
		StateBoomyControl(this);)
		break;
	case 0xb:
		IMPLEMENTATION_GUARD(
		FUN_0012ceb0(this);)
	}

	return;
}

void CActorBoomy::UpdateTypeInfo()
{
	CActorHero* pCVar1;

	if (this->curBoomyTypeId == 0) {
		this->aBoomyTypeInfo[0].field_0x4 = this->aBoomyTypeInfo[0].field_0x4;
		this->aBoomyTypeInfo[0].visionRange = this->aBoomyTypeInfo[0].visionRange;
		this->aBoomyTypeInfo[0].fxColorA = 0xffffffff;
		this->aBoomyTypeInfo[0].hitProjectileType = 4;
	}
	else {
		this->aBoomyTypeInfo[0].field_0x4 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
		this->aBoomyTypeInfo[0].visionRange = this->aBoomyTypeInfo[this->curBoomyTypeId].visionRange;
		this->aBoomyTypeInfo[0].hitDamage = this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
		this->aBoomyTypeInfo[0].fxColorA = this->aBoomyTypeInfo[this->curBoomyTypeId].fxColorA;
		this->aBoomyTypeInfo[0].hitProjectileType = this->aBoomyTypeInfo[this->curBoomyTypeId].hitProjectileType;
	}

	pCVar1 = this->pHero;
	if ((((pCVar1 != (CActorHero*)0x0) && (pCVar1->typeID == 6)) && (this->curBoomyTypeId == 1)) && (pCVar1->curBehaviourId == 8)) {
		this->aBoomyTypeInfo[0].field_0x4 = this->aBoomyTypeInfo[2].field_0x4;
		this->aBoomyTypeInfo[0].visionRange = this->aBoomyTypeInfo[1].visionRange;
		this->aBoomyTypeInfo[0].hitDamage = this->aBoomyTypeInfo[1].hitDamage;
		this->aBoomyTypeInfo[0].fxColorA = this->aBoomyTypeInfo[1].fxColorA;
		this->aBoomyTypeInfo[0].hitProjectileType = 4;
	}

	this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;

	return;
}

static edF32VECTOR4 edF32VECTOR4_00449a00;

void CActorBoomy::ClearLocalData()
{
	this->flags = this->flags & 0xfffffffd;
	this->flags = this->flags | 1;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();

	this->flags = this->flags & 0xfffffbff;

	this->fxTail.Reset();
	this->fxTail.SetPatchActive(0);

	this->fxLightEmitterA.ResetRays();
	this->fxLightEmitterB.ResetRays();

	this->field_0x1dc = 0.1f;

	(this->pCollisionData)->actorFieldA = this->pHero;

	this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
	this->field_0x294 = 1;

	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 2;
	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xffffffef;
	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffe;
	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffb;

	this->field_0x610 = gF32Matrix4Unit;

	edF32VECTOR4_00449a00 = gF32Vertex4Zero;

	SetState(BOOMY_STATE_DEFAULT, -1);

	return;
}

float DBG_TAIL_MIN_DIST = 3.0f;

void CActorBoomy::StateBoomyGetTarget()
{
	CActor* pCVar1;
	int iVar2;
	float* pfVar3;
	edF32MATRIX4* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	GetPositionMsgParams local_80;
	GetPositionMsgParams local_50;
	edF32VECTOR4 local_20;

	UpdateTypeInfo();

	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 2;

	local_50.field_0x0 = 3;
	iVar2 = DoMessage(this->pHero, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_50);
	if (iVar2 == 1) {
		this->targetPositionOffset = local_50.vectorFieldB;
	}
	else {
		(this->targetPositionOffset).x = 0.0f;
		(this->targetPositionOffset).y = 0.0f;
		(this->targetPositionOffset).z = 0.0f;
		(this->targetPositionOffset).w = 0.0f;
	}
	local_20 = this->pHero->currentLocation;

	edF32Vector4AddHard(&local_20, &local_20, &this->targetPositionOffset);

	pCVar1 = this->pHero;
	this->rotationQuat = pCVar1->rotationQuat;

	pCVar1 = this->pHero;
	this->dynamic.rotationQuat = pCVar1->rotationQuat;

	UpdatePosition(&local_20, true);

	pCVar1 = this->pTargetActor;
	if (pCVar1 != (CActor*)0x0) {
		local_80.field_0x0 = 1;
		if (&this->field_0x220 == (edF32VECTOR4*)0x0) {
			local_80.vectorFieldA = this->currentLocation;
		}
		else {
			local_80.vectorFieldA = (this->field_0x220);
		}

		iVar2 = DoMessage(pCVar1, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_80);
		if (iVar2 == 1) {
			this->targetPosition = pCVar1->currentLocation;
			this->targetPositionOffset = local_80.vectorFieldB;
		}
		else {
			(this->targetPositionOffset).x = 0.0f;
			(this->targetPositionOffset).y = 0.0f;
			(this->targetPositionOffset).z = 0.0f;
			(this->targetPositionOffset).w = 0.0f;
		}
	}

	this->pSpline->SetKeyPosition(0, &this->currentLocation);
	CalculateTrajectory(0);
	this->field_0x1e4 = this->field_0x1e0 * 0.0f;

	this->flags = this->flags | 0x80;
	this->flags = this->flags & 0xffffffdf;
	EvaluateDisplayState();
	this->flags = this->flags | 0x400;

	this->field_0x650 = 0;
	this->pAnimationController->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);

	this->fxTail.field_0x70 = gF32Matrix4Unit;
	(this->fxTail).field_0x40 = (this->fxTail).field_0x3c;
	(this->fxTail).field_0xb8 = 16.0f / (float)(this->fxTail).count_0x34;

	this->fxTail.SetPatchActive(1);
	(this->speedDyn).currentAlpha = (this->pSpline->aPoints->position).y;
	this->field_0x1d4 = DBG_TAIL_MIN_DIST <= this->distanceToTarget;
	if (this->field_0x2c4 == 2) {
		IMPLEMENTATION_GUARD_AUDIO(
		CActorSound::SoundStart
		(this->field_0x2b4, (CActor*)this, 0, (this->field_0x2bc).pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);)
		this->field_0x654 = 0;
		SetState(BOOMY_STATE_CONTROL, -1);
	}
	else {
		IMPLEMENTATION_GUARD_AUDIO(
		CActorSound::SoundStart
		(this->field_0x2b4, (CActor*)this, 0, (this->field_0x2b8).pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);)
		SetState(BOOMY_STATE_GOTO_TARGET, -1);
	}

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	return;
}


void CActorBoomy::StateBoomyGotoTarget()
{
	byte bVar1;
	CActor* pCVar2;
	CCollision* pCVar3;
	bool bCollideWithEnvironment;
	bool bHitTarget;
	int iVar6;
	edF32MATRIX4* peVar7;
	edF32MATRIX4* peVar8;
	bool bCanPassThrough;
	float fVar10;
	float fVar11;
	float fVar12;
	GetPositionMsgParams local_230;
	CActorsTable actorsTable;
	_msg_hit_param uStack240;
	edF32VECTOR4 aSplinePositions[6];

	actorsTable.nbEntries = 0;
	bHitTarget = GotoTarget(&actorsTable, aSplinePositions, 6, 1);
	bCanPassThrough = false;

	if ((((this->hitActorsTable).nbEntries != 0) && (pCVar2 = (this->hitActorsTable).aEntries[0], pCVar2 != (CActor*)0x0)) &&
		((pCVar2 != this->pHero || (bHitTarget != false)))) {
		pCVar3 = this->pCollisionData;
		bVar1 = pCVar3->flags_0x4;
		if ((bVar1 & 7) == 0) {
			local_230.field_0x0 = 0;
			local_230.vectorFieldA = this->currentLocation;
			uStack240.field_0x40 = this->currentLocation;
			iVar6 = DoMessage((this->hitActorsTable).aEntries[0], MESSAGE_GET_VISUAL_DETECTION_POINT, &uStack240);
			if (iVar6 != 0) {
				edF32Vector4AddHard(&uStack240.field_0x40, &this->targetPosition, &local_230.vectorFieldB);
			}
		}
		else {
			if ((bVar1 & 1) == 0) {
				if ((bVar1 & 2) == 0) {
					if ((bVar1 & 4) != 0) {
						uStack240.field_0x40 = pCVar3->aCollisionContact[2].field_0x10;
					}
				}
				else {
					uStack240.field_0x40 = pCVar3->aCollisionContact[1].field_0x10;
				}
			}
			else {
				uStack240.field_0x40 = pCVar3->aCollisionContact[0].field_0x10;
			}
		}

		uStack240.projectileType = HIT_TYPE_BOOMY;
		uStack240.field_0x4 = 2;
		if (this->field_0x2c4 == 2) {
			uStack240.field_0x4 = 4;
		}
		else {
			if (this->field_0x2c4 == 1) {
				uStack240.field_0x4 = 3;
			}
		}

		bVar1 = this->field_0x2c4;
		if (bVar1 == 2) {
			uStack240.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
		}
		else {
			if (bVar1 == 1) {
				uStack240.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
			}
			else {
				if (bVar1 == 0) {
					uStack240.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
				}
				else {
					uStack240.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
				}
			}
		}

		uStack240.field_0x30 = this->field_0x2c0 * 0.25f;
		uStack240.field_0x20 = this->field_0x210;
		edF32Vector4NormalizeHard(&uStack240.field_0x20, &uStack240.field_0x20);
		DoMessage((this->hitActorsTable).aEntries[0], MESSAGE_KICKED, &uStack240);
		bCanPassThrough = this->hitActorsTable.aEntries[0]->CanPassThrough();
		if (bCanPassThrough == false) {
			UpdatePosition(&uStack240.field_0x40, true);
		}
		else {
			(this->hitActorsTable).nbEntries = 0;
			actorsTable.nbEntries = 0;
		}
	}

	bCollideWithEnvironment = ((this->pCollisionData)->flags_0x4 & 7) != 0;

	if (((bCollideWithEnvironment) && (actorsTable.nbEntries == 0)) && (bCanPassThrough == false)) {
		_BSpline_InitWhenHit(bHitTarget == false);

		if (this->distanceToTarget <= 8.0f) {
			this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffd;
		}

		this->field_0x610 = gF32Matrix4Unit;
		edF32VECTOR4_00449a00 = gF32Vector4Zero;

		SetState(BOOMY_STATE_GOTO_LAUNCHER, -1);
	}
	else {
		if (((this->distanceToTarget <= 0.3f) || (0.5f <= this->field_0x1e8)) || (actorsTable.nbEntries != 0)) {
			if (((bCollideWithEnvironment) || (actorsTable.nbEntries != 0)) || ((this->pTargetActor != (CActor*)0x0 && ((this->field_0x2c4 != 1 && (this->distanceToTarget <= 0.3f)))))) {
				_BSpline_InitWhenHit(true);

				if (this->distanceToTarget <= 8.0f) {
					this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffd;
				}
			}
			else {
				this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 1;
				this->pTargetActor = this->pHero;
				CalculateTrajectory(1);
				pCVar2 = this->pHero;
				GetPositionMsgParams local_260;
				local_260.field_0x0 = 2;
				local_260.vectorFieldA = this->currentLocation;
				iVar6 = DoMessage(pCVar2, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_260);
				if (iVar6 == 1) {
					this->targetPosition = pCVar2->currentLocation;
					this->targetPositionOffset = local_260.vectorFieldB;
				}
				else {
					(this->targetPositionOffset).x = 0.0f;
					(this->targetPositionOffset).y = 0.0f;
					(this->targetPositionOffset).z = 0.0f;
					(this->targetPositionOffset).w = 0.0f;
				}

				if (this->distanceToTarget <= 8.0f) {
					this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffd;
				}
			}

			this->field_0x610 = gF32Matrix4Unit;
			edF32VECTOR4_00449a00 = gF32Vector4Zero;

			SetState(BOOMY_STATE_GOTO_LAUNCHER, -1);
		}
	}
	return;
}

void CActorBoomy::StateBoomyGotoLauncher()
{
	byte bVar1;
	CCollision* pCVar2;
	int iVar3;
	CActor* pCVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	GetPositionMsgParams local_2b0;
	CActorsTable actorsTable;
	_msg_hit_param local_f0;
	edF32VECTOR4 aSplinePositions[6];

	actorsTable.nbEntries = 0;
	pCVar4 = this->pHero;
	local_2b0.field_0x0 = 2;
	local_2b0.vectorFieldA = this->currentLocation;
	iVar3 = DoMessage(pCVar4, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_2b0);

	if (iVar3 == 1) {
		this->targetPosition = pCVar4->currentLocation;
		this->targetPositionOffset = local_2b0.vectorFieldB;
	}
	else {
		(this->targetPositionOffset).x = 0.0f;
		(this->targetPositionOffset).y = 0.0f;
		(this->targetPositionOffset).z = 0.0f;
		(this->targetPositionOffset).w = 0.0f;
	}

	GotoTarget(&actorsTable, aSplinePositions, 6, 0);

	if (actorsTable.nbEntries != 0) {
		local_f0.field_0x4 = 2;
		local_f0.projectileType = 4;
		if (this->field_0x2c4 == 2) {
			local_f0.field_0x4 = 4;
		}
		else {
			if (this->field_0x2c4 == 1) {
				local_f0.field_0x4 = 3;
			}
		}

		bVar1 = this->field_0x2c4;
		if (bVar1 == 2) {
			local_f0.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
		}
		else {
			if (bVar1 == 1) {
				local_f0.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
			}
			else {
				if (bVar1 == 0) {
					local_f0.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
				}
				else {
					local_f0.damage = (float)this->aBoomyTypeInfo[this->curBoomyTypeId].hitDamage;
				}
			}
		}

		local_f0.field_0x30 = this->field_0x2c0 * 0.25f;
		edF32Vector4NormalizeHard(&local_f0.field_0x20, &this->field_0x210);

		pCVar2 = this->pCollisionData;
		bVar1 = pCVar2->flags_0x4;
		if ((bVar1 & 7) != 0) {
			if ((bVar1 & 1) == 0) {
				if ((bVar1 & 2) == 0) {
					if ((bVar1 & 4) != 0) {
						local_f0.field_0x40 = pCVar2->aCollisionContact[2].field_0x10;
					}
				}
				else {
					local_f0.field_0x40 = pCVar2->aCollisionContact[1].field_0x10;
				}
			}
			else {
				local_f0.field_0x40 = pCVar2->aCollisionContact[0].field_0x10;
			}
		}

		while (actorsTable.nbEntries != 0) {
			pCVar4 = actorsTable.PopCurrent();
			if (pCVar4 != this->pHero) {
				DoMessage(pCVar4, MESSAGE_KICKED, &local_f0);
			}
		}
	}

	if ((this->distanceToTarget / this->field_0x2c0 <= 0.06f) || (this->field_0x1d0 != 0)) {
		if (this->field_0x1d0 == 0) {
			DoMessage(this->pHero, (ACTOR_MESSAGE)5, 0);
			this->field_0x1d0 = 1;
			if ((this->aBoomyTypeInfo[0].flags & 0x10) != 0) {
				_msg_hit_param aiStack640;
				aiStack640.projectileType = this->aBoomyTypeInfo[0].hitProjectileType;
				aiStack640.damage = (float)this->aBoomyTypeInfo[0].hitDamage;
				DoMessage(this->pHero, MESSAGE_KICKED, &aiStack640);
				this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xffffffef;
			}
		}

		if (1.0f <= this->field_0x1e8) {
			this->dynamic.speed = 0.0f;

			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			EvaluateDisplayState();
			this->flags = this->flags | 0x400;

			this->field_0x2b4->SoundStop(0);

			SetState(9, -1);
		}
	}

	return;
}

bool CActorBoomy::GotoTarget(CActorsTable* pTable, edF32VECTOR4* aSplinePositions, int nbSplinePositions, int param_5)
{
	CActor* pCVar1;
	bool bVar2;
	edF32VECTOR4* pSplinePosition;
	bool bVar5;
	int iVar6;
	int currentIndex;
	float fVar8;
	float fVar9;
	float fVar10;
	edF32VECTOR4 finalTargetPosition;
	edF32VECTOR4 newRotation;
	uint local_8;
	uint local_4;

	BSpline_UpdateTargetPos(this->pTargetActor);

	this->vision.location = this->currentLocation;
	this->vision.rotationQuat = this->rotationQuat;

	edF32Vector4AddHard(&finalTargetPosition, &this->targetPosition, &this->targetPositionOffset);

	if (this->field_0x1d4 == 0) {
		fVar8 = this->field_0x1e4 / this->field_0x1e0;
		this->field_0x1e8 = fVar8;
		if (1.0f < fVar8) {
			this->field_0x1e8 = 1.0f;
		}

		this->pSpline->GetPosition(this->field_0x1e8, aSplinePositions + nbSplinePositions + -1);
		pSplinePosition = aSplinePositions + nbSplinePositions + -1;
		this->field_0x1e4 = this->field_0x1e4 + GetTimer()->cutsceneDeltaTime;
		edF32VECTOR4 splineToTarget = finalTargetPosition - *pSplinePosition;
		this->distanceToTarget = splineToTarget.x * splineToTarget.x + splineToTarget.y * splineToTarget.y + splineToTarget.z * splineToTarget.z;
	}
	else {
		this->distanceToTarget = 99999.0f;

		for (currentIndex = 0; currentIndex < nbSplinePositions; currentIndex = currentIndex + 1) {
			fVar8 = this->field_0x1e4 / this->field_0x1e0;
			this->field_0x1e8 = fVar8;
			if (1.0f < fVar8) {
				this->field_0x1e8 = 1.0f;
			}

			this->pSpline->GetPosition(this->field_0x1e8, aSplinePositions + currentIndex);
			pSplinePosition = aSplinePositions + currentIndex;
			this->field_0x1e4 = this->field_0x1e4 + GetTimer()->cutsceneDeltaTime / (float)nbSplinePositions;
			edF32VECTOR4 splineToTarget = finalTargetPosition - *pSplinePosition;
			fVar8 = splineToTarget.x * splineToTarget.x + splineToTarget.y * splineToTarget.y + splineToTarget.z * splineToTarget.z;
			if (fVar8 < this->distanceToTarget) {
				this->distanceToTarget = fVar8;
			}
		}
	}

	this->distanceToTarget = sqrtf(this->distanceToTarget);
	if ((this->pTargetActor != (CActor*)0x0) && (this->distanceToTarget <= 0.3f)) {
		this->hitActorsTable.Add(this->pTargetActor);
	}

	edF32Vector4SubHard(&newRotation, aSplinePositions + nbSplinePositions + -1, &this->currentLocation);
	fVar8 = edF32Vector4SafeNormalize1Hard(&newRotation, &newRotation);
	if (fabs(fVar8) <= 0.001f) {
		pCVar1 = this->pHero;
		fVar8 = 1.0f;
		newRotation = pCVar1->rotationQuat;
	}

	this->dynamic.rotationQuat = newRotation;
	this->dynamic.speed = fVar8 / GetTimer()->cutsceneDeltaTime;
	this->rotationQuat = this->dynamic.rotationQuat;

	fVar8 = GetAngleYFromVector(&this->rotationQuat);
	this->rotationEuler.x = 0.0f;
	this->rotationEuler.y = fVar8;
	this->rotationEuler.z = -0.7853982f;

	local_4 = this->aBoomyTypeInfo[0].fxColorA;
	(this->fxTail).field_0x28 = (this->fxTail).field_0x24;
	(this->fxTail).field_0x24.rgba = local_4;
	local_8 = this->aBoomyTypeInfo[0].fxColorA;
	this->fxLightEmitterB.ChangeColors(local_8);
	if ((this->field_0x1d4 == 0) || ((this->aBoomyTypeInfo[0].flags & 2) == 0)) {
		this->fxLightEmitterB.Manage(0, 0);
		if (this->field_0x1d4 != 0) {
			this->fxTail.Manage(aSplinePositions, 0, 1);
		}
	}
	else {
		fVar8 = this->rotationEuler.y;
		(this->fxLightEmitterB).field_0x40 = this->rotationEuler.x + 1.570796f;
		(this->fxLightEmitterB).field_0x44 = fVar8 + 3.141593f;
		for (currentIndex = 0; currentIndex < nbSplinePositions; currentIndex = currentIndex + 2) {
			this->fxLightEmitterB.GenerateNewOne(aSplinePositions + currentIndex);
		}

		this->fxLightEmitterB.Manage(aSplinePositions + nbSplinePositions + -1, 0xffffffff);

		(this->fxTail).instanceIndex = (this->fxTail).field_0x50;
		(this->fxTail).field_0x50 = this->rotationEuler;

		if ((this->field_0x2b0 == 0) || (this->field_0x1e8 != 1.0f)) {
			this->fxTail.Manage(aSplinePositions, nbSplinePositions, 0);
		}
		else {
			this->fxTail.Manage(aSplinePositions + nbSplinePositions + -1, 1, 0);
		}
	}

	ManageDyn(4.0f, 0, pTable);

	bVar5 = false;
	currentIndex = 0;
	while (currentIndex < pTable->nbEntries) {
		pCVar1 = pTable->aEntries[currentIndex];
		if ((pCVar1 == this->pHero) && (param_5 == 0)) {
			bVar5 = true;
		}

		bVar2 = this->hitActorsTable.IsInList(pCVar1);
		if ((bVar2 == false) && (pCVar1 != this->pHero)) {
			this->hitActorsTable.Add(pCVar1);
			iVar6 = currentIndex;
		}
		else {
			iVar6 = currentIndex + -1;
			pTable->Pop(currentIndex);
		}

		currentIndex = iVar6 + 1;
	}

	return bVar5;
}



static edF32VECTOR4 edF32VECTOR4_0040e2a0 = { 0.0f, 0.0f, 0.0f, 0.0f };
static float FLOAT_0040e2b0 = 0.765f;
static float FLOAT_0040e2b4 = 0.0f;
static float FLOAT_0040e2b8 = -0.3f;

void CActorBoomy::CalculateTrajectory(byte param_2)
{
	byte bVar1;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	this->field_0x1d0 = 0;
	this->field_0x2b0 = param_2;

	if (this->pTargetActor == (CActor*)0x0) {
		(this->targetPositionOffset).x = 0.0f;
		(this->targetPositionOffset).y = 0.0f;
		(this->targetPositionOffset).z = 0.0f;
		(this->targetPositionOffset).w = 0.0f;
	}

	BSpline_UpdateTargetPos(this->pTargetActor);

	bVar1 = this->field_0x2b0;
	if (bVar1 == 2) {
		_BSpline_ComputeWhenHit();
	}
	else {
		if (bVar1 == 1) {
			edF32Vector4AddHard(&eStack16, &this->targetPosition, &this->targetPositionOffset);
			edF32Matrix4MulF32Vector4Hard(&eStack32, &this->pHero->pMeshTransform->base.transformA, &edF32VECTOR4_0040e2a0);
			edF32Vector4AddHard(&eStack32, &eStack16, &eStack32);
			this->pSpline->SetKeyPosition(4, &eStack32);
			this->pSpline->SetKeyTCB(FLOAT_0040e2b0, FLOAT_0040e2b4, FLOAT_0040e2b8, 4);
			this->pSpline->PreComputeSpline(1);
			this->field_0x1e0 = this->field_0x1d8 / this->field_0x2c0;
		}
		else {
			if (bVar1 == 0) {
				_BSpline_ComputeToTarget();
			}
		}
	}
	return;
}

void CActorBoomy::BSpline_UpdateTargetPos(CActor* pTargetActor)
{
	byte bVar1;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	if (((this->aBoomyTypeInfo[0].flags & 1) == 0) || (pTargetActor == (CActor*)0x0)) {
		edF32Matrix4MulF32Vector4Hard(&this->targetPosition, &this->field_0x610, &this->targetPosition);
	}
	else {
		this->targetPosition = pTargetActor->currentLocation;

		bVar1 = this->field_0x2b0;
		if (bVar1 == 2) {
			_BSpline_ComputeWhenHit();
		}
		else {
			if (bVar1 == 1) {
				edF32Vector4AddHard(&eStack32, &this->targetPosition, &this->targetPositionOffset);
				edF32Matrix4MulF32Vector4Hard(&eStack48, &this->pHero->pMeshTransform->base.transformA, &edF32VECTOR4_0040e2a0);
				edF32Vector4AddHard(&eStack48, &eStack32, &eStack48);
				this->pSpline->SetKeyPosition(4, &eStack48);
				this->pSpline->SetKeyTCB(FLOAT_0040e2b0, FLOAT_0040e2b4, FLOAT_0040e2b8, 4);
				this->pSpline->PreComputeSpline(1);
				this->field_0x1e0 = this->field_0x1d8 / this->field_0x2c0;
			}
			else {
				if (bVar1 == 0) {
					_BSpline_ComputeToTarget();
				}
			}
		}
	}

	edF32Vector4AddHard(&eStack16, &this->targetPosition, &this->targetPositionOffset);
	edF32Vector4SubHard(&this->field_0x210, &eStack16, &this->currentLocation);
	this->distanceToTarget = edF32Vector4GetDistHard(&this->field_0x210);

	return;
}


void CActorBoomy::_BSpline_ComputeWhenHit()
{
	edF32VECTOR4 eStack64;
	edF32VECTOR4 finalTargetPosition;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	edF32Vector4AddHard(&finalTargetPosition, &this->targetPosition, &this->targetPositionOffset);
	local_20 = finalTargetPosition - this->pSpline->aPoints->position;

	this->field_0x1d8 = edF32Vector4GetDistHard(&local_20);
	edF32Vector4ScaleHard(0.25f, &local_20, &local_20);
	edF32Vector4CrossProductHard(&eStack16, &local_20, &gF32Vector4UnitY);
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	edF32Vector4ScaleHard(this->field_0x1d8 * this->field_0x1dc * 0.75f, &eStack16, &eStack16);
	edF32Vector4AddHard(&eStack64, &this->pSpline->aPoints[0].position, &eStack16);
	edF32Vector4AddHard(&eStack64, &eStack64, &local_20);
	this->pSpline->SetKeyPosition(1, &eStack64);

	edF32Vector4AddHard(&eStack64, &this->pSpline->aPoints[1].position, &local_20);
	edF32Vector4AddHard(&eStack64, &eStack64, &local_20);
	this->pSpline->SetKeyPosition(3, &eStack64);

	edF32Vector4ScaleHard(0.3333333f, &eStack16, &eStack16);
	edF32Vector4AddHard(&eStack64, &this->pSpline->aPoints[1].position, &eStack16);
	edF32Vector4AddHard(&eStack64, &eStack64, &local_20);
	this->pSpline->SetKeyPosition(2, &eStack64);
	this->pSpline->SetKeyPosition(4, &finalTargetPosition);

	this->pSpline->SetKeyTCB(0.0f, 0.0f, 0.0f, 4);
	this->pSpline->PreComputeSpline(1);

	this->field_0x1e0 = this->field_0x1d8 / this->field_0x2c0;

	return;
}

void CActorBoomy::_BSpline_ComputeToTarget()
{
	float fVar1;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	edF32Vector4AddHard(&local_30, &this->targetPosition, &this->targetPositionOffset);
	local_20 = local_30 - this->pSpline->aPoints->position;

	this->field_0x1d8 = edF32Vector4GetDistHard(&local_20);

	edF32Vector4ScaleHard(0.8f, &local_20, &local_20);
	edF32Vector4CrossProductHard(&eStack16, &local_20, &gF32Vector4UnitY);
	edF32Vector4NormalizeHard(&eStack16, &eStack16);
	edF32Vector4ScaleHard(this->field_0x1d8 * this->field_0x1dc, &eStack16, &eStack16);
	edF32Vector4AddHard(&eStack64, &this->pSpline->aPoints->position, &eStack16);
	edF32Vector4AddHard(&eStack64, &eStack64, &local_20);
	this->pSpline->SetKeyPosition(1, &eStack64);
	this->pSpline->SetKeyPosition(2, &local_30);
	edF32Vector4SubHard(&eStack64, &this->pSpline->aPoints->position, &eStack16);
	edF32Vector4AddHard(&eStack64, &eStack64, &local_20);
	this->pSpline->SetKeyPosition(3, &eStack64);
	this->pSpline->SetKeyPosition(4, &this->pSpline->aPoints->position);
	this->pSpline->PreComputeSpline(1);
	this->field_0x1e0 = this->field_0x1d8 / this->field_0x2c0;

	return;
}

void CActorBoomy::_BSpline_InitWhenHit(bool bNotTarget)
{
	byte bVar1;
	CCollision* pCVar2;
	CActor* pReceiver;
	bool bVar3;
	int iVar4;
	s_collision_contact* psVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	GetPositionMsgParams local_a0;
	GetPositionMsgParams local_60;
	edF32VECTOR4 hitPointNormal;
	edF32VECTOR4 hitPointPosition;
	int local_10;
	int* local_c;

	pCVar2 = this->pCollisionData;
	bVar1 = pCVar2->flags_0x4;

	if ((bVar1 & 7) == 0) {
		edF32Vector4ScaleHard(-1.0f , &hitPointNormal, &this->field_0x210);
		local_60.field_0x0 = 0;
		local_60.vectorFieldA = this->currentLocation;
		hitPointPosition = this->currentLocation;
		iVar4 = DoMessage(this->pTargetActor, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_60);
		if (iVar4 != 0) {
			edF32Vector4AddHard(&hitPointPosition, &this->targetPosition, &local_60.vectorFieldB);
		}
	}
	else {
		psVar5 = (s_collision_contact*)0x0;
		if ((bVar1 & 1) != 0) {
			psVar5 = pCVar2->aCollisionContact;
		}

		if ((bVar1 & 2) != 0) {
			psVar5 = pCVar2->aCollisionContact + 1;
		}

		if ((bVar1 & 4) != 0) {
			psVar5 = pCVar2->aCollisionContact + 2;
		}

		hitPointPosition = psVar5->field_0x10;
		hitPointNormal = psVar5->location;
	}

	// Set the first spline position to the hit position and update our position to that position.
	this->pSpline->SetKeyPosition(0, &hitPointPosition);
	UpdatePosition(&hitPointPosition, true);

	if ((bNotTarget != false) && ((this->pTargetActor == (CActor*)0x0 || (this->pTargetActor != this->pHero)))) {
		local_10 = 0;
		local_c = (int*)0x0;
		if (this->field_0x3cc != 0xffffffff) {
			IMPLEMENTATION_GUARD_FX(
			CFxManager::GetDynamicFx(CScene::ptable.g_EffectsManager_004516b8, &local_10, this->field_0x3cc, 0xffffffffffffffff);
			fVar9 = gF32Vector4Zero.w;
			fVar6 = gF32Vector4Zero.z;
			if (((local_c == (int*)0x0) || (local_10 == 0)) || (local_10 != local_c[6])) {
				bVar3 = false;
			}
			else {
				bVar3 = true;
			}

			if (bVar3) {
				fVar7 = GetAngleXFromVector(&hitPointNormal);
				fVar7 = edF32Between_0_2Pi(fVar7 + 1.570796);
				fVar8 = GetAngleYFromVector(&hitPointNormal);
				fVar8 = edF32Between_0_2Pi(fVar8 - 1.570796);
				if (((local_c != (int*)0x0) && (local_10 != 0)) && (local_10 == local_c[6])) {
					(**(code**)(*local_c + 0x38))();
				}
				if (((local_c != (int*)0x0) && (local_10 != 0)) && (local_10 == local_c[6])) {
					local_c[0x14] = (int)fVar7;
					local_c[0x15] = (int)fVar8;
					local_c[0x16] = (int)fVar6;
					local_c[0x17] = (int)fVar9;
				}
				if (((local_c != (int*)0x0) && (local_10 != 0)) && (local_10 == local_c[6])) {
					(**(code**)(*local_c + 0x10))(0, 0);
				}
			})
		}
	}

	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 1;
	this->pTargetActor = this->pHero;

	CalculateTrajectory(2);

	this->field_0x1e4 = this->field_0x1e0 * 0.0f;
	pReceiver = this->pHero;
	local_a0.field_0x0 = 2;
	local_a0.vectorFieldA = this->currentLocation;
	iVar4 = DoMessage(pReceiver, MESSAGE_GET_VISUAL_DETECTION_POINT, &local_a0);
	if (iVar4 == 1) {
		this->targetPosition = pReceiver->currentLocation;
		this->targetPositionOffset = local_a0.vectorFieldB;
	}
	else {
		(this->targetPositionOffset).x = 0.0f;
		(this->targetPositionOffset).y = 0.0f;
		(this->targetPositionOffset).z = 0.0f;
		(this->targetPositionOffset).w = 0.0f;
	}

	return;
}

void CActorBoomy::UpdateFromOwner(int param_2, edF32VECTOR4* pDirection)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	GetPositionMsgParams local_50;
	edF32VECTOR4 local_20;

	local_20 = this->pHero->currentLocation;
	local_50.field_0x0 = param_2;
	iVar1 = DoMessage(this->pHero, (ACTOR_MESSAGE)7, &local_50);
	if (iVar1 != 0) {
		edF32Vector4AddHard(&local_20, &local_20, &local_50.vectorFieldB);
	}

	this->rotationQuat = *pDirection;
	this->dynamic.rotationQuat = *pDirection;

	UpdatePosition(&local_20, true);

	this->vision.location = local_20;
	this->vision.rotationQuat = this->rotationQuat;

	return;
}

void CActorBoomy::GetActorsInVision(float maxDistance, CActorsTable* pTable)
{
	pTable->nbEntries = 0;
	this->vision.ScanAccurate(maxDistance, pTable, 0);

	return;
}


void CActorBoomy::SetTarget(CActor* pActor, edF32VECTOR4* pPosition)
{
	if ((this->aBoomyTypeInfo[0].flags & 4) == 0) {
		this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 8;
		this->pTargetActor = pActor;
		this->targetPosition = pActor->currentLocation;
		this->field_0x220 = *pPosition;
	}
	return;
}


void CActorBoomy::SetTarget(edF32VECTOR4* pPosition)
{
	if ((this->aBoomyTypeInfo[0].flags & 4) == 0) {
		this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffff7;
		this->pTargetActor = (CActor*)0x0;
		this->targetPosition = *pPosition;
		this->field_0x220 = *pPosition;
	}
	return;
}

CActor* CActorBoomy::GetBestActorInVision()
{
	CActorsTable local_110;

	local_110.nbEntries = 0;
	this->vision.Scan(&local_110, 0);

	if (local_110.nbEntries == 0) {
		local_110.aEntries[0] = (CActor*)0x0;
	}

	return local_110.aEntries[0];
}

void CBehaviourBoomyLaunch::Manage()
{
	this->pOwner->BehaviourBoomyLaunch_Manage();

	return;
}

void CBehaviourBoomyLaunch::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBoomy*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(BOOMY_STATE_DEFAULT, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	return;
}