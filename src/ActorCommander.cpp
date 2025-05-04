#include "ActorCommander.h"
#include "MemoryStream.h"
#include "ActorManager.h"
#include "ActorWolfen.h"
#include "ActorHero.h"
#include "EventManager.h"
#include "MathOps.h"
#include "TimeController.h"
#include "CameraGame.h"



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

void CActorCommander::Term()
{
	CTeamElt* pCVar1;

	pCVar1 = this->aTeamElt;
	if (pCVar1 != (CTeamElt*)0x0) {
		if (pCVar1 != (CTeamElt*)0x0) {
			IMPLEMENTATION_GUARD_LOG(); // VECTOR?
			delete pCVar1;
		}

		this->aTeamElt = (CTeamElt*)0x0;
	}

	this->nbTeams = 0;

	this->squad.Term();

	CActor::Term();

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

		iVar1 = this->squad.NbElt();
		if (iVar1 != 0) {
			edF32Vector4SubHard(&eStack112, &this->currentLocation, &local_20);
			edF32Matrix4TranslateHard(&eStack96, &gF32Matrix4Unit, &eStack112);
			IMPLEMENTATION_GUARD_LOG(
			FUN_00352d90((int)&(this->squad).chessboard, &eStack96);)
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
			if (((0.0f < pEntryWolfen->GetLifeInterface()->GetValue()) && (pEntryWolfen->IsKindOfObject(0x10) != false)) &&
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

	this->squad.Clear();

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

	this->camFigData.pCommanderRef_0x230 = this;
	this->camFigData.Reset();
	this->field_0x9f0 = (CActorFighter*)0x0;
	//this->field_0x9f4 = 0;
	//this->field_0x164 = 0;
	//this->field_0x160 = 0;
	//this->field_0x168 = 0;
	//this->field_0x174 = 0;
	this->field_0x2f4 = 0;
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

	if (pIntruder->IsKindOfObject(OBJ_TYPE_FIGHTER) != false) {
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

void CActorCommander::EndFightIntruder(CActorFighter* pIntruder)
{
	CTeamElt* pCVar1;
	CTeamElt* pTeamElt;
	int iVar2;

	iVar2 = 0;
	pTeamElt = (CTeamElt*)0x0;
	if (0 < this->nbTeams) {
		pCVar1 = this->aTeamElt;
		do {
			pTeamElt = pCVar1;
			if (pTeamElt->pEnemyActor == pIntruder) break;
			iVar2 = iVar2 + 1;
			pCVar1 = pTeamElt + 1;
			pTeamElt = (CTeamElt*)0x0;
		} while (iVar2 < this->nbTeams);
	}

	this->squad.RemoveFighter(pTeamElt);
	pIntruder->SetStandAnim(-1);

	return;
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
	long lVar10;
	CCamFigData* pCVar11;
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
		IMPLEMENTATION_GUARD(
		pFightCamera = (CCameraGame*)CActorHero::_gThis->pFightCamera;
		pCVar11 = CCameraGame::_pfig_data;
		if (CCameraGame::_b_use_fig_data == 0) {
			pCVar11 = (CCamFigData*)0x0;
		}
		if ((pCVar11 != &this->camFigData) || ((this->camFigData).field_0x2a0 == 0)) {
			pCVar6 = (*((pFightCamera->view).camera.objBase.pVTable)->GetTarget)((CCamera*)pFightCamera);
			CCamFigData::SetPosParam(&this->camFigData, &pCVar6->currentLocation);
			CCameraGame::Fight_SetConfig(pFightCamera, (long)(int)&this->camFigData);
			this->field_0x9f0 = 0;
			CCamFigData::IsValid(&this->camFigData, 1);
		}
		CSquad::ComputeBoundingSphere(&this->squad, &local_10);
		CCamFigData::SetBoundingSphere(&this->camFigData, &local_10);
		pCVar7 = CActor::GetBehaviour((CActor*)this, this->curBehaviourId);
		lVar10 = (*(code*)pCVar7->pVTable->LoadContext)();
		if (lVar10 == 0) {
			local_30.x = local_10.x;
			local_30.y = local_10.y;
			local_30.z = local_10.z;
			local_30.w = 1.0;
			CCamFigData::SetAdversaryPos(&this->camFigData, &local_30);
		}
		else {
			iVar9 = (int)lVar10;
			local_20.z = *(float*)(iVar9 + 0x38);
			local_20.w = *(float*)(iVar9 + 0x3c);
			local_20.x = (float)*(undefined8*)(iVar9 + 0x30);
			local_20.y = (float)((ulong) * (undefined8*)(iVar9 + 0x30) >> 0x20) - *(float*)(iVar9 + 0xec);
			CCamFigData::SetAdversaryPos(&this->camFigData, &local_20);
		}
		pCVar1 = CActorHero::_gThis;
		if ((((CActorHero::_gThis != (CActorHero*)0x0) &&
			(pCurAdversary = (CActorHero::_gThis->character).pAdversary, pCurAdversary != (CActorFighter*)0x0)) &&
			(pCurAdversary != (CActorFighter*)this->field_0x9f0)) &&
			((bVar3 = CActorFighter::FUN_0031b790
			((CActorFighter*)CActorHero::_gThis,
				(CActorHero::_gThis->character).characterBase.base.base.actorState), bVar3 != false &&
				(((pCVar1->character).fightFlags & 0x40) != 0)))) {
			pCurAdversary = (pCVar1->character).pAdversary;
			pCVar8 = (*((pCurAdversary->characterBase).base.base.pVTable)->GetLifeInterface)((CActor*)pCurAdversary);
			fVar16 = (*pCVar8->pVtable->GetValue)((CInterface*)pCVar8);
			if ((fVar16 <= 0.0) &&
				(((iVar9 = (pCVar1->character).pBlow, iVar9 != 0 && ((*(byte*)(iVar9 + 6) & 1) != 0)) ||
					((iVar9 = (pCVar1->character).field_0x840, iVar9 != 0 && (*(int*)(iVar9 + 0x38) == 2)))))) {
				fVar16 = GetAngleYFromVector(&CActorHero::_gThis->rotationQuat);
				fVar17 = (*((pFightCamera->view).camera.objBase.pVTable)->GetAngleBeta)(pFightCamera);
				fVar17 = edF32GetAnglesDelta(fVar17, fVar16 - 0.7853982);
				t0 = (*((pFightCamera->view).camera.objBase.pVTable)->GetAngleBeta)(pFightCamera);
				fVar16 = edF32GetAnglesDelta(t0, fVar16 + 0.7853982);
				if (ABS(fVar17) <= ABS(fVar16)) {
					fVar16 = fVar17;
				}
				fVar17 = (*((pFightCamera->view).camera.objBase.pVTable)->GetAngleBeta)();
				iVar9 = FUN_003c79f0(fVar16 + fVar17, 0.1396263, &this->camFigData);
				if (iVar9 != 0) {
					uVar15 = 2;
					if (0.0 <= fVar16) {
						uVar15 = 3;
					}
					fVar17 = (*((pFightCamera->view).camera.objBase.pVTable)->GetAngleBeta)();
					CCamFigData::FUN_003c5ba0(fVar16 + fVar17, &this->camFigData, uVar15, 1);
				}

				this->field_0x9f0 = (pCVar1->character).pAdversary;
			}
		}

		if ((this->camFigData).field_0x2b4 == 1) {
			FUN_001717b0(pFightCamera);
		})
	}
	else {
		if (((this->flags_0x18c & 7) == 0) || (1.0f <= this->field_0x1d4)) {
			this->camFigData.IsValid(0);
			this->field_0x9f0 = (CActorFighter*)0x0;
		}
	}

	return;
}

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

	IMPLEMENTATION_GUARD(
	if ((GetStateFlags(this->actorState) & 0x100) == 0) {
		this_00 = (this->squad).eltTable.aEntries[0]->pEnemyActor->pAdversary;
		if ((this_00 != (CActorFighter*)0x0) &&
			(bVar1 = this_00->FUN_0031b790(this_00->actorState), bVar1 != false)) {
			bVar1 = false;
			if ((this_00->pFighterCombo != (s_fighter_combo*)0x0) && ((this_00->pFighterCombo->field_0x4 & 0x100U) != 0)) {
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

	this->squad.SynchronizePawns();)
	return;
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
	if ((int)uVar1 < 0) {
		fVar4 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar4 = fVar4 + fVar4;
	}
	else {
		fVar4 = (float)uVar1;
	}

	//CChessBoard::Init(fVar3, fVar3 * fVar4, &this->chessboard, uVar1, uVar2);

	this->aSubObjs[0].field_0x0 = 0;
	this->aSubObjs[0].field_0x4 = 0.6f;
	this->aSubObjs[0].field_0xc = 0;
	this->aSubObjs[0].field_0x8 = 0;
	memset(this->aSubObjs[0].field_0x10, 0, sizeof(this->aSubObjs[0].field_0x10));

	this->aSubObjs[1].field_0x0 = 1;
	this->aSubObjs[1].field_0x4 = 0.0f;
	this->aSubObjs[1].field_0xc = 0;
	this->aSubObjs[1].field_0x8 = 0;
	memset(this->aSubObjs[1].field_0x10, 0, sizeof(this->aSubObjs[1].field_0x10));

	return;
}

void CSquad::Term()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CSquad::Clear()
{
	int iVar1;
	uint uVar2;
	SquadSubObj_0x20* pCVar3;
	uint uVar4;
	uint uVar5;

	iVar1 = (this->eltTable).entryCount;
	while (iVar1 != 0) {
		CActorWolfen* pWolfen = static_cast<CActorWolfen*>((this->eltTable).aEntries[iVar1 - 1]->pEnemyActor);
		pWolfen->TermFightAction();
		this->eltTable.PopCurrent();
		iVar1 = (this->eltTable).entryCount;
	}

	SquadSubObj_0xb0* pSubObj_0xb0 = this->aSubObjs;

	uVar5 = 0;
	do {
		uVar4 = 0;
		pCVar3 = pSubObj_0xb0->field_0x10;

		if (pSubObj_0xb0->field_0x8 != 0) {
			do {
				uVar2 = pCVar3->flags & 1;
				if (((uVar2 != 0) && (uVar4 != 0xffffffff)) && (uVar2 != 0)) {
					CActorWolfen* pWolfen = pCVar3->pWolfen;
					if (pWolfen != (CActorWolfen*)0x0) {
						IMPLEMENTATION_GUARD(
						pWolfen->DisableFightAction();)
					}

					pWolfen = pCVar3->pWolfen;
					if (pWolfen != (CActorWolfen*)0x0) {
						pCVar3->field_0x4 = pWolfen;
					}

					if ((pCVar3->flags & 5) != 0) {
						pSubObj_0xb0->field_0xc = pSubObj_0xb0->field_0xc + 1;
					}

					pCVar3->flags = 0;
					pCVar3->field_0x10 = 0;
					pCVar3->pWolfen = (CActorWolfen*)0x0;
					pCVar3->field_0x8 = 0;
				}

				CActorWolfen* pWolfen = pCVar3->pWolfen;
				if (pWolfen != (CActorWolfen*)0x0) {
					pCVar3->field_0x4 = pWolfen;
				}

				if ((pCVar3->flags & 5) != 0) {
					pSubObj_0xb0->field_0xc = pSubObj_0xb0->field_0xc + 1;
				}

				pCVar3->flags = 0;
				pCVar3->field_0x10 = 0;
				uVar4 = uVar4 + 1;
				pCVar3->pWolfen = (CActorWolfen*)0x0;
				pCVar3->field_0x8 = 0;
				pCVar3 = pCVar3 + 1;
			} while (uVar4 < pSubObj_0xb0->field_0x8);
		}

		uVar5 = uVar5 + 1;
		pSubObj_0xb0 = pSubObj_0xb0 + 1;
	} while (uVar5 < 2);

	return;
}

int CSquad::NbElt()
{
	return this->eltTable.entryCount;
}

void CSquad::ClearSquadDeadAndNonFightingElt()
{
	CTeamElt* pEntry;
	CActorWolfen* pWolfen;
	CTeamElt** pEntries;
	int curIndex;

	curIndex = 0;
	pEntries = this->eltTable.aEntries;
	if (0 < (this->eltTable).entryCount) {
		do {
			pEntry = *pEntries;
			if (pEntry != (CTeamElt*)0x0) {
				pWolfen = static_cast<CActorWolfen*>(pEntry->pEnemyActor);
			
				if ((pWolfen->GetLifeInterface()->GetValue() <= 0.0f) || (pWolfen->IsFightRelated(pWolfen->curBehaviourId) == false) || ((pWolfen->flags & 4) == 0)) {
					pWolfen->TermFightAction();
					this->eltTable.Pop(curIndex);
				}
			}

			curIndex = curIndex + 1;
			pEntries = pEntries + 1;
		} while (curIndex < (this->eltTable).entryCount);
	}

	return;
}

void CSquad::RemoveFighter(CTeamElt* pTeamElt)
{
	int iVar1;
	CTeamElt** pCVar2;
	int iVar3;

	iVar1 = (this->eltTable).entryCount;
	iVar3 = 0;
	pCVar2 = this->eltTable.aEntries;
	if (0 < iVar1) {
		do {
			if (*pCVar2 == pTeamElt) {
				CActorWolfen* pWolfen = static_cast<CActorWolfen*>(pTeamElt->pEnemyActor);
				pWolfen->TermFightAction();
				this->eltTable.Pop(iVar3);
				IMPLEMENTATION_GUARD(
				SynchronizePawns();)
				return;
			}

			iVar3 = iVar3 + 1;
			pCVar2 = pCVar2 + 1;
		} while (iVar3 < iVar1);
	}

	return;
}

CTeamElt::CTeamElt()
{
	this->field_0x0 = 2;
	this->field_0x4 = 2;
	this->field_0x14 = -1;

	return;
}
