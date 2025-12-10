#include "ActorNoseMonster.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "ed3D/ed3DG2D.h"
#include "FileManager3D.h"
#include "EventManager.h"
#include "ActorHero.h"
#include "TimeController.h"

CActorNoseMonster::CActorNoseMonster()
{
	this->field_0x6fd = 0;

	return;
}

void CActorNoseMonster::Create(ByteCode* pByteCode)
{
	ZonePathEntryList* pZVar1;
	ed_zone_3d* peVar4;
	uint uVar5;
	int iVar6;
	float fVar7;

	CActorAutonomous::Create(pByteCode);

	this->zoneRef.index = pByteCode->GetS32();

	fVar7 = pByteCode->GetF32();
	this->field_0x384 = fVar7;
	if (fVar7 <= 0.0) {
		this->field_0x384 = 5.0f;
	}

	this->field_0x354 = pByteCode->GetF32();
	this->field_0x358 = pByteCode->GetF32();
	this->field_0x35c = pByteCode->GetF32();
	if (fVar7 <= 0.0) {
		this->field_0x35c = 5.0f;
	}

	this->field_0x360 = pByteCode->GetF32();

	fVar7 = (this->field_0x360 * 6.283185f) / 60.0f;
	this->field_0x360 = fVar7;
	if (fVar7 < 0.0f) {
		this->field_0x360 = 0.3141593f;
	}

	if (2.21f <= CScene::_pinstance->field_0x1c) {
		this->field_0x378 = pByteCode->GetF32();
	}
	else {
		this->field_0x378 = 0.0f;
	}

	if (2.21f <= CScene::_pinstance->field_0x1c) {
		this->field_0x37c = pByteCode->GetF32();
	}
	else {
		this->field_0x37c = 1.0f;
	}

	this->field_0x364 = pByteCode->GetU32();

	this->field_0x370 = pByteCode->GetF32();
	if ((fVar7 < 0.0f) || (3.141593f < fVar7)) {
		this->field_0x370 = 0.7853982f;
	}
	else {
		this->field_0x370 = fVar7 * 0.01745329f;
	}

	fVar7 = pByteCode->GetF32();
	this->field_0x374 = fVar7;
	if (fVar7 < 0.0f) {
		this->field_0x374 = 1.5f;
	}

	pZVar1 = (ZonePathEntryList*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)pZVar1->aEntries;
	if (pZVar1->nbEntries != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pZVar1->nbEntries * sizeof(ZonePathEntry);
	}
	this->pZonePathFollowList = pZVar1;


	this->field_0x36c = pByteCode->GetU32();

	this->fireShot.Create(pByteCode);

	this->field_0x630 = pByteCode->GetS32();
	this->field_0x380 = pByteCode->GetU32();
	this->field_0x690 = pByteCode->GetF32();
	this->field_0x694 = pByteCode->GetF32();
	this->field_0x698 = pByteCode->GetF32();
	this->field_0x69c = pByteCode->GetF32();
	this->field_0x6a0 = pByteCode->GetU32();
	this->field_0x6a4 = pByteCode->GetU32();
	this->field_0x6a8 = pByteCode->GetU32();

	this->ntfSwitch.Create(pByteCode);

	return;
}

uint aPustuleBoneIds[6] = {
	0x219C9DA3,
	0x219C97A3,
	0x2A4E8A9D,
	0x2A4E849D,
	0x324E46A4,
	0x324E40A4,
};

void CActorNoseMonster::Init()
{
	bool bVar1;
	ZonePathEntryList* pZVar2;
	uint uVar3;
	ZonePathEntry* pSVar4;
	S_NTF_TARGET_STREAM_REF* pSVar5;
	int iVar6;
	ZonePathEntry* pZVar7;
	uint uVar8;
	int iVar9;
	uint* puVar10;
	CAnimation* pAnim;

	CActorAutonomous::Init();

	this->zoneRef.Init();

	pZVar7 = this->pZonePathFollowList->aEntries;
	for (iVar9 = this->pZonePathFollowList->nbEntries; iVar9 != 0; iVar9 = iVar9 + -1) {
		pZVar7->zoneRef.Init();
		pZVar7->pathFollowRef.Init();
		pZVar7 = pZVar7 + 1;
	}

	pZVar2 = this->pZonePathFollowList;
	iVar9 = 0;
	if (pZVar2 != (ZonePathEntryList*)0x0) {
		iVar9 = pZVar2->nbEntries;
	}

	iVar6 = iVar9 + -1;
	uVar8 = this->field_0x364;
	if (0 < iVar6) {
		pSVar4 = pZVar2->aEntries + iVar9;
		do {
			uVar3 = pSVar4->pathFollowRef.Get()->splinePointCount;
			if (uVar8 < uVar3) {
				uVar8 = uVar3;
			}

			iVar6 = iVar6 + -1;
			pSVar4 = pSVar4 + -1;
		} while (0 < iVar6);
	}

	this->fireShot.Init();
	(this->fireShot).field_0x294 = uVar8;

	pAnim = this->pAnimationController;
	if (pAnim != (CAnimation*)0x0) {
		pAnim->RegisterBone(0x45544554);
		pAnim->RegisterBone(0x8d9674ed);
		pAnim->RegisterBone(0x75ada3bb);
		iVar9 = 5;
		puVar10 = aPustuleBoneIds + 5;
		do {
			pAnim->RegisterBone(*puVar10);
			puVar10 = puVar10 + -1;
			bVar1 = iVar9 != 0;
			iVar9 = iVar9 + -1;
		} while (bVar1);
	}
	
	this->ntfSwitch.Init();
	
	Reset();

	return;
}

void CActorNoseMonster::Term()
{
	bool bVar1;
	uint* pBoneId;
	int iVar3;
	CAnimation* pAnim;

	CActorAutonomous::Term();

	pAnim = this->pAnimationController;

	if (pAnim != (CAnimation*)0x0) {
		iVar3 = 5;
		pBoneId = aPustuleBoneIds + 5;
		do {
			pAnim->UnRegisterBone(*pBoneId);
			pBoneId = pBoneId + -1;
			bVar1 = iVar3 != 0;
			iVar3 = iVar3 + -1;
		} while (bVar1);

		pAnim->UnRegisterBone(0x75ada3bb);
		pAnim->UnRegisterBone(0x8d9674ed);
		pAnim->UnRegisterBone(0x45544554);
	}

	return;
}

void CActorNoseMonster::Reset()
{
	CinNamedObject30* pCVar1;
	float* pfVar2;
	float* pfVar3;
	S_NTF_TARGET_STREAM_REF* pSVar4;
	uint uVar5;
	float* pfVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	float fVar10;
	edF32VECTOR4 local_10;

	CActorAutonomous::Reset();

	this->field_0x6fc = 0;
	this->field_0x63c = 1.2f;
	this->field_0x68c = 1.2f;
	this->shootTimer = this->field_0x374;
	this->field_0x644 = -1;
	this->bBurstFire = 0;
	this->pustuleBits = this->field_0x380;
	uVar5 = this->pustuleBits;

	pfVar3 = this->field_0x670 + 5;
	pfVar6 = &this->field_0x688;
	while (pfVar2 = pfVar3, this->field_0x670 < pfVar6) {
		if ((uVar5 & 1) == 0) {
			*pfVar2 = this->field_0x384;
		}
		else {
			*pfVar2 = 0.0f;
		}
		uVar5 = uVar5 >> 1;
		pfVar3 = pfVar2 + -1;
		pfVar6 = pfVar2;
	}

	pCVar1 = this->pCinData;
	this->rotationEuler.xyz = pCVar1->rotationEuler;

	UpdatePosition(&this->currentLocation, true);

	this->rotationEuler.y = 0.0f;
	SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	UpdatePosition(&this->currentLocation, true);

	local_10.x = this->rotationQuat.x;
	local_10.z = this->rotationQuat.z;
	local_10.w = 0.0f;
	local_10.y = 0.0f;

	edF32Vector4NormalizeHard(&local_10, &local_10);
	fVar9 = GetAngleYFromVector(&local_10);
	this->field_0x668 = fVar9;
	this->field_0x66c = fVar9;
	this->field_0x688 = 0.0f;

	SetLookingAtOff();
	SetLookingAtBounds(this->field_0x690, this->field_0x694, this->field_0x698, this->field_0x69c);
	SetLookingAtBones(0x45544554, 0x75ada3bb);

	this->fireShot.Reset();
	this->ntfSwitch.Reset(this);

	if (this->field_0x6fd != 0) {
		SetBehaviour(NOSE_MONSTER_BEHAVIOUR_DEATH, -1, -1);
		SetState(0x11, -1);
	}

	return;
}

void CActorNoseMonster::CheckpointReset()
{
	CActorAutonomous::CheckpointReset();

	Reset();

	return;
}

struct S_SAVE_CLASS_NOSE_MONSTER
{
	int field_0x0;
};

void CActorNoseMonster::SaveContext(void* pData, uint mode, uint maxSize)
{
	if (mode == 1) {
		S_SAVE_CLASS_NOSE_MONSTER* pSaveData = reinterpret_cast<S_SAVE_CLASS_NOSE_MONSTER*>(pData);
		pSaveData->field_0x0 = this->curBehaviourId != NOSE_MONSTER_BEHAVIOUR_DEATH;
	}

	return;
}

void CActorNoseMonster::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_NOSE_MONSTER* pSaveData = reinterpret_cast<S_SAVE_CLASS_NOSE_MONSTER*>(pData);

	if ((mode == 1) && (pSaveData->field_0x0 == 0)) {
		this->field_0x6fd = 1;

		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;

		SetBehaviour(NOSE_MONSTER_BEHAVIOUR_DEATH, 0x11, -1);
	}

	return;
}

CBehaviour* CActorNoseMonster::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == NOSE_MONSTER_BEHAVIOUR_DEATH) {
		pBehaviour = &this->behaviourDeath;
	}
	else {
		if (behaviourType == NOSE_MONSTER_BEHAVIOUR_ATTACK) {
			pBehaviour = &this->behaviourAttack;
		}
		else {
			if (behaviourType == NOSE_MONSTER_BEHAVIOUR_SLEEP) {
				pBehaviour = &this->behaviourSleep;
			}
			else {
				pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorNoseMonster::_gStateCfg_NoseMonster[13] = {
	StateConfig(0x06, 0x0),
	StateConfig(0x07, 0x0),
	StateConfig(0x08, 0x304),
	StateConfig(0x09, 0x304),
	StateConfig(0x0a, 0x304),
	StateConfig(0x0b, 0x200),
	StateConfig(0x0c, 0x400),
	StateConfig(0x0d, 0x400),
	StateConfig(0x0e, 0x200),
	StateConfig(0x0f, 0x0),
	StateConfig(0x10, 0x0),
	StateConfig(0x11, 0x0),
	StateConfig(0x12, 0x0),
};

StateConfig* CActorNoseMonster::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_NoseMonster + state + -5;
	}

	return pStateConfig;
}

void CActorNoseMonster::UpdateLookingAt()
{
	bool bVar1;
	StateConfig* pSVar2;
	int nodeIndex;
	edF32MATRIX4* peVar4;
	edF32MATRIX3* peVar5;
	edF32VECTOR4* v0;
	uint* pBoneId;
	uint bits;
	int curBoneIndex;
	float fVar9;
	float fVar10;
	float fVar11;

	if ((GetStateFlags(this->actorState) & 0x200) != 0) {
		CActorAutonomous::UpdateLookingAt();
	}

	bits = this->pustuleBits;
	curBoneIndex = 5;
	pBoneId = aPustuleBoneIds + 5;
	do {
		nodeIndex = this->pAnimationController->anmSkeleton.NodeIndexFromID(*pBoneId);
		if ((bits & 1) != 0) {
			peVar5 = TheAnimStage.pConstantMatrixData + nodeIndex;
			peVar4 = (TheAnimStage.pRelativeTransformMatrixBuffer)->matrices + nodeIndex;
			peVar4->rowY = peVar5->rowY;
			peVar5 = TheAnimStage.pConstantMatrixData + nodeIndex;
			peVar4 = (TheAnimStage.pRelativeTransformMatrixBuffer)->matrices + nodeIndex;
			peVar4->rowX = peVar5->rowX;
			peVar5 = TheAnimStage.pConstantMatrixData + nodeIndex;
			peVar4 = (TheAnimStage.pRelativeTransformMatrixBuffer)->matrices + nodeIndex;
			peVar4->rowZ = peVar5->rowZ;
			(TheAnimStage.pRelativeTransformMatrixBuffer)->matrices[nodeIndex].da = 1.0f;
			v0 = &(TheAnimStage.pRelativeTransformMatrixBuffer)->matrices[nodeIndex].rowZ;
			edF32Vector4ScaleHard(0.0f, v0, v0);
		}

		bits = bits >> 1;
		pBoneId = pBoneId + -1;
		bVar1 = curBoneIndex != 0;
		curBoneIndex = curBoneIndex + -1;
	} while (bVar1);

	return;
}

void CActorNoseMonster::CinematicMode_Leave(int behaviourId)
{
	if (this->field_0x6fd != 0) {
		behaviourId = 4;
	}

	CActor::CinematicMode_Leave(behaviourId);

	return;
}

int CActorNoseMonster::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
}

void CActorNoseMonster::BehaviourInitState(int newState)
{
	uint uVar1;
	CCollision* pCol;
	bool bVar3;
	edF32MATRIX4* peVar4;
	S_NTF_TARGET_STREAM_REF* pSVar5;
	edF32VECTOR4* peVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	CAnimation* pAnim;

	switch (newState) {
	case 5:
		pCol = this->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 | 0x81000;
		break;
	case 6:
	{
		CFxHandle local_8;

		CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&local_8, this->field_0x6a8, FX_MATERIAL_SELECTOR_NONE);

		if (local_8.IsValid()) {
			local_8.SpatializeOnActor(2, this, 0x0);
			local_8.Start();
		}

		ntfSwitch.SwitchOn(this);
		break;
	}
	case NOSE_MONSTER_STATE_ATTACK_WAIT:
		SetLookingAtOn(2.0f);
		break;
	case 10:
		local_20.x = CActorHero::_gThis->currentLocation.x;
		local_20.z = CActorHero::_gThis->currentLocation.z;
		local_20.w = CActorHero::_gThis->currentLocation.w;
		local_20.y = CActorHero::_gThis->currentLocation.y - (CActorHero::_gThis->distanceToGround - 0.5f);
		fVar9 = this->field_0x35c;
		peVar4 = this->pAnimationController->GetCurBoneMatrix(0x8d9674ed);
		local_30 = peVar4->rowT;
		edF32Matrix4MulF32Vector4Hard(&local_30, &this->pMeshTransform->base.transformA, &local_30);
		this->fireShot.FireNewShot(fVar9, &local_30, &local_20, this);
		break;
	case NOSE_MONSTER_STATE_ATTACK_SHOOT:
		if (this->bBurstFire != 0) {
			this->field_0x63c = 0.0f;
			this->field_0x650 = this->field_0x370;
			fVar9 = edF32Between_Pi(this->rotationEuler.y);
			if (fVar9 < 0.0) {
				fVar9 = edF32Between_Pi(this->field_0x650 / 2.0f + this->rotationEuler.y);
				this->field_0x664 = fVar9;
				this->field_0x650 = -this->field_0x650;
			}
			else {
				fVar9 = edF32Between_Pi(this->rotationEuler.y - this->field_0x650 / 2.0f);
				this->field_0x664 = fVar9;
			}

			uVar1 = this->field_0x364;
			if ((int)uVar1 < 0) {
				fVar9 = (float)(uVar1 >> 1 | uVar1 & 1);
				fVar9 = fVar9 + fVar9;
			}
			else {
				fVar9 = (float)uVar1;
			}

			this->field_0x650 = this->field_0x650 / fVar9;
			fVar9 = edF32Between_Pi(this->field_0x664 + this->field_0x650);
			this->field_0x66c = fVar9;
			this->rotationEuler.y = this->field_0x664;
			SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
			this->field_0x654 = 0;
			return;
		}
		this->field_0x654 = 0;
	case NOSE_MONSTER_STATE_ATTACK_BURST_FIRE:
		if (((newState == NOSE_MONSTER_STATE_ATTACK_BURST_FIRE) && (pAnim = this->pAnimationController, pAnim != (CAnimation*)0x0)) &&
			(this->field_0x37c != 0.0f)) {
			pAnim->anmBinMetaAnimator.SetLayerTimeWarper(1.0f / this->field_0x37c, 0);
		}
		if (this->bBurstFire == 0) {
			local_40.x = this->rotationQuat.x;
			local_40.z = this->rotationQuat.z;
			local_40.w = 0.0f;
			local_40.y = 0.0f;
			edF32Vector4NormalizeHard(&local_40, &local_40);
			fVar9 = GetAngleYFromVector(&local_40);
			this->field_0x664 = fVar9;

			peVar6 = (this->pZonePathFollowList->aEntries[this->field_0x644].pathFollowRef.Get())->aSplinePoints;
			if (peVar6 == (edF32VECTOR4*)0x0) {
				peVar6 = &gF32Vertex4Zero;
			}
			else {
				peVar6 = peVar6 + this->field_0x654;
			}

			edF32Vector4SubHard(&eStack80, peVar6, &this->currentLocation);
			eStack80.y = 0.0f;
			edF32Vector4NormalizeHard(&eStack80, &eStack80);
			fVar9 = GetAngleYFromVector(&eStack80);
			fVar9 = edF32Between_Pi(fVar9);
			this->field_0x66c = fVar9;
		}
		else {
			fVar9 = this->field_0x664 + this->field_0x650;
			this->field_0x664 = fVar9;
			fVar9 = edF32Between_Pi(fVar9);
			this->field_0x664 = fVar9;
			fVar9 = this->field_0x66c + this->field_0x650;
			this->field_0x66c = fVar9;
			fVar9 = edF32Between_Pi(fVar9);
			this->field_0x66c = fVar9;
		}
		break;
	case 0x10:
		this->field_0x6fd = 1;

		ntfSwitch.pTargetStreamRef->SwitchOff(this);

		if (this->pAnimationController != (CAnimation*)0x0) {
			this->field_0x668 = this->rotationEuler.y;
			this->field_0x66c = ((this->pCinData)->rotationEuler).y;
			fVar9 = edF32GetAnglesDelta(this->field_0x668, this->field_0x66c);
			this->field_0x650 = fVar9;
		}
		break;
	case 0x11:
		pCol = this->pCollisionData;
		pCol->flags_0x0 = pCol->flags_0x0 & 0xfff7efff;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;
	}

	return;
}

void CActorNoseMonster::ManageBehaviourAttack()
{
	CAnimation* pAnim;
	edAnmLayer* peVar2;
	bool bVar3;
	edF32VECTOR4* peVar4;
	ed_g2d_manager* pManager;
	ed_g2d_material* pMaterial;
	ed_g2d_texture* peVar5;
	edF32VECTOR4* peVar6;
	edF32MATRIX4* peVar7;
	Timer* pTVar8;
	edF32VECTOR4* peVar9;
	float fVar10;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	(this->ntfSwitch).pStreamEventCamera->Manage(this);

	ManageMaterial();
	this->fireShot.ManageShots();
	ManageOrientation();

	switch (this->actorState) {
	case NOSE_MONSTER_STATE_ATTACK_WAIT:
		StateNoseMonsterWait();
		break;
	case 8:
		FUN_002f7230();
		break;
	case 9:
		pAnim = this->pAnimationController;
		if (pAnim == (CAnimation*)0x0) {
			SetState(10, -1);
		}
		else {
			if (pAnim->IsCurrentLayerAnimEndReached(0)) {
				SetState(10, -1);
			}
		}
		break;
	case 10:
		pAnim = this->pAnimationController;
		if (pAnim == (CAnimation*)0x0) {
			SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
		}
		else {
			if (pAnim->IsCurrentLayerAnimEndReached(0)) {
				SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
			}
		}
		break;
	case NOSE_MONSTER_STATE_ATTACK_SHOOT:
		pAnim = this->pAnimationController;
		if (pAnim == (CAnimation*)0x0) {
			SetState(NOSE_MONSTER_STATE_ATTACK_BURST_FIRE, -1);
		}
		else {
			if (pAnim->IsCurrentLayerAnimEndReached(0)) {
				if (this->bBurstFire == 0) {
					peVar9 = (this->pZonePathFollowList->aEntries[this->field_0x644].pathFollowRef.Get())->aSplinePoints;
					fVar10 = this->field_0x35c;
					if (peVar9 == (edF32VECTOR4*)0x0) {
						peVar9 = &gF32Vertex4Zero;
					}
					else {
						peVar9 = peVar9 + this->field_0x654;
					}

					peVar7 = pAnim->GetCurBoneMatrix(0x8d9674ed);
					local_40 = peVar7->rowT;
					edF32Matrix4MulF32Vector4Hard(&local_40, &this->pMeshTransform->base.transformA, &local_40);
					this->fireShot.FireNewShot(fVar10, &local_40, peVar9, this);
				}
				else {
					SetVectorFromAngleY(this->rotationEuler.y, &eStack32);
					edF32Vector4ScaleHard(this->field_0x65c, &eStack32, &eStack32);
					edF32Vector4AddHard(&eStack32, &eStack32, &this->currentLocation);
					eStack32.y = this->field_0x658;
					fVar10 = this->field_0x35c;
					peVar7 = this->pAnimationController->GetCurBoneMatrix(0x8d9674ed);
					local_30 = peVar7->rowT;
					edF32Matrix4MulF32Vector4Hard(&local_30, &this->pMeshTransform->base.transformA, &local_30);
					this->fireShot.FireNewShot(fVar10, &local_30, &eStack32, this);
				}

				this->field_0x654 = this->field_0x654 + 1;

				SetState(NOSE_MONSTER_STATE_ATTACK_BURST_FIRE, -1);
			}
		}
		break;
	case NOSE_MONSTER_STATE_ATTACK_BURST_FIRE:
		StateNoseMonsterBurstFire();
		break;
	case 0xd:
		this->field_0x6fc = 1;

		pAnim = this->pAnimationController;
		if (pAnim == (CAnimation*)0x0) {
			SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
		}
		else {
			if (pAnim->IsCurrentLayerAnimEndReached(0)) {
				SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
			}
		}
		break;
	case 0xe:
		if (0.0f < this->field_0x63c) {
			this->field_0x63c = this->field_0x63c - Timer::GetTimer()->cutsceneDeltaTime;
		}

		pAnim = this->pAnimationController;
		if (pAnim == (CAnimation*)0x0) {
			SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
		}
		else {
			if (pAnim->IsCurrentLayerAnimEndReached(0)) {
				SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
			}
		}
		break;
	case NOSE_MONSTER_STATE_ATTACK_PUSTULE_DESTROYED:
		StateNoseMonsterPustuleDestroyed();
	}

	UpdatePosition(&this->currentLocation, true);

	return;
}

int INT_00448ad4 = 3;

void CActorNoseMonster::ManageMaterial()
{
	ed_g2d_manager* pManager;
	ed_g2d_material* pMaterial;
	ed_g2d_texture* pTexture;
	edF32VECTOR4 animSpeedNormalExtruder;
	edF32VECTOR4* pAnimSpeedNormalExtruder;

	pManager = CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(this->pCinData->textureIndex);
	pMaterial = ed3DG2DGetG2DMaterialFromIndex(pManager, INT_00448ad4);

	if (((pMaterial != (ed_g2d_material*)0x0) && (pTexture = ed3DG2DGetTextureFromMaterial(pMaterial, 0), pTexture != (ed_g2d_texture*)0x0)) &&
		(pAnimSpeedNormalExtruder = &animSpeedNormalExtruder, pTexture->pAnimSpeedNormalExtruder != 0x0)) {
		memset(pAnimSpeedNormalExtruder, 0, sizeof(edF32VECTOR4));
		animSpeedNormalExtruder.z = this->field_0x688 * 0.5f;
		FUN_001156e0((this->pCinData)->textureIndex, INT_00448ad4, &animSpeedNormalExtruder, (edF32VECTOR4*)0x0);
	}

	return;
}

void CActorNoseMonster::ManageOrientation()
{
	int iVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	CActorHero* pHero;
	uint uVar5;
	StateConfig* pSVar7;
	float fVar8;
	float fVar9;
	float puVar10;
	float fVar11;
	edF32VECTOR4 eStack16;

	pHero = CActorHero::_gThis;

	if ((GetStateFlags(this->actorState) & 0x200) == 0) {
		if ((GetStateFlags(this->actorState) & 0x400) == 0) {
			SetLookingAtRotationHeight(1.0f, &pHero->currentLocation);
		}
		else {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				fVar8 = this->field_0x66c;
				this->rotationEuler.x = 0.0f;
				this->rotationEuler.y = fVar8;
				this->rotationEuler.z = 0.0f;
				SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
			}
			else {
				fVar8 = edF32GetAnglesDelta(this->field_0x664, this->field_0x66c);
				if (((this->pAnimationController->anmBinMetaAnimator).aAnimData)->animPlayState == 1) {
					fVar9 = this->pAnimationController->anmBinMetaAnimator.GetLayerAnimTime(0, 1);
				}
				else {
					fVar9 = 0.0f;
				}

				fVar8 = this->field_0x664 + fVar8 * fVar9;
				this->field_0x668 = fVar8;
				fVar8 = edF32Between_Pi(fVar8);
				this->field_0x668 = fVar8;
				this->rotationEuler.x = 0.0f;
				this->rotationEuler.y = fVar8;
				this->rotationEuler.z = 0.0f;
				SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
			}

			SetLookingAt(0.0f, 0.0f, 1.0f);
		}
	}
	else {
		edF32Vector4SubHard(&eStack16, &pHero->currentLocation, &this->currentLocation);
		eStack16.y = 0.0f;
		edF32Vector4NormalizeHard(&eStack16, &eStack16);
		fVar8 = GetAngleYFromVector(&eStack16);
		fVar8 = edF32Between_Pi(fVar8);
		fVar11 = this->field_0x360 * Timer::GetTimer()->cutsceneDeltaTime;
		fVar9 = edF32GetAnglesDelta(this->field_0x668, fVar8);
		if (fabs(fVar9) < fVar11) {
			this->field_0x668 = fVar8;
		}
		else {
			if (0.0f <= fVar9) {
				puVar10 = 1.0f;
			}
			else {
				puVar10 = -1.0f;
			}

			this->field_0x668 = this->field_0x668 + fVar11 * puVar10;
		}

		fVar9 = edF32Between_Pi(this->field_0x668);
		this->field_0x668 = fVar9;
		this->field_0x66c = fVar8;
		fVar8 = this->field_0x668;
		this->rotationEuler.x = 0.0f;
		this->rotationEuler.y = fVar8;
		this->rotationEuler.z = 0.0f;
		SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
		SetLookingAtRotationHeight(1.0f, &pHero->currentLocation);
	}

	return;
}

void CActorNoseMonster::StateNoseMonsterWait()
{
	int iVar2;
	float fVar3;
	CEventManager* pEventManager;
	CActorHero* pHero;

	fVar3 = Timer::GetTimer()->cutsceneDeltaTime;

	if (0.0f < this->field_0x63c) {
		this->field_0x63c = this->field_0x63c - fVar3;
	}

	if (0.0f < this->shootTimer) {
		this->shootTimer = this->shootTimer - fVar3;
	}

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pHero = CActorHero::_gThis;
	this->field_0x644 = 0;

	while (true) {
		if (this->pZonePathFollowList == (ZonePathEntryList*)0x0) {
			iVar2 = 0;
		}
		else {
			iVar2 = this->pZonePathFollowList->nbEntries;
		}

		if ((iVar2 <= this->field_0x644) || (iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId,
				this->pZonePathFollowList->aEntries[this->field_0x644].zoneRef.Get(), &pHero->currentLocation, 0), iVar2 == 1)) 
			break;

		this->field_0x644 = this->field_0x644 + 1;
	}

	if (this->pZonePathFollowList == (ZonePathEntryList*)0x0) {
		iVar2 = 0;
	}
	else {
		iVar2 = this->pZonePathFollowList->nbEntries;
	}

	if (this->field_0x644 == iVar2) {
		this->field_0x644 = -1;
	}

	fVar3 = this->field_0x68c * 0.75f;

	if ((this->field_0x63c <= fVar3) &&
		(fVar3 = ((fVar3 - this->field_0x63c) * 0.25f) / fVar3, this->field_0x688 = fVar3, 0.5f <= fVar3)) {
		FUN_002f7760();
	}

	if ((this->shootTimer < 0.0f) && (this->field_0x644 != -1)) {
		SetState(NOSE_MONSTER_STATE_ATTACK_SHOOT, -1);
	}

	if (((this->field_0x63c <= 0.0f) && (this->field_0x644 == -1)) &&
		(fVar3 = edF32GetAnglesDelta(this->field_0x668, this->field_0x66c), fabs(fVar3) < 0.3926991f)) {
		this->field_0x68c = 0.0f;
		SetState(8, -1);
	}

	return;
}

void CActorNoseMonster::FUN_002f7230()
{
	bool bVar4;
	float fVar6;

	CAnimation* pAnim = this->pAnimationController;
	bVar4 = false;
	if (pAnim == (CAnimation*)0x0) {
		bVar4 = true;
	}
	else {
		fVar6 = this->field_0x68c + Timer::GetTimer()->cutsceneDeltaTime;
		this->field_0x68c = fVar6;
		fVar6 = fVar6 / 0.75f + 0.25f;
		this->field_0x688 = fVar6;
		if (1.0f < fVar6) {
			this->field_0x688 = 1.0f;
		}

		if (0.5f <= this->field_0x688) {
			FUN_002f7760();
		}

		if (pAnim->IsCurrentLayerAnimEndReached(0)) {
			bVar4 = true;
		}
	}

	if (bVar4) {
		this->field_0x688 = 0.0f;
		SetState(9, -1);
	}

	return;
}

void CActorNoseMonster::StateNoseMonsterBurstFire()
{
	edAnmLayer* peVar1;
	CPathFollow* pCVar2;
	bool bVar3;
	edF32MATRIX4* peVar4;
	edF32VECTOR4* peVar5;
	uint uVar6;
	float fVar7;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;
	CAnimation* pAnim;

	pAnim = this->pAnimationController;
	if (pAnim != (CAnimation*)0x0) {
		if (pAnim->IsCurrentLayerAnimEndReached(0)) {
			if (this->bBurstFire == 0) {
				pCVar2 = this->pZonePathFollowList->aEntries[this->field_0x644].pathFollowRef.Get();
				peVar5 = pCVar2->aSplinePoints;
				uVar6 = pCVar2->splinePointCount;
				fVar7 = this->field_0x35c;
				if (peVar5 == (edF32VECTOR4*)0x0) {
					peVar5 = &gF32Vertex4Zero;
				}
				else {
					peVar5 = peVar5 + this->field_0x654;
				}
				peVar4 = pAnim->GetCurBoneMatrix(0x8d9674ed);
				local_30 = peVar4->rowT;
				edF32Matrix4MulF32Vector4Hard(&local_30, &this->pMeshTransform->base.transformA, &local_30);
				this->fireShot.FireNewShot(fVar7, &local_30, peVar5, this);
			}
			else {
				SetVectorFromAngleY(this->rotationEuler.y, &eStack16);
				edF32Vector4ScaleHard(this->field_0x65c, &eStack16, &eStack16);
				edF32Vector4AddHard(&eStack16, &eStack16, &this->currentLocation);
				eStack16.y = this->field_0x658;
				fVar7 = this->field_0x35c;
				peVar4 = this->pAnimationController->GetCurBoneMatrix(0x8d9674ed);
				local_20 = peVar4->rowT;
				edF32Matrix4MulF32Vector4Hard(&local_20, &this->pMeshTransform->base.transformA, &local_20);
				this->fireShot.FireNewShot(fVar7, &local_20, &eStack16, this);
				uVar6 = this->field_0x364;
			}

			this->field_0x654 = this->field_0x654 + 1;
			if (this->field_0x654 == uVar6) {
				SetState(0xd, -1);
			}
			else {
				if (this->bBurstFire == 0) {
					local_40.x = this->rotationQuat.x;
					local_40.z = this->rotationQuat.z;
					local_40.w = 0.0f;
					local_40.y = 0.0f;
					edF32Vector4NormalizeHard(&local_40, &local_40);
					this->field_0x664 = GetAngleYFromVector(&local_40);

					peVar5 = (this->pZonePathFollowList->aEntries[this->field_0x644].pathFollowRef.Get())->aSplinePoints;
					if (peVar5 == (edF32VECTOR4*)0x0) {
						peVar5 = &gF32Vertex4Zero;
					}
					else {
						peVar5 = peVar5 + this->field_0x654;
					}

					edF32Vector4SubHard(&eStack80, peVar5, &this->currentLocation);
					eStack80.y = 0.0f;
					edF32Vector4NormalizeHard(&eStack80, &eStack80);
					this->field_0x66c = edF32Between_Pi(GetAngleYFromVector(&eStack80));
				}
				else {
					fVar7 = this->field_0x664 + this->field_0x650;
					this->field_0x664 = fVar7;
					fVar7 = edF32Between_Pi(fVar7);
					this->field_0x664 = fVar7;
					fVar7 = this->field_0x66c + this->field_0x650;
					this->field_0x66c = fVar7;
					fVar7 = edF32Between_Pi(fVar7);
					this->field_0x66c = fVar7;
				}
			}
		}
	}

	return;
}

void CActorNoseMonster::StateNoseMonsterPustuleDestroyed()
{
	CAnimation* pAnim;
	bool bVar3;
	float fVar4;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;
	bool bIsDead;

	bIsDead = false;
	pAnim = this->pAnimationController;
	if (pAnim == (CAnimation*)0x0) {
		if (this->pustuleBits == 0x3f) {
			bIsDead = true;
		}
	}
	else {
		if ((pAnim->IsCurrentLayerAnimEndReached(0)) && (this->pustuleBits == 0x3f)) {
			bIsDead = true;
		}
	}
	if (bIsDead) {
		SetBehaviour(NOSE_MONSTER_BEHAVIOUR_DEATH, -1, -1);
	}
	else {
		if (this->pustuleBits != 0x3f) {
			local_10.x = CActorHero::_gThis->currentLocation.x;
			local_10.z = CActorHero::_gThis->currentLocation.z;
			local_10.w = CActorHero::_gThis->currentLocation.w;
			local_10.y = CActorHero::_gThis->currentLocation.y + 0.5f;
			this->field_0x688 = 0.0f;
			this->field_0x658 = local_10.y;
			edF32Vector4SubHard(&eStack32, &local_10, &this->currentLocation);
			fVar4 = edF32Vector4GetLengthSoft(&eStack32);
			this->field_0x65c = fVar4;
			this->shootTimer = this->field_0x374;
			this->bBurstFire = 1;
			SetState(NOSE_MONSTER_STATE_ATTACK_SHOOT, -1);
		}
	}

	return;
}


void CActorNoseMonster::FUN_002f7760()
{
	bool bVar1;
	CNewFx* pCVar2;
	uint* puVar3;
	uint uVar4;
	CFxHandle* pHandle;
	int iVar5;
	CFxManager* pFxManager;

	pFxManager = CScene::ptable.g_EffectsManager_004516b8;
	pHandle = this->field_0x6cc;
	uVar4 = this->pustuleBits;
	iVar5 = 5;
	puVar3 = aPustuleBoneIds + 5;
	do {
		if ((uVar4 & 1) == 0) {
			bVar1 = pHandle->IsValid();

			if (!bVar1) {
				pFxManager->GetDynamicFx(pHandle, this->field_0x6a4, FX_MATERIAL_SELECTOR_NONE);

				if (pHandle->IsValid()) {
					pHandle->SpatializeOnActor(2, this, *puVar3);
					pHandle->Start();
				}
			}
		}

		uVar4 = uVar4 >> 1;
		pHandle = pHandle + 1;
		puVar3 = puVar3 + -1;
		bVar1 = iVar5 != 0;
		iVar5 = iVar5 + -1;
	} while (bVar1);

	return;
}

void CBehaviourNoseMonster::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourNoseMonster::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorNoseMonster*>(pOwner);

	return;
}

void CBehaviourNoseMonster::End(int newBehaviourId)
{
	return;
}

void CBehaviourNoseMonster::InitState(int newState)
{
	this->pOwner->BehaviourInitState(newState);

	return;
}

void CBehaviourNoseMonster::TermState(int oldState, int newState)
{
	bool bVar1;
	CNewFx* pCVar2;
	CFxHandle* pCVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	CAnimation* pAnim;
	CActorNoseMonster* pNoseMonster;

	pNoseMonster = this->pOwner;

	if (oldState == 0xd) {
		pNoseMonster->field_0x63c = pNoseMonster->field_0x378;
		pNoseMonster->field_0x68c = pNoseMonster->field_0x378;
		pNoseMonster->bBurstFire = 0;
	}
	else {
		if (oldState == NOSE_MONSTER_STATE_ATTACK_BURST_FIRE) {
			pAnim = pNoseMonster->pAnimationController;
			if ((pAnim != (CAnimation*)0x0) && (pNoseMonster->field_0x37c != 0.0f)) {
				pAnim->anmBinMetaAnimator.SetLayerTimeWarper(1.0f, 0);
			}
		}
		else {
			if (oldState == 8) {
				pCVar3 = pNoseMonster->field_0x6cc;
				iVar4 = 5;
				do {
					if (pCVar3->IsValid()) {
						pCVar3->Stop();
					}

					pCVar3 = pCVar3 + 1;
					bVar1 = iVar4 != 0;
					iVar4 = iVar4 + -1;
				} while (bVar1);
			}
			else {
				if (oldState == 10) {
					fVar5 = pNoseMonster->field_0x354;
					if (pNoseMonster->field_0x358 != 0.0) {
						fVar7 = pNoseMonster->field_0x358 * fVar5;
						fVar6 = fVar5;
						do {
							fVar5 = edFRndGauss(fVar6, fVar7);
						} while (fVar5 < 0.0f);
					}

					pNoseMonster->field_0x63c = fVar5;
					pNoseMonster->field_0x68c = fVar5;
				}
			}
		}
	}

	return;
}

void CBehaviourNoseMonsterSleep::Manage()
{
	ed_zone_3d* pZone;
	int iVar7;
	CActorNoseMonster* pNoseMonster;

	pNoseMonster = this->pOwner;
	pNoseMonster->ManageMaterial();

	(pNoseMonster->ntfSwitch).pStreamEventCamera->Manage(pNoseMonster);

	pNoseMonster->ManageOrientation();

	iVar7 = pNoseMonster->actorState;
	if (iVar7 == 6) {
		CAnimation* pAnim = pNoseMonster->pAnimationController;
		if (pAnim == (CAnimation*)0x0) {
			pNoseMonster->SetBehaviour(NOSE_MONSTER_BEHAVIOUR_ATTACK, -1, -1);
		}
		else {
			if (pAnim->IsCurrentLayerAnimEndReached(0)) {
				pNoseMonster->SetBehaviour(NOSE_MONSTER_BEHAVIOUR_ATTACK, -1, -1);
			}
		}
	}
	else {
		if (((iVar7 == 5) && (pZone = (pNoseMonster->zoneRef).Get(), pZone != (ed_zone_3d*)0x0)) &&
			(iVar7 = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0), iVar7 == 1)) {
			pNoseMonster->SetState(6, -1);
		}
	}

	return;
}

void CBehaviourNoseMonsterSleep::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourNoseMonster::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNoseMonsterAttack::Manage()
{
	this->pOwner->ManageBehaviourAttack();

	return;
}

void CBehaviourNoseMonsterAttack::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourNoseMonster::Begin(pOwner, newState, newAnimationType);

	this->targetedPustuleIndex = 6;
	this->field_0x8 = 0;
	
	if (newState == -1) {
		this->pOwner->SetState(NOSE_MONSTER_STATE_ATTACK_WAIT, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourNoseMonsterAttack::End(int newBehaviourId)
{
	return;
}

int CBehaviourNoseMonsterAttack::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	CActorNoseMonster* pNoseMonster;
	CActorHero* pHero;
	StateConfig* pSVar4;
	edF32MATRIX4* peVar5;
	//CActorFighterVTable** ppCVar6;
	int newTargetedPustuleIndex;
	uint uVar8;
	int iVar9;
	float fVar10;
	//CActorFighterVTable* pCVar11;
	float fVar12;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;

	if (msg == 2) {
		_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
		pNoseMonster = this->pOwner;
		newTargetedPustuleIndex = pNoseMonster->actorState;

		if (((pNoseMonster->GetStateFlags(pNoseMonster->actorState) & 0x104) == 0x104) && (this->targetedPustuleIndex != 6)) {
			local_70 = pHitParam->field_0x20;
			edF32Vector4SubHard(&eStack128, &this->pOwner->currentLocation, &CActorHero::_gThis->currentLocation);
			local_70.y = 0.0f;
			eStack128.y = 0.0f;
			fVar12 = edF32Vector4DotProductHard(&eStack128, &local_70);
			if (0.0f < fVar12) {
				if (this->field_0x8 == 1) {
					fVar12 = this->pOwner->field_0x670[this->targetedPustuleIndex];
				}
				else {
					fVar12 = 1.0f;
				}

				pNoseMonster = this->pOwner;
				if (this->pOwner->field_0x670[this->targetedPustuleIndex] <= fVar12) {
					pNoseMonster->pustuleBits = 1 << (5U - this->targetedPustuleIndex & 0x1f) | pNoseMonster->pustuleBits;
					this->pOwner->field_0x670[this->targetedPustuleIndex] = 0.0f;

					CFxHandle local_8;
					CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&local_8, this->pOwner->field_0x6a0, FX_MATERIAL_SELECTOR_NONE);
					bVar1 = local_8.IsValid();
					if (bVar1) {
						local_8.SpatializeOnActor(2, this->pOwner, aPustuleBoneIds[this->targetedPustuleIndex]);
						local_8.Start();
					}

					this->targetedPustuleIndex = 6;
					pNoseMonster = this->pOwner;
					if (pNoseMonster->pustuleBits == 0x3f) {
						pNoseMonster->SetLookingAtOff();
						pNoseMonster->SetBehaviour(NOSE_MONSTER_BEHAVIOUR_DEATH, -1, -1);
						this->pOwner->fireShot.KillAllProjectiles();
						return 1;
					}

					pNoseMonster->SetState(NOSE_MONSTER_STATE_ATTACK_PUSTULE_DESTROYED, -1);

					return 1;
				}

				this->pOwner->field_0x670[this->targetedPustuleIndex] = this->pOwner->field_0x670[this->targetedPustuleIndex] - fVar12;
			}

			this->targetedPustuleIndex = 6;
		}
	}
	else {
		if (msg == 7) {
			GetPositionMsgParams* pGetPositionParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			if (((pGetPositionParams->field_0x0 == 0) || (pGetPositionParams->field_0x0 == 1)) &&
				(pNoseMonster = this->pOwner, 0.5f < pNoseMonster->field_0x688)) {
				pHero = CActorHero::_gThis;

				if ((pNoseMonster->GetStateFlags(pNoseMonster->actorState) & 0x104) == 0x104) {
					iVar9 = 6;
					fVar12 = 0.0f;
					uVar8 = this->pOwner->pustuleBits;
					newTargetedPustuleIndex = 6;

					while (bVar1 = iVar9 != 0, iVar9 = iVar9 + -1, bVar1) {
						if ((uVar8 & 1) == 0) {
							peVar5 = this->pOwner->pAnimationController->GetCurBoneMatrix(aPustuleBoneIds[iVar9]);
							edF32Matrix4MulF32Vector4Hard(&eStack32, &this->pOwner->pMeshTransform->base.transformA, &peVar5->rowT);
							if (fabs(eStack32.y - pHero->currentLocation.y) <= 1.0f) {
								edF32Vector4SubHard(&eStack48, &pHero->currentLocation, &this->pOwner->currentLocation);
								eStack48.y = 0.0f;

								pNoseMonster = this->pOwner;
								local_40.x = pNoseMonster->rotationQuat.x;
								local_40.z = pNoseMonster->rotationQuat.z;
								local_40.w = pNoseMonster->rotationQuat.w;
								local_40.y = 0.0f;

								fVar10 = edF32Vector4DotProductHard(&local_40, &eStack48);
								if (0.0f < fVar10) {
									local_50.x = pHero->rotationQuat.x;
									local_50.z = pHero->rotationQuat.z;
									local_50.w = pHero->rotationQuat.w;
									local_50.y = 0.0f;
									edF32Vector4NormalizeHard(&local_50, &local_50);
									edF32Vector4SubHard(&eStack96, &eStack32, &pHero->currentLocation);
									eStack96.y = 0.0f;
									edF32Vector4NormalizeHard(&eStack96, &eStack96);
									fVar10 = edF32Vector4DotProductHard(&local_50, &eStack96);
									if (fVar12 < fVar10) {
										edF32Vector4SubHard(&pGetPositionParams->vectorFieldB, &eStack32, &this->pOwner->currentLocation);
										fVar12 = fVar10;
										newTargetedPustuleIndex = iVar9;
									}
								}
							}
						}

						uVar8 = uVar8 >> 1;
					}

					this->targetedPustuleIndex = newTargetedPustuleIndex;

					if (newTargetedPustuleIndex != 6) {
						return 1;
					}
				}
			}
		}
		else {
			if (msg != 0x4a) {
				return 0;
			}

			if (pMsgParam == (void*)0x0) {
				this->field_0x8 = 1;
			}
			else {
				if ((int)pMsgParam == 1) {
					this->field_0x8 = 0;
				}
			}
		}
	}

	return 0;
}

void CBehaviourNoseMonsterDeath::Manage()
{
	int iVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	bool bVar4;
	bool bVar5;
	edF32VECTOR4* peVar6;
	ed_g2d_manager* pManager;
	ed_g2d_material* pMaterial;
	ed_g2d_texture* peVar7;
	edF32VECTOR4* peVar8;
	edF32VECTOR4* peVar9;
	edF32VECTOR4 local_10;
	CActorNoseMonster* pNoseMonster;

	pNoseMonster = this->pOwner;

	(pNoseMonster->ntfSwitch).pStreamEventCamera->Manage(pNoseMonster);
	pNoseMonster->ManageMaterial();
	pNoseMonster->fireShot.ManageShots();

	iVar1 = pNoseMonster->actorState;
	if ((iVar1 != 0x11) && (iVar1 == 0x10)) {
		pNoseMonster->field_0x688 = 0.0;
		pCVar2 = pNoseMonster->pAnimationController;
		bVar5 = false;
		if (pCVar2 == (CAnimation*)0x0) {
			bVar5 = true;
		}
		else {
			if (pCVar2->IsCurrentLayerAnimEndReached(0)) {
				bVar5 = true;
			}
		}

		if (bVar5) {
			pNoseMonster->SetState(0x11, -1);
		}
	}

	return;
}

void CBehaviourNoseMonsterDeath::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourNoseMonster::Begin(pOwner, newState, newAnimationType);

	CActorNoseMonster* pNoseMonster = this->pOwner;

	if (pNoseMonster->field_0x6fd == 0) {
		if (newState == -1) {
			pNoseMonster->SetState(0x10, -1);
		}
		else {
			pNoseMonster->SetState(newState, newAnimationType);
		}
	}
	else {
		pNoseMonster->SetState(0x11, -1);
	}

	return;
}
