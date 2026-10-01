#include "ActorMiniGame.h"
#include "MemoryStream.h"

char CHighScoreArray::_STRING_Init[4] = { 0, 0, 0, 0 };

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
	uint count;
	int iVar1;
	S_MINI_GAME_BET* pCurBet;

	this->nbBets = pByteCode->GetS32();
	count = this->nbBets;

	if (count != 0) {
		this->aBets = new S_MINI_GAME_BET[count];
		iVar1 = 0;
		if (0 < this->nbBets) {
			do {
				pCurBet = this->aBets + iVar1;
				pCurBet->cost = pByteCode->GetS32();
				pCurBet->reward = pByteCode->GetS32();
				iVar1 = iVar1 + 1;
			} while (iVar1 < this->nbBets);
		}
	}

	return;
}

void CBehaviourMiniGameBetting::Init(CActor* pOwner)
{
	int iVar2;

	iVar2 = 0;
	if (0 < this->nbBets) {
		do {
			this->aBets[iVar2].bAvailable = true;
			iVar2 = iVar2 + 1;
		} while (iVar2 < this->nbBets);
	}

	return;
}

void CBehaviourMiniGameBetting::Manage()
{
	return;
}

void CBehaviourMiniGameBetting::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourMiniGame::Begin(pOwner, newState, newAnimationType);

	return;
}

int CBehaviourMiniGameBetting::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorMiniGame* pMiniGame;

	if (msg == 0x5b) {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(5, -1);
	}

	return 0;
}

void CBehaviourMiniGameTraining::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGameTraining::Init(CActor* pOwner)
{
	char cVar1;
	char cVar2;
	char cVar3;
	S_MINI_GAME_SCORE* pScore;
	int iVar6;
	int iVar7;
	int iVar8;
	float fVar13;
	float local_8;
	char local_4;
	char local_3;
	char local_2;

	this->pOwner = static_cast<CActorMiniGame*>(pOwner);
	this->nbScores = 5;
	if (this->nbScores != 0) {
		this->aScores = new S_MINI_GAME_SCORE[this->nbScores];
	}

	iVar8 = 0;
	if (0 < this->nbScores) {
		do {
			this->aScores[iVar8].score = -1.0f;
			this->aScores[iVar8].name[0] = CHighScoreArray::_STRING_Init[0];
			this->aScores[iVar8].name[1] = CHighScoreArray::_STRING_Init[1];
			this->aScores[iVar8].name[2] = CHighScoreArray::_STRING_Init[2];
			this->aScores[iVar8].name[3] = CHighScoreArray::_STRING_Init[3];
			this->aScores[iVar8].name[3] = 0;
			iVar8 = iVar8 + 1;
		} while (iVar8 < this->nbScores);
	}

	iVar7 = 0;
	while (true) {
		if ((this->pOwner->nbScores <= iVar7) || (4 < iVar7)) break;
		pScore = this->pOwner->aScores + iVar7;
		if (iVar7 <= this->nbScores) {
			local_4 = this->aScores[iVar7].name[0];
			local_3 = this->aScores[iVar7].name[1];
			local_2 = this->aScores[iVar7].name[2];
			local_8 = this->aScores[iVar7].score;
			this->aScores[iVar7].score = pScore->score;
			if (pScore->name != (char*)0x0) {
				memcpy(this->aScores[iVar7].name, pScore->name, 4);
				this->aScores[iVar7].name[3] = 0;
			}

			iVar6 = iVar7 + 1;
			if (iVar6 < this->nbScores) {
				do {
					cVar1 = this->aScores[iVar6].name[0];
					cVar2 = this->aScores[iVar6].name[1];
					cVar3 = this->aScores[iVar6].name[2];
					fVar13 = this->aScores[iVar6].score;
					this->aScores[iVar6].name[0] = local_4;
					this->aScores[iVar6].name[1] = local_3;
					this->aScores[iVar6].name[2] = local_2;
					this->aScores[iVar6].name[3] = 0;
					this->aScores[iVar6].score = local_8;
					local_8 = fVar13;
					local_4 = cVar1;
					local_3 = cVar2;
					local_2 = cVar3;
					iVar6 = iVar6 + 1;
				} while (iVar6 < this->nbScores);
			}
		}
		iVar7 = iVar7 + 1;
	}

	return;
}

void CBehaviourMiniGameTraining::Manage()
{
	return;
}

void CBehaviourMiniGameTraining::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourMiniGame::Begin(pOwner, newState, newAnimationType);

	return;
}

int CBehaviourMiniGameTraining::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorMiniGame* pMiniGame;

	if (msg == 0x5b) {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(5, -1);
	}

	return 0;
}

void CBehaviourMiniGameMulti::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGameMulti::Init(CActor* pOwner)
{
	char cVar1;
	char cVar2;
	char cVar3;
	S_MINI_GAME_SCORE* pScore;
	int iVar6;
	int iVar7;
	int iVar8;
	float fVar13;
	float local_8;
	char local_4;
	char local_3;
	char local_2;

	this->pOwner = static_cast<CActorMiniGame*>(pOwner);
	this->nbScores = 5;
	if (this->nbScores != 0) {
		this->aScores = new S_MINI_GAME_SCORE[this->nbScores];
	}

	iVar8 = 0;
	if (0 < this->nbScores) {
		do {
			this->aScores[iVar8].score = -1.0f;
			this->aScores[iVar8].name[0] = CHighScoreArray::_STRING_Init[0];
			this->aScores[iVar8].name[1] = CHighScoreArray::_STRING_Init[1];
			this->aScores[iVar8].name[2] = CHighScoreArray::_STRING_Init[2];
			this->aScores[iVar8].name[3] = CHighScoreArray::_STRING_Init[3];
			this->aScores[iVar8].name[3] = 0;
			iVar8 = iVar8 + 1;
		} while (iVar8 < this->nbScores);
	}

	iVar7 = 0;
	while (true) {
		if ((this->pOwner->nbScores <= iVar7) || (4 < iVar7)) break;
		pScore = this->pOwner->aScores + iVar7;
		if (iVar7 <= this->nbScores) {
			local_4 = this->aScores[iVar7].name[0];
			local_3 = this->aScores[iVar7].name[1];
			local_2 = this->aScores[iVar7].name[2];
			local_8 = this->aScores[iVar7].score;
			this->aScores[iVar7].score = pScore->score;
			if (pScore->name != (char*)0x0) {
				memcpy(this->aScores[iVar7].name, pScore->name, 4);
				this->aScores[iVar7].name[3] = 0;
			}

			iVar6 = iVar7 + 1;
			if (iVar6 < this->nbScores) {
				do {
					cVar1 = this->aScores[iVar6].name[0];
					cVar2 = this->aScores[iVar6].name[1];
					cVar3 = this->aScores[iVar6].name[2];
					fVar13 = this->aScores[iVar6].score;
					this->aScores[iVar6].name[0] = local_4;
					this->aScores[iVar6].name[1] = local_3;
					this->aScores[iVar6].name[2] = local_2;
					this->aScores[iVar6].name[3] = 0;
					this->aScores[iVar6].score = local_8;
					local_8 = fVar13;
					local_4 = cVar1;
					local_3 = cVar2;
					local_2 = cVar3;
					iVar6 = iVar6 + 1;
				} while (iVar6 < this->nbScores);
			}
		}

		iVar7 = iVar7 + 1;
	}

	this->nbPlayers = 2;

	return;
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
