#include "ActorMiniGame.h"
#include "MemoryStream.h"

void CActorMiniGame::PrevFinalAction()
{
	int iVar1;

	iVar1 = this->field_0x1cc;
	do {
		iVar1 = (iVar1 + 2) % 3;
		if (iVar1 == 1) {
			if (this->curBehaviourId != 3) {
				this->field_0x1cc = 1;
				return;
			}
		}
		else {
			if (iVar1 != 0) {
				this->field_0x1cc = iVar1;
				return;
			}

			if ((this->field_0x1b0 == 0) || (this->field_0x1b0 == 1)) {
				this->field_0x1cc = 0;
				return;
			}
		}

		if (iVar1 == this->field_0x1cc) {
			return;
		}
	} while (true);
}

void CActorMiniGame::NextFinalAction()
{
	int iVar1;

	iVar1 = this->field_0x1cc;
	do {
		iVar1 = (iVar1 + 1) % 3;
		if (iVar1 == 1) {
			if (this->curBehaviourId != 3) {
				this->field_0x1cc = 1;
				return;
			}
		}
		else {
			if (iVar1 != 0) {
				this->field_0x1cc = iVar1;
				return;
			}

			if ((this->field_0x1b0 == 0) || (this->field_0x1b0 == 1)) {
				this->field_0x1cc = 0;
				return;
			}
		}

		if (iVar1 == this->field_0x1cc) {
			return;
		}
	} while( true );
}

CBehaviourMiniGameBetting* CActorMiniGame::GetBhvBetting()
{
	return (CBehaviourMiniGameBetting*)0x0;
}

CBehaviourMiniGameTraining* CActorMiniGame::GetBhvTraining()
{
	return (CBehaviourMiniGameTraining*)0x0;
}

CBehaviourMiniGameMulti* CActorMiniGame::GetBhvMulti()
{
	return (CBehaviourMiniGameMulti*)0x0;
}

int CActorMiniGame::GetUnity()
{
	return 0;
}

void CActorMiniGame::SetYouAreChosen(byte param_2)
{
	this->field_0x1b8 = param_2;
	this->field_0x19c.Switch(this);

	return;
}

bool CActorMiniGame::MustStop()
{
	return this->bMustStop;
}

void CActorMiniGame::UpdateCurHighScoreName(char* pName)
{
	S_MINI_GAME_SCORE* pSVar1;
	int iVar2;

	iVar2 = this->curBehaviourId;
	if (iVar2 == 3) {
		memcpy(this->defaultScore.name, pName, 3);
		this->defaultScore.name[3] = 0;
	}
	else {
		pSVar1 = (S_MINI_GAME_SCORE*)0x0;
		if (iVar2 == 2) {
			CBehaviourMiniGameTraining* pBehaviour = GetBhvTraining();
			if (this->field_0x1b4 < pBehaviour->nbScores) {
				pSVar1 = pBehaviour->aScores + this->field_0x1b4;
			}
		}
		else {
			if (iVar2 == 4) {
				CBehaviourMiniGameMulti* pBehaviour = GetBhvMulti();
				if (this->field_0x1b4 < pBehaviour->nbScores) {
					pSVar1 = pBehaviour->aScores + this->field_0x1b4;
				}
			}
		}
		if (pSVar1 != (S_MINI_GAME_SCORE*)0x0) {
			memcpy(pSVar1->name, pName, 3);
			pSVar1->name[3] = 0;
		}
	}

	return;
}

CActorMiniGame::CActorMiniGame()
{
	this->field_0x174 = 0;
	this->field_0x178 = (float*)0x0;
	this->nbScores = 0;
	this->aScores = (S_MINI_GAME_SCORE*)0x0;

	return;
}

void CActorMiniGame::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

void CActorMiniGame::Init()
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::Reset()
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::CheckpointReset()
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::SaveContext(void* pData, uint mode, uint maxSize)
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::LoadContext(void* pData, uint mode, uint maxSize)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorMiniGame::BuildBehaviour(int behaviourType)
{
	IMPLEMENTATION_GUARD();
	return nullptr;
}

StateConfig* CActorMiniGame::GetStateCfg(int state)
{
	IMPLEMENTATION_GUARD();
	return nullptr;
}

int CActorMiniGame::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

void CActorMiniGame::FUN_003ace00()
{
	this->field_0x1d8 = this->field_0x1d8 - 1;
}

void CBehaviourMiniGame::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGame::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorMiniGame*>(pOwner);

	return;
}

int CBehaviourMiniGame::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourMiniGame::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourMiniGameBetting::Create(ByteCode* pByteCode)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameBetting::Init(CActor* pOwner)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameBetting::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameBetting::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

int CBehaviourMiniGameBetting::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

void CBehaviourMiniGameTraining::Create(ByteCode* pByteCode)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameTraining::Init(CActor* pOwner)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameTraining::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameTraining::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	IMPLEMENTATION_GUARD();
}

int CBehaviourMiniGameTraining::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
	return 0;
}

void CBehaviourMiniGameMulti::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGameMulti::Init(CActor* pOwner)
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourMiniGameMulti::Term()
{
	return;
}

void CBehaviourMiniGameMulti::Manage()
{
	return;
}

void CBehaviourMiniGameMulti::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourMiniGame::Begin(pOwner, newState, newAnimationType);

	return;
}

int CBehaviourMiniGameMulti::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorMiniGame* pMiniGame;

	if (msg == 0x5b) {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(5, -1);
	}

	return 0;
}

void CBehaviourMiniGameMulti::AddOnePlayer()
{
	if (this->nbPlayers < 6) {
		this->nbPlayers = this->nbPlayers + 1;
	}

	return;
}

void CBehaviourMiniGameMulti::SubOnePlayer()
{
	if (2 < this->nbPlayers) {
		this->nbPlayers = this->nbPlayers + -1;
	}

	return;
}
