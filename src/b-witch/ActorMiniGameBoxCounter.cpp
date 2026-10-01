#include "ActorMiniGameBoxCounter.h"
#include "MemoryStream.h"
#include "Rendering/edCTextFormat.h"
#include "TranslatedTextData.h"
#include "kya.h"
#include "TimeController.h"

StateConfig CActorMiniGameBoxCounter::_gStateCfg_MBC[1] =
{
	StateConfig(0, 0)
};

void CActorMiniGameBoxCounter::Create(ByteCode* pByteCode)
{
	CActorMiniGame::Create(pByteCode);
	this->timeLimit = pByteCode->GetF32();

	return;
}

void CActorMiniGameBoxCounter::Init()
{
	CActorMiniGame::Init();

	this->field_0x1d0 = 0.0f;
	this->timeRemaining = this->timeLimit;

	return;
}

void CActorMiniGameBoxCounter::Reset()
{
	CActorMiniGame::Reset();

	this->field_0x1d0 = 0.0f;
	this->timeRemaining = this->timeLimit;

	return;
}

void CActorMiniGameBoxCounter::CheckpointReset()
{
	CActorMiniGame::CheckpointReset();

	this->field_0x1d0 = 0.0f;
	this->timeRemaining = this->timeLimit;

	return;
}

CBehaviour* CActorMiniGameBoxCounter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == 4) {
		pBehaviour = &this->multiBehaviour;
	}
	else {
		if (behaviourType == 3) {
			pBehaviour = &this->bettingBehaviour;
		}
		else {
			if (behaviourType == 2) {
				pBehaviour = &this->trainingBehaviour;
			}
			else {
				pBehaviour = CActor::BuildBehaviour(behaviourType);
			}
		}
	}

	return pBehaviour;
}

StateConfig* CActorMiniGameBoxCounter::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 8) {
		pStateConfig = CActorMiniGame::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_MBC + state + -8;
	}

	return pStateConfig;
}

int CActorMiniGameBoxCounter::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x59) {
		this->field_0x1d0 = this->field_0x1d0 + 1.0f;
		return 1;
	}

	if (msg == 0x5a) {
		this->field_0x1d0 = this->field_0x1d0 - 1.0f;
		if (this->field_0x1d0 < 0.0f) {
			this->field_0x1d0 = 0.0f;
		}

		return 1;
	}

	return CActorMiniGame::InterpretMessage(pSender, msg, pMsgParam);
}

CBehaviourMiniGameBetting* CActorMiniGameBoxCounter::GetBhvBetting()
{
	return &this->bettingBehaviour;
}

CBehaviourMiniGameTraining* CActorMiniGameBoxCounter::GetBhvTraining()
{
	return &this->trainingBehaviour;
}

CBehaviourMiniGameMulti* CActorMiniGameBoxCounter::GetBhvMulti()
{
	return &this->multiBehaviour;
}

int CActorMiniGameBoxCounter::GetUnity()
{
	return 3;
}

ulong CActorMiniGameBoxCounter::GetScoreLabelMessageHash()
{
	return 0x1e161d0a08130845;
}

ulong CActorMiniGameBoxCounter::GetHighScoreLabelMessageHash()
{
	return 0x5d595e4914151216;
}

void CActorMiniGameBoxCounter::SetYouAreChosen(byte bChosen)
{
	this->field_0x1b8 = bChosen;
	this->field_0x19c.Switch(this);
}

bool CActorMiniGameBoxCounter::MustStop()
{
	if (this->timeLimit != -1.0f) {
		this->timeRemaining = this->timeRemaining - GetTimer()->cutsceneDeltaTime;
		if (this->timeRemaining < GetTimer()->cutsceneDeltaTime) {
			return true;
		}
	}

	return this->bMustStop;
}

float CActorMiniGameBoxCounter::GetExtraHudHeight()
{
	float fVar1;

	fVar1 = 0.0f;
	if (this->timeLimit != -1.0f) {
		fVar1 = (float)gVideoConfig.screenHeight * 0.06f + 0.0f;
	}

	return fVar1;
}

void CActorMiniGameBoxCounter::DrawExtraHud(float param_1, float param_2)
{
	char* pcVar1;
	float fVar2;
	float fVar3;
	edCTextFormat auStack5392;

	if (this->timeLimit != -1.0f) {
		fVar2 = (float)gVideoConfig.screenWidth;
		fVar3 = (float)gVideoConfig.screenHeight * 0.06f + 0.0f;
		pcVar1 = gMessageManager.get_message(0x5b5b5b4944041217);
		FormatScore(this->timeRemaining * 100.0f, &auStack5392, pcVar1, 1, 1);
		auStack5392.Display(param_1 + fVar2 * 0.09f + 0.0f, param_2 + fVar3);
	}

	return;
}

void CBehaviourMiniGameBoxCounterTraining::Manage()
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

void CBehaviourMiniGameBoxCounterTraining::Begin(CActor* pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameBoxCounterTraining::InitState(int newState)
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
				CActorMiniGameBoxCounter* pBoxCounter = static_cast<CActorMiniGameBoxCounter*>(pMiniGame);
				pBoxCounter->timeRemaining = pBoxCounter->timeLimit;
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

void CBehaviourMiniGameBoxCounterTraining::TermState(int oldState, int newState)
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

int CBehaviourMiniGameBoxCounterTraining::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameTraining::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameBoxCounterBetting::Manage()
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

void CBehaviourMiniGameBoxCounterBetting::Begin(CActor * pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameBoxCounterBetting::InitState(int newState)
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
				CActorMiniGameBoxCounter* pBoxCounter = static_cast<CActorMiniGameBoxCounter*>(pMiniGame);
				pBoxCounter->timeRemaining = pBoxCounter->timeLimit;
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

void CBehaviourMiniGameBoxCounterBetting::TermState(int oldState, int newState)
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

int CBehaviourMiniGameBoxCounterBetting::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameBetting::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameBoxCounterMulti::Manage()
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

void CBehaviourMiniGameBoxCounterMulti::Begin(CActor * pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameBoxCounterMulti::InitState(int newState)
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
				CActorMiniGameBoxCounter* pBoxCounter = static_cast<CActorMiniGameBoxCounter*>(pMiniGame);
				pBoxCounter->timeRemaining = pBoxCounter->timeLimit;
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

void CBehaviourMiniGameBoxCounterMulti::TermState(int oldState, int newState)
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

int CBehaviourMiniGameBoxCounterMulti::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameMulti::InterpretMessage(pSender, msg, pMsgParam);
}
