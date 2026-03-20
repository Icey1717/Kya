#include "ActorStiller.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "MathOps.h"
#include "ActorHero.h"
#include "EventManager.h"

void CActorStiller::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	uint uVar3;
	int iVar4;
	float fVar5;

	CActor::Create(pByteCode);

	this->vision.Create(this, pByteCode);

	this->field_0x1d0 = pByteCode->GetU32();
	this->field_0x1d4 = pByteCode->GetU32();
	this->field_0x1d8 = pByteCode->GetU32();
	this->field_0x1dc = pByteCode->GetU32();

	this->pathFollowReader.Create(pByteCode);

	this->field_0x1e0 = pByteCode->GetF32();
	this->lifeBase.Create(pByteCode);

	this->field_0x1ec = pByteCode->GetF32();
	this->field_0x1f0 = pByteCode->GetF32();
	this->field_0x1f4 = pByteCode->GetF32();

	this->field_0x1f8 = pByteCode->GetU32();

	S_ACTOR_STREAM_REF* pActorStreamRef = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pActorStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pActorStreamRef->entryCount * 4;
	}
	this->field_0x1fc = pActorStreamRef;

	this->field_0x200 = pByteCode->GetS32();

	S_NTF_TARGET_STREAM_REF::Create(&this->field_0x204, pByteCode);

	S_STREAM_EVENT_CAMERA* pCameraEvent = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + sizeof(S_STREAM_EVENT_CAMERA);
	this->field_0x208 = pCameraEvent;

	S_NTF_TARGET_STREAM_REF::Create(&this->field_0x20c, pByteCode);

	pCameraEvent = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + sizeof(S_STREAM_EVENT_CAMERA);
	this->field_0x210 = pCameraEvent;

	this->field_0x214 = pByteCode->GetS32();


	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorStiller::Init()
{
	int* piVar1;
	S_STREAM_REF<CActor>* pRef;
	int iVar2;
	CActor* pCVar3;
	int iVar4;

	CActor::Init();

	this->pathFollowReader.Init();

	for (int i = 0; i < this->field_0x1fc->entryCount; i++) {
		this->field_0x1fc->aEntries[i].Init();
	}

	this->field_0x204->Init();
	this->field_0x208->Init();

	this->field_0x20c->Init();
	this->field_0x210->Init();

	for (int i = 0; i < this->field_0x1fc->entryCount; i++) {
		this->field_0x1fc->aEntries[i].Reset();
	}

	this->vision.Reset();

	if (this->field_0x214 == 0) {
		this->pathFollowReader.Reset();
	}

	this->lifeBase.Reset();

	this->pTarget = (CActor*)0x0;

	this->field_0x204->Reset();
	this->field_0x208->Reset(this);

	this->field_0x20c->Reset();
	this->field_0x210->Reset(this);

	this->pAnimationController->RegisterBone(this->field_0x1d0);
	this->pAnimationController->RegisterBone(this->field_0x1d4);

	this->addOnGenerator.Init(1);

	return;
}

void CActorStiller::Term()
{
	CActor::Term();
	this->addOnGenerator.Term();

	return;
}

void CActorStiller::Draw()
{
	CActor::Draw();
	this->vision.Draw();

	return;
}

void CActorStiller::Reset()
{
	CActor::Reset();

	for (int i = 0; i < this->field_0x1fc->entryCount; i++) {
		this->field_0x1fc->aEntries[i].Reset();
	}

	vision.Reset();

	if (this->field_0x214 == 0) {
		this->pathFollowReader.Reset();
	}

	lifeBase.Reset();

	this->pTarget = (CActor*)0x0;

	this->field_0x204->Reset();
	this->field_0x208->Reset(this);

	this->field_0x20c->Reset();
	this->field_0x210->Reset(this);

	return;
}

CBehaviour* CActorStiller::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == STILLER_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorStiller::_gStateCfg_STI[0xa] =
{
	StateConfig(0x0, 0x300),
	StateConfig(0x8, 0x4),
	StateConfig(0x9, 0x4),
	StateConfig(0x6, 0x4),
	StateConfig(0x7, 0x4),
	StateConfig(0xa, 0x100),
	StateConfig(0xa, 0x100),
	StateConfig(0x0, 0x100),
	StateConfig(0x0, 0x101),
	StateConfig(0x0, 0x4),
};

StateConfig* CActorStiller::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 0xa) < 5);
		pStateConfig = _gStateCfg_STI + state + -5;
	}

	return pStateConfig;
}

void CActorStiller::ChangeManageState(int state)
{
	StateConfig* pSVar1;
	int iVar2;
	uint uVar3;

	CActor::ChangeManageState(state);

	if (state == 0) {
		if ((GetStateFlags(this->actorState) & 0x100) == 0) {
			this->pAnimationController->UnRegisterBone(this->field_0x1d4);
		}
	}
	else {
		if ((GetStateFlags(this->actorState) & 0x100) == 0) {
			this->pAnimationController->RegisterBone(this->field_0x1d4);
		}
	}

	return;
}

CVision* CActorStiller::GetVision()
{
	return &this->vision;
}

int CActorStiller::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	undefined8 uVar2;
	float fVar3;
	float fVar4;
	uint uVar5;
	StateConfig* pSVar6;
	int iVar7;
	float fVar8;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	if (msg == 3) {
		IMPLEMENTATION_GUARD(
		if (this->field_0x214 != 0) {
			uVar2 = *(undefined8*)&this->currentLocation;
			local_20.z = this->currentLocation.z;
			local_20.w = this->currentLocation.w;
			local_20.x = (float)uVar2;
			local_20.y = (float)((ulong)uVar2 >> 0x20) + 0.5;
			ActorComponent34Func_00397180(&this->addOnGenerator, &local_20);
			SetState(0xe, -1);
			return 1;
		}

		SetState(0xd, -1);
			)
		return 1;
	}

	if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
		uVar5 = GetStateFlags(this->actorState) & 1;

		bVar1 = uVar5 != 0;
		if (bVar1) {
			bVar1 = this->field_0x214 != 0;
		}

		if (!bVar1) {
			bVar1 = (GetStateFlags(this->actorState) & 100) == 0;
		}

		if (bVar1) {
			GetPositionMsgParams* pGetPosMsgParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			if (this->field_0x1d4 != 0) {
				SV_GetBoneWorldPosition(this->field_0x1d4, &eStack16);
				edF32Vector4SubHard(&pGetPosMsgParams->vectorFieldB, &eStack16, &this->currentLocation);
				return 1;
			}

			pGetPosMsgParams->vectorFieldB = gF32Vector4Zero;
		}
	}
	else {
		if (msg != MESSAGE_KICKED) {
			iVar7 = CActor::InterpretMessage(pSender, msg, (GetPositionMsgParams*)pMsgParam);
			return iVar7;
		}

		uVar5 = GetStateFlags(this->actorState) & 1;

		bVar1 = uVar5 != 0;
		if (bVar1) {
			bVar1 = this->field_0x214 != 0;
		}

		if (!bVar1) {
			uVar5 = GetStateFlags(this->actorState) & 0x100;
			bVar1 = uVar5 == 0;
		}

		_msg_hit_param* pHitMsgParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);

		if ((bVar1) && (pHitMsgParam->projectileType == 4)) {
			if ((this->field_0x1f8 & 1) == 0) {
				this->lifeBase.LifeDecrease(pHitMsgParam->damage);
			}

			fVar8 = this->lifeBase.GetValue();
			if (0.0f < fVar8) {
				SetState(STILLER_STAND_STATE_DEAD, -1);
				this->lifeBase.SetValue(this->lifeBase.GetValueMax());
				return 1;
			}

			SetState(0xd, -1);
			return 1;
		}
	}

	return 0;
}

void CActorStiller::BehaviourStillerStand_InitState(int newState)
{
	int iVar1;
	int iVar2;
	CAnimation* pAnimation;

	if (newState == 8) {
		pAnimation = this->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->RegisterBone(this->field_0x1d0);
		}
	}
	else {
		if ((newState == 9) || (newState == 0xd)) {
			this->field_0x204->Switch(this);
			this->field_0x208->SwitchOn(this);
		}
	}

	return;
}

void CActorStiller::BehaviourStillerStand_Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float newValue;

	this->field_0x208->Manage(this);
	this->field_0x210->Manage(this);

	switch (this->actorState) {
	case STILLER_STAND_STATE_IDLE:
		bVar3 = CheckDetectArea();
		if (bVar3 != false) {
			SetState(6, -1);
		}
		break;
	case 6:
		bVar3 = CheckDetectArea();
		if (bVar3 == false) {
			SetState(10, -1);
		}
		else {
			bVar3 = CheckAttackArea();

			if (bVar3 == false) {
				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					SetState(7, -1);
				}
			}
			else {
				SetState(8, -1);
			}
		}
		break;
	case 7:
		bVar3 = CheckAttackArea();
		if (bVar3 == false) {
			bVar3 = CheckDetectArea();
			if (bVar3 == false) {
				SetState(10, -1);
			}
		}
		else {
			SetState(STILLER_STAND_STATE_ATTACK, -1);
		}
		break;
	case STILLER_STAND_STATE_ATTACK:
		StateAttack();
		break;
	case 9:
		if (((this->field_0x1f8 & 1) == 0) || (bVar3 = CheckAttackArea(), bVar3 == false)) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				if ((this->field_0x1f8 & 1) == 0) {
					SetState(STILLER_STAND_STATE_WAKE, -1);
				}
				else {
					SetState(7, -1);
				}
			}
		}
		else {
			SetState(STILLER_STAND_STATE_ATTACK, -1);
		}
		break;
	case 10:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			SetState(STILLER_STAND_STATE_IDLE, -1);
		}
		bVar3 = CheckDetectArea();
		if (bVar3 == false) {
			bVar3 = CheckAttackArea();
			if (bVar3 != false) {
				SetState(STILLER_STAND_STATE_ATTACK, -1);
			}
		}
		else {
			SetState(6, -1);
		}
		break;
	case STILLER_STAND_STATE_RESTORE:
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			this->lifeBase.SetValue(this->lifeBase.GetValueMax());
			SetState(STILLER_STAND_STATE_WAKE, -1);
		}
		break;
	case STILLER_STAND_STATE_WAKE:
		bVar3 = CheckDetectArea();
		if (bVar3 == false) {
			SetState(5, -1);
		}
		else {
			if (this->field_0x1f4 < this->timeInAir) {
				SetState(6, -1);
			}
		}
	}
	
	return;
}

void CActorStiller::BehaviourStillerStand_TermState(int oldState)
{
	int iVar1;
	int iVar2;
	CAnimation* pAnimation;

	if (oldState == 8) {
		pAnimation = this->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->UnRegisterBone(this->field_0x1d0);
		}
	}
	else {
		if (oldState == STILLER_STAND_STATE_WAKE) {
			for (int i = 0; i < this->field_0x20c->entryCount; i++) {
				this->field_0x20c->aEntries[i].Switch(this);
			}

			this->field_0x210->SwitchOn(this);
		}
	}
	return;
}

void CActorStiller::StateAttack()
{
	S_ACTOR_STREAM_REF* pSVar1;
	CAnimation* pCVar2;
	edAnmLayer* peVar3;
	CActorHero* pReceiver;
	bool bVar4;
	CActor* pTarget;
	edF32VECTOR4* peVar6;
	edF32VECTOR4* peVar7;
	CActor* pCVar8;
	int iVar9;
	int iVar10;
	int iVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack352;
	edF32VECTOR4 local_150;
	_msg_hit_param local_140;
	edF32VECTOR4 eStack192;
	edF32VECTOR4 local_b0;
	_msg_hit_param local_a0;
	edF32VECTOR4 local_20;
	_msg_hit_param* local_8;
	_msg_hit_param* local_4;

	SV_GetBoneWorldPosition(this->field_0x1d0, &local_20);
	(this->vision).location = local_20;
	(this->vision).rotationQuat = this->rotationQuat;

	pReceiver = CActorHero::_gThis;
	if ((this->pTarget != (CActor*)0x0) && (pTarget = this->vision.ScanForTarget(CActorHero::_gThis, 1), pTarget != (CActor*)0x0)) {
		local_a0.projectileType = 1;
		local_a0.damage = this->field_0x1ec;
		local_a0.field_0x30 = this->field_0x1e0;
		edF32Vector4SubHard(&eStack192, &this->currentLocation, &pReceiver->currentLocation);
		if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
			eStack192.y = 0.0f;
			edF32Vector4SafeNormalize1Hard(&local_b0, &eStack192);
		}
		else {
			peVar6 = this->pathFollowReader.GetWayPoint(0);
			peVar7 = this->pathFollowReader.GetWayPoint(1);
			edF32Vector4SubHard(&local_b0, peVar6, peVar7);
			local_b0.y = 0.0f;
			edF32Vector4SafeNormalize1Hard(&local_b0, &local_b0);
		}
		local_a0.field_0x20 = local_b0;
		DoMessage(pReceiver, MESSAGE_KICKED, &local_a0);
	}

	iVar11 = 0;
	while (true) {
		pSVar1 = this->field_0x1fc;
		iVar9 = 0;
		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar9 = pSVar1->entryCount;
		}

		if (iVar9 <= iVar11) break;

		pTarget = pSVar1->aEntries[iVar11].Get();
		pCVar8 = this->vision.ScanForTarget(pTarget, 1);
		if (pCVar8 != (CActor*)0x0) {
			local_140.projectileType = 1;
			local_140.damage = this->field_0x1ec;
			local_140.field_0x30 = this->field_0x1e0;
			edF32Vector4SubHard(&eStack352, &this->currentLocation, &pTarget->currentLocation);
			if ((this->pathFollowReader).pPathFollow == (CPathFollow*)0x0) {
				eStack352.y = 0.0f;
				edF32Vector4SafeNormalize1Hard(&local_150, &eStack352);
			}
			else {
				peVar6 = this->pathFollowReader.GetWayPoint(0);
				peVar7 = this->pathFollowReader.GetWayPoint(1);
				edF32Vector4SubHard(&local_150, peVar6, peVar7);
				local_150.y = 0.0f;
				edF32Vector4SafeNormalize1Hard(&local_150, &local_150);
			}

			local_140.field_0x20 = local_150;
			DoMessage(pTarget, MESSAGE_KICKED, &local_140);
		}

		iVar11 = iVar11 + 1;
	}

	if (this->field_0x1f0 < 0.0f) {
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			bVar4 = CheckAttackArea();
			if (bVar4 == false) {
				SetState(7, -1);
			}
			else {
				RestartCurAnim();
				this->timeInAir = 0.0f;
			}
		}
	}
	else {
		if (this->field_0x1f0 < this->timeInAir) {
			bVar4 = CheckAttackArea();
			if (bVar4 == false) {
				SetState(7, -1);
			}
			else {
				RestartCurAnim();
				this->timeInAir = 0.0f;
			}
		}
	}

	return;
}

bool CActorStiller::CheckDetectArea()
{
	S_ACTOR_STREAM_REF* piVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	ed_zone_3d* pZone;
	uint fVar;
	CEventManager* pEventManager;
	CActor* pNewTarget;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	fVar = this->field_0x1d8;
	if ((fVar != 0xffffffff) && (pZone = (ed_zone_3d*)0x0, fVar != 0xffffffff)) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, fVar);
	}

	pNewTarget = CActorHero::_gThis;
	iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
	if (iVar2 == 1) {
		this->pTarget = pNewTarget;
	}

	else {
		iVar5 = 0;
		while (true) {
			piVar1 = this->field_0x1fc;
			iVar4 = 0;
			if (piVar1 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar4 = piVar1->entryCount;
			}

			if (iVar4 <= iVar5) {
				this->pTarget = (CActorHero*)0x0;
				return false;
			}

			pNewTarget = this->field_0x1fc->aEntries[iVar5].Get();
			iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pNewTarget->currentLocation, 0);

			if (iVar3 == 1) break;

			iVar5 = iVar5 + 1;
		}

		this->pTarget = pNewTarget;
	}

	return true;
}

bool CActorStiller::CheckAttackArea()
{
	S_ACTOR_STREAM_REF* pSVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	bool bVar5;
	ed_zone_3d* pZone;
	CEventManager* pEventManager;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	bVar5 = false;
	if (this->field_0x1d8 != 0xffffffff) {
		pZone = (ed_zone_3d*)0x0;
		if (this->field_0x1dc != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->field_0x1dc);
		}
	}

	iVar2 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &CActorHero::_gThis->currentLocation, 0);
	if ((this->pTarget != (CActor*)0x0) && (iVar2 == 1)) {
		bVar5 = true;
	}

	iVar4 = 0;
	while (true) {
		pSVar1 = this->field_0x1fc;
		iVar3 = 0;
		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = pSVar1->entryCount;
		}

		if (iVar3 <= iVar4) break;

		iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, &pSVar1->aEntries[iVar4].Get()->currentLocation, 0);

		if (iVar3 == 1) {
			bVar5 = true;
		}

		iVar4 = iVar4 + 1;
	}

	return bVar5;
}

void CBehaviourStiller::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourStiller::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorStiller*>(pOwner);

	return;
}

int CBehaviourStiller::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourStiller::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourStillerStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourStillerStand::Manage()
{
	this->pOwner->BehaviourStillerStand_Manage();

	return;
}

void CBehaviourStillerStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorStiller*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourStillerStand::InitState(int newState)
{
	this->pOwner->BehaviourStillerStand_InitState(newState);

	return;
}

void CBehaviourStillerStand::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourStillerStand_TermState(oldState);

	return;
}

int CBehaviourStillerStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}