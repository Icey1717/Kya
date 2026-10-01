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

void CActorMiniGame::SetScoreName(char* pName)
{
	S_MINI_GAME_SCORE* pSVar1;
	int iVar2;

	iVar2 = this->curBehaviourId;
	if (iVar2 == 3) {
		memcpy(this->field_0x188, pName, 3);
		this->field_0x188[3] = 0;
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

void CActorMiniGame::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

void CActorMiniGame::FUN_003ace00()
{
	this->field_0x1d8 = this->field_0x1d8 - 1;
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
