#include "ActorBoomy.h"
#include "MemoryStream.h"
#include "CameraGame.h"
#include "edMathTCBSpline.h"
#include "CollisionManager.h"
#include "LevelScheduleManager.h"
#include "MathOps.h"

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

	this->field_0x3e0 = (CActor*)0x0;

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

	IMPLEMENTATION_GUARD_FX(
	this->fxTail.Create(0.5f, count, 4, this->particleID_0x3d0);
	(this->fxTail).field_0xb0 = uVar4;
	CFxTail::ChangeOrder(&this->fxTail, s_ZXY_00428f30);)

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
			pCVar9->field_0xc = pByteCode->GetS32();
			pCVar9->field_0x10 = pByteCode->GetU32();
			iVar10 = iVar10 + 1;
			pCVar9->field_0x14 = 4;
			pCVar9 = pCVar9 + 1;
		} while (iVar10 < iVar5);
	}

	this->aBoomyTypeInfo[0].flags = 0;

	this->curBoomyTypeId = CLevelScheduler::ScenVar_Get(9);

	UpdateTypeInfo();

	this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
	(this->vision).visionRange_0x34 = this->aBoomyTypeInfo[this->curBoomyTypeId].visionRange;

	this->pSpline = new CedMathTCBSpline(5, 0);

	return;
}

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

	IMPLEMENTATION_GUARD_FX(
	this->fxTail.Init(16.0f, this->objectId);

	local_4 = this->aBoomyTypeInfo[0].field_0x10;
	iVar2 = (this->fxTail).count_0x34;
	if (iVar2 == 0) {
		trap(7);
	}

	(this->fxTail).field_0x30 = (float)((int)((local_4 >> 0x18) * 6) / iVar2);

	this->field_0x530.Create(0.4f, 0x80, 0x10);
	this->field_0x530.ChangeMaterialId(this->particleID_0x3d4, 0);
	this->field_0x530.ChangeRadius(0.1f, 0.2f);
	this->field_0x530.ChangeLengthAndWidth(1.0f, 0.2f, 0.2f, 0.2f);
	this->field_0x530.ChangeSpeedAndAccel(3.0f, 4.0f, 5.0f, -8.0f);
	(this->field_0x530).field_0x20 = edF32VECTOR4_0040e200;
	this->field_0x530.ChangeColors(0x20406040);
	this->field_0x530.ChangeStallTime((this->field_0x530).field_0x60);

	this->fxLightEmitter.Create(0.54f, 0x80, 0x18);
	this->fxLightEmitter.ChangeMaterialId(this->particleID_0x3d4, 1);
	this->fxLightEmitter.ChangeRadius(0.1f, 0.2f);
	this->fxLightEmitter.ChangeLengthAndWidth(1.0f, 0.05f, 0.15f, 0.0f);
	this->fxLightEmitter.ChangeSpeedAndAccel(4.0f, 5.0f, 6.0f, -9.0f);
	(this->fxLightEmitter).field_0x20 = edF32VECTOR4_0040e210;
	this->fxLightEmitter.ChangeColors(0x80808080);
	this->fxLightEmitter.ChangeStallTime((this->fxLightEmitter).field_0x60);)

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
		this->fxLightEmitter.Draw(0xffffffff, 0, 0);

		if (this->field_0x2c4 == 2) {
			this->field_0x530.Draw(0xffffffff, 0, 0);
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
	if (uVar4 == 0x62) {
		IMPLEMENTATION_GUARD(
		if (pMsgParam == 3) {
			newHash = 1;
		}
		else {
			if (pMsgParam == 2) {
				newHash = 1;
			}
			else {
				newHash = 1;
				if (pMsgParam != 1) {
					newHash = uVar4;
				}
			}
		}
		CActor::SV_PatchMaterial((CActor*)this, 1, newHash, (ed_g2d_manager*)0x0);
		this->curBoomyTypeId = pMsgParam;
		fVar5 = this->aBoomyTypeInfo[this->curBoomyTypeId].visionRange;
		pCVar2 = (*(this->pVTable)->GetVision)((CActorWolfen*)this);
		pCVar2->visionRange_0x34 = fVar5;)
	}
	else {
		if (uVar4 == 6) {
			IMPLEMENTATION_GUARD(
			(this->base).dynamic.speed = 0.0;
			if (this->field_0x2c4 == 2) {
				(*(this->pVTable)->SetState)((CActor*)this, 0xb, -1);
			}
			else {
				(*(this->pVTable)->SetState)((CActor*)this, 9, -1);
			}
			return 1;)
		}

		if (uVar4 == 0xc) {
			IMPLEMENTATION_GUARD(
			*(float*)pMsgParam = this->field_0x2c0;
			return 1;)
		}

		if (uVar4 == 4) {
			IMPLEMENTATION_GUARD(
			this->flags = this->flags | 2;
			this->flags = this->flags & 0xfffffffe;
			this->flags = this->flags | 0x80;
			this->flags = this->flags & 0xffffffdf;
			CActor::EvaluateDisplayState((CActor*)this);
			this->flags = this->flags | 0x400;
			this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 4;
			this->field_0x2c4 = (byte)pMsgParam;
			if (this->field_0x2c4 == 2) {
				this->field_0xf0 = 5.0;
				pCVar1 = this->pShadow;
				if (pCVar1 != (CShadow*)0x0) {
					CShadow::SetMaterialId(pCVar1, this->particleID_0x3dc);
					((this->pShadow)->base).field_0x30 = 0x5060a0a0;
					((this->pShadow)->base).field_0x48 = 0.75;
					(this->pShadow)->field_0x50 = 0.75;
				}
				CFxLightEmitter::InitRays(&this->field_0x530, (this->field_0x530).field_0x4c);
				CFxLightEmitter::InitRays(&this->fxLightEmitter, (this->fxLightEmitter).field_0x4c);
				CFxLightEmitter::ChangeGenAtHand(&this->fxLightEmitter, 0);
				this->field_0x1f0 = 0;
			}
			else {
				this->field_0xf0 = 5.0;
				pCVar1 = this->pShadow;
				if (pCVar1 != (CShadow*)0x0) {
					CShadow::SetMaterialId(pCVar1, this->particleID_0x3d8);
					((this->pShadow)->base).field_0x30 = 0x50808080;
					((this->pShadow)->base).field_0x48 = 0.3;
					(this->pShadow)->field_0x50 = 0.3;
				}
				CFxLightEmitter::ChangeGenAtHand(&this->fxLightEmitter, 1);
			}
			this->field_0x2c8 = 0;
			(*(this->pVTable)->SetState)((CActor*)this, 6, -1);
			return 1;)
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
		CFxLightEmitter::ChangeColors(&this->fxLightEmitter, 0x80808080);
	})

	switch (this->actorState) {
	case 6:
		IMPLEMENTATION_GUARD(
		StateBoomyGetTarget(this);)
		break;
	case 7:
		IMPLEMENTATION_GUARD(
		StateBoomyGotoTarget(this);)
		break;
	case 8:
		IMPLEMENTATION_GUARD(
		StateBoomyGotoLauncher(this);)
		break;
	case 9:
		IMPLEMENTATION_GUARD(
		this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
		this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffe;
		this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffb;
		this->field_0x294 = 1;
		bVar1 = CFxLightEmitter::Manage(&this->fxLightEmitter, 0, 0);
		iVar3 = CFxTail::Manage(&this->fxTail, 0, 0.0, 1);
		bVar2 = CFxLightEmitter::Manage(&this->field_0x530, 0, 0);
		if ((bVar1 != false && iVar3 != 0) && bVar2 != false) {
			this->flags = this->flags & 0xfffffffd;
			this->flags = this->flags | 1;
			this->flags = this->flags & 0xffffff7f;
			this->flags = this->flags | 0x20;
			CActor::EvaluateDisplayState((CActor*)this);
			this->flags = this->flags & 0xfffffbff;
			CFxTail::Reset(&this->fxTail);
			CFxTail::SetPatchActive(&this->fxTail, 0);
		})
		break;
	case 10:
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
	CActor* pCVar1;

	if (this->curBoomyTypeId == 0) {
		this->aBoomyTypeInfo[0].field_0x4 = this->aBoomyTypeInfo[0].field_0x4;
		this->aBoomyTypeInfo[0].visionRange = this->aBoomyTypeInfo[0].visionRange;
		this->aBoomyTypeInfo[0].field_0x10 = 0xffffffff;
		this->aBoomyTypeInfo[0].field_0x14 = 4;
	}
	else {
		this->aBoomyTypeInfo[0].field_0x4 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
		this->aBoomyTypeInfo[0].visionRange = this->aBoomyTypeInfo[this->curBoomyTypeId].visionRange;
		this->aBoomyTypeInfo[0].field_0xc = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0xc;
		this->aBoomyTypeInfo[0].field_0x10 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x10;
		this->aBoomyTypeInfo[0].field_0x14 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x14;
	}

	pCVar1 = this->field_0x3e0;
	if ((((pCVar1 != (CActor*)0x0) && (pCVar1->typeID == 6)) && (this->curBoomyTypeId == 1)) &&
		(pCVar1->curBehaviourId == 8)) {
		this->aBoomyTypeInfo[0].field_0x4 = this->aBoomyTypeInfo[2].field_0x4;
		this->aBoomyTypeInfo[0].visionRange = this->aBoomyTypeInfo[1].visionRange;
		this->aBoomyTypeInfo[0].field_0xc = this->aBoomyTypeInfo[1].field_0xc;
		this->aBoomyTypeInfo[0].field_0x10 = this->aBoomyTypeInfo[1].field_0x10;
		this->aBoomyTypeInfo[0].field_0x14 = 4;
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

	IMPLEMENTATION_GUARD_FX(
	this->fxTail.Reset();
	this->fxTail.SetPatchActive(0);

	this->field_0x530.ResetRays();
	this->fxLightEmitter.ResetRays();)

	this->field_0x1dc = 0.1f;

	(this->pCollisionData)->actorFieldA = this->field_0x3e0;

	this->field_0x2c0 = this->aBoomyTypeInfo[this->curBoomyTypeId].field_0x4;
	this->field_0x294 = 1;

	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags | 2;
	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xffffffef;
	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffe;
	this->aBoomyTypeInfo[0].flags = this->aBoomyTypeInfo[0].flags & 0xfffffffb;

	this->field_0x610 = gF32Matrix4Unit;

	edF32VECTOR4_00449a00 = gF32Vertex4Zero;

	SetState(5, -1);

	return;
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
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}
	return;
}
