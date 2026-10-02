#include "ActorMiniGamesManager.h"
#include "MemoryStream.h"
#include "ActorMiniGame.h"
#include "ActorNativCmd.h"
#include "ActorMiniGamesOrganizer.h"
#include "LevelScheduler.h"

bool CActorMiniGamesManager::IsBetAvailable(int cost, int reward)
{
	Episode* pEVar1;
	int iVar2;

	iVar2 = cost + CLevelScheduler::_gGameNfo.bet;
	pEVar1 = CLevelScheduler::GetLastEpisode();
	if (pEVar1->bet < iVar2) {
		pEVar1 = CLevelScheduler::GetLastEpisode();
		iVar2 = pEVar1->bet;
	}
	return reward <= iVar2;
}

bool CActorMiniGamesManager::PlaceBet(int cost)
{
	bool bVar1;

	bVar1 = cost <= CLevelScheduler::_gGameNfo.nbMoney;
	if (bVar1) {
		CLevelScheduler::gThis->Money_GiveToBet(cost);
	}
	return bVar1;
}

int CActorMiniGamesManager::FUN_003ad9f0(int param_2)
{
	int iVar2;
	int iVar3;
	CActorMiniGame* pMiniGame;

	iVar3 = 0;
	iVar2 = -1;
	if (0 < this->nbMiniGames) {
		do {
			pMiniGame = this->aMiniGames[iVar3];
			if (param_2 < pMiniGame->field_0x1d8) {
				if (param_2 != -1) {
					pMiniGame->FUN_003ace00();
				}

				if (iVar2 < pMiniGame->field_0x1d8) {
					iVar2 = pMiniGame->field_0x1d8;
				}
			}

			iVar3 = iVar3 + 1;
		} while (iVar3 < this->nbMiniGames);
	}

	this->nextMiniGameOrder = iVar2 + 1;

	return this->nextMiniGameOrder;
}



CActorMiniGamesManager::CActorMiniGamesManager()
{
	this->aMiniGames = (CActorMiniGame**)0x0;
	this->nbMaxMiniGames = 0;
}

CActorMiniGamesManager::~CActorMiniGamesManager()
{
	if (this->nbMaxMiniGames != 0) {
		delete[] this->aMiniGames;
	}
}

void CActorMiniGamesManager::Create(ByteCode* pByteCode)
{
	CActor::Create(pByteCode);

	this->pOrganizerStreamRefs = S_ACTOR_STREAM_REF::Create(pByteCode);
	this->actorRef.index = pByteCode->GetS32();

	return;
}

void CActorMiniGamesManager::Init()
{
	CActorMiniGamesOrganizer* pCVar1;
	S_ACTOR_STREAM_REF* pRef;
	int iVar2;
	int iVar3;

	CActor::Init();

	this->pOrganizerStreamRefs->Init();
	this->actorRef.Init();
	this->nbMaxMiniGames = 0;

	iVar2 = 0;
	while (true) {
		pRef = this->pOrganizerStreamRefs;
		iVar3 = 0;
		if (pRef != (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = pRef->entryCount;
		}

		if (iVar3 <= iVar2) break;

		pCVar1 = static_cast<CActorMiniGamesOrganizer*>(pRef->aEntries[iVar2].Get());
		iVar3 = 0;
		if (pCVar1->pMiniGameStreamRefs != (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = pCVar1->pMiniGameStreamRefs->entryCount;
		}

		iVar2 = iVar2 + 1;
		this->nbMaxMiniGames = this->nbMaxMiniGames + iVar3;
	}

	if (this->nbMaxMiniGames != 0) {
		// ClearLocalData searches the entire allocation, including unused entries.
		this->aMiniGames = new CActorMiniGame*[this->nbMaxMiniGames]();
	}

	iVar2 = 0;
	while (true) {
		pRef = this->pOrganizerStreamRefs;
		iVar3 = 0;
		if (pRef != (S_ACTOR_STREAM_REF*)0x0) {
			iVar3 = pRef->entryCount;
		}

		if (iVar3 <= iVar2) break;

		pCVar1 = static_cast<CActorMiniGamesOrganizer*>(pRef->aEntries[iVar2].Get());
		iVar2 = iVar2 + 1;
		pCVar1->field_0x9f4 = this;
	}

	ClearLocalData();

	this->pOrganizerStreamRefs->Reset();
	this->field_0x174 = 0;

	return;
}

void CActorMiniGamesManager::Reset()
{
	CActor::Reset();

	this->pOrganizerStreamRefs->Reset();
	this->field_0x174 = 0;

	return;
}

void CActorMiniGamesManager::CheckpointReset()
{
	CActorMiniGame** ppCVar1;
	int iVar2;
	int iVar3;
	int iVar4;

	CActor::CheckpointReset();

	iVar3 = -1;
	iVar4 = 0;
	if (0 < this->nbMiniGames) {
		ppCVar1 = this->aMiniGames;
		do {
			iVar2 = (*ppCVar1)->field_0x1d8;
			if (iVar3 < iVar2) {
				iVar3 = iVar2;
			}

			iVar4 = iVar4 + 1;
			ppCVar1 = ppCVar1 + 1;
		} while (iVar4 < this->nbMiniGames);
	}

	this->nextMiniGameOrder = iVar3;

	return;
}

CBehaviour* CActorMiniGamesManager::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == MINI_GAMES_MANAGER_BEHAVIOUR_STAND) {
		pNewBehaviour = &this->behaviourStand;
	}
	else {
		pNewBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pNewBehaviour;
}

StateConfig CActorMiniGamesManager::_gStateCfg_MGM[1] =
{
	StateConfig(0, 0)
};

StateConfig* CActorMiniGamesManager::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 1);
		pStateConfig = _gStateCfg_MGM + state + -5;
	}

	return pStateConfig;
}

int CActorMiniGamesManager::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorMiniGame** ppCVar1;
	CActorMiniGame* pCVar2;
	int iVar3;
	int iVar4;
	int result;

	if (msg == MESSAGE_NATIV_CMD) {
		if (static_cast<CActorNativMsgParam_0xe*>(pMsgParam)->type == 0x11) {
			iVar3 = 0;
			if (0 < this->nbMiniGames) {
				ppCVar1 = this->aMiniGames;
				do {
					pCVar2 = *ppCVar1;
					if (pCVar2->field_0x1d8 == 0) goto LAB_003adc60;
					iVar3 = iVar3 + 1;
					ppCVar1 = ppCVar1 + 1;
				} while (iVar3 < this->nbMiniGames);
			}

			pCVar2 = (CActorMiniGame*)0x0;
		LAB_003adc60:
			iVar3 = -1;
			pCVar2->field_0x1d8 = -1;
			iVar4 = 0;

			if (0 < this->nbMiniGames) {
				do {
					pCVar2 = this->aMiniGames[iVar4];
					if (0 < pCVar2->field_0x1d8) {
						pCVar2->FUN_003ace00();

						if (iVar3 < pCVar2->field_0x1d8) {
							iVar3 = pCVar2->field_0x1d8;
						}
					}

					iVar4 = iVar4 + 1;
				} while (iVar4 < this->nbMiniGames);
			}

			this->nextMiniGameOrder = iVar3 + 1;
		}
		result = 0;
	}
	else {
		result = CActor::InterpretMessage(pSender, msg, pMsgParam);
	}

	return result;
}

void CActorMiniGamesManager::ClearLocalData()
{
	CActorMiniGame* pCVar1;
	CActorMiniGamesOrganizer* pCVar2;
	S_ACTOR_STREAM_REF* pSVar3;
	bool bVar4;
	int iVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	int iVar9;
	CActorMiniGame** ppCVar10;

	iVar5 = 0;
	iVar6 = 0;
	do {
		pSVar3 = this->pOrganizerStreamRefs;
		iVar7 = 0;
		if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar7 = pSVar3->entryCount;
		}

		if (iVar7 <= iVar6) {
			this->nbMiniGames = iVar5;
			return;
		}

		pCVar2 = static_cast<CActorMiniGamesOrganizer*>(pSVar3->aEntries[iVar6].Get());
		iVar8 = 0;
		while (true) {
			pSVar3 = pCVar2->pMiniGameStreamRefs;
			iVar7 = 0;
			if (pSVar3 != (S_ACTOR_STREAM_REF*)0x0) {
				iVar7 = pSVar3->entryCount;
			}

			if (iVar7 <= iVar8) break;

			pCVar1 = static_cast<CActorMiniGame*>(pSVar3->aEntries[iVar8].Get());
			iVar9 = 0;
			if (0 < this->nbMaxMiniGames) {
				ppCVar10 = this->aMiniGames;
				do {
					bVar4 = true;
					if (pCVar1 == *ppCVar10) goto LAB_003adfc0;
					iVar9 = iVar9 + 1;
					ppCVar10 = ppCVar10 + 1;
				} while (iVar9 < this->nbMaxMiniGames);
			}
			bVar4 = false;
		LAB_003adfc0:
			if (!bVar4) {
				this->aMiniGames[iVar5] = pCVar1;
				iVar5 = iVar5 + 1;
			}
			iVar8 = iVar8 + 1;
		}
		iVar6 = iVar6 + 1;
	} while (true);
}

void CBehaviourMiniGamesManager::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGamesManager::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorMiniGamesManager*>(pOwner);

	return;
}

int CBehaviourMiniGamesManager::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourMiniGamesManager::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourMiniGamesManagerStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGamesManagerStand::Manage()
{
	return;
}

void CBehaviourMiniGamesManagerStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourMiniGamesManager::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMiniGamesManagerStand::InitState(int newState)
{
	return;
}

void CBehaviourMiniGamesManagerStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourMiniGamesManagerStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
