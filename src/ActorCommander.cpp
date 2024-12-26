#include "ActorCommander.h"
#include "MemoryStream.h"
#include "ActorManager.h"
#include "ActorWolfen.h"
#include "ActorHero.h"
#include "EventManager.h"
#include "MathOps.h"
#include "TimeController.h"



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
			if (pWolfen->IsKindOfObject(0x10) != false) {
				pWolfen->pCommander = this;
			}

			iVar5 = iVar5 + 1;
		} while (iVar5 < this->nbTeams);
	}

	this->actorRef.Init();

	ClearLocalData();

	return;
}

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

		IMPLEMENTATION_GUARD_LOG(
		iVar1 = CSquad::NbElt(&this->squad);
		if (iVar1 != 0) {
			edF32Vector4SubHard(&eStack112, &this->currentLocation, &local_20);
			edF32Matrix4TranslateHard(&eStack96, &gF32Matrix4Unit, &eStack112);
			FUN_00352d90((int)&(this->squad).chessboard, &eStack96);
		})
	}

	IMPLEMENTATION_GUARD_LOG(
	(*(code*)(this->pVTable)->field_0xfc)(this);)
	CheckExorcism();
	CheckArea_IntruderDetectArea();
	CheckArea_IntruderGuardArea();
	CheckArea_SoldiersDetectArea();
	CheckArea_SoldiersGuardArea();

	IMPLEMENTATION_GUARD_LOG(
	(this->checkpointData_0x190).pWaypoint = (CWayPoint*)0x0;
	iVar1 = 0;
	if (0 < this->nbTeams) {
		iVar4 = 0;
		do {
			piVar1 = *(CTeamElt**)((int)&this->aTeamElt->pEnemyActor + iVar4);
			uVar2 = (**(code**)(piVar1->field_0x0 + 0x138))(piVar1);
			fVar5 = (float)(**(code**)(*(int*)uVar2 + 0x24))(uVar2);
			if (((0.0 < fVar5) && (lVar3 = (**(code**)(piVar1->field_0x0 + 0xc))(piVar1, 0x10), lVar3 != 0)) &&
				((*(uint*)&piVar1[0x7a].field_0x8 & 7) != 0)) {
				(this->checkpointData_0x190).pWaypoint =
					(CWayPoint*)((int)&(((this->checkpointData_0x190).pWaypoint)->field_0x0).x + 1);
			}
			iVar1 = iVar1 + 1;
			iVar4 = iVar4 + 0x18;
		} while (iVar1 < this->nbTeams);
	})

	this->bInCombat_0x1b0 = 0;
	CActor::Manage();
	IMPLEMENTATION_GUARD_LOG(
	_UpdateCamera();
	this->field_0x2f4 = 0;)
	return;
}


void CActorCommander::Reset()
{
	CActor::Reset();
	ClearLocalData();

	return;
}

CBehaviour* CActorCommander::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 3) {
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

void CActorCommander::ChangeManageState(int state)
{
	if ((state == 0) && ((this->camFigData).field_0x2a0 != 0)) {
		this->camFigData.IsValid(0);
	}

	CActor::ChangeManageState(state);

	return;
}

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

	//this->squad.Clear();

	this->field_0x1e0 = 0;
	//this->field_0x1e4 = 0;
	//this->field_0x1e8 = 0;
	this->field_0x194 = 0;
	this->targetPosition = gF32Vertex4Zero;
	this->targetGroundPosition = gF32Vertex4Zero;

	this->bInCombat_0x1b0 = 1;
	this->count_0x1d0 = 0;
	//iVar5 = 0;
	//if (0 < this->nbTeams) {
	//	iVar4 = 0;
	//	do {
	//		iVar5 = iVar5 + 1;
	//		puVar4 = (CTeamElt*)((int)&this->aTeamElt->field_0x0 + iVar4);
	//		puVar4->field_0x0 = 2;
	//		puVar4->field_0x4 = 2;
	//		puVar4->field_0x14 = -1;
	//		iVar4 = iVar4 + 0x18;
	//	} while (iVar5 < this->nbTeams);
	//}

	//(this->camFigData).pCommanderRef_0x230 = this;
	//CCamFigData::Reset(&this->camFigData);
	//this->field_0x9f0 = 0;
	//this->field_0x9f4 = 0;
	//this->field_0x164 = 0;
	//this->field_0x160 = 0;
	//this->field_0x168 = 0;
	//this->field_0x174 = 0;
	//this->field_0x2f4 = 0;
	//this->field_0x6d0 = 1;
	return;
}

bool CActorCommander::BeginFightIntruder(CActor* pInstigator, CActor* pIntruder)
{
	bool bSuccess;
	CTeamElt* pCVar2;
	CTeamElt* pCVar3;
	int iVar4;

	bSuccess = false;

	if (pIntruder->IsKindOfObject(8) != false) {
		iVar4 = 0;
		pCVar3 = (CTeamElt*)0x0;
		if (0 < this->nbTeams) {
			pCVar2 = this->aTeamElt;
			do {
				pCVar3 = pCVar2;

				if (pCVar3->pEnemyActor == pInstigator) break;

				iVar4 = iVar4 + 1;
				pCVar2 = pCVar3 + 1;
				pCVar3 = (CTeamElt*)0x0;
			} while (iVar4 < this->nbTeams);
		}

		IMPLEMENTATION_GUARD(
		bSuccess = CSquad::AddFighter(&this->squad, pCVar3);
		CActorFighter::SetStandAnim((CActorFighter*)pInstigator, 0xe2);)
	}

	return bSuccess;
}

void CActorCommander::AddTracked()
{
	this->count_0x1d0 = this->count_0x1d0 + -1;

	if ((this->count_0x1d0 == 0) && ((this->flags & 2) != 0)) {
		this->flags = this->flags & 0xfffffffc;
	}

	return;
}

void CActorCommander::RemoveTracked()
{
	this->count_0x1d0 = this->count_0x1d0 + 1;

	if ((this->flags & 2) == 0) {
		this->flags = this->flags | 2;
		this->flags = this->flags & 0xfffffffe;
	}

	return;
}

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

void CActorCommander::CheckExorcism()
{
	CMagicInterface* pMagicInterface;
	float fVar2;
	float fVar3;
	undefined4 local_20;
	float local_1c;
	undefined4* local_4;
	CActorWolfen* pWolfen;

	if (0 < (this->exorcismActorsTable).entryCount) {
		if (this->field_0x1e0 == 0) {
			this->field_0x1e0 = 1;
			IMPLEMENTATION_GUARD(
				pMagicInterface = &CActorHero::_gThis->magicInterface;
			fVar2 = pMagicInterface->GetValue();
			fVar3 = pMagicInterface->GetTransit();
			this->field_0x1dc = fVar2 + fVar3;);
		}

		if (0.5f <= Timer::GetTimer()->scaledTotalTime - this->field_0x1e4) {
			pWolfen = static_cast<CActorWolfen*>((this->exorcismActorsTable).aEntries[0]);

			IMPLEMENTATION_GUARD(
			local_20 = 0xe;
			local_1c = this->field_0x1dc;
			fVar2 = pWolfen->field_0xb84 - pWolfen->field_0xb88;
			if (local_1c < fVar2) {
				this->field_0x1dc = 0.0f;
			}
			else {
				this->field_0x1dc = this->field_0x1dc - (pWolfen->field_0xb84 - pWolfen->field_0xb88);
				local_1c = fVar2;
			}
			local_4 = &local_20;
			CActor::DoMessage((CActor*)this, (CActor*)pWolfen, 0x1a, (uint)local_4);
			this->exorcismActorsTable.Pop(0);
			this->field_0x1e4 = Timer::GetTimer()->scaledTotalTime;)
		}

		if ((this->exorcismActorsTable).entryCount == 0) {
			this->field_0x1e0 = 0;
		}
	}

	return;
}

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

bool CActorCommander::CheckZone_00170f90(edF32VECTOR4* v0)
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

void CSquad::Create(ByteCode* pByteCode)
{
	uint uVar1;
	uint uVar2;
	float fVar3;
	float fVar4;

	fVar3 = pByteCode->GetF32();
	uVar1 = pByteCode->GetU32();
	uVar2 = pByteCode->GetU32();
	//if ((int)uVar1 < 0) {
	//	fVar4 = (float)(uVar1 >> 1 | uVar1 & 1);
	//	fVar4 = fVar4 + fVar4;
	//}
	//else {
	//	fVar4 = (float)uVar1;
	//}
	//CChessBoard::Init(fVar3, fVar3 * fVar4, &this->chessboard, uVar1, uVar2);
	//(this->field_0x0).x = 0.0;
	//(this->field_0x0).y = 0.6;
	//(this->field_0x0).w = 0.0;
	//(this->field_0x0).z = 0.0;
	//memset(this->field_0x10, 0, 0xa0);
	//this->field_0xb0 = 1;
	//this->field_0xb4 = 0;
	//this->field_0xbc = 0;
	//this->field_0xb8 = 0;
	//memset(this->field_0xc0, 0, 0xa0);
	return;
}

CTeamElt::CTeamElt()
{
	this->field_0x0 = 2;
	this->field_0x4 = 2;
	this->field_0x14 = -1;

	return;
}
