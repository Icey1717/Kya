#include "ActorBlazer.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "EventManager.h"
#include "ActorHero.h"

void CActorBlazer::Create(ByteCode* pByteCode)
{
	S_TILT_CONFIG* pSVar1;
	CCollision* pCVar2;
	float fVar3;
	uint uVar4;
	CWayPoint* pCVar5;

	CActorAutonomous::Create(pByteCode);

	this->field_0x358 = pByteCode->GetU32();
	this->field_0x35c = pByteCode->GetU32();
	this->field_0x368 = pByteCode->GetF32();
	this->field_0x364 = pByteCode->GetF32();
	this->field_0x36c = pByteCode->GetF32();
	this->field_0x370 = pByteCode->GetU32();
	this->field_0x374 = pByteCode->GetF32();
	this->field_0x378 = pByteCode->GetF32();
	this->field_0x37c = pByteCode->GetF32();
	this->stunnedDuration = pByteCode->GetF32();
	this->field_0x384 = pByteCode->GetF32();

	fVar3 = pByteCode->GetF32();
	this->field_0x388 = fVar3 * 0.01745329f;

	if (2.24f <= CScene::_pinstance->field_0x1c) {
		(this->field_0x38c).index = pByteCode->GetS32();
		(this->field_0x390).index = pByteCode->GetS32();
	}

	this->field_0x398 = pByteCode->GetU32();
	this->field_0x394 = pByteCode->GetU32();
	this->pathFollowReader.Create(pByteCode);
	this->field_0x3ac = pByteCode->GetF32();
	this->field_0x3b0 = pByteCode->GetF32();
	this->field_0x3b4 = pByteCode->GetU32();
	this->field_0x3b8 = pByteCode->GetF32();

	this->fireShot.Create(pByteCode);

	pSVar1 = reinterpret_cast<S_TILT_CONFIG*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pSVar1 + 1);
	this->field_0x360 = pSVar1;

	pCVar2 = this->pCollisionData;
	if (pCVar2 != (CCollision*)0x0) {
		pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xffffffc4;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x800480;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x100000;
	}

	return;
}

void CActorBlazer::Init()
{
	CActorAutonomous::Init();

	StoreCollisionSphere();

	this->pathFollowReader.Init();
	this->fireShot.Init();
	this->field_0x360->tiltStreamDef.Init();
	this->field_0x360->pushStreamDef.Init();
	this->field_0x660.Init(this->field_0x360->field_0x0, this, &this->field_0x360->tiltStreamDef);
	this->field_0x660.pushData.Init();
	if (2.24f <= CScene::_pinstance->field_0x1c) {
		this->field_0x38c.Init();
		this->field_0x390.Init();
	}

	ClearLocalData();

	return;
}

void CActorBlazer::Reset()
{
	CActorAutonomous::Reset();
	ClearLocalData();
	this->fireShot.Reset();

	return;
}

void CActorBlazer::CheckpointReset()
{
	CActorMovable::CheckpointReset();

	this->field_0x344 = this->field_0x344 & 0xfe;

	return;
}

CBehaviour* CActorBlazer::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == BLAZER_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorBlazer::_gStateCfg_BLA[] = {
	 StateConfig(0x0C, 0x0000), // [0]
	 StateConfig(0x0D, 0x4000), // [1]
	 StateConfig(0x0E, 0x5004), // [2]
	 StateConfig(0x14, 0x0800), // [3]
	 StateConfig(0x15, 0x0800), // [4]
	 StateConfig(0x16, 0x2800), // [5]
	 StateConfig(0x16, 0x0800), // [6]
	 StateConfig(0x18, 0x4000), // [7]
	 StateConfig(0x16, 0x2800), // [8]
	 StateConfig(0x17, 0x4800), // [9]
	 StateConfig(0x00, 0x0001), // [10]
	 StateConfig(0x00, 0x0000), // [11]
};

StateConfig* CActorBlazer::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state - 6) < 12);
		pStateConfig = _gStateCfg_BLA + state + -6;
	}

	return pStateConfig;
}

int CActorBlazer::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	StateConfig* pSVar2;
	uint uVar3;
	int result;
	float fVar4;

	if (msg == 2) {
		_msg_hit_param* pHitParam = static_cast<_msg_hit_param*>(pMsgParam);

		if (pHitParam->projectileType == 4) {
			fVar4 = GetLifeInterface()->GetValue();
			if (0.0f < fVar4) {
				if ((GetStateFlags(this->actorState) & 0x4000) != 0) {
					LifeDecrease(pHitParam->damage);

					fVar4 = GetLifeInterface()->GetValue();
					if (fVar4 <= 0.0f) {
						SetState(BLAZER_STATE_STUNNED_START_A, -1);
					}
					else {
						if (this->currentAnimType == 0x14) {
							RestartCurAnim();
						}
						else {
							PlayAnim(0x14);
						}
					}

					return 1;
				}
			}
		}
		result = 0;
	}
	else {
		result = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

void CActorBlazer::ClearLocalData()
{
	if ((this->pathFollowReader).pPathFollow != (CPathFollow*)0x0) {
		this->pathFollowReader.Reset();
	}

	this->field_0x6fc = this->field_0x36c;
	this->field_0x700 = this->field_0x374;
	this->field_0x6f8 = 0.0f;
	this->field_0x704 = 0;
	this->field_0x6f4 = false;
	this->field_0x6f0 = (CActor*)0x0;
	this->field_0x70c = 1.0f;
	this->field_0x710 = false;
	this->field_0x711 = false;

	LifeRestore();

	this->field_0x660.Reset();
	this->field_0x660.pushData.Reset();
	RestoreCollisionSphere(0.0f);

	this->field_0x6c0 = this->baseLocation;

	return;
}

void CActorBlazer::BehaviourBlazerStand_InitState(int newState)
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	StateConfig* pSVar5;
	edF32VECTOR4* peVar6;
	uint uVar7;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if ((GetStateFlags(newState) & 0x1000) == 0) {
		iVar1 = this->prevActorState;
		uVar7 = 0;
		if (iVar1 != -1) {
			pSVar5 = GetStateCfg(iVar1);
			uVar7 = pSVar5->flags_0x4;
		}
		if ((uVar7 & 0x1000) != 0) {
			this->pAnimationController->UnRegisterBone(this->field_0x358);
			this->pAnimationController->UnRegisterBone(this->field_0x35c);
			SetLookingAtOff();
		}
	}

	if ((GetStateFlags(newState) & 0x1000) != 0) {
		if ((GetStateFlags(this->prevActorState) & 0x1000) == 0) {
			this->pAnimationController->RegisterBone(this->field_0x358);
			this->pAnimationController->RegisterBone(this->field_0x35c);
			SetLookingAtBones(this->field_0x35c, this->field_0x358);
			SetLookingAtBounds(-0.08726646f, 0.08726646f, -0.7853982f, 0.7853982f);
		}
	}
	switch (newState) {
	case BLAZER_STATE_ATTACK:
		this->field_0x6fc = 0.0f;
		this->dynamic.speed = 0.0f;
		this->pAnimationController->RegisterBone(this->field_0x3b4);
		break;
	case BLAZER_STATE_STUNNED_START_B:
		StoreCollisionSphere();

		local_10.x = 0.0f;
		local_10.w = 1.0f;
		local_10.y = 0.0f;
		local_10.z = 0.0f;

		local_20.x = 0.01f;
		local_20.y = 0.01f;
		local_20.z = 0.01f;
		local_20.w = 0.0f;

		ChangeCollisionSphere(0.5f, &local_20, &local_10);
		break;
	case 0xc:
		this->pathFollowReader.Reset();
		peVar6 = this->pathFollowReader.GetWayPoint();
		fVar2 = peVar6->x - this->currentLocation.x;
		fVar3 = peVar6->y - this->currentLocation.y;
		fVar4 = peVar6->z - this->currentLocation.z;
		this->field_0x708 = sqrtf(fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4) / this->field_0x3b0;
		peVar6 = this->pathFollowReader.GetWayPoint();
		edF32Vector4SubHard(&local_30, peVar6, &this->field_0x6c0);
		edF32Vector4SafeNormalize0Hard(&local_30, &local_30);
		this->dynamic.rotationQuat = local_30;
		this->dynamic.speed = this->field_0x708;
		break;
	case BLAZER_STATE_RESET:
		RestoreCollisionSphere(0.1f);
	}

	return;
}

void CActorBlazer::BehaviourBlazerStand_Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	CScene* pCVar4;
	StateConfig* pSVar5;
	uint uVar6;
	int iVar7;
	CLifeInterface* pCVar8;
	ulong uVar9;
	ulong uVar10;
	float fVar11;

	if (((GetStateFlags(this->actorState) & 0x1000) != 0) && (this->field_0x6f0 != (CActor*)0x0)) {
		SetLookingAtOn(0);
		SetLookingAtRotationHeight(3.141593f, &this->field_0x6f0->currentLocation);
	}

	FUN_003d8640();
	this->fireShot.ManageShots();
	pCVar4 = CScene::_pinstance;

	switch (this->actorState) {
	case BLAZER_STATE_IDLE:
		iVar7 = CheckArea();
		if (iVar7 == 1) {
			SetState(7, -1);
		}
		break;
	case 7:
		if (this->field_0x6f0 != (CActor*)0x0) {
			SV_UpdateOrientationToPosition2D(this->field_0x364 * 2.0f, &this->field_0x6f0->currentLocation);
		}

		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			iVar7 = FUN_003d7eb0(this->field_0x6f0);
			SetState(BLAZER_STATE_ATTACK, iVar7);
		}
		break;
	case BLAZER_STATE_ATTACK:
		StateBlazerAttack();
		break;
	case BLAZER_STATE_STUNNED_START_A:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(BLAZER_STATE_STUNNED_START_B, -1);
		}
		break;
	case BLAZER_STATE_STUNNED_START_B:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
				if (0.0f < this->stunnedDuration) {
					SetState(BLAZER_STATE_STUNNED, -1);
				}
				else {
					SetState(BLAZER_STATE_RESET, -1);
				}
			}
			else {
				SetState(0xc, -1);
			}
		}
		break;
	case 0xb:
		if (this->stunnedDuration < this->timeInAir) {
			SetState(BLAZER_STATE_RECOVER, -1);
		}
		break;
	case 0xc:
		IMPLEMENTATION_GUARD(
		ActorInStateFunc_003d6a20();)
		break;
	case 0xd:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(BLAZER_STATE_IDLE, -1);
		}
		else {
			iVar7 = CheckArea();
			if (iVar7 == 1) {
				SetState(7, -1);
			}
		}
		break;
	case BLAZER_STATE_RECOVER:
		uVar9 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
		CScene::_pinstance->field_0x38 = uVar9;
		uVar10 = pCVar4->field_0x38 * 0x343fd + 0x269ec3;
		pCVar4->field_0x38 = uVar10;
		(this->field_0x6e0).x = (this->field_0x6e0).x + (static_cast<float>((uint)(uVar9 >> 0x10) & 0x7fff) * 0.1f) / 32767.0f + -0.05f;
		(this->field_0x6e0).y = (this->field_0x6e0).y;
		(this->field_0x6e0).z = (this->field_0x6e0).z + (static_cast<float>((uint)(uVar10 >> 0x10) & 0x7fff) * 0.1f) / 32767.0f + -0.05f;
		(this->field_0x6e0).w = 0.0f;
		if (0.5f < this->timeInAir) {
			SetState(BLAZER_STATE_RESET, -1);
		}
		break;
	case BLAZER_STATE_RESET:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			iVar7 = CheckArea();
			if (iVar7 == 2) {
				SetState(BLAZER_STATE_IDLE, -1);
			}
			else {
				SetState(BLAZER_STATE_ATTACK, -1);
			}
		}
	}

	FUN_003d8060();

	if (this->currentAnimType == 0xf) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			PlayAnim(0x10);
		}
	}

	if (this->currentAnimType == 0x10) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			PlayAnim(0x11);
		}
	}

	if (this->currentAnimType == 0x14) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			pCVar8 = GetLifeInterface();
			fVar11 = pCVar8->GetValue();
			if (fVar11 <= 0.0f) {
				SetState(BLAZER_STATE_STUNNED_START_B, -1);
			}
			else {
				SetState(BLAZER_STATE_ATTACK, -1);
			}
		}
	}

	return;
}

void CActorBlazer::StateBlazerAttack()
{
	bool bVar3;
	int inAnimType;
	int iVar5;
	float fVar6;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	fVar6 = GetLifeInterface()->GetValue();
	if (0.0f < fVar6) {
		inAnimType = FUN_003d7eb0(this->field_0x6f0);

		iVar5 = CheckArea();
		if (iVar5 == 2) {
			SetState(0xd, -1);
		}
		else {
			if (((this->field_0x6f6 == false) || (iVar5 = this->currentAnimType, iVar5 - 0xfU < 2)) || (iVar5 == 0x11)) {
				if (this->field_0x6f5 != false) {
					iVar5 = this->currentAnimType;
					if (((iVar5 != 0xf) && (iVar5 != 0x10)) && (iVar5 != 0x11)) {
						this->field_0x6f8 = 0.0f;
						PlayAnim(0xf);
					}

					this->field_0x6f5 = false;
				}

				if ((this->field_0x6f0 != (CActor*)0x0) && (this->field_0x6f4 != false)) {
					SV_GetBoneWorldPosition(this->field_0x3b4, &eStack48);
					edF32Vector4SubHard(&eStack32, &this->field_0x6f0->currentLocation, &this->currentLocation);
					eStack32.y = 0.0f;
					edF32Vector4NormalizeHard(&eStack32, &eStack32);
					edF32Vector4ScaleHard(this->field_0x3b8, &eStack32, &eStack32);
					edF32Vector4AddHard(&eStack16, &eStack48, &eStack32);
					this->fireShot.FireNewShotStraight(&eStack48, &eStack16, this);
					this->field_0x6fc = this->field_0x6f8;
					this->field_0x6f8 = -10000.0f;
					this->field_0x704 = this->field_0x704 + 1;
				}
			}

			iVar5 = this->currentAnimType;
			if (iVar5 == 0x11) {
				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					if (this->field_0x6f5 == false) {
						PlayAnim(0xe);
					}
					else {
						this->field_0x6f8 = 0.0f;
						this->field_0x6fc = 0.0f;
						PlayAnim(0xf);
					}
				}
			}
			else {
				if (((iVar5 == 0xe) && (this->field_0x6f6 != false)) && (inAnimType != -1)) {
					PlayAnim(inAnimType);
				}
			}
		}
	}

	return;
}

void CActorBlazer::FUN_003d8640()
{
	bool bVar4;
	bool bVar5;
	float fVar7;
	bool bVar6;

	bVar6 = false;
	bVar5 = false;
	bVar4 = false;

	this->field_0x700 = this->field_0x700 + GetTimer()->cutsceneDeltaTime;

	if (this->field_0x700 < this->field_0x374) goto LAB_003d8760;

	this->field_0x700 = this->field_0x374;
	fVar7 = this->field_0x6fc + GetTimer()->cutsceneDeltaTime;
	this->field_0x6fc = fVar7;
	bVar4 = false;
	bVar5 = bVar6;
	if (((this->field_0x36c <= fVar7) && (bVar4 = true, this->actorState == 8)) && (this->currentAnimType == 0x10)) {
		fVar7 = this->field_0x6f8 + GetTimer()->cutsceneDeltaTime;
		this->field_0x6f8 = fVar7;
		if (fVar7 <= this->field_0x378) {
			if (!this->pAnimationController->IsCurrentLayerAnimEndReached(0)) goto LAB_003d8748;
		}
		bVar5 = true;
	}

LAB_003d8748:
	if (this->field_0x704 == this->field_0x370) {
		this->field_0x704 = 0;
		this->field_0x700 = 0.0f;
	}

LAB_003d8760:
	this->field_0x6f5 = bVar4;
	this->field_0x6f4 = bVar5;

	return;
}

void CActorBlazer::Blazer_UpdatePosition(edF32VECTOR4* pPosition, int param_3, CActorsTable* pActorsTable, edF32MATRIX4* param_5)
{
	undefined8 uVar1;
	bool bVar2;
	float fVar3;
	float fVar4;
	edF32MATRIX4 eStack144;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	bVar2 = SV_MOV_UpdatePush(this->field_0x360->field_0x0, &(this->field_0x660).pushData, &this->field_0x360->pushStreamDef);

	if ((bVar2 != false) || (this->field_0x6f6 != false)) {
		param_3 = 1;
	}

	local_10.x = 0.0f;
	local_10.y = -(this->field_0x660).pushData.oscValue.value;
	local_10.z = 0.0f;
	local_10.w = 0.0f;

	if ((this->flags & 0x1000) != 0) {
		SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	}

	bVar2 = SV_MOV_UpdateTilt(this->field_0x360->field_0x0, &this->field_0x660, &this->field_0x360->tiltStreamDef);
	if (bVar2 == false) {
		if ((param_3 == 0) && (this->pTiedActor == (CActor*)0x0)) {
			ManageDyn(4.0f, 0, pActorsTable);
		}
		else {
			GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
			SV_UpdatePosition_Rel(pPosition, param_3, 1, pActorsTable, &local_10);
		}
	}
	else {
		edQuatToMatrix4Hard(&(this->field_0x660).oscQuat.quat, &eStack80);
		edF32Matrix4FromEulerSoft(&eStack144, &this->field_0x6d0.xyz, "XYZ");
		eStack144.rowT = *pPosition;
		edF32Matrix4MulF32Matrix4Hard(&eStack144, &eStack80, &eStack144);
		if (param_5 != (edF32MATRIX4*)0x0) {
			edF32Matrix4MulF32Matrix4Hard(&eStack144, param_5, &eStack144);
		}

		SV_UpdateMatrix_Rel(&eStack144, 1, 1, pActorsTable, &local_10);
	}

	this->rotationQuat = this->rotationQuat;

	if ((this->flags & 0x1000) != 0) {
		GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
	}

	if ((this->field_0x6f6 != false) && (this->field_0x6f0 != (CActor*)0x0)) {
		bVar2 = SV_UpdateOrientationToPosition2D(this->field_0x364, &this->field_0x6f0->currentLocation);
		if (bVar2 == true) {
			this->field_0x6f6 = false;
		}

		if ((this->flags & 0x1000) != 0) {
			GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
		}
	}

	UpdatePosition(&this->currentLocation, false);

	return;
}

int CActorBlazer::CheckArea()
{
	uint zoneId;
	CActorHero* pHero;
	CEventManager* pEventManager;
	ed_zone_3d* peVar3;
	int iVar4;
	int iVar5;
	ed_zone_3d* unaff_s0_lo;
	ed_zone_3d* unaff_s1_lo;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	if (this->field_0x394 != 0xffffffff) {
		zoneId = this->field_0x398;
		peVar3 = (ed_zone_3d*)0x0;
		if (zoneId != 0xffffffff) {
			if (zoneId != 0xffffffff) {
				peVar3 = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
			}
			unaff_s1_lo = (ed_zone_3d*)0x0;
			unaff_s0_lo = peVar3;
			if (this->field_0x394 != 0xffffffff) {
				unaff_s1_lo = edEventGetChunkZone(pEventManager->activeChunkId, this->field_0x394);
			}
		}
	}

	pHero = CActorHero::_gThis;
	iVar4 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, unaff_s0_lo, &CActorHero::_gThis->currentLocation, 0);
	iVar5 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, unaff_s1_lo, &pHero->currentLocation, 0);

	if ((iVar5 == 1) || (iVar4 == 1)) {
		iVar5 = 1;
		if (iVar4 == 1) {
			this->field_0x6f0 = static_cast<CActor*>(pHero);
		}
		else {
			iVar5 = -1;
		}
	}
	else {
		this->field_0x6f0 = (CActor*)0x0;
		iVar5 = 2;
	}

	return iVar5;
}

int CActorBlazer::FUN_003d7eb0(CActor* param_2)
{
	int iVar1;
	float puVar2;
	float puVar3;
	float fVar2;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if ((this->flags & 0x1000) != 0) {
		SetVectorFromAngles(&this->rotationQuat, (edF32VECTOR3*)&this->rotationEuler);
	}

	iVar1 = -1;
	if (param_2 != (CActor*)0x0) {
		iVar1 = -1;
		edF32Vector4SubHard(&local_30, &param_2->currentLocation, &this->currentLocation);
		edF32Vector4NormalizeHard(&local_30, &local_30);
		local_10.x = this->rotationQuat.x;
		local_10.y = 0.0f;
		local_10.z = this->rotationQuat.z;
		local_10.w = 0.0f;
		edF32Vector4NormalizeHard(&local_10, &local_10);
		local_20.y = 0.0f;
		local_20.w = 0.0f;
		local_20.x = local_30.x;
		local_20.z = local_30.z;
		edF32Vector4NormalizeHard(&local_20, &local_20);

		GetTimer();

		puVar2 = edF32Vector4DotProductHard(&local_10, &local_20);
		if (1.0f < puVar2) {
			puVar3 = 1.0f;
		}
		else {
			puVar3 = -1.0f;
			if (-1.0f <= puVar2) {
				puVar3 = puVar2;
			}
		}

		fVar2 = acosf(puVar3);
		if (local_10.x * local_20.z - local_20.x * local_10.z < 0.0f) {
			if (0.7853982f < fVar2) {
				iVar1 = 0x13;
				this->field_0x6f6 = true;
			}
		}
		else {
			if (0.7853982f < fVar2) {
				iVar1 = 0x12;
				this->field_0x6f6 = true;
			}
		}
	}

	return iVar1;
}

void CActorBlazer::FUN_003d8060()
{
	StateConfig* pSVar2;
	uint uVar3;
	CActor** pCVar4;
	int iVar5;
	float fVar6;
	CActorsTable local_290;
	edF32VECTOR4 local_180;
	CActorsTable local_170;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	if ((GetStateFlags(this->actorState) & 0x800) != 0) {
		if (this->field_0x710 == false) {
			if ((this->flags & 0x1000) != 0) {
				GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
				this->flags = this->flags | 0x1000;
			}

			(this->field_0x6e0).x = 0.0f;
			(this->field_0x6e0).y = 0.0f;
			(this->field_0x6e0).z = 0.0f;
			(this->field_0x6e0).w = 0.0f;

			(this->field_0x6d0) = this->rotationEuler;
			this->field_0x710 = true;
			this->field_0x711 = false;
		}

		edF32Vector4ScaleHard(this->dynamic.speed * GetTimer()->cutsceneDeltaTime, &eStack16, &this->dynamic.rotationQuat);
		edF32Vector4AddHard(&this->field_0x6c0, &this->field_0x6c0, &eStack16);
		local_20 = (this->field_0x6c0);
		edF32Matrix4FromEulerSoft(&eStack96, &this->field_0x6e0.xyz, "XYZ");

		if (this->field_0x711 == false) {
			if ((GetStateFlags(this->actorState) & 0x2000) == 0) goto LAB_003d81b0;

			local_170.nbEntries = 0;
			Blazer_UpdatePosition(&local_20, 1, &local_170, &eStack96);

			iVar5 = 0;
			if (0 < local_170.nbEntries) {
				pCVar4 = local_170.aEntries;
				do {
					if ((*pCVar4)->typeID == ACTOR_HERO_PRIVATE) {
						this->timeInAir = 0.0f;
						break;
					}
					iVar5 = iVar5 + 1;
					pCVar4 = pCVar4 + 1;
				} while (iVar5 < local_170.nbEntries);
			}
		}
		else {
		LAB_003d81b0:
			Blazer_UpdatePosition(&local_20, 1, (CActorsTable*)0x0, &eStack96);
		}

		SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
	}

	if ((GetStateFlags(this->actorState) & 0x800) != 0) {
		return;
	}
	if (this->field_0x710 == true) {
		if ((this->flags & 0x1000) == 0) {
			SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
			this->flags = this->flags & 0xffffefff;
		}

		this->field_0x710 = false;
	}

	local_180 = (this->field_0x6c0);
	fVar6 = this->rotationEuler.y;
	this->rotationEuler.x = 0.0f;
	this->rotationEuler.y = fVar6;
	this->rotationEuler.z = 0.0f;

	if (this->field_0x711 == false) {
		if ((GetStateFlags(this->actorState) & 0x2000) != 0) {
			local_290.nbEntries = 0;
			Blazer_UpdatePosition(&local_180, 0, &local_290, (edF32MATRIX4*)0x0);
			iVar5 = 0;
			if (0 < local_290.nbEntries) {
				pCVar4 = local_290.aEntries;
				do {
					if ((*pCVar4)->typeID == ACTOR_HERO_PRIVATE) {
						this->timeInAir = 0.0f;
						break;
					}
					iVar5 = iVar5 + 1;
					pCVar4 = pCVar4 + 1;
				} while (iVar5 < local_290.nbEntries);
			}

			goto LAB_003d83a8;
		}
	}

	Blazer_UpdatePosition(&local_180, 0, (CActorsTable*)0x0, (edF32MATRIX4*)0x0);

LAB_003d83a8:
	GetAnglesFromVector((edF32VECTOR3*)&this->rotationEuler, &this->rotationQuat);

	return;
}

void CBehaviourBlazer::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourBlazer::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBlazer*>(pOwner);

	return;
}

int CBehaviourBlazer::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourBlazer::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourBlazerStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourBlazerStand::Manage()
{
	this->pOwner->BehaviourBlazerStand_Manage();
}

void CBehaviourBlazerStand::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CBehaviourBlazer::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(BLAZER_STATE_IDLE, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourBlazerStand::InitState(int newState)
{
	this->pOwner->BehaviourBlazerStand_InitState(newState);
}

void CBehaviourBlazerStand::TermState(int oldState, int newState)
{
	CinNamedObject30* pCVar1;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CActorBlazer* pBlazer;

	pBlazer = this->pOwner;
	switch (oldState) {
	case BLAZER_STATE_ATTACK:
		pBlazer->pAnimationController->UnRegisterBone(pBlazer->field_0x3b4);
		break;
	case BLAZER_STATE_STUNNED_START_B:
		local_10.w = 1.0f;
		local_10.x = 0.0f;
		local_10.y = 0.0f;
		local_10.z = 0.0f;

		local_20.w = 0.0f;
		local_20.x = 0.01f;
		local_20.y = 0.01f;
		local_20.z = 0.01f;

		pBlazer->ChangeCollisionSphere(0.0f, &local_20, &local_10);
		break;
	case 0xc:
		pBlazer->dynamic.speed = 0.0f;
		break;
	case 0xd:
		pCVar1 = pBlazer->pCinData;
		pBlazer->rotationEuler.xyz = pCVar1->rotationEuler;
		pBlazer->UpdatePosition(&pBlazer->baseLocation, true);
		break;
	case BLAZER_STATE_RESET:
		pBlazer->LifeRestore();
		pBlazer->RestoreCollisionSphere(0.0f);
	}

	return;
}

int CBehaviourBlazerStand::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return 0;
}
