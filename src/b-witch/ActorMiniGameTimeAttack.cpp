#include "ActorMiniGameTimeAttack.h"
#include "MemoryStream.h"
#include "Rendering/edCTextFormat.h"
#include "TranslatedTextData.h"
#include "kya.h"
#include "TimeController.h"

StateConfig CActorMiniGameTimeAttack::_gStateCfg_MTA[1] =
{
	StateConfig(0, 0)
};

void CActorMiniGameTimeAttack::Create(ByteCode* pByteCode)
{
	int iVar1;

	CActorMiniGame::Create(pByteCode);
	this->field_0x1e0 = pByteCode->GetS32();
	ConvertScores();

	return;
}

void CActorMiniGameTimeAttack::Init()
{
	CActorMiniGame::Init();

	this->field_0x1d0 = 0.0f;
	this->field_0x1e4 = 0;

	return;
}

void CActorMiniGameTimeAttack::Reset()
{
	CActorMiniGame::Reset();

	this->field_0x1d0 = 0.0f;
	this->field_0x1e4 = 0;

	return;
}

void CActorMiniGameTimeAttack::CheckpointReset()
{
	CActorMiniGame::CheckpointReset();

	this->field_0x1d0 = 0.0f;
	this->field_0x1e4 = 0;

	return;
}

CBehaviour* CActorMiniGameTimeAttack::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 4) {
		pBehaviour = &this->field_0x210;
	}
	else {
		if (behaviourType == 3) {
			pBehaviour = &this->field_0x1f8;
		}
		else {
			if (behaviourType == 2) {
				pBehaviour = &this->field_0x1e8;
			}
			else {
				pBehaviour = CActor::BuildBehaviour(behaviourType);
			}
		}
	}
	return pBehaviour;
}

StateConfig* CActorMiniGameTimeAttack::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 8) {
		pStateConfig = CActorMiniGame::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_MTA + state + -8;
	}
	return pStateConfig;
}

int CActorMiniGameTimeAttack::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;

	if (msg == 0x59) {
		this->field_0x1e4 = this->field_0x1e4 + 1;
		iVar1 = 0;
	}
	else {
		iVar1 = CActorMiniGame::InterpretMessage(pSender, msg, pMsgParam);
	}
	return iVar1;
}

CBehaviourMiniGameBetting* CActorMiniGameTimeAttack::GetBhvBetting()
{
	return &this->field_0x1f8;
}

CBehaviourMiniGameTraining* CActorMiniGameTimeAttack::GetBhvTraining()
{
	return &this->field_0x1e8;
}

CBehaviourMiniGameMulti* CActorMiniGameTimeAttack::GetBhvMulti()
{
	return &this->field_0x210;
}

int CActorMiniGameTimeAttack::GetUnity()
{
	return 1;
}

ulong CActorMiniGameTimeAttack::GetScoreLabelMessageHash()
{
	return 0x1e160d01150e030a;
}

ulong CActorMiniGameTimeAttack::GetHighScoreLabelMessageHash()
{
	return 0x564443425b151206;
}

bool CActorMiniGameTimeAttack::MustStop()
{
	bool bVar1;

	if (this->field_0x1e0 == -1) {
		bVar1 = this->bMustStop;
	}
	else {
		bVar1 = this->field_0x1e4 == this->field_0x1e0;
	}
	return bVar1;
}

float CActorMiniGameTimeAttack::GetExtraHudHeight()
{
	float fVar1;

	fVar1 = 0.0f;
	if (this->field_0x1e0 != -1) {
		fVar1 = (float)gVideoConfig.screenHeight * 0.06f + 0.0f;
	}
	return fVar1;
}

void CActorMiniGameTimeAttack::DrawExtraHud(float param_1, float param_2)
{
	char* pcVar1;
	float fVar2;
	float fVar3;
	edCTextFormat auStack5392;

	if (this->field_0x1e0 != -1) {
		fVar2 = (float)gVideoConfig.screenWidth;
		fVar3 = (float)gVideoConfig.screenHeight * 0.06f + 0.0f;
		pcVar1 = gMessageManager.get_message(0x4c45000b18150210);
		auStack5392.FormatString("%s%s%d", pcVar1, "  ", this->field_0x1e4);
		auStack5392.Display(param_1 + fVar2 * 0.09f + 0.0f, param_2 + fVar3);
	}

	return;
}

void CActorMiniGameTimeAttack::ConvertScores()
{
	float* pScore;
	int iVar3;

	iVar3 = 0;
	if (0 < this->field_0x174) {
		do {
			pScore = &this->field_0x178[iVar3];
			iVar3 = iVar3 + 1;
			*pScore = *pScore * 100.0f;
		} while (iVar3 < this->field_0x174);
	}

	iVar3 = 0;
	if (0 < this->nbScores) {
		do {
			pScore = &this->aScores[iVar3].score;
			iVar3 = iVar3 + 1;
			*pScore = *pScore * 100.0f;
		} while (iVar3 < this->nbScores);
	}

	this->defaultScore.score = this->defaultScore.score * 100.0f;

	return;
}

void CBehaviourMiniGameTimeAttackTraining::Manage()
{
	int curState;
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	curState = pMiniGame->actorState;
	if (curState == 6) {
		pMiniGame->FUN_003a9f70();
	}
	else {
		if (curState == 8) {
			pMiniGame->field_0x1d0 = pMiniGame->field_0x1d0 + GetTimer()->cutsceneDeltaTime * 100.0f;
			pMiniGame->FUN_003ac170(0xffffffff);
			if (pMiniGame->MustStop() != 0) {
				pMiniGame->FUN_003ac030(pMiniGame->field_0x1d0, 7, pMiniGame->field_0x1bc);
			}
		}
		else {
			if (curState == 7) {
				pMiniGame->FUN_003a9d80();
			}
			else {
				if (curState == 5) {
					pMiniGame->StateMiniGameStand(this, 8);
				}
			}
		}
	}

	return;
}

void CBehaviourMiniGameTimeAttackTraining::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorMiniGame* pMiniGame;

	CBehaviourMiniGameTraining::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(5, -1);
	}
	else {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMiniGameTimeAttackTraining::InitState(int newState)
{
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	if (newState == 7) {
		pMiniGame->FUN_003a9f30();
	}
	else {
		if (newState == 6) {
			pMiniGame->FUN_003a9f80();
		}
		else {
			if (newState == 8) {
				pMiniGame->field_0x1d0 = 0.0f;
				pMiniGame->flags = pMiniGame->flags | 2;
				pMiniGame->flags = pMiniGame->flags & 0xfffffffe;
			}
			else {
				if (newState == 5) {
					pMiniGame->StateMiniGameStandInit();
				}
			}
		}
	}

	return;
}

void CBehaviourMiniGameTimeAttackTraining::TermState(int oldState, int newState)
{
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	if (oldState == 7) {
		pMiniGame->FUN_003a9d60();
	}
	else {
		if (oldState == 6) {
			pMiniGame->FUN_003a9f50();
		}
		else {
			if (oldState == 8) {
				pMiniGame->flags = pMiniGame->flags & 0xfffffffc;
			}
		}
	}

	return;
}

int CBehaviourMiniGameTimeAttackTraining::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameTraining::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameTimeAttackBetting::Manage()
{
	int curState;
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	curState = pMiniGame->actorState;
	if (curState == 6) {
		pMiniGame->FUN_003a9f70();
	}
	else {
		if (curState == 8) {
			pMiniGame->field_0x1d0 = pMiniGame->field_0x1d0 + GetTimer()->cutsceneDeltaTime * 100.0f;
			pMiniGame->FUN_003ac170(0xffffffff);
			if (pMiniGame->MustStop() != 0) {
				pMiniGame->FUN_003ac030(pMiniGame->field_0x1d0, 7, pMiniGame->field_0x1bc);
			}
		}
		else {
			if (curState == 7) {
				pMiniGame->FUN_003a9d80();
			}
			else {
				if (curState == 5) {
					pMiniGame->StateMiniGameStand(this, 8);
				}
			}
		}
	}

	return;
}

void CBehaviourMiniGameTimeAttackBetting::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CActorMiniGame* pMiniGame;

	CBehaviourMiniGameBetting::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(5, -1);
	}
	else {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMiniGameTimeAttackBetting::InitState(int newState)
{
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	if (newState == 7) {
		pMiniGame->FUN_003a9f30();
	}
	else {
		if (newState == 6) {
			pMiniGame->FUN_003a9f80();
		}
		else {
			if (newState == 8) {
				pMiniGame->field_0x1d0 = 0.0f;
				pMiniGame->flags = pMiniGame->flags | 2;
				pMiniGame->flags = pMiniGame->flags & 0xfffffffe;
			}
			else {
				if (newState == 5) {
					pMiniGame->StateMiniGameStandInit();
				}
			}
		}
	}

	return;
}

void CBehaviourMiniGameTimeAttackBetting::TermState(int oldState, int newState)
{
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	if (oldState == 7) {
		pMiniGame->FUN_003a9d60();
	}
	else {
		if (oldState == 6) {
			pMiniGame->FUN_003a9f50();
		}
		else {
			if (oldState == 8) {
				pMiniGame->flags = pMiniGame->flags & 0xfffffffc;
			}
		}
	}

	return;
}

int CBehaviourMiniGameTimeAttackBetting::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameBetting::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameTimeAttackMulti::Manage()
{
	int curState;
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	curState = pMiniGame->actorState;
	if (curState == 6) {
		pMiniGame->FUN_003a9f70();
	}
	else {
		if (curState == 8) {
			pMiniGame->field_0x1d0 = pMiniGame->field_0x1d0 + GetTimer()->cutsceneDeltaTime * 100.0f;
			pMiniGame->FUN_003ac170(0xffffffff);
			if (pMiniGame->MustStop() != 0) {
				pMiniGame->FUN_003ac030(pMiniGame->field_0x1d0, 7, pMiniGame->field_0x1bc);
			}
		}
		else {
			if (curState == 7) {
				pMiniGame->FUN_003a9d80();
			}
			else {
				if (curState == 5) {
					pMiniGame->StateMiniGameStand(this, 8);
				}
			}
		}
	}

	return;
}

void CBehaviourMiniGameTimeAttackMulti::Begin(CActor * pOwner, int newState, int newAnimationType)
{
	CActorMiniGame* pMiniGame;

	CBehaviourMiniGameMulti::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(5, -1);
	}
	else {
		pMiniGame = this->pOwner;
		pMiniGame->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMiniGameTimeAttackMulti::InitState(int newState)
{
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	if (newState == 7) {
		pMiniGame->FUN_003a9f30();
	}
	else {
		if (newState == 6) {
			pMiniGame->FUN_003a9f80();
		}
		else {
			if (newState == 8) {
				pMiniGame->field_0x1d0 = 0.0f;
				pMiniGame->flags = pMiniGame->flags | 2;
				pMiniGame->flags = pMiniGame->flags & 0xfffffffe;
			}
			else {
				if (newState == 5) {
					pMiniGame->StateMiniGameStandInit();
				}
			}
		}
	}

	return;
}

void CBehaviourMiniGameTimeAttackMulti::TermState(int oldState, int newState)
{
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	if (oldState == 7) {
		pMiniGame->FUN_003a9d60();
	}
	else {
		if (oldState == 6) {
			pMiniGame->FUN_003a9f50();
		}
		else {
			if (oldState == 8) {
				pMiniGame->flags = pMiniGame->flags & 0xfffffffc;
			}
		}
	}

	return;
}

int CBehaviourMiniGameTimeAttackMulti::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameMulti::InterpretMessage(pSender, msg, pMsgParam);
}
