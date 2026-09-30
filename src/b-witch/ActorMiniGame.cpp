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

CBehaviourMiniGameMulti* CActorMiniGame::GetMultiBehaviour()
{
	return static_cast<CBehaviourMiniGameMulti*>(GetBehaviour(3));
}

CBehaviourMiniGameTrain* CActorMiniGame::GetTrainBehaviour()
{
	return static_cast<CBehaviourMiniGameTrain*>(GetBehaviour(2));
}

CBehaviourMiniGameSolo* CActorMiniGame::GetSoloBehaviour()
{
	return static_cast<CBehaviourMiniGameSolo*>(GetBehaviour(4));
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
			CBehaviourMiniGameTrain* pBehaviour = GetTrainBehaviour();
			if (this->field_0x1b4 < pBehaviour->nbScores) {
				pSVar1 = pBehaviour->aScores + this->field_0x1b4;
			}
		}
		else {
			if (iVar2 == 4) {
				CBehaviourMiniGameSolo* pBehaviour = GetSoloBehaviour();
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
