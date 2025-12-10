#include "ActorCheckpointManager.h"
#include "MemoryStream.h"
#include "ActorHero.h"
#include "EventManager.h"
#include "TimeController.h"
#include "LevelScheduler.h"
#include "WayPoint.h"

void CActorCheckpointManager::Create(ByteCode* pByteCode)
{
	int iVar1;
	int* piVar2;
	S_CHECKPOINT* pSVar3;
	EventChunk_24* pEVar4;
	CWayPoint* pMVar5;
	int iVar6;
	uint uVar7;
	ActorAndWaypoint* pAVar8;
	int iVar9;
	int iVar10;

	CActor::Create(pByteCode);
	this->aCheckpoints = (S_CHECKPOINT*)0x0;
	this->currentCheckpointIndex = -1;
	this->checkpointCount = pByteCode->GetS32();
	uVar7 = this->checkpointCount;
	if (uVar7 != 0) {
		this->aCheckpoints = new S_CHECKPOINT[uVar7];
		pSVar3 = this->aCheckpoints;
		iVar1 = 0;
		if (0 < this->checkpointCount) {
			do {
				pSVar3->pZone.index = pByteCode->GetS32();
				pSVar3->pWayPointA.index = pByteCode->GetS32();
				pSVar3->sectorId = pByteCode->GetS32();
				pSVar3->pWayPointB.index = pByteCode->GetS32();
				pSVar3->flags = pByteCode->GetU32();
				pSVar3->actorWaypointsCount = pByteCode->GetS32();

				uVar7 = pSVar3->actorWaypointsCount;
				if (uVar7 == 0) {
					pSVar3->aActorWaypoints = (ActorAndWaypoint*)0x0;
				}
				else {
					pSVar3->aActorWaypoints = new ActorAndWaypoint[uVar7];
					iVar6 = 0;
					if (0 < pSVar3->actorWaypointsCount) {
						do {
							pSVar3->aActorWaypoints[iVar6].pActor.index = pByteCode->GetS32();
							pSVar3->aActorWaypoints[iVar6].pWaypoint.index = pByteCode->GetS32();
							iVar6 = iVar6 + 1;
						} while (iVar6 < pSVar3->actorWaypointsCount);
					}
				}

				iVar1 = iVar1 + 1;
				pSVar3 = pSVar3 + 1;
			} while (iVar1 < this->checkpointCount);
		}
	}

	return;
}

void CActorCheckpointManager::Init()
{
	int iVar2;
	S_CHECKPOINT* pCheckpoint;
	int iVar3;

	CActor::Init();

	pCheckpoint = this->aCheckpoints;
	iVar3 = 0;
	if (0 < this->checkpointCount) {
		do {
			pCheckpoint->pZone.Init();
			pCheckpoint->pWayPointA.Init();
			pCheckpoint->pWayPointB.Init();

			if (pCheckpoint->pWayPointB.Get() != (CWayPoint*)0x0) {
				pCheckpoint->flags = pCheckpoint->flags | 0x40000000;
			}

			iVar2 = 0;
			if (0 < pCheckpoint->actorWaypointsCount) {
				do {
					pCheckpoint->aActorWaypoints[iVar2].pActor.Init();
					pCheckpoint->aActorWaypoints[iVar2].pWaypoint.Init();

					iVar2 = iVar2 + 1;
				} while (iVar2 < pCheckpoint->actorWaypointsCount);
			}
			iVar3 = iVar3 + 1;
			pCheckpoint = pCheckpoint + 1;
		} while (iVar3 < this->checkpointCount);
	}

	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;

	EvaluateDisplayState();
	return;
}

void CActorCheckpointManager::Reset()
{
	CActor::Reset();
	this->currentCheckpointIndex = -1;
	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;
	this->flags = this->flags & 0xffffff7f;
	this->flags = this->flags | 0x20;
	EvaluateDisplayState();
}

CBehaviour* CActorCheckpointManager::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 2) {
		pBehaviour = &this->behaviourCheckpoint;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

void CActorCheckpointManager::ManageCheckpoints()
{
	CActorHero* pHero;
	CEventManager* pEventManager;
	bool bVar2;
	int iVar3;
	int index;
	S_CHECKPOINT* pCheckpoint;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pHero = CActorHero::_gThis;
	if (CActorHero::_gThis != (CActorHero*)0x0) {
		pCheckpoint = this->aCheckpoints;
		index = 0;
		if (0 < this->checkpointCount) {
			do {
				if ((((pCheckpoint->pZone.Get() != (ed_zone_3d*)0x0) && (pCheckpoint->pWayPointA.Get() != (CWayPoint*)0x0)) &&
					(this->currentCheckpointIndex != index)) &&
					((iVar3 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pCheckpoint->pZone.Get(), &pHero->currentLocation, 0), iVar3 != 2 &&
						(bVar2 = pHero->CanActivateCheckpoint(pCheckpoint->flags), bVar2 != false)))) {
					ActivateCheckpoint(index);
					return;
				}

				index = index + 1;
				pCheckpoint = pCheckpoint + 1;
			} while (index < this->checkpointCount);
		}
	}
	return;
}

void CActorCheckpointManager::ActivateCheckpoint(int index)
{
	CWayPoint* pCVar2;
	CLevelScheduler* pLevelScheduleManager;
	ActorAndWaypoint* piVar4;
	Timer* pTVar3;
	int iVar4;
	int iVar5;
	S_CHECKPOINT* pPrevCheckpoint;
	S_CHECKPOINT* pNewCheckpoint;
	float fVar8;
	float fVar9;
	_evt_checkpoint_param evtChekpointParam;
	CActorAutonomous* pCVar1;

	pNewCheckpoint = this->aCheckpoints + index;
	if (pNewCheckpoint->pWayPointA.Get() != (CWayPoint*)0x0) {
		pPrevCheckpoint = (S_CHECKPOINT*)0x0;

		if (this->currentCheckpointIndex != -1) {
			pPrevCheckpoint = this->aCheckpoints + this->currentCheckpointIndex;
		}

		this->currentCheckpointIndex = index;

		evtChekpointParam.pWayPointA = pNewCheckpoint->pWayPointA.Get();
		evtChekpointParam.sectorId = pNewCheckpoint->sectorId;
		evtChekpointParam.pWayPointB = pNewCheckpoint->pWayPointB.Get();
		evtChekpointParam.flags = pNewCheckpoint->flags;
		CActorHero::_gThis->ActivateCheckpoint(&evtChekpointParam);

		IMPLEMENTATION_GUARD_AUDIO(
		SoundManagerFunc_001819e0(CScene::ptable.g_AudioManager_00451698);)

		if ((pPrevCheckpoint != (S_CHECKPOINT*)0x0) && (iVar5 = 0, 0 < pPrevCheckpoint->actorWaypointsCount)) {
			do {
				CActorMovable* pMovable = static_cast<CActorMovable*>(pPrevCheckpoint->aActorWaypoints[iVar5].pActor.Get());
				pMovable->field_0x1c0.x = 1e+30f;
				iVar5 = iVar5 + 1;
			} while (iVar5 < pPrevCheckpoint->actorWaypointsCount);
		}

		iVar5 = 0;
		if (0 < pNewCheckpoint->actorWaypointsCount) {
			iVar4 = 0;
			do {
				CActorMovable* pMovable = static_cast<CActorMovable*>(pNewCheckpoint->aActorWaypoints[iVar5].pActor.Get());
				pMovable->field_0x1c0.xyz = pNewCheckpoint->aActorWaypoints[iVar5].pWaypoint.Get()->location;
				pMovable->field_0x1c0.w = 1.0f;
				iVar5 = iVar5 + 1;
			} while (iVar5 < pNewCheckpoint->actorWaypointsCount);
		}

		pLevelScheduleManager = CLevelScheduler::gThis;
		pTVar3 = Timer::GetTimer();
		if (10.0f < pTVar3->scaledTotalTime) {
			pLevelScheduleManager->SetLevelTimerFunc_002df450(1.0f, 0);
		}
	}

	return;
}

void CBehaviourCheckpoint::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorCheckpointManager*>(pOwner);
}

void CBehaviourCheckpoint::Manage()
{
	this->pOwner->ManageCheckpoints();
}

void CBehaviourCheckpoint::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	if (newState == -1) {
		this->pOwner->SetState(0, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}
