#include "ActorCommander.h"
#include "MemoryStream.h"
#include "ActorManager.h"
#include "ActorWolfen.h"
#include "ActorHero.h"
#include "EventManager.h"
#include "MathOps.h"
#include "TimeController.h"
#include "CameraGame.h"
#include "CinematicManager.h"


StateConfig CActorCommander::_gStateCfg_CMD[4] =
{
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x300),
	StateConfig(0x0, 0x300),
	StateConfig(0x0, 0x0),
};

CActorCommander::CActorCommander()
{
	this->field_0x1e8 = 0;
}

void CActorCommander::Create(ByteCode* pByteCode)
{
	ulong uVar1;
	float fVar2;
	uint uVar3;
	float fVar4;
	CActor* pCVar5;
	int iVar6;
	int* piVar7;
	CTeamElt* pCVar8;
	int iVar9;
	ACommanderComponent_10* pAVar10;
	int iVar11;
	int iVar12;
	int iVar13;

	CActor::Create(pByteCode);
	this->detectAreaZoneId = pByteCode->GetU32();
	this->guardAreaZoneId = pByteCode->GetU32();
	this->field_0x180 = pByteCode->GetU32();
	this->newPathFindingId = pByteCode->GetS32();
	this->actorRef.index = pByteCode->GetS32();
	this->squad.Create(pByteCode);

	this->nbTeams = pByteCode->GetS32();
	uVar3 = this->nbTeams;
	if (uVar3 < 1) {
		this->aTeamElt = (CTeamElt*)0x0;
	}
	else {
		this->aTeamElt = new CTeamElt[uVar3];
	}

	iVar6 = 0;
	if (0 < this->nbTeams) {
		do {
			iVar9 = pByteCode->GetS32();
			this->aTeamElt[iVar6].enemyIndex = iVar9;
			iVar6 = iVar6 + 1;
		} while (iVar6 < this->nbTeams);
	}

	this->flags_0x18c = 0;
	this->curPathFindingId = -1;

	this->count_0x16c = pByteCode->GetS32();
	if (this->count_0x16c == 0) {
		this->aComponent_0x170 = (ACommanderComponent_10*)0x0;
	}
	else {
		this->aComponent_0x170 = new ACommanderComponent_10[this->count_0x16c];
		iVar9 = 0;
		iVar12 = 0;
		iVar6 = 0;
		if (0 < this->count_0x16c) {
			iVar13 = 0;
			do {
				iVar11 = pByteCode->GetS32();
				//*(int*)(&this->field_0x170->field_0x0 + iVar13) = iVar11;
				iVar11 = pByteCode->GetS32();
				iVar6 = iVar6 + 1;
				//*(int*)(&this->field_0x170->field_0x8 + iVar13) = iVar11;
				//piVar7 = (int*)(&this->field_0x170->field_0x0 + iVar13);
				//iVar9 = iVar9 + *piVar7;
				//piVar7[1] = iVar9;
				//iVar12 = iVar12 + piVar7[2];
				//*(int*)(&this->field_0x170->field_0xc + iVar13) = iVar12;
				//iVar13 = iVar13 + 0x10;
			} while (iVar6 < this->count_0x16c);
		}
	}

	this->camFigData.Create(pByteCode);

	static edF32VECTOR4 edF32VECTOR4_0040e720 = { 0.0f, 1.6f, 0.0f, 0.0f };

	this->camFigData.field_0x290 = edF32VECTOR4_0040e720;
	this->camFigData.IsValid(0);
	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::Init()
{
	CActorManager* pCVar1;
	bool bVar2;
	CActorFighter* pCVar3;
	CTeamElt* pCurTeam;
	int iVar5;
	CActorWolfen* pWolfen;

	CActor::Init();

	pCVar1 = CScene::ptable.g_ActorManager_004516a4;
	iVar5 = 0;
	if (0 < this->nbTeams) {
		do {
			pCurTeam = this->aTeamElt + iVar5;
			pCVar3 = (CActorFighter*)0x0;
			if (pCurTeam->enemyIndex != -1) {
				pCVar3 = static_cast<CActorFighter*>(pCVar1->aActors[pCurTeam->enemyIndex]);
			}
			pCurTeam->pEnemyActor = pCVar3;

			pWolfen = static_cast<CActorWolfen*>(pCurTeam->pEnemyActor);
			if (pWolfen->IsKindOfObject(OBJ_TYPE_WOLFEN) != false) {
				pWolfen->pCommander = this;
			}

			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbTeams);
	}

	this->actorRef.Init();

	ClearLocalData();

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::Term()
{
	CTeamElt* pCVar1;

	pCVar1 = this->aTeamElt;
	if (pCVar1 != (CTeamElt*)0x0) {
		if (pCVar1 != (CTeamElt*)0x0) {
			delete pCVar1;
		}

		this->aTeamElt = (CTeamElt*)0x0;
	}

	this->nbTeams = 0;

	this->squad.Term();

	CActor::Term();

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::Manage()
{
	int iVar1;
	undefined8 uVar2;
	long lVar3;
	int iVar4;
	float fVar5;
	edF32VECTOR4 eStack112;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CTeamElt* piVar1;

	if (this->pTiedActor != (CActor*)0x0) {
		local_10 = this->baseLocation;
		local_20 = this->currentLocation;
		SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);

		iVar1 = this->squad.NbElt();
		if (iVar1 != 0) {
			edF32Vector4SubHard(&eStack112, &this->currentLocation, &local_20);
			edF32Matrix4TranslateHard(&eStack96, &gF32Matrix4Unit, &eStack112);
			(this->squad).chessboard.FUN_00352d90(&eStack96);
		}
	}

	_UpdateSquad();
	CheckExorcism();
	CheckArea_IntruderDetectArea();
	CheckArea_IntruderGuardArea();
	CheckArea_SoldiersDetectArea();
	CheckArea_SoldiersGuardArea();

	this->field_0x194 = 0;

	iVar1 = 0;
	if (0 < this->nbTeams) {
		do {
			CActorWolfen* pEntryWolfen = static_cast<CActorWolfen*>(this->aTeamElt[iVar1].pEnemyActor);
			if (((0.0f < pEntryWolfen->GetLifeInterface()->GetValue()) && (pEntryWolfen->IsKindOfObject(OBJ_TYPE_WOLFEN) != false)) &&
				((pEntryWolfen->combatFlags_0xb78 & 7) != 0)) {
				this->field_0x194 = this->field_0x194 + 1;
			}

			iVar1 = iVar1 + 1;
		} while (iVar1 < this->nbTeams);
	}

	this->bInCombat_0x1b0 = 0;
	CActor::Manage();

	_UpdateCamera();
	this->field_0x2f4 = 0;

	return;
}


// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::Reset()
{
	CActor::Reset();
	ClearLocalData();

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
CBehaviour* CActorCommander::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == COMMANDER_BEHAVIOUR_BEAT_UP) {
		pBehaviour = new CBehaviourCommanderBeatUp;
	}
	else {
		if (behaviourType == 2) {
			pBehaviour = &this->behaviourCommanderDefault;
		}
		else {
			if (behaviourType == 0) {
				pBehaviour = new CBehaviourCommanderDefault;
			}
			else {
				pBehaviour = CActor::BuildBehaviour(behaviourType);
			}
		}
	}

	return pBehaviour;
}

StateConfig* CActorCommander::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_CMD + state + -5;
	}

	return pStateConfig;
}

void CActorCommander::ChangeManageState(int state)
{
	if ((state == 0) && ((this->camFigData).field_0x2a0 != 0)) {
		this->camFigData.IsValid(0);
	}

	CActor::ChangeManageState(state);

	return;
}

int CActorCommander::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorFighter* pCVar1;
	CEventManager* pCVar2;
	bool bVar3;
	CPathFinderClient* this_00;
	ed_zone_3d* pZone;
	int iVar4;
	CTeamElt* pCVar5;
	CTeamElt* pTeamElt;
	int iVar6;
	edF32VECTOR4 local_10;

	if (msg == 0x10) {
		if (this->count_0x16c != 0) {
			this->flags_0x18c = this->flags_0x18c & 0xfffffff7;
			this->field_0x174 = 0;
			return 1;
		}
	}
	else {
		if (msg == 0xf) {
			if (this->count_0x16c != 0) {
				this->flags_0x18c = this->flags_0x18c | 8;
				this->field_0x174 = pSender;

				if (this->field_0x194 < 1) {
					pSender->SV_GetActorColCenter(&local_10);
					this->targetPosition = local_10;
				}

				pSender->SV_GetGroundPosition(&local_10);

				pCVar1 = this->aTeamElt->pEnemyActor;
				this_00 = pCVar1->GetPathfinderClient();
				pCVar2 = CScene::ptable.g_EventManager_006f5080;
				pZone = (ed_zone_3d*)0x0;
				if (this->guardAreaZoneId != 0xffffffff) {
					pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->guardAreaZoneId);
				}

				iVar4 = edEventComputeZoneAgainstVertex(pCVar2->activeChunkId, pZone, &local_10, 0);
				if ((iVar4 == 1) &&
					((this_00->id == -1 || (bVar3 = this_00->IsValidPosition(&local_10), bVar3 != false)))) {
					this->targetGroundPosition = local_10;
					this->bInCombat_0x1b0 = 1;
				}

				return 1;
			}
		}
		else {
			if (msg == 0x1b) {
				int* pMsg = reinterpret_cast<int*>(pMsgParam);

				iVar4 = *pMsg;
				if (iVar4 == 3) {
					return 1;
				}

				if (iVar4 == 2) {
					this->field_0x9f4 = static_cast<CActorFighter*>(pSender);
					pCVar1 = this->field_0x9f4;
					iVar4 = 0;
					pTeamElt = (CTeamElt*)0x0;
					if (0 < this->nbTeams) {
						pCVar5 = this->aTeamElt;
						do {
							pTeamElt = pCVar5;
							if (pTeamElt->pEnemyActor == pCVar1) break;
							iVar4 = iVar4 + 1;
							pCVar5 = pTeamElt + 1;
							pTeamElt = (CTeamElt*)0x0;
						} while (iVar4 < this->nbTeams);
					}

					this->squad.RemoveFighter(pTeamElt);
					pCVar1->SetStandAnim(-1);

					SetBehaviour(COMMANDER_BEHAVIOUR_BEAT_UP, -1, -1);
					return 1;
				}

				if (iVar4 == 0) {
					this->exorcismActorsTable.Add(pSender);
					return 1;
				}
			}
			else {
				int* pMsg = reinterpret_cast<int*>(pMsgParam);

				if ((msg == 0x1a) && ((*pMsg == 0xd || (*pMsg == 0xc)))) {
					iVar4 = 0;
					if (0 < this->nbTeams) {
						do {
							if (this->aTeamElt[iVar4].pEnemyActor != pSender) {
								DoMessage(pSender, (ACTOR_MESSAGE)msg, pMsgParam);
							}

							iVar4 = iVar4 + 1;
						} while (iVar4 < this->nbTeams);
					}

					return 1;
				}
			}
		}
	}

	iVar4 = CActor::InterpretMessage(pSender, msg, pMsgParam);
	return iVar4;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::ClearLocalData()
{
	float fVar1;
	float fVar2;
	float fVar3;
	CTeamElt* puVar4;
	int iVar4;
	int iVar5;
	InitPathfindingClientMsgParams local_20;

	this->flags_0x18c = 0;

	if (this->curPathFindingId != this->newPathFindingId) {
		this->curPathFindingId = this->newPathFindingId;
		local_20.msgId = 10;
		local_20.newId = this->curPathFindingId;
		iVar5 = 0;
		if (0 < this->nbTeams) {
			do {
				local_20.pActor = this->aTeamElt[iVar5].pEnemyActor;
				DoMessage(this->aTeamElt[iVar5].pEnemyActor, (ACTOR_MESSAGE)0x1a, &local_20);
				iVar5 = iVar5 + 1;
			} while (iVar5 < this->nbTeams);
		}
	}

	this->squad.Clear();

	this->field_0x1e0 = 0;
	//this->field_0x1e4 = 0;
	//this->field_0x1e8 = 0;
	this->field_0x194 = 0;
	this->targetPosition = gF32Vertex4Zero;
	this->targetGroundPosition = gF32Vertex4Zero;

	this->bInCombat_0x1b0 = 1;
	this->count_0x1d0 = 0;

	iVar5 = 0;
	if (0 < this->nbTeams) {
		do {
			puVar4 = this->aTeamElt + iVar5;
			puVar4->field_0x0 = 2;
			puVar4->field_0x4 = 2;
			puVar4->field_0x14 = -1;

			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbTeams);
	}

	this->camFigData.pCommanderRef_0x230 = this;
	this->camFigData.Reset();
	this->field_0x9f0 = (CActorFighter*)0x0;
	this->field_0x9f4 = (CActorFighter*)0x0;
	//this->field_0x164 = 0;
	//this->field_0x160 = 0;
	this->field_0x168 = 0;
	this->field_0x174 = (CActor*)0x0;
	this->field_0x2f4 = 0;
	this->field_0x6d0 = 1;
	return;
}

bool CActorCommander::BeginFightIntruder(CActor* pInstigator, CActor* pIntruder)
{
	bool bSuccess;

	bSuccess = false;

	if (pIntruder->IsKindOfObject(OBJ_TYPE_FIGHTER) != false) {
		CActorFighter* pActorFighter = static_cast<CActorFighter*>(pInstigator);
		bSuccess = this->squad.AddFighter(GetTeamElt(pActorFighter));
		pActorFighter->SetStandAnim(0xe2);
	}

	return bSuccess;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::EndFightIntruder(CActorFighter* pIntruder)
{
	this->squad.RemoveFighter(GetTeamElt(pIntruder));
	pIntruder->SetStandAnim(-1);

	return;
}

void CActorCommander::AddTracked()
{
	this->count_0x1d0 = this->count_0x1d0 + 1;

	if ((this->flags & 2) == 0) {
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
	}

	return;
}

void CActorCommander::RemoveTracked()
{
	this->count_0x1d0 = this->count_0x1d0 + -1;

	if ((this->count_0x1d0 == 0) && ((this->flags & 2) != 0)) {
		this->flags = this->flags & 0xfffffffc;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
CActorFighter* CActorCommander::GetIntruder()
{
	CActor* pIntruder;

	pIntruder = this->actorRef.Get();
	if (pIntruder == (CActor*)0x0) {
		pIntruder = CActorHero::_gThis;
	}

	return static_cast<CActorFighter*>(pIntruder);
}

int CActorCommander::CheckDetectArea(edF32VECTOR4* v0)
{
	CEventManager* pEventManager;
	ed_zone_3d* pZone;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pZone = (ed_zone_3d*)0x0;
	if (this->detectAreaZoneId != 0xffffffff) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->detectAreaZoneId);
	}

	return edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, v0, 0);
}

int CActorCommander::CheckGuardArea(edF32VECTOR4* v0)
{
	CEventManager* pEventManager;
	ed_zone_3d* pZone;

	pEventManager = CScene::ptable.g_EventManager_006f5080;
	pZone = (ed_zone_3d*)0x0;
	if (this->guardAreaZoneId != 0xffffffff) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->guardAreaZoneId);
	}

	return edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, pZone, v0, 0);
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::CheckExorcism()
{
	CMagicInterface* pMagicInterface;
	float fVar2;
	float fVar3;
	InitPathfindingClientMsgParams local_20;
	CActorWolfen* pWolfen;

	if (0 < (this->exorcismActorsTable).nbEntries) {
		if (this->field_0x1e0 == 0) {
			this->field_0x1e0 = 1;
			pMagicInterface = &CActorHero::_gThis->magicInterface;
			fVar2 = pMagicInterface->GetValue();
			fVar3 = pMagicInterface->GetTransit();
			this->field_0x1dc = fVar2 + fVar3;
		}

		if (0.5f <= Timer::GetTimer()->scaledTotalTime - this->field_0x1e4) {
			pWolfen = static_cast<CActorWolfen*>((this->exorcismActorsTable).aEntries[0]);

			local_20.msgId = 0xe;
			local_20.time = this->field_0x1dc;
			fVar2 = pWolfen->nbRequiredMagicForExorcism - pWolfen->nbConsumedMagicForExorcism;
			if (local_20.time < fVar2) {
				this->field_0x1dc = 0.0f;
			}
			else {
				this->field_0x1dc = this->field_0x1dc - (pWolfen->nbRequiredMagicForExorcism - pWolfen->nbConsumedMagicForExorcism);
				local_20.time = fVar2;
			}
			DoMessage(pWolfen, (ACTOR_MESSAGE)0x1a, &local_20);

			this->exorcismActorsTable.Pop(0);
			this->field_0x1e4 = Timer::GetTimer()->scaledTotalTime;
		}

		if ((this->exorcismActorsTable).nbEntries == 0) {
			this->field_0x1e0 = 0;
		}
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::CheckArea_IntruderDetectArea()
{
	bool bDetected;
	ed_zone_3d* pZone;
	int zoneComputeResult;
	uint uVar4;
	InitPathfindingClientMsgParams initPathfindingClientMsgParams;

	zoneComputeResult = 2;
	bDetected = false;
	initPathfindingClientMsgParams.pActor = static_cast<CActorFighter*>(this->actorRef.Get());
	initPathfindingClientMsgParams.msgId = 0;

	if (initPathfindingClientMsgParams.pActor == (CActorFighter*)0x0) {
		initPathfindingClientMsgParams.pActor = CActorHero::_gThis;
	}

	if (initPathfindingClientMsgParams.pActor != (CActorFighter*)0x0) {
		pZone = (ed_zone_3d*)0x0;

		if (this->detectAreaZoneId != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->detectAreaZoneId);
		}

		zoneComputeResult = edEventComputeZoneAgainstVertex(CScene::ptable.g_EventManager_006f5080->activeChunkId, pZone, &initPathfindingClientMsgParams.pActor->currentLocation, 0);
	}

	uVar4 = this->flags_0x18c & 2;
	if ((uVar4 == 0) || (zoneComputeResult != 2)) {
		if ((uVar4 == 0) && (zoneComputeResult == 1)) {
			initPathfindingClientMsgParams.msgId = 1;
			bDetected = true;
			this->flags_0x18c = this->flags_0x18c | 2;
		}
	}
	else {
		bDetected = true;
		this->flags_0x18c = this->flags_0x18c & 0xfffffffd;
		initPathfindingClientMsgParams.msgId = 2;
	}

	if (bDetected) {
		zoneComputeResult = 0;

		if (0 < this->nbTeams) {
			do {
				DoMessage(this->aTeamElt[zoneComputeResult].pEnemyActor, (ACTOR_MESSAGE)0x1a, &initPathfindingClientMsgParams);
				zoneComputeResult = zoneComputeResult + 1;
			} while (zoneComputeResult < this->nbTeams);
		}
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::CheckArea_IntruderGuardArea()
{
	bool bDetected;
	ed_zone_3d* pZone;
	int zoneComputeResult;
	uint uVar4;
	InitPathfindingClientMsgParams initPathfindingClientMsgParams;

	zoneComputeResult = 2;
	bDetected = false;
	initPathfindingClientMsgParams.pActor = static_cast<CActorFighter*>(this->actorRef.Get());
	initPathfindingClientMsgParams.msgId = 0;

	if (initPathfindingClientMsgParams.pActor == (CActorFighter*)0x0) {
		initPathfindingClientMsgParams.pActor = CActorHero::_gThis;
	}

	if (initPathfindingClientMsgParams.pActor != (CActorFighter*)0x0) {
		pZone = (ed_zone_3d*)0x0;

		if (this->guardAreaZoneId != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->guardAreaZoneId);
		}

		zoneComputeResult = edEventComputeZoneAgainstVertex(CScene::ptable.g_EventManager_006f5080->activeChunkId, pZone, &initPathfindingClientMsgParams.pActor->currentLocation, 0);
	}

	uVar4 = this->flags_0x18c & 4;
	if ((uVar4 == 0) || (zoneComputeResult != 2)) {
		if ((uVar4 == 0) && (zoneComputeResult == 1)) {
			initPathfindingClientMsgParams.msgId = 3;
			bDetected = true;
			this->flags_0x18c = this->flags_0x18c | 4;
		}
	}
	else {
		bDetected = true;
		this->flags_0x18c = this->flags_0x18c & 0xfffffffb;
		initPathfindingClientMsgParams.msgId = 4;
	}

	if (bDetected) {
		zoneComputeResult = 0;

		if (0 < this->nbTeams) {
			do {
				DoMessage(this->aTeamElt[zoneComputeResult].pEnemyActor, (ACTOR_MESSAGE)0x1a, &initPathfindingClientMsgParams);
				zoneComputeResult = zoneComputeResult + 1;
			} while (zoneComputeResult < this->nbTeams);
		}
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::CheckArea_SoldiersDetectArea()
{
	CActorFighter* pReceiver;
	ed_zone_3d* pZone;
	int zoneComupteResult;
	CTeamElt* pTeamElt;
	int curTeamEltIndex;
	InitPathfindingClientMsgParams local_20;

	pZone = (ed_zone_3d*)0x0;
	if (this->detectAreaZoneId != 0xffffffff) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->detectAreaZoneId);
	}

	curTeamEltIndex = 0;
	if (0 < this->nbTeams) {
		do {
			pTeamElt = this->aTeamElt + curTeamEltIndex;

			pReceiver = pTeamElt->pEnemyActor;

			zoneComupteResult = edEventComputeZoneAgainstVertex(CScene::ptable.g_EventManager_006f5080->activeChunkId, pZone, &pReceiver->currentLocation, 0);
			if (zoneComupteResult != pTeamElt->field_0x4) {
				if ((zoneComupteResult == 1) && (pTeamElt->field_0x4 == 2)) {
					local_20.msgId = 8;
				}
				else {
					if ((zoneComupteResult == 2) && (pTeamElt->field_0x4 == 1)) {
						local_20.msgId = 9;
					}
				}

				local_20.pActor = static_cast<CActorFighter*>(pReceiver);
				DoMessage(pReceiver, (ACTOR_MESSAGE)0x1a, &local_20);
				pTeamElt->field_0x4 = zoneComupteResult;
			}

			curTeamEltIndex = curTeamEltIndex + 1;
		} while (curTeamEltIndex < this->nbTeams);
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::CheckArea_SoldiersGuardArea()
{
	CActorFighter* pReceiver;
	ed_zone_3d* pZone;
	int zoneComupteResult;
	CTeamElt* pTeamElt;
	int curTeamEltIndex;
	InitPathfindingClientMsgParams local_20;

	pZone = (ed_zone_3d*)0x0;
	if (this->guardAreaZoneId != 0xffffffff) {
		pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, this->guardAreaZoneId);
	}

	curTeamEltIndex = 0;
	if (0 < this->nbTeams) {
		do {
			pTeamElt = this->aTeamElt + curTeamEltIndex;

			pReceiver = pTeamElt->pEnemyActor;

			zoneComupteResult = edEventComputeZoneAgainstVertex(CScene::ptable.g_EventManager_006f5080->activeChunkId, pZone, &pReceiver->currentLocation, 0);
			if (zoneComupteResult != pTeamElt->field_0x0) {
				if ((zoneComupteResult == 1) && (pTeamElt->field_0x0 == 2)) {
					local_20.msgId = 6;
				}
				else {
					if ((zoneComupteResult == 2) && (pTeamElt->field_0x0 == 1)) {
						local_20.msgId = 7;
					}
				}

				local_20.pActor = static_cast<CActorFighter*>(pReceiver);
				DoMessage(pReceiver, (ACTOR_MESSAGE)0x1a, &local_20);
				pTeamElt->field_0x0 = zoneComupteResult;
			}

			curTeamEltIndex = curTeamEltIndex + 1;
		} while (curTeamEltIndex < this->nbTeams);
	}

	return;
}

int CActorCommander::CheckZone_00170f90(edF32VECTOR4* v0)
{
	uint zoneId;
	ed_zone_3d* pZone;
	int result;

	zoneId = this->field_0x180;
	result = 2;

	if (zoneId != 0xffffffff) {
		pZone = (ed_zone_3d*)0x0;
		if (zoneId != 0xffffffff) {
			pZone = edEventGetChunkZone((CScene::ptable.g_EventManager_006f5080)->activeChunkId, zoneId);
		}
		result = edEventComputeZoneAgainstVertex((CScene::ptable.g_EventManager_006f5080)->activeChunkId, pZone, v0, 0);
	}

	return result;
}

CTeamElt* CActorCommander::GetTeamElt(CActor* pActor)
{
	CTeamElt* pTeamElt;
	int iVar1;

	iVar1 = 0;
	if (0 < this->nbTeams) {
		pTeamElt = this->aTeamElt;
		do {
			if (pTeamElt->pEnemyActor == (CActorFighter*)pActor) {
				return pTeamElt;
			}

			iVar1 = iVar1 + 1;
			pTeamElt = pTeamElt + 1;
		} while (iVar1 < this->nbTeams);
	}

	return (CTeamElt*)0x0;
}

void CActorCommander::FUN_001717b0(CCameraGame* pCamera)
{
	CTeamElt* pCVar1;
	uint uVar2;
	bool bVar3;
	bool bVar4;
	int iVar5;
	ulong uVar6;
	ulong uVar7;
	CChessBoard* pChess;
	undefined4 uVar8;
	int iVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_10;

	iVar9 = 0;
	bVar3 = false;
	pChess = &(this->squad).chessboard;

	while ((iVar5 = this->squad.NbElt(), iVar9 < iVar5 && (!bVar3))) {
		pCVar1 = (this->squad).eltTable.aEntries[iVar9];
		iVar5 = pCVar1->pawnId;
		if ((iVar5 == -1) || ((bVar4 = pChess->PawnShallMove(iVar5), bVar4 == false || pCVar1->pEnemyActor->curBehaviourId != 3))) {
			iVar9 = iVar9 + 1;
		}
		else {
			bVar3 = true;
		}
	}

	uVar2 = (this->camFigData).field_0x260;
	if ((uVar2 & 3) == 0) {
		(this->camFigData).field_0x2b8 = 0;
	}
	else {
		if (((uVar2 & 1) != 0) == ((uVar2 & 2) != 0)) {
			fVar12 = (this->camFigData).field_0x26c;
			fVar10 = (this->camFigData).field_0x270;
			local_10 = pCamera->transformationMatrix.rowZ;
			local_10.y = 0.0f;

			edF32Vector4GetNegHard(&local_10, &local_10);
			edF32Vector4NormalizeHard(&local_10, &local_10);

			if (0.0f <= fVar12) {
				fVar11 = -3.141593f;
			}
			else {
				fVar11 = 3.141593f;
			}
			
			uVar6 = pChess->GetPawnsInSection(fVar11, &local_10);
			if (0.0f <= fVar10) {
				fVar11 = -3.141593f;
			}
			else {
				fVar11 = 3.141593f;
			}
			uVar7 = pChess->GetPawnsInSection(fVar11, &local_10);
			fVar11 = fVar10;
			if (((uVar6 <= uVar7) && (fVar11 = fVar12, uVar6 == uVar7)) && (fVar11 = fVar10, fabs(fVar12) <= fabs(fVar10))) {
				fVar11 = fVar12;
			}
			uVar8 = 2;
			if (0.0f <= fVar11) {
				uVar8 = 3;
			}
			fVar12 = pCamera->GetAngleBeta();
			this->camFigData.FUN_003c5ba0(fVar11 + fVar12, uVar8, 0);
		}
		else {
			if ((uVar2 & 1) == 0) {
				fVar12 = (this->camFigData).field_0x270;
			}
			else {
				fVar12 = (this->camFigData).field_0x26c;
			}

			uVar8 = 2;
			if (0.0f <= fVar12) {
				uVar8 = 3;
			}

			fVar10 = pCamera->GetAngleBeta();
			this->camFigData.FUN_003c5ba0(fVar12 + fVar10, uVar8, 0);
		}
	}

	return;
}

bool CActorCommander::FUN_001710c0()
{
	bool bVar1;
	int iVar3;
	bool bVar4;

	bVar4 = false;
	if (this->field_0x1e0 == 0) {
		iVar3 = 0;

		if (0 < this->nbTeams) {
			do {
				bVar1 = this->aTeamElt[iVar3].pEnemyActor->FUN_001740a0();
				if (bVar1 != false) {
					return true;
				}
				iVar3 = iVar3 + 1;
			} while (iVar3 < this->nbTeams);
		}
	}
	else {
		bVar4 = true;
	}

	return bVar4;
}

void CActorCommander::StateCommanderWaitReorganize(int param_2, int param_3)
{
	CActorFighter* pCVar1;
	CTeamElt* pCVar2;
	bool bVar3;
	int iVar4;
	int iVar5;
	long lVar6;
	CTeamElt** pCVar7;
	CTeamElt* pCVar8;
	CTeamElt* pCVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_10;

	StateCommanderBeatUp();

	iVar4 = this->squad.NbElt();
	if ((iVar4 != 0) && (lVar6 = this->squad.HasActiveSemaphore(0), lVar6 == 0)) {
		pCVar1 = this->field_0x9f4;
		fVar11 = 1.0f;
		iVar4 = 0;
		bVar3 = false;
		pCVar9 = (CTeamElt*)0x0;
		pCVar7 = (this->squad).eltTable.aEntries;
		if ((pCVar1->flags & 0x1000) == 0) {
			local_10 = pCVar1->rotationQuat;
		}
		else {
			SetVectorFromAngleY(pCVar1->rotationEuler.y, &local_10);
		}
		for (; iVar5 = this->squad.NbElt(), iVar4 < iVar5; iVar4 = iVar4 + 1) {
			pCVar2 = *pCVar7;
			fVar10 = fVar11;
			pCVar8 = pCVar9;
			if (pCVar2->pEnemyActor->curBehaviourId == 3) {
				fVar10 = edF32Vector4DotProductHard(&local_10, &pCVar2->pEnemyActor->rotationQuat);
				pCVar8 = pCVar2;
				if (fVar11 <= fVar10) {
					fVar10 = fVar11;
					pCVar8 = pCVar9;
				}

				if (!bVar3) {
					bVar3 = this->squad.chessboard.PawnShallMove(pCVar2->pawnId);
					bVar3 = bVar3 != false;
				}
			}

			fVar11 = fVar10;
			pCVar7 = pCVar7 + 1;
			pCVar9 = pCVar8;
		}

		if (((pCVar9 != (CTeamElt*)0x0) && (((fVar11 < -0.866f || (param_3 == 0)) || (!bVar3)))) && (static_cast<CActorWolfen*>(pCVar9->pEnemyActor)->ForceFightAction(0, false), param_2 != -1)) {
			SetState(param_2, -1);
		}
	}

	return;
}

void CActorCommander::StateCommanderBeatUp()
{
	int iVar1;
	CCameraManager* pCameraManager;
	CPathFinderClient* pPathFinderClient;
	CChessBoard* pChessboard;
	edF32VECTOR4 local_10;
	CActorFighter* pFighter;

	iVar1 = this->squad.NbElt();
	if (iVar1 != 0) {
		pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);
		local_10 = pCameraManager->transformationMatrix.rowZ;

		pChessboard = &this->squad.chessboard;
		pFighter = this->field_0x9f4->field_0x354;

		this->squad.chessboard.field_0x230 = pFighter->currentLocation;
		this->squad.chessboard.field_0x240 = gF32Vector4UnitZ;

		this->squad.chessboard.writeLayerMask = 3;
		this->squad.chessboard.readLayerMask = 3;

		pChessboard->ValidateAll();
		local_10.y = 0.0f;
		edF32Vector4GetNegHard(&local_10, &local_10);
		edF32Vector4NormalizeHard(&local_10, &local_10);

		this->squad.chessboard.writeLayerMask = 1;
		this->squad.chessboard.readLayerMask = 1;
		pChessboard->InvalidateSection(1.570796f, &local_10);
		pFighter = this->field_0x9f4;
		if ((pFighter->flags & 0x1000) == 0) {
			local_10 = pFighter->rotationQuat;
		}
		else {
			SetVectorFromAngleY(pFighter->rotationEuler.y, &local_10);
		}

		edF32Vector4GetNegHard(&local_10, &local_10);

		this->squad.chessboard.writeLayerMask = 1;
		this->squad.chessboard.readLayerMask = 1;
		pChessboard->InvalidateSection(3.141593f, &local_10);
		pPathFinderClient = this->field_0x9f4->GetPathfinderClient();

		if (pPathFinderClient->id == -1) {
			this->squad.chessboard.writeLayerMask = 2;
			this->squad.chessboard.readLayerMask = 2;
			pChessboard->InvalidateByZone(this->guardAreaZoneId);
		}
		else {
			this->squad.chessboard.writeLayerMask = 2;
			this->squad.chessboard.readLayerMask = 2;
			pChessboard->InvalidateByPath(pPathFinderClient);
		}

		_UpdatePattern();
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::_UpdateCamera()
{
	CCameraGame* pFightCamera;
	bool bVar2;
	bool bVar3;
	StateConfig* pSVar4;
	CActor* pCVar6;
	CBehaviour* pCVar7;
	CLifeInterface* pCVar8;
	int iVar9;
	s_fighter_blow* pBlow;
	s_fighter_grab* pGrab;
	CActorFighter* pFocus;
	CCamFigData* pFigData;
	uint uVar12;
	CTeamElt* pCVar13;
	CActorFighter* pCurAdversary;
	undefined4 uVar15;
	float fVar16;
	float fVar17;
	float t0;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	ed_Bound_Sphere local_10;

	bVar3 = false;
	pCurAdversary = (CActorFighter*)0x0;

	if (CActorHero::_gThis != (CActorHero*)0x0) {
		if (CActorHero::_gThis->IsFightRelated(CActorHero::_gThis->curBehaviourId) == false) {
			this->field_0x1d4 = this->field_0x1d4 + GetTimer()->cutsceneDeltaTime;
		}
		else {
			this->field_0x1d4 = 0.0f;
			pCurAdversary = CActorHero::_gThis->pAdversary;
			if (pCurAdversary == (CActorFighter*)0x0) {
				if ((CActorHero::_gThis->GetStateFlags(CActorHero::_gThis->actorState) & 0xff800) == 0x4000) {
					this->camFigData.IsValid(0);
				}
				else {
					pCurAdversary = CActorHero::_gThis->field_0x354;
				}
			}
		}
	}

	if (pCurAdversary != (CActorFighter*)0x0) {
		for (iVar9 = 0; (pCVar13 = (CTeamElt*)0x0, iVar9 < this->nbTeams && (pCVar13 = this->aTeamElt + iVar9, pCVar13->pEnemyActor != pCurAdversary)); iVar9 = iVar9 + 1) {
		}

		if (pCVar13 != (CTeamElt*)0x0) {
			bVar3 = true;
		}
	}

	if (bVar3) {
		pFightCamera = (CCameraGame*)CActorHero::_gThis->pFightCamera;
		pFigData = CCameraGame::_pfig_data;

		if (CCameraGame::_b_use_fig_data == 0) {
			pFigData = (CCamFigData*)0x0;
		}

		if ((pFigData != &this->camFigData) || ((this->camFigData).field_0x2a0 == 0)) {
			pCVar6 = pFightCamera->GetTarget();
			this->camFigData.SetPosParam(&pCVar6->currentLocation);
			pFightCamera->Fight_SetConfig(&this->camFigData);

			this->field_0x9f0 = 0;
			this->camFigData.IsValid(1);
		}

		this->squad.ComputeBoundingSphere(&local_10);
		this->camFigData.SetBoundingSphere(&local_10);

		pCVar7 = GetBehaviour(this->curBehaviourId);
		CBehaviourCommander* pBehaviourCommander = static_cast<CBehaviourCommander*>(pCVar7);
		pFocus = pBehaviourCommander->GetFocus();
		if (pFocus == (CActorFighter*)0x0) {
			local_30.xyz = local_10.xyz;
			local_30.w = 1.0f;
			this->camFigData.SetAdversaryPos(&local_30);
		}
		else {
			local_20.x = pFocus->currentLocation.x;
			local_20.y = pFocus->currentLocation.y - pFocus->distanceToGround;
			local_20.z = pFocus->currentLocation.z;
			local_20.w = pFocus->currentLocation.w;
			this->camFigData.SetAdversaryPos(&local_20);
		}

	
		CActorHero* pCVar1 = CActorHero::_gThis;
		if ((((CActorHero::_gThis != (CActorHero*)0x0) && (pCurAdversary = CActorHero::_gThis->pAdversary, pCurAdversary != (CActorFighter*)0x0)) &&
			(pCurAdversary != this->field_0x9f0)) && ((bVar3 = CActorHero::_gThis->FUN_0031b790(CActorHero::_gThis->actorState), bVar3 != false &&
				((pCVar1->fightFlags & FIGHT_FLAG_ATTACK_CONNECTED) != 0)))) {
			pCurAdversary = pCVar1->pAdversary;
			fVar16 = pCurAdversary->GetLifeInterface()->GetValue();
			if ((fVar16 <= 0.0f) && (((pBlow = pCVar1->pBlow, pBlow != (s_fighter_blow*)0x0 && ((pBlow->field_0x4.field_0x2byte & 1) != 0)) ||
					((pGrab = pCVar1->pExecutingGrab, pGrab != (s_fighter_grab*)0x0 && (pGrab->field_0x38 == 2)))))) {
				fVar16 = GetAngleYFromVector(&CActorHero::_gThis->rotationQuat);
				fVar17 = edF32GetAnglesDelta(pFightCamera->GetAngleBeta(), fVar16 - 0.7853982f);
				fVar16 = edF32GetAnglesDelta(pFightCamera->GetAngleBeta(), fVar16 + 0.7853982f);

				if (fabs(fVar17) <= fabs(fVar16)) {
					fVar16 = fVar17;
				}

				fVar17 = pFightCamera->GetAngleBeta();
				iVar9 = this->camFigData.FUN_003c79f0(fVar16 + fVar17, 0.1396263f);
				if (iVar9 != 0) {
					uVar15 = 2;
					if (0.0f <= fVar16) {
						uVar15 = 3;
					}

					fVar17 = pFightCamera->GetAngleBeta();
					this->camFigData.FUN_003c5ba0(fVar16 + fVar17, uVar15, 1);
				}

				this->field_0x9f0 = pCVar1->pAdversary;
			}
		}

		if ((this->camFigData).field_0x2b4 == 1) {
			FUN_001717b0(pFightCamera);
		}
	}
	else {
		if (((this->flags_0x18c & 7) == 0) || (1.0f <= this->field_0x1d4)) {
			this->camFigData.IsValid(0);
			this->field_0x9f0 = (CActorFighter*)0x0;
		}
	}
		
	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::StateCommanderDefault()
{
	CActorFighter* pFirstEntry;
	bool bVar2;
	CCameraManager* pCameraManager;
	int iVar3;
	int iVar4;
	int* piVar5;
	long lVar6;
	CChessBoard* pChessBoard;
	CActorFighter* pAdversary;
	float fVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_10;

	pCameraManager = (CCameraManager*)CScene::GetManager(MO_Camera);

	local_10 = (pCameraManager->transformationMatrix).rowZ;

	pChessBoard = &(this->squad).chessboard;

	local_10.y = 0.0f;

	edF32Vector4GetNegHard(&local_10, &local_10);
	edF32Vector4NormalizeHard(&local_10, &local_10);

	iVar3 = this->squad.NbElt();
	if (iVar3 == 0) {
		(this->squad).chessboard.field_0x240 = gF32Vector4UnitZ;
		(this->squad).chessboard.field_0x230 = pCameraManager->activeCameraLookAt;
		(this->squad).chessboard.writeLayerMask = 3;
		(this->squad).chessboard.readLayerMask = 3;

		pChessBoard->ValidateAll();

		(this->squad).chessboard.writeLayerMask = 1;
		(this->squad).chessboard.readLayerMask = 1;
		pChessBoard->InvalidateSection(1.570796f, &local_10);
	}
	else {
		pFirstEntry = (this->squad).eltTable.aEntries[0]->pEnemyActor;

		pAdversary = pFirstEntry->pAdversary;
		if (pAdversary == (CActorFighter*)0x0) {
			pAdversary = pFirstEntry->field_0x354;
		}

		if (pAdversary != (CActorFighter*)0x0) {
			fVar10 = (this->squad).chessboard.field_0x230.x - pAdversary->currentLocation.x;
			fVar12 = (this->squad).chessboard.field_0x230.z - pAdversary->currentLocation.z;
			fVar11 = (this->squad).chessboard.field_0x210 + (this->squad).chessboard.field_0x21c / 2.0f;
			if (fVar11 * fVar11 < fVar10 * fVar10 + fVar12 * fVar12) {
				(this->squad).chessboard.field_0x220 = 1;
				(this->squad).chessboard.field_0x230 = pAdversary->currentLocation;
			}
			else {
				iVar3 = 0;
				bVar2 = false;
				if (pAdversary->IsInHitState() == 0) {
					while ((iVar4 = this->squad.NbElt(), iVar3 < iVar4 && (!bVar2))) {
						if ((this->squad).eltTable.aEntries[iVar3]->pEnemyActor->IsInHitState() != 0) {
							bVar2 = true;
							(this->squad).chessboard.field_0x230 = pAdversary->currentLocation;
						}

						iVar3 = iVar3 + 1;
					}
				}
				else {
					pAdversary = (CActorFighter*)pAdversary->field_0x634;
					while ((iVar4 = this->squad.NbElt(), iVar3 < iVar4 && (!bVar2))) {
						if ((this->squad).eltTable.aEntries[iVar3]->pEnemyActor == pAdversary) {
							bVar2 = true;
							(this->squad).chessboard.field_0x230 = pAdversary->currentLocation;
						}

						iVar3 = iVar3 + 1;
					}
				}
			}
		}

		(this->squad).chessboard.writeLayerMask = 3;
		(this->squad).chessboard.readLayerMask = 3;

		pChessBoard->ValidateAll();

		(this->squad).chessboard.writeLayerMask = 2;
		(this->squad).chessboard.readLayerMask = 2;

		CPathFinderClient* pPathFinderClient = pFirstEntry->GetPathfinderClient();

		if (pPathFinderClient->id == -1) {
			if (pFirstEntry->IsKindOfObject(OBJ_TYPE_WOLFEN) == 0) {
				pChessBoard->InvalidateByRayTrace();
			}
			else {
				CActorWolfen* pActorWolfen = static_cast<CActorWolfen*>(pFirstEntry);
				pChessBoard->InvalidateByZone(pActorWolfen->pCommander->guardAreaZoneId);
			}
		}
		else {
			pChessBoard->InvalidateByPath(pPathFinderClient);
		}

		(this->squad).chessboard.writeLayerMask = 1;
		(this->squad).chessboard.readLayerMask = 1;

		pChessBoard->InvalidateSection(1.570796f, &local_10);
	}

	_UpdatePattern();

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
bool CActorCommander::CanContinueToFight(CActorFighter* pFighter)
{
	return (this->squad).eltTable.IsInList(GetTeamElt(pFighter));
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
bool CActorCommander::CanReleaseSemaphore(CActorWolfen* pWolfen)
{
	CBehaviourCommander* pCVar1;
	ulong uVar2;
	CTeamElt* pCVar3;
	int iVar4;
	CTeamElt* pCVar5;

	pCVar1 = static_cast<CBehaviourCommander*>(GetBehaviour(this->curBehaviourId));
	iVar4 = 0;
	pCVar5 = (CTeamElt*)0x0;

	if (0 < this->nbTeams) {
		pCVar3 = this->aTeamElt;
		do {
			pCVar5 = pCVar3;

			if ((CActorWolfen*)pCVar3->pEnemyActor == pWolfen) break;

			iVar4 = iVar4 + 1;
			pCVar3 = pCVar3 + 1;
			pCVar5 = (CTeamElt*)0x0;
		} while (iVar4 < this->nbTeams);
	}

	uVar2 = pCVar1->CanReleaseSemaphore(pCVar5->field_0x14, pWolfen);
	return uVar2 & 0xff;
}

void CActorCommander::ReleaseSemaphore(int index, CActorWolfen* pWolfen)
{
	this->squad.ForceReleaseSemaphore(index, GetTeamElt(pWolfen), (this->flags_0x18c & 6) != 0);
}

bool CActorCommander::QuerySemaphoreCold(int index, CActorWolfen* pWolfen)
{
	return this->squad.QuerySemaphoreCold(index, GetTeamElt(pWolfen));
}

bool CActorCommander::QuerySemaphoreWarm(int index, CActorWolfen* pWolfen)
{
	return this->squad.QuerySemaphoreWarm(index, GetTeamElt(pWolfen));
}

bool CActorCommander::IsValidEnemy(CActorWolfen* pWolfen)
{
	return this->squad.eltTable.IsInList(GetTeamElt(pWolfen));
}

bool CActorCommander::CanFightIntruder(CActorFighter* pIntruder)
{
	int iVar2;
	CTeamElt* pCVar3;
	CTeamElt* pCVar4;
	bool bCanFight;

	iVar2 = 0;
	bCanFight = false;
	pCVar4 = (CTeamElt*)0x0;
	if (0 < this->nbTeams) {
		pCVar3 = this->aTeamElt;
		do {
			pCVar4 = pCVar3;

			if (pCVar4->pEnemyActor == pIntruder) break;

			iVar2 = iVar2 + 1;
			pCVar3 = pCVar4 + 1;
			pCVar4 = (CTeamElt*)0x0;
		} while (iVar2 < this->nbTeams);
	}

	if (this->squad.eltTable.IsInList(pCVar4) == false) {
		iVar2 = this->squad.NbElt();
		if (iVar2 < 5) {
			bCanFight = true;
		}
	}
	else {
		bCanFight = true;
	}

	return bCanFight;
}

void CActorCommander::KickIntruderFighter()
{
	CTeamElt* pTeamElt;
	CTeamElt** pIt;
	int i;

	i = 0;
	pIt = this->squad.eltTable.aEntries;
	do {
		pTeamElt = *pIt;
		if (pTeamElt->field_0x14 == -1) {
			this->squad.RemoveElt(pTeamElt);
			return;
		}

		i = i + 1;
		pIt = pIt + 1;
	} while (i < 5);

	return;
}



// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::_UpdateSquad()
{
	CActorFighter* this_00;
	bool bVar1;
	int iVar2;
	uint uVar3;
	StateConfig* pSVar4;

	this->squad.ClearSquadDeadAndNonFightingElt();
	iVar2 = this->squad.NbElt();
	if (iVar2 == 0) {
		return;
	}

	if ((GetStateFlags(this->actorState) & 0x100) == 0) {
		this_00 = (this->squad).eltTable.aEntries[0]->pEnemyActor->pAdversary;
		if ((this_00 != (CActorFighter*)0x0) &&
			(bVar1 = this_00->FUN_0031b790(this_00->actorState), bVar1 != false)) {
			bVar1 = false;
			if ((this_00->pFighterCombo != (s_fighter_combo*)0x0) && ((this_00->pFighterCombo->field_0x4.field_0x0ushort & 0x100U) != 0)) {
				bVar1 = true;
			}

			if (!bVar1) goto LAB_00172118;
		}

		this->squad.ManageSemaphore(0);
	}
LAB_00172118:
	if ((GetStateFlags(this->actorState) & 0x200) == 0) {
		this->squad.ManageSemaphore(1);
	}

	this->squad.SynchronizePawns();

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::_UpdateSequence()
{
	int iVar1;
	uint uVar2;
	bool bVar3;

	iVar1 = this->count_0x16c;
	if (iVar1 != 0) {
		bVar3 = iVar1 == 0;
		if (!bVar3) {
			bVar3 = this->field_0x168 == iVar1 + -1;
		}
		if (!bVar3) {
			uVar2 = this->flags_0x18c;
			if ((uVar2 & 0x10) == 0) {
				if ((uVar2 & 10) == 10) {
					this->flags_0x18c = this->flags_0x18c | 0x10;
				}
			}
			else {
				if ((uVar2 & 6) != 0) {
					UpdateKilled();
					UpdateAttack();
				}

				if (((this->flags_0x18c & 8) == 0) || ((~this->flags_0x18c & 6) == 6)) {
					this->flags_0x18c = this->flags_0x18c & 0xffffffef;
				}
			}
		}

		if ((((this->flags_0x18c & 8) != 0) && ((~this->flags_0x18c & 6) == 6)) && (this->field_0x174 != (CActor*)0x0)) {
			DoMessage(this->field_0x174, (ACTOR_MESSAGE)0x10, 0);
		}
	}
	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CActorCommander::_UpdatePattern()
{
	if (this->field_0x6d0 != 0) {
		this->squad.SynchronizePattern((CChessBoardPawnsRefTable*)0x0);
	}
	return;
}

bool CActorCommander::UpdateKilled()
{
	bool bVar1;
	CTeamElt* iVar3;
	int iVar5;
	float fVar6;

	this->field_0x160 = 0;
	this->field_0x164 = 0;
	iVar5 = 0;
	if (0 < this->aComponent_0x170[this->count_0x16c + -1].field_0xc) {
		do {
			iVar3 = this->aTeamElt + iVar5;
			fVar6 = iVar3->pEnemyActor->GetLifeInterface()->GetValue();
			if (fVar6 <= 0.0f) {
				this->field_0x164 = this->field_0x164 + 1;
				this->field_0x160 = this->field_0x160 + 1;
			}
			else {
				bVar1 = (this->squad).eltTable.IsInList(iVar3);
				CActorWolfen* pWolfen = static_cast<CActorWolfen*>(iVar3->pEnemyActor);
				if ((bVar1 != false) || (pWolfen->IsFreeToFight() == false)) {
					this->field_0x160 = this->field_0x160 + 1;
				}
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < this->aComponent_0x170[this->count_0x16c + -1].field_0xc);
	}

	this->field_0x168 = 0;

	while ((this->aComponent_0x170[this->field_0x168].field_0x4 <= this->field_0x164 && (this->field_0x168 < this->count_0x16c))) {
		this->field_0x168 = this->field_0x168 + 1;
	}

	this->field_0x168 = this->field_0x168 + -1;

	return true;
}

void CActorCommander::UpdateAttack()
{
	int iVar1;
	int iVar2;
	bool bVar3;
	CTeamElt* pCVar4;
	int iVar6;
	int iVar7;
	undefined4 local_60[3];
	CActorHero* local_54;
	CFixedTable<CTeamElt*, 16> local_50;
	undefined4* local_4;

	if (this->field_0x160 < this->aComponent_0x170[this->field_0x168].field_0xc) {
		local_50.nbEntries = 0;
		iVar6 = 0;
		iVar7 = this->field_0x160;
		iVar1 = this->aComponent_0x170[this->field_0x168].field_0xc;

		while ((iVar2 = local_50.nbEntries, iVar6 < this->nbTeams && (local_50.nbEntries < iVar1 - iVar7))) {
			pCVar4 = this->aTeamElt + iVar6;
			bVar3 = (this->squad).eltTable.IsInList(pCVar4);
			CActorWolfen* pWolfen = static_cast<CActorWolfen*>(pCVar4->pEnemyActor);
			if ((bVar3 == false) && (bVar3 = pWolfen->IsFreeToFight(), bVar3 != false)) {
				local_50.Add(pCVar4);
			}

			iVar6 = iVar6 + 1;
		}
		iVar7 = 0;
		if (0 < local_50.nbEntries) {
			do {
				pCVar4 = local_50.PopCurrent();
				if (pCVar4 != (CTeamElt*)0x0) {
					local_60[0] = 5;

					local_54 = static_cast<CActorHero*>(this->actorRef.Get());
					if (local_54 == (CActorHero*)0x0) {
						local_54 = CActorHero::_gThis;
					}

					local_4 = local_60;
					DoMessage(pCVar4->pEnemyActor, (ACTOR_MESSAGE)0x1a, local_4);
					this->field_0x160 = this->field_0x160 + 1;
				}

				iVar7 = iVar7 + 1;
			} while (iVar7 < iVar2);
		}
	}

	return;
}

void CBehaviourCommander::Create(ByteCode* pByteCode)
{
	S_TARGET_ON_OFF_STREAM_REF* pSVar1;
	int pCVar2;
	S_STREAM_EVENT_CAMERA* pSVar3;

	this->squadConfig.Create(pByteCode);

	this->onOffSwitch.Create(pByteCode);

	return;
}

void CBehaviourCommander::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorCommander*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->squad.InitSemaphores(&this->squadConfig);

	return;
}

bool CBehaviourCommander::CanReleaseSemaphore(int, CActorWolfen*)
{
	return 1 < this->pOwner->squad.NbElt();
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
CActorFighter* CBehaviourCommander::GetFocus()
{
	return CActorHero::_gThis->pAdversary;
}

// Should be in: D:/Projects/b-witch/ActorCmd.h
void CBehaviourCommander::TestSwitch()
{
	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
bool CBehaviourCommander::UpdateTeamAnim()
{
	CTeamElt* pCurEntry;
	CScene* pCVar2;
	bool bVar3;
	int iVar4;
	CTeamElt** pEntryIt;
	int iVar7;
	CSquad* pSquad;
	float fVar8;
	uint uVar9;
	undefined4 local_20[7];
	undefined4* local_4;

	pSquad = &this->pOwner->squad;
	if (this->field_0x18 == 0) {
		bVar3 = false;
	}
	else {
		local_20[0] = 0xf;
		iVar7 = 0;
		iVar4 = pSquad->NbElt();
		if (0 < iVar4) {
			pEntryIt = pSquad->eltTable.aEntries;
			do {
				pCVar2 = CScene::_pinstance;
				pCurEntry = *pEntryIt;
				if (pCurEntry->field_0x14 == -1) {
					if ((int)CScene::Rand() < 0) {
						bVar3 = false;
					}
					else {
						fVar8 = this->healthRatio * 32767.0f;
						if (fVar8 < 2.147484e+09f) {
							uVar9 = (uint)fVar8;
						}
						else {
							uVar9 = (int)(fVar8 - 2.147484e+09f) | 0x80000000;
						}

						bVar3 = CScene::Rand() <= uVar9;
					}
					if (bVar3) {
						local_4 = local_20;
						this->pOwner->DoMessage(pCurEntry->pEnemyActor, (ACTOR_MESSAGE)0x1a, local_20);
					}
				}

				pEntryIt = pEntryIt + 1;
				iVar7 = iVar7 + 1;
				iVar4 = pSquad->NbElt();
			} while (iVar7 < iVar4);
		}

		this->field_0x18 = 0;
		bVar3 = true;
	}

	return bVar3;
}

void CBehaviourCommanderDefault::Init(CActor* pOwner)
{
	this->onOffSwitch.Init();

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CBehaviourCommanderDefault::Manage()
{
	CActorCommander* pCommander;

	this->pOwner->_UpdateSequence();

	pCommander = this->pOwner;
	if (pCommander->actorState == 5) {
		pCommander->StateCommanderDefault();
	}

	TestSwitch();

	this->onOffSwitch.pStreamEventCamera->Manage(this->pOwner);

	UpdateTeamAnim();

	return;
}

void CBehaviourCommanderDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourCommander::Begin(pOwner, newState, newAnimationType);

	this->field_0x20 = 0;

	this->onOffSwitch.Reset(this->pOwner);

	return;
}

int CBehaviourCommanderDefault::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CLifeInterface* pLife;
	int iVar2;
	undefined8 uVar3;
	float fVar4;
	float fVar5;

	if (msg == MESSAGE_FIGHT_ACTION_SUCCESS) {
		_msg_fight_action_success_params* pParams = reinterpret_cast<_msg_fight_action_success_params*>(pMsgParam);
		if (pParams->field_0x0 == 1) {
			CActorFighter* pAdversary = pParams->pAdversary;
			this->field_0x18 = 1;
			iVar2 = 1;
			this->healthRatio = 1.0f - pAdversary->GetLifeInterface()->GetValue() / pAdversary->GetLifeInterface()->GetValueMax();
		}
		else {
			iVar2 = 0;
		}
	}
	else {
		iVar2 = 0;
	}

	return iVar2;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
void CBehaviourCommanderDefault::TestSwitch()
{
	CActorCommander* pCommander;
	int* piVar2;
	bool bVar3;
	undefined8 uVar4;
	long lVar5;
	S_TARGET_ON_OFF_STREAM_REF* pSVar6;
	byte bVar7;
	int iVar8;
	int iVar9;

	pCommander = this->pOwner;
	iVar9 = pCommander->count_0x16c;
	bVar7 = 1;
	if (iVar9 == 0) {
		for (iVar8 = 0; (bVar7 != 0 && (pCommander = this->pOwner, iVar8 < pCommander->nbTeams)); iVar8 = iVar8 + 1) {
			CActorWolfen* pWolfen = static_cast<CActorWolfen*>(pCommander->aTeamElt[iVar8].pEnemyActor);
			if ((0.0f < pWolfen->GetLifeInterface()->GetValue()) || ((pWolfen->IsKindOfObject(OBJ_TYPE_WOLFEN) != 0 && (pWolfen->exorcisedState != 2)))) {
				bVar7 = 0;
			}
		}
	}
	else {
		bVar3 = iVar9 == 0;
		if (!bVar3) {
			bVar3 = pCommander->field_0x168 == iVar9 + -1;
		}

		if (!bVar3) {
			bVar7 = 0;
		}
	}

	if (this->field_0x20 != bVar7) {
		if ((this->field_0x20 == 1) && (bVar7 == 0)) {
			this->onOffSwitch.SwitchOff(pCommander);
		}

		if ((this->field_0x20 == 0) && (bVar7 == 1)) {
			this->onOffSwitch.SwitchOn(pCommander);
		}

		this->field_0x20 = bVar7;
	}

	return;
}

// Should be in: D:/Projects/b-witch/ActorCmd.cpp
bool CBehaviourCommanderDefault::UpdateTeamAnim()
{
	if (!CBehaviourCommander::UpdateTeamAnim()) {
		int iVar8 = 0;
		CSquad* pSquad = &this->pOwner->squad;
		int nbElt = pSquad->NbElt();
		CTeamElt** pEntryIt = pSquad->eltTable.aEntries;
		if (0 < nbElt) {
			do {
				CTeamElt* pCurEntry = *pEntryIt;
				if ((pCurEntry->field_0x14 == -1) && (pCurEntry->pEnemyActor->actorState == 6)) {
					float fVar10 = ((float)rand() / 2.147484e+09f) * 2.0f + 2.0f;
					nbElt = rand();
					int iVar5 = rand();
					pCurEntry->pEnemyActor->PlayWaitingAnimation(((float)iVar5 / 2.147484e+09f) * 2.5f + 0.5f, ((float)nbElt / 2.147484e+09f) * 0.6f + 0.8f,
						0xe3, -1, (char)(int)fVar10);
				}

				pEntryIt = pEntryIt + 1;
				iVar8 = iVar8 + 1;
				nbElt = pSquad->NbElt();
			} while (iVar8 < nbElt);
		}
	}

	return true;
}

void CBehaviourCommanderBeatUp::Init(CActor* pOwner)
{
	this->onOffSwitch.Init();

	return;
}

void CBehaviourCommanderBeatUp::Term()
{
	return;
}

void CBehaviourCommanderBeatUp::Manage()
{
	CActorCommander* pCommander;

	pCommander = this->pOwner;

	if (pCommander->actorState == 7) {
		pCommander->StateCommanderWaitReorganize(-1, 0);
	}
	else {
		if (pCommander->actorState == 6) {
			pCommander->StateCommanderWaitReorganize(7, 1);
		}
	}

	UpdateTeamAnim();

	return;
}

void CBehaviourCommanderBeatUp::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorCommander* pCommander;

	this->pOwner = static_cast<CActorCommander*>(pOwner);

	if (newState == -1) {
		pCommander = this->pOwner;
		pCommander->SetState(5, -1);
	}
	else {
		pCommander = this->pOwner;
		pCommander->SetState(newState, newAnimationType);
	}

	this->pOwner->squad.InitSemaphores(&this->squadConfig);

	if (newState == -1) {
		pCommander = this->pOwner;
		pCommander->SetState(6, -1);
	}
	else {
		pCommander = this->pOwner;
		pCommander->SetState(newState, newAnimationType);
	}

	this->pOwner->squad.ForceReleaseSemaphore(0, (CTeamElt*)0x0, true);

	return;
}

void CBehaviourCommanderBeatUp::End(int newBehaviourId)
{
	return;
}

int CBehaviourCommanderBeatUp::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	undefined uVar1;
	CTeamElt* pCVar2;
	CTeamElt* pTeamElt;
	int iVar3;
	CActorHero* pCVar4;
	CActorCommander* pCommander;
	CActorFighter* pFighter;

	if (msg == 0x1b) {
		if (*reinterpret_cast<int*>(pMsgParam) == 2) {
			iVar3 = 0;
		}
		else {
			if (*reinterpret_cast<int*>(pMsgParam) == 3) {
				pCommander = this->pOwner;
				pCVar4 = (CActorHero*)(pCommander->actorRef).Get();
				if (pCVar4 == (CActorHero*)0x0) {
					pCVar4 = CActorHero::_gThis;
				}
				pFighter = pCommander->field_0x9f4;
				uVar1 = pCVar4->IsKindOfObject(8);
				if (uVar1 != 0) {
					iVar3 = 0;
					pTeamElt = (CTeamElt*)0x0;
					if (0 < pCommander->nbTeams) {
						pCVar2 = pCommander->aTeamElt;
						do {
							pTeamElt = pCVar2;
							if (pTeamElt->pEnemyActor == pFighter) break;
							iVar3 = iVar3 + 1;
							pCVar2 = pTeamElt + 1;
							pTeamElt = (CTeamElt*)0x0;
						} while (iVar3 < pCommander->nbTeams);
					}

					pCommander->squad.AddFighter(pTeamElt);
					pFighter->SetStandAnim(0xe2);
				}

				(this->pOwner)->field_0x9f4 = (CActorFighter*)0x0;
				pCommander = this->pOwner;
				pCommander->SetBehaviour(2, -1, -1);
				iVar3 = 1;
			}
			else {
				iVar3 = 0;
			}
		}
	}
	else {
		iVar3 = 0;
	}

	return iVar3;
}

bool CBehaviourCommanderBeatUp::CanReleaseSemaphore(int param_2, CActorWolfen*)
{
	return param_2 != 0;
}

CActorFighter* CBehaviourCommanderBeatUp::GetFocus()
{
	int iVar2;
	CActorFighter* pFocus;
	int iVar5;
	CTeamElt* iVar1;

	iVar5 = 0;
	pFocus = (CActorFighter*)0x0;
	while ((iVar2 = this->pOwner->squad.NbElt(), iVar5 < iVar2 && (pFocus == (CActorFighter*)0x0))) {
		iVar1 = (this->pOwner->squad).eltTable.aEntries[iVar5];
		if (iVar1->field_0x14 == 0) {
			pFocus = iVar1->pEnemyActor;
		}

		iVar5 = iVar5 + 1;
	}

	return pFocus;
}

bool CBehaviourCommanderBeatUp::UpdateTeamAnim()
{
	CTeamElt* pCVar1;
	bool bVar2;
	CScene* pCVar3;
	int iVar4;
	int iVar5;
	ulong uVar6;
	CSquad* pCVar7;
	CSquad* pCVar8;
	int iVar9;
	uint uVar10;
	float fVar11;
	InitPathfindingClientMsgParams local_20;

	pCVar7 = &this->pOwner->squad;
	if (this->field_0x18 == 0) {
		bVar2 = false;
	}
	else {
		local_20.msgId = 0xf;
		iVar9 = 0;
		iVar4 = pCVar7->NbElt();
		if (0 < iVar4) {
			CTeamElt** ppTeam = pCVar7->eltTable.aEntries;
			do {
				pCVar3 = CScene::_pinstance;
				pCVar1 = *ppTeam;

				if (pCVar1->field_0x14 == -1) {
					uVar6 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
					CScene::_pinstance->field_0x38 = uVar6;

					if ((int)((uint)(uVar6 >> 0x10) & 0x7fff) < 0) {
						bVar2 = false;
					}
					else {
						uVar6 = pCVar3->field_0x38 * 0x343fd + 0x269ec3;
						pCVar3->field_0x38 = uVar6;
						fVar11 = this->healthRatio * 32767.0f;
						if (fVar11 < 2.147484e+09f) {
							uVar10 = (uint)fVar11;
						}
						else {
							uVar10 = (int)(fVar11 - 2.147484e+09f) | 0x80000000;
						}

						bVar2 = ((uint)(uVar6 >> 0x10) & 0x7fff) <= uVar10;
					}

					if (bVar2) {
						this->pOwner->DoMessage(pCVar1->pEnemyActor, (ACTOR_MESSAGE)0x1a, &local_20);
					}
				}

				ppTeam = ppTeam + 1;
				iVar9 = iVar9 + 1;
				iVar4 = pCVar7->NbElt();
			} while (iVar9 < iVar4);
		}

		this->field_0x18 = 0;
		bVar2 = true;
	}

	if (!bVar2) {
		iVar9 = 0;
		pCVar8 = &this->pOwner->squad;
		iVar4 = pCVar8->NbElt();
		CTeamElt** ppTeam = pCVar8->eltTable.aEntries;
		if (0 < iVar4) {
			do {
				pCVar1 = *ppTeam;

				if ((pCVar1->field_0x14 == -1) && (pCVar1->pEnemyActor->actorState == 6)) {
					iVar4 = rand();
					fVar11 = ((float)iVar4 / 2.147484e+09f) * 2.0f + 3.0f;
					iVar4 = rand();
					iVar5 = rand();
					pCVar1->pEnemyActor->PlayWaitingAnimation(((float)iVar5 / 2.147484e+09f) * 1.0f + 1.0f, ((float)iVar4 / 2.147484e+09f) * 0.8f + 0.6f, 0xe4, -1, (char)(int)fVar11);
				}

				ppTeam = ppTeam + 1;
				iVar9 = iVar9 + 1;
				iVar4 = pCVar8->NbElt();
			} while (iVar9 < iVar4);
		}
	}

	return true;
}
