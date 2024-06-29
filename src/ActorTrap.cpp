#include "ActorTrap.h"
#include "MemoryStream.h"
#include "ActorProjectile.h"
#include "MathOps.h"
#include "ActorHero.h"
#include "ActorManager.h"

CActorTrap::CActorTrap()
{
	this->dynamic.weightB = 0.0f;
}

void CActorTrap::Create(ByteCode* pByteCode)
{
	CActorMovable::Create(pByteCode);
	this->trapStandActorTrap.CreateForTrap(pByteCode);
}

void CActorTrap::Init()
{
	CActor::Init();
}

void CActorTrap::Draw()
{
	CActor::Draw();
}

void CActorTrap::Reset()
{
	CActorMovable::Reset();
}

CBehaviour* CActorTrap::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
		pBehaviour = new CBehaviourInactive;
	}
	else {
		if (behaviourType == 2) {
			pBehaviour = &this->trapStandActorTrap;
		}
		else {
			pBehaviour = CActor::BuildBehaviour(behaviourType);
		}
	}

	return pBehaviour;
}

AnimResult CActorTrap::gStateCfg_TRP[14] =
{
	AnimResult(0x6, 0x20000),
	AnimResult(0x7, 0x20000),
	AnimResult(0x8, 0x6),
	AnimResult(0x8, 0xA06),
	AnimResult(0x9, 0x12A06),
	AnimResult(0xA, 0x12A06),
	AnimResult(0xA, 0x12806),
	AnimResult(0xB, 0x12806),
	AnimResult(0xB, 0x12A06),
	AnimResult(0xC, 0x1100),
	AnimResult(0xC, 0x1100),
	AnimResult(0xC, 0x1100),
	AnimResult(0xC, 0x100),
	AnimResult(0x7, 0x100),
};

AnimResult* CActorTrap::GetStateCfg(int state)
{
	AnimResult* pStateCfg;

	if (state < 5) {
		pStateCfg = CActorMovable::GetStateCfg(state);
	}
	else {
		pStateCfg = gStateCfg_TRP + (state - 5);
	}

	return pStateCfg;
}

CBehaviourTrapStand_ActorTrap::CBehaviourTrapStand_ActorTrap()
{
	this->field_0x6c = (CinNamedObject30*)0x0;
	this->field_0x70 = (CActor*)0x0;
	this->field_0x74 = (CinNamedObject30*)0x0;
	this->field_0x78 = (CActor*)0x0;
}

void CBehaviourTrapStand_ActorTrap::Create(ByteCode* pByteCode)
{
	uint uVar1;
	int iVar2;

	uVar1 = pByteCode->GetU32();
	this->flags = uVar1;

	iVar2 = pByteCode->GetS32();
	this->field_0xc = iVar2;

	uVar1 = pByteCode->GetU32();
	this->field_0x28 = uVar1;

	return;
}

void CBehaviourTrapStand_ActorTrap::Manage()
{
	BehaviourTrapStand_Manage();
	return;
}

void CBehaviourTrapStand_ActorTrap::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorTrap* pTrap;

	CBehaviourTrapStand::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		pTrap = this->pOwner;
		pTrap->SetState(6, -1);
	}
	else {
		pTrap = this->pOwner;
		pTrap->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourTrapStand_ActorTrap::BehaviourTrapStand_Manage()
{
	CActorTrap* pTrap;
	CCollision* pCVar2;
	uint uVar3;
	CAnimation* pCVar4;
	edAnmLayer* peVar5;
	CinNamedObject30* pCVar6;
	int* piVar7;
	bool bVar8;
	AnimResult* pAVar9;
	long lVar10;
	uint uVar11;
	int iVar12;
	int iVar13;
	int iVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	edF32VECTOR4 local_20;
	undefined4 local_4;

	if (this->field_0x48 == (S_TRAP_STREAM_REF*)0x0) {
		iVar14 = 0;
	}
	else {
		iVar14 = this->field_0x48->entryCount;
	}

	iVar13 = 0;
	if (0 < iVar14) {
		do {
			this->field_0x48->aEntries[iVar13].eventCamera.Manage(this->pOwner);
			iVar13 = iVar13 + 1;
		} while (iVar13 < iVar14);
	}

	pTrap = this->pOwner;

	if (pTrap->pTiedActor != (CActor*)0x0) {
		iVar14 = pTrap->actorState;

		if (iVar14 == -1) {
			uVar11 = 0;
		}
		else {
			pAVar9 = pTrap->GetStateCfg(iVar14);
			uVar11 = pAVar9->flags_0x4;
		}

		if ((uVar11 & 0x20000) != 0) {
			pTrap = this->pOwner;
			local_20 = pTrap->baseLocation;
			this->pOwner->SV_UpdatePosition_Rel(&local_20, 0, 1, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
		}
	}
	
	pTrap = this->pOwner;

	switch (pTrap->actorState) {
	case 5:
		pCVar4 = pTrap->pAnimationController;
		peVar5 = (pCVar4->anmBinMetaAnimator).aAnimData;
		bVar8 = false;

		if ((peVar5->currentAnimDesc).animType == pCVar4->currentAnimType_0x30) {
			if (peVar5->animPlayState == 0) {
				bVar8 = false;
			}
			else {
				bVar8 = (peVar5->field_0xcc & 2) != 0;
			}
		}

		if (bVar8) {
			pTrap->SetState(6, -1);
		}
		break;
	case 6:
		lVar10 = DetectActor();
		if (lVar10 != 0) {
			if (lVar10 == 2) {
				this->pOwner->SetState(0x12, -1);
			}
			else {
				if (lVar10 == 1) {
					this->pOwner->SetState(7, -1);
				}
			}
		}
		break;
	case 7:
		if (0.18f < pTrap->timeInAir) {
			pCVar2 = pTrap->pCollisionData;
			pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
			local_4 = 1;
			this->pOwner->DoMessage(this->field_0x64, (ACTOR_MESSAGE)0x31, (MSG_PARAM)1);
			this->pOwner->SetState(8, -1);
		}
		break;
	case 8:
		IMPLEMENTATION_GUARD(
		StateTrapCatch_2_2(this, 0x12, (int**)&DAT_00000009);)
		break;
	case 9:
		IMPLEMENTATION_GUARD(
		StateTrapCatchStand_1_2(this, 10, 0xb);)
		break;
	case 10:
		IMPLEMENTATION_GUARD(
		StateTrapCatchStand_2_2(this, 0xb);)
		break;
	case 0xc:
		uVar11 = this->field_0x50;
		uVar3 = this->field_0x54;
		if (uVar3 == uVar11) {
			pTrap->SetState(0xe, -1);
		}
		else {
			fVar15 = pTrap->timeInAir;
			if ((int)uVar3 < 0) {
				fVar17 = (float)(uVar3 >> 1 | uVar3 & 1);
				fVar17 = fVar17 + fVar17;
			}
			else {
				fVar17 = (float)uVar3;
			}
			if ((int)uVar11 < 0) {
				fVar16 = (float)(uVar11 >> 1 | uVar11 & 1);
				fVar16 = fVar16 + fVar16;
			}
			else {
				fVar16 = (float)uVar11;
			}
			if ((fVar17 * this->field_0x4c) / fVar16 < fVar15) {
				pTrap->SetState(10, -1);
			}
			else {
				if (this->field_0x4c < fVar15) {
					pTrap->SetState(10, -1);
				}
			}
		}
		break;
	case 0xd:
		IMPLEMENTATION_GUARD(
		StateTrapCatchStruggle(this, 0xc, 0xe, 10);)
		break;
	case 0xe:
		if (0.1f < pTrap->timeInAir) {
			pTrap->SetState(0xf, -1);
		}
		break;
	case 0xf:
		pCVar4 = pTrap->pAnimationController;
		peVar5 = (pCVar4->anmBinMetaAnimator).aAnimData;
		bVar8 = false;

		if ((peVar5->currentAnimDesc).animType == pCVar4->currentAnimType_0x30) {
			if (peVar5->animPlayState == 0) {
				bVar8 = false;
			}
			else {
				bVar8 = (peVar5->field_0xcc & 2) != 0;
			}
		}

		if (bVar8) {
			pTrap->SetState(0x12, -1);
		}
		break;
	case 0x10:
		if (0.1f < pTrap->timeInAir) {
			pTrap->SetState(0x11, -1);
		}
		break;
	case 0x11:
		if (0.1f < pTrap->timeInAir) {
			pTrap->SetState(0x12, -1);
		}
		break;
	case 0x12:
		if (((this->field_0x78 == (CActor*)0x0) || (this->field_0x74 == (CinNamedObject30*)0x0)) ||
			(this->field_0x74 != this->field_0x78->pCinData)) {
			bVar8 = false;
		}
		else {
			bVar8 = true;
		}
		if ((!bVar8) && (1.0 < pTrap->timeInAir)) {
			if (this->field_0xc == pTrap->curBehaviourId) {
				pTrap->UpdatePosition(&pTrap->baseLocation, true);
				pTrap = this->pOwner;
				pCVar6 = pTrap->pCinData;
				pTrap->rotationEuler.xyz = pCVar6->rotationEuler;
				SetVectorFromAngles(&this->pOwner->rotationQuat, &this->pOwner->rotationEuler.xyz);
				pTrap = this->pOwner;
				if (pTrap->curBehaviourId == 2) {
					pTrap->SetBehaviour(-1, -1, -1);
					pTrap->SetBehaviour(2, 5, -1);
				}
			}
			else {
				pTrap->SetBehaviour(this->field_0xc, -1, -1);
			}
		}
	}

	if (((this->field_0x70 == (CActor*)0x0) || (this->field_0x6c == (CinNamedObject30*)0x0)) ||
		(this->field_0x6c != this->field_0x70->pCinData)) {
		bVar8 = false;
	}
	else {
		bVar8 = true;
	}

	if (bVar8) {
		pTrap = this->pOwner;
		iVar14 = pTrap->actorState;
		if (iVar14 == -1) {
			uVar11 = 0;
		}
		else {
			pAVar9 = pTrap->GetStateCfg(iVar14);
			uVar11 = pAVar9->flags_0x4 & 0x2000;
		}

		CActor* pOtherActor;
		if (((uVar11 == 0) && (pOtherActor = this->field_0x70, pOtherActor != (CActor*)0x0)) &&
			((this->field_0x6c != (CinNamedObject30*)0x0 && (this->field_0x6c == pOtherActor->pCinData)))) {
			pOtherActor->Manage();
		}
		else {
			pTrap = this->pOwner;
			iVar14 = pTrap->actorState;
			if (iVar14 == -1) {
				uVar11 = 0;
			}
			else {
				pAVar9 = pTrap->GetStateCfg(iVar14);
				uVar11 = pAVar9->flags_0x4 & 0x2000;
			}

			if (uVar11 != 0) {
				IMPLEMENTATION_GUARD_LOG(
					CParticlesManager::GetDynamicFx
					(CScene::ptable.g_EffectsManager_004516b8, (undefined4*)&this->field_0x6c, this->field_0x20,
						0xffffffffffffffff);)

					CActor* pOtherActor = this->field_0x70;
				if (((pOtherActor != (CActor*)0x0) && (this->field_0x6c != (CinNamedObject30*)0x0)) &&
					(this->field_0x6c == pOtherActor->pCinData)) {
					IMPLEMENTATION_GUARD(
						pOtherActor->CheckpointReset(2, this->pOwner, 0);)
				}
				pOtherActor = this->field_0x70;
				if (((pOtherActor != (CActor*)0x0) && (this->field_0x6c != (CinNamedObject30*)0x0)) &&
					(this->field_0x6c == pOtherActor->pCinData)) {
					pOtherActor->InitDlistPatchable();
				}
			}
		}
	}
	return;
}

void CBehaviourTrapStand_ActorTrap::CreateForTrap(ByteCode* pByteCode)
{
	uint uVar2;
	int iVar3;
	float fVar4;

	S_ACTOR_STREAM_REF* pStream = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pStream + 1);
	if (pStream->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (pStream->entryCount * sizeof(S_STREAM_REF<CActor>));
	}
	this->pActorStream = pStream;

	uVar2 = pByteCode->GetU32();
	this->field_0x14 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->field_0x18 = uVar2;
	uVar2 = pByteCode->GetU32();
	this->field_0x1c = uVar2;
	iVar3 = pByteCode->GetS32();
	this->field_0x20 = iVar3;

	if (CScene::_pinstance->field_0x1c < 2.24f) {
		this->field_0x24 = -1;
	}
	else {
		iVar3 = pByteCode->GetS32();
		this->field_0x24 = iVar3;
	}

	iVar3 = pByteCode->GetS32();
	this->actorRef.index = iVar3;

	fVar4 = pByteCode->GetF32();
	this->field_0x30 = fVar4;
	fVar4 = pByteCode->GetF32();
	this->field_0x34 = fVar4;
	this->pathFollowReader.Create(pByteCode);
	fVar4 = pByteCode->GetF32();
	this->field_0x4c = fVar4;
	uVar2 = pByteCode->GetU32();
	this->field_0x50 = uVar2;

	S_TRAP_STREAM_REF* pTrapStream = reinterpret_cast<S_TRAP_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = reinterpret_cast<char*>(pTrapStream + 1);
	if (pTrapStream->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pTrapStream->entryCount * sizeof(S_TRAP_STREAM_ENTRY);
	}
	this->field_0x48 = pTrapStream;

	return;
}

void CBehaviourTrapStand::Init(CActor* pOwner)
{
	int iVar12;

	this->pOwner = reinterpret_cast<CActorTrap*>(pOwner);

	S_ACTOR_STREAM_REF* pStreamRef = this->pActorStream;
	S_STREAM_REF<CActor>* pStreamEntry = pStreamRef->aEntries;
	for (iVar12 = pStreamRef->entryCount; iVar12 != 0; iVar12 = iVar12 + -1) {
		pStreamEntry->Init();
		pStreamEntry = pStreamEntry + 1;
	}

	actorRef.Init();

	pathFollowReader.Init();

	S_TRAP_STREAM_REF* pTrapStream = this->field_0x48;
	S_TRAP_STREAM_ENTRY* pTrapStreamEntry = pTrapStream->aEntries;
	for (iVar12 = pTrapStream->entryCount; iVar12 != 0; iVar12 = iVar12 + -1) {
		pTrapStreamEntry->onoff.Init();
		pTrapStreamEntry->eventCamera.Init();
		pTrapStreamEntry = pTrapStreamEntry + 1;
	}

	this->field_0x64 = (CActor*)0x0;
}

void CBehaviourTrapStand::SectorChange(int oldSectorId, int newSectorId)
{
	CActorTrap* pCVar1;

	if ((this->flags & 1) != 0) {
		pCVar1 = this->pOwner;
		if (this->field_0xc != pCVar1->curBehaviourId) {
			pCVar1->SetBehaviour(this->field_0xc, -1, -1);
		}
	}
	return;
}

void CBehaviourTrapStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorTrap* pCVar1;
	int iVar2;
	edF32VECTOR4* v0;
	int iVar3;
	S_TRAP_STREAM_ENTRY* pSwitchTarget;
	int iVar4;
	int entryCount;

	this->field_0x60 = this->pOwner->subObjA->boundingSphere.w;
	this->field_0x64 = (CActor*)0x0;

	if (this->field_0x48 == (S_TRAP_STREAM_REF*)0x0) {
		entryCount = 0;
	}
	else {
		entryCount = this->field_0x48->entryCount;
	}

	iVar4 = 0;
	if (0 < entryCount) {
		do {
			this->field_0x48->aEntries[iVar4].onoff.Reset();
			this->field_0x48->aEntries[iVar4].eventCamera.Reset(pOwner);
			iVar4 = iVar4 + 1;
		} while (iVar4 < entryCount);
	}

	this->field_0x5c = 0;
	if ((this->pathFollowReader).pPathFollow != (CPathFollow*)0x0) {
		this->pathFollowReader.Reset();
		pCVar1 = this->pOwner;
		v0 = this->pathFollowReader.GetWayPoint();
		pCVar1->UpdatePosition(v0, true);
		this->pathFollowReader.NextWayPoint();
	}

	this->field_0x68 = false;
	this->field_0x69 = true;

	pCVar1 = this->pOwner;
	pSwitchTarget = this->field_0x48->aEntries;
	for (entryCount = this->field_0x48->entryCount; entryCount != 0; entryCount = entryCount + -1) {
		pSwitchTarget->onoff.Reset();
		pSwitchTarget->eventCamera.Reset(pCVar1);
		pSwitchTarget = pSwitchTarget + 1;
	}

	return;
}

void CBehaviourTrapStand::InitState(int newState)
{
	int iVar1;
	KyaUpdateObjA* pKVar2;
	CCollision* pColissionData;
	CActorTrap* pTrap;
	CActor* pCVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	undefined4 local_14;
	undefined4 local_10;
	undefined4 local_c;
	undefined4 local_8;
	undefined4 local_4;

	switch (newState) {
	case 6:
		pColissionData = this->pOwner->pCollisionData;
		pColissionData->flags_0x0 = pColissionData->flags_0x0 & 0xfff7efff;
		pTrap = this->pOwner;
		pTrap->flags = pTrap->flags & 0xffffff5f;
		pTrap->EvaluateDisplayState();
		this->pOwner->dynamic.speed = 0.0f;
		break;
	case 7:
		pTrap = this->pOwner;
		local_30.xyz = pTrap->rotationQuat.xyz;
		local_30.y = 0.0f;
		edF32Vector4NormalizeHard(&local_30, &local_30);
		pTrap = this->pOwner;
		pTrap->rotationQuat = local_30;
		GetAnglesFromVector(&this->pOwner->rotationEuler, &this->pOwner->rotationQuat);
		break;
	case 9:
		if (this->field_0x64->typeID == 6) {
			local_4 = 0;
			this->pOwner->DoMessage(this->field_0x64, (ACTOR_MESSAGE)0x3f, (MSG_PARAM)0);
		}
		break;
	case 0xb:
		if ((this->actorRef.Get() != (CActor*)0x0) && (this->field_0x68 == false)) {
			CActorProjectile* pProjectile = reinterpret_cast<CActorProjectile*>(this->actorRef.Get());
			this->field_0x68 = true;
			pProjectile->AppearToDie();
		}
		break;
	case 0xc:
		if (this->pOwner->actorState != 0xd) {
			this->field_0x54 = 0;
		}

		if ((this->actorRef.Get() != (CActor*)0x0) && (this->field_0x68 == false)) {
			CActorProjectile* pProjectile = reinterpret_cast<CActorProjectile*>(this->actorRef.Get());
			this->field_0x68 = true;
			pProjectile->AppearToDie();
		}
		break;
	case 0xd:
		this->field_0x54 = 0;
		break;
	case 0xe:
		pColissionData = this->pOwner->pCollisionData;
		pColissionData->flags_0x0 = pColissionData->flags_0x0 & 0xfff7efff;
		pTrap = this->pOwner;
		iVar1 = pTrap->prevActorState;
		if ((iVar1 != 7) && (iVar1 != 8)) {
			local_40.x = pTrap->currentLocation.x;
			local_40.z = pTrap->currentLocation.z;
			local_40.w = pTrap->currentLocation.w;
			local_40.y = pTrap->currentLocation.y - 1.58f;
			this->pOwner->UpdatePosition(&local_40, true);
		}

		this->field_0x60 = ((this->pOwner->subObjA)->boundingSphere).w;
		pKVar2 = this->pOwner->subObjA;
		local_50.xyz = (pKVar2->boundingSphere).xyz;
		local_50.w = 0.0f;
		this->pOwner->SetLocalBoundingSphere(15.0f, &local_50);
		break;
	case 0xf:
		local_8 = 1;
		this->pOwner->DoMessage(this->field_0x64, (ACTOR_MESSAGE)0x32, (MSG_PARAM)1);
		pColissionData = this->pOwner->pCollisionData;
		pColissionData->flags_0x0 = pColissionData->flags_0x0 & 0xfff7efff;
		if (this->field_0x69 != false) {
			local_c = 0;
			this->pOwner->DoMessage(this->field_0x64, (ACTOR_MESSAGE)0x40, (MSG_PARAM)0);
		}
		break;
	case 0x10:
		pTrap = this->pOwner;
		iVar1 = pTrap->prevActorState;
		if ((iVar1 != 7) && (iVar1 != 8)) {
			local_60.x = pTrap->currentLocation.x;
			local_60.z = pTrap->currentLocation.z;
			local_60.w = pTrap->currentLocation.w;
			local_60.y = pTrap->currentLocation.y - 1.58f;
			this->pOwner->UpdatePosition(&local_60, true);
		}
		this->field_0x60 = ((this->pOwner->subObjA)->boundingSphere).w;
		pKVar2 = this->pOwner->subObjA;
		local_70.x = (pKVar2->boundingSphere).x;
		local_70.y = (pKVar2->boundingSphere).y;
		local_70.z = (pKVar2->boundingSphere).z;
		local_70.w = 0.0;
		this->pOwner->SetLocalBoundingSphere(15.0f, &local_70);
		break;
	case 0x11:
		pColissionData = this->pOwner->pCollisionData;
		pColissionData->flags_0x0 = pColissionData->flags_0x0 & 0xfff7efff;
		local_10 = 1;
		this->pOwner->DoMessage(this->field_0x64, (ACTOR_MESSAGE)0x32, (MSG_PARAM)1);
		if (this->field_0x69 != false) {
			local_14 = 0;
			this->pOwner->DoMessage(this->field_0x64, (ACTOR_MESSAGE)0x40, (MSG_PARAM)0);
		}
		break;
	case 0x12:
		pTrap = this->pOwner;
		pTrap->flags = pTrap->flags & 0xffffff7f;
		pTrap->flags = pTrap->flags | 0x20;
		pTrap->EvaluateDisplayState();
		if ((this->field_0x24 != 0xffffffff) && (this->pOwner->prevActorState == 6)) {
			IMPLEMENTATION_GUARD_LOG(
			CParticlesManager::GetDynamicFx(CScene::ptable.g_EffectsManager_004516b8, (undefined4*)(this + 1), this->field_0x24, 0xffffffffffffffff);)

			pTrap = this->pOwner;
			pCVar5 = this->field_0x78;
			if ((pCVar5 != (CActorTrap*)0x0) && (this->field_0x74 != (CinNamedObject30*)0x0) &&	(this->field_0x74 == pCVar5->pCinData)) {
				pCVar5->currentLocation.x = pTrap->currentLocation.x;
				pCVar5->currentLocation.y = pTrap->currentLocation.y - 2.0f;
				pCVar5->currentLocation.z = pTrap->currentLocation.z;
				pCVar5->currentLocation.w = pTrap->currentLocation.w;
			}

			pCVar5 = this->field_0x78;
			if (((pCVar5 != (CActorTrap*)0x0) && (this->field_0x74 != (CinNamedObject30*)0x0) &&
				(this->field_0x74 == pCVar5->pCinData))) {
				pCVar5->InitDlistPatchable();
			}
		}
	}
	return;
}

void CBehaviourTrapStand::TermState(int state, int)
{
	CActorTrap* pTrap;
	int iVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	S_STREAM_NTF_TARGET_SWITCH* pTargetSwitch;

	if (state == 0x12) {
		pTrap = this->pOwner;
		pTrap->flags = pTrap->flags & 0xffffff5f;
		pTrap->EvaluateDisplayState();
	}
	else {
		if (((state != 0xd) && (state != 7)) && (state == 8)) {
			iVar4 = 0;
			if (this->field_0x48 != (S_TRAP_STREAM_REF*)0x0) {
				iVar4 = this->field_0x48->entryCount;
			}

			iVar3 = 0;
			if (0 < iVar4) {
				do {
					pTrap = this->pOwner;

					this->field_0x48->aEntries[iVar3].onoff.Switch(pTrap);
					this->field_0x48->aEntries[iVar3].onoff.PostSwitch(pTrap);
					this->field_0x48->aEntries[iVar3].eventCamera.SwitchOn(pTrap);
					iVar3 = iVar3 + 1;
				} while (iVar3 < iVar4);
			}
		}
	}
	return;
}

int CBehaviourTrapStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

bool gTrpDetectCallback(CActor* pActor, void* pParams)
{
	bool bVar1;
	bool bVar2;
	uint uVar3;
	AnimResult* pAVar4;
	edF32VECTOR4* v2;
	float fVar5;
	CActorConeInfluence actorConeInfluence;
	edF32VECTOR4 local_10;

	CActor* pOtherActor = (CActor*)pParams;

	bVar1 = false;
	bVar2 = pActor->IsKindOfObject(4);
	if (((bVar2 != false) && (pActor->typeID != 0x2c)) && (pActor->typeID != 0x1c)) {
		if (pActor->actorState == -1) {
			uVar3 = 0;
		}
		else {
			pAVar4 = pActor->GetStateCfg(pActor->actorState);
			uVar3 = pAVar4->flags_0x4 & 0x400;
		}
		if (uVar3 == 0) {
			if (pActor->actorState == -1) {
				uVar3 = 0;
			}
			else {
				pAVar4 = pActor->GetStateCfg(pActor->actorState);
				uVar3 = pAVar4->flags_0x4 & 0x100;
			}
			if ((uVar3 != 0) || ((pActor->pCollisionData->flags_0x4 & 2) != 0)) {
				pOtherActor->GetBehaviour(2);
				bVar1 = true;
			}
		}
	}

	if (bVar1) {
		actorConeInfluence.field_0x20.x = 0.0f;
		actorConeInfluence.field_0x20.y = 0.0f;
		actorConeInfluence.field_0x20.z = 0.0f;
		actorConeInfluence.field_0x20.w = 0.0f;

		actorConeInfluence.field_0x8 = 3.0f;
		actorConeInfluence.field_0x0 = 1.2f;
		actorConeInfluence.field_0x4 = 1.5f;
		actorConeInfluence.field_0xc = 0.2f;

		actorConeInfluence.field_0x10 = 0.2f;
		actorConeInfluence.field_0x14 = 2;
		v2 = pOtherActor->GetBottomPosition();
		edF32Vector4SubHard(&local_10, &pOtherActor->currentLocation, v2);
		actorConeInfluence.field_0x20.y = local_10.y - 0.3f;
		actorConeInfluence.field_0x20.x = local_10.x;
		actorConeInfluence.field_0x20.z = local_10.z;
		actorConeInfluence.field_0x20.w = 0.0f;

		local_10.y = actorConeInfluence.field_0x20.y;

		fVar5 = pOtherActor->SV_AttractActorInAConeAboveMe(pActor, &actorConeInfluence);
		if (fVar5 < 2.5f) {
			return true;
		}
	}

	return false;
}

int CBehaviourTrapStand::DetectActor()
{
	CActorTrap* pCVar1;
	CActorHero* pCVar2;
	float fVar3;
	float fVar4;
	int iVar5;
	int iVar6;
	CActorHero* unaff_s0_lo;
	CActorHero* pReceiver;
	float fVar7;
	float fVar8;
	edF32VECTOR4 sphere;
	CFixedTable<CActor*, 64> intersectActorTable;
	undefined4 local_4;

	pCVar2 = CActorHero::_gThis;
	sphere.w = 1.5f;
	intersectActorTable.entryCount = 0;
	pCVar1 = this->pOwner;

	sphere.x = pCVar1->currentLocation.x;
	sphere.z = pCVar1->currentLocation.z;
	sphere.y = pCVar1->currentLocation.y - 0.4f;

	pReceiver = (CActorHero*)0x0;
	if ((this->flags & 8) == 0) {
		if (this->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = 0;
		}
		else {
			iVar6 = this->pActorStream->entryCount;
		}

		if (iVar6 == 0) {
			CScene::ptable.g_ActorManager_004516a4->cluster.GetActorsIntersectingSphereWithCriterion(&intersectActorTable, &sphere, gTrpDetectCallback, this->pOwner);
			if (intersectActorTable.entryCount != 0) {
				pReceiver = (CActorHero*)intersectActorTable.aEntries[0];
			}
		}
		else {
			iVar6 = 0;
			fVar7 = 1e-20f;
			while (this->field_0x64 == (CActor*)0x0) {
				if (this->pActorStream == (S_ACTOR_STREAM_REF*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = this->pActorStream->entryCount;
				}

				if (iVar5 <= iVar6) break;

				pCVar1 = this->pOwner;
				pCVar2 = (CActorHero*)this->pActorStream->aEntries[iVar6].Get();

				fVar8 = pCVar2->currentLocation.x - pCVar1->currentLocation.x;
				fVar3 = pCVar2->currentLocation.y - pCVar1->currentLocation.y;
				fVar4 = pCVar2->currentLocation.z - pCVar1->currentLocation.z;
				fVar8 = sqrtf(fVar8 * fVar8 + fVar3 * fVar3 + fVar4 * fVar4);

				if (fVar7 <= fVar8) {
					fVar8 = fVar7;
					pCVar2 = unaff_s0_lo;
				}

				unaff_s0_lo = pCVar2;
				iVar6 = iVar6 + 1;
				fVar7 = fVar8;
			}

			iVar6 = gTrpDetectCallback((CActor*)unaff_s0_lo, this->pOwner);
			if (iVar6 != 0) {
				pReceiver = unaff_s0_lo;
			}
		}
	}
	else {
		pCVar1 = this->pOwner;
		fVar7 = CActorHero::_gThis->currentLocation.x - pCVar1->currentLocation.x;
		fVar8 = CActorHero::_gThis->currentLocation.y - pCVar1->currentLocation.y;
		fVar3 = CActorHero::_gThis->currentLocation.z - pCVar1->currentLocation.z;
		if ((sqrtf(fVar7 * fVar7 + fVar8 * fVar8 + fVar3 * fVar3) < ((CActorHero::_gThis->pCollisionData)->pObbPrim->field_0x90).y + 1.5f) &&
			(iVar6 = gTrpDetectCallback((CActor*)CActorHero::_gThis, this->pOwner), iVar6 != 0)) {
			pReceiver = pCVar2;
		}
	}

	if (pReceiver != (CActorHero*)0x0) {
		iVar6 = 2;
		if (((this->flags & 1) == 0) || (pReceiver != CActorHero::_gThis)) {
			local_4 = 0;
			iVar6 = this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x3f, 0);
		}

		if (iVar6 == 2) {
			return 2;
		}

		if (iVar6 == 1) {
			this->field_0x64 = (CActor*)pReceiver;
			return 1;
		}
	}

	return 0;
}

void CBehaviourTrap::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorTrap*>(pOwner);
}
