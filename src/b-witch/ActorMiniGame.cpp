#include "ActorMiniGame.h"
#include "MemoryStream.h"
#include "ActorFactory.h"
#include "ActorMiniGamesOrganizer.h"
#include "ActorMiniGamesManager.h"
#include "ActorNativShop.h"
#include "LargeObject.h"
#include "Rendering/edCTextFormat.h"
#include "edStr.h"
#include "kya.h"

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

ulong CActorMiniGame::GetScoreLabelMessageHash()
{
	return 0xffffffffffffffff;
}

ulong CActorMiniGame::GetHighScoreLabelMessageHash()
{
	return 0xffffffffffffffff;
}

float CActorMiniGame::GetExtraHudHeight()
{
	return -1.0f;
}

void CActorMiniGame::DrawExtraHud(float param_1, float param_2)
{
	return;
}

void CActorMiniGame::DrawScoreUnitLabel(float param_1, float param_2)
{
	long lVar1;
	edCTextFormat textFormat;

	lVar1 = GetUnity();
	if (((lVar1 != 1) && (lVar1 != 3)) && (lVar1 == 2)) {
		if (gVideoConfig.omode == 2) {
			textFormat.FormatString("Y.");
		}
		else {
			textFormat.FormatString("m");
		}

		textFormat.Display(param_1, param_2);
	}

	return;
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
			if (this->field_0x1b4 < pBehaviour->scoreList.nbScores) {
				pSVar1 = pBehaviour->scoreList.aScores + this->field_0x1b4;
			}
		}
		else {
			if (iVar2 == 4) {
				CBehaviourMiniGameMulti* pBehaviour = GetBhvMulti();
				if (this->field_0x1b4 < pBehaviour->scoreList.nbScores) {
					pSVar1 = pBehaviour->scoreList.aScores + this->field_0x1b4;
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

void CActorMiniGame::DrawHighScoreArray(float param_1, float param_2, float param_3, float param_4, S_MINI_GAME_SCORE_LIST* pList)
{
	S_MINI_GAME_SCORE* pScore;
	int iVar3;

	iVar3 = 0;
	if (0 < pList->nbScores) {
		do {
			edCTextFormat auStack5392;
			edCTextFormat auStack10784;
			pScore = pList->aScores + iVar3;
			GetUnity();
			FormatScore(pScore->score, &auStack10784, pScore->name, 0, 1);
			auStack10784.Display(param_1, param_2);
			param_2 = param_2 + param_4 * auStack10784.field_0xc;
			iVar3 = iVar3 + 1;
		} while (iVar3 < pList->nbScores);
	}
	return;
}

void CActorMiniGame::FormatScore(float param_1, edCTextFormat* pFormat, char* param_4, int unity, int param_6)
{
	char* pcVar1;
	int iVar2;
	char* pcVar3;
	int iVar4;
	int iVar5;
	int hundredths;
	int seconds;
	char scoreBuffer[32];

	if (unity == 0) {
		unity = GetUnity();
	}
	if (unity == 3) {
		if (param_1 == -1.0f) {
			pFormat->FormatString("%s%s--", param_4, "     ");
		}
		else {
			pFormat->FormatString("%s%s%d ", param_4, "     ", (int)param_1);
		}
	}
	else {
		if (unity == 2) {
			if (gVideoConfig.omode == 2) {
				pcVar3 = "Y.";
			}
			else {
				pcVar3 = "m";
			}
			iVar2 = 2;
			if (999.99f <= param_1 && param_1 <= 9999.99f) {
				iVar2 = 1;
			}
			if (9999.99f <= param_1 && param_1 <= 99999.99f) {
				iVar2 = 0;
			}
			if (param_1 == -1.0f) {
				if (param_6 == 0) {
					pFormat->FormatString("%s%s--", param_4, "   ");
				}
				else {
					pFormat->FormatString("%s%s-- %s", param_4, "   ", pcVar3);
				}
			}
			else {
				if (param_6 == 0) {
					pcVar3 = edFloat2String(param_1, iVar2, scoreBuffer, 0);
					pFormat->FormatString("%s%s%s", param_4, "   ", pcVar3);
				}
				else {
					pcVar1 = edFloat2String(param_1, iVar2, scoreBuffer, 0);
					pFormat->FormatString("%s%s%s  %s", param_4, "   ", pcVar1, pcVar3);
				}
			}
		}
		else {
			if (unity == 1) {
				if (param_1 == -1.0f) {
					pFormat->FormatString("%s%s--'--\"--", param_4, "     ");
				}
				else {
					hundredths = (int)param_1 - (int)((float)(int)param_1 / 100.0f) * 100;
					iVar5 = (int)((float)((int)param_1 - hundredths) / 100.0f);
					iVar4 = (int)((float)iVar5 / 60.0f);
					iVar2 = iVar4 + (int)((float)iVar4 / 10.0f) * -10;
					seconds = iVar5 + iVar4 * -0x3c;
					pFormat->FormatString("%s%s%d%d'%d%d\"%d%d ", param_4, "     ",
						(int)((float)(iVar4 - iVar2) / 10.0f), iVar2,
						(int)((float)((int)((float)seconds / 10.0f) * 10) / 10.0f),
						seconds - (int)((float)seconds / 10.0f) * 10,
						(int)((float)hundredths / 10.0f),
						hundredths - (int)((float)hundredths / 10.0f) * 10);
				}
			}
		}
	}
	return;
}

void CActorMiniGame::FormatScore(float param_1, edCTextFormat* pFormat, int param_4)
{
	char* pcVar1;
	int iVar2;
	char* pcVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	int hundredths;
	int seconds;
	char acStack128[128];
	char scoreBuffer[32];
	CActorMiniGamesOrganizer* pMiniGameOrganizer;
	char* pLetter0;
	char* pLetter1;
	char* pLetter2;

	if (param_4 == 0) {
		param_4 = GetUnity();
	}
	pMiniGameOrganizer = this->field_0x1c0;
	pcVar1 = pMiniGameOrganizer->FUN_003b2a00();
	edStrCopy(acStack128, pcVar1);
	if (param_4 == 3) {
		if (param_1 == -1.0f) {
			strcat(acStack128, " --");
			pLetter0 = pMiniGameOrganizer->GetCurNameLetter(0);
			pLetter1 = pMiniGameOrganizer->GetCurNameLetter(1);
			pLetter2 = pMiniGameOrganizer->GetCurNameLetter(2);
			pFormat->FormatString(acStack128, pLetter0, pLetter1, pLetter2, 0x17);
		}
		else {
			strcat(acStack128, " %d");
			pLetter0 = pMiniGameOrganizer->GetCurNameLetter(0);
			pLetter1 = pMiniGameOrganizer->GetCurNameLetter(1);
			pLetter2 = pMiniGameOrganizer->GetCurNameLetter(2);
			pFormat->FormatString(acStack128, pLetter0, pLetter1, pLetter2, (int)param_1);
		}
	}
	else {
		if (param_4 == 2) {
			if (gVideoConfig.omode == 2) {
				pcVar1 = "Y.";
			}
			else {
				pcVar1 = "m";
			}
			iVar2 = 2;
			if (999.99f <= param_1 && param_1 <= 9999.99f) {
				iVar2 = 1;
			}
			if (9999.99f <= param_1 && param_1 <= 99999.99f) {
				iVar2 = 0;
			}
			if (param_1 == -1.0f) {
				strcat(acStack128, " --  %s");
				pLetter0 = pMiniGameOrganizer->GetCurNameLetter(0);
				pLetter1 = pMiniGameOrganizer->GetCurNameLetter(1);
				pLetter2 = pMiniGameOrganizer->GetCurNameLetter(2);
				pFormat->FormatString(acStack128, pLetter0, pLetter1, pLetter2, pcVar1);
			}
			else {
				strcat(acStack128, " %s  %s");
				pLetter0 = pMiniGameOrganizer->GetCurNameLetter(0);
				pLetter1 = pMiniGameOrganizer->GetCurNameLetter(1);
				pLetter2 = pMiniGameOrganizer->GetCurNameLetter(2);
				pcVar5 = edFloat2String(param_1, iVar2, scoreBuffer, 0);
				pFormat->FormatString(acStack128, pLetter0, pLetter1, pLetter2, pcVar5, pcVar1);
			}
		}
		else {
			if (param_4 == 1) {
				if (param_1 == -1.0f) {
					strcat(acStack128, " --'--\"--\n");
					pLetter0 = pMiniGameOrganizer->GetCurNameLetter(0);
					pLetter1 = pMiniGameOrganizer->GetCurNameLetter(1);
					pLetter2 = pMiniGameOrganizer->GetCurNameLetter(2);
					pFormat->FormatString(acStack128, pLetter0, pLetter1, pLetter2, 0x17);
				}
				else {
					hundredths = (int)param_1 - (int)((float)(int)param_1 / 100.0f) * 100;
					iVar8 = (int)((float)((int)param_1 - hundredths) / 100.0f);
					iVar7 = (int)((float)iVar8 / 60.0f);
					iVar6 = iVar7 + (int)((float)iVar7 / 10.0f) * -10;
					seconds = iVar8 + iVar7 * -0x3c;
					strcat(acStack128, "%d%d'%d%d\"%d%d");
					pLetter0 = pMiniGameOrganizer->GetCurNameLetter(0);
					pLetter1 = pMiniGameOrganizer->GetCurNameLetter(1);
					pLetter2 = pMiniGameOrganizer->GetCurNameLetter(2);
					pFormat->FormatString(acStack128, pLetter0, pLetter1, pLetter2,
						(int)((float)(iVar7 - iVar6) / 10.0f), iVar6,
						(int)((float)((int)((float)seconds / 10.0f) * 10) / 10.0f),
						seconds - (int)((float)seconds / 10.0f) * 10,
						(int)((float)hundredths / 10.0f),
						hundredths - (int)((float)hundredths / 10.0f) * 10);
				}
			}
		}
	}
	return;
}


char* CActorMiniGame::FUN_003ace10()
{
	char* pcVar1;

	pcVar1 = gMessageManager.get_message(this->field_0x160);

	return pcVar1;
}

void CActorMiniGame::FUN_003a9f70()
{
	return;
}

void CActorMiniGame::FUN_003a9d80()
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::StateMiniGameStandInit()
{
	this->field_0x1b9 = 0;

	return;
}

void CActorMiniGame::StateMiniGameStand(CBehaviourMiniGame* pBehaviour, int param_3)
{
	long lVar1;

	if (this->field_0x1b8 != 0) {
		if (this->field_0x1b9 != 0) {
			DoMessage(this->field_0x1c0, (ACTOR_MESSAGE)0x55, 0);
			SetState(param_3, -1);
		}

		lVar1 = MustStop();
		if (lVar1 != 0) {
			FUN_003ac030(0.0f, 6, 0);
		}
	}

	return;
}


void CActorMiniGame::FUN_003a9f30()
{
	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;

	return;
}

void CActorMiniGame::FUN_003a9f80()
{
	int iVar1;
	int iVar2;
	CBehaviourMiniGameMulti* pMulti;

	this->flags = this->flags | 2;
	this->flags = this->flags & 0xfffffffe;

	if (this->curBehaviourId == 4) {
		pMulti = GetBhvMulti();
		iVar1 = pMulti->winner;
		iVar2 = pMulti->nbPlayers;
		if (iVar2 == 0) {
			trap(7);
		}

		pMulti = GetBhvMulti();
		pMulti->winner = (iVar1 + 1) % iVar2;
	}

	return;
}

void CActorMiniGame::FUN_003ac030(float param_1, int param_3, int param_4)
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::FUN_003ac170(uint param_2)
{
	IMPLEMENTATION_GUARD();
}

void CActorMiniGame::FUN_003a9d60()
{
	this->flags = this->flags & 0xfffffffc;

	return;
}

void CActorMiniGame::FUN_003a9f50()
{
	this->flags = this->flags & 0xfffffffc;

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
	S_MINI_GAME_SCORE* pScore;
	char* pName;
	int iVar5;
	int iVar12;
	float fVar13;
	float fVar14;
	S_STREAM_REF<CWayPoint> local_4;

	CActor::Create(pByteCode);
	this->field_0x160 = pByteCode->GetU64();
	this->field_0x168 = pByteCode->GetU32();
	this->field_0x16c = pByteCode->GetU32();
	this->field_0x170 = S_ACTOR_STREAM_REF::Create(pByteCode);
	this->field_0x174 = pByteCode->GetS32();
	if (this->field_0x174 != 0) {
		this->field_0x178 = new float[this->field_0x174];
		iVar5 = 0;
		if (0 < this->field_0x174) {
			do {
				this->field_0x178[iVar5] = pByteCode->GetF32();
				iVar5 = iVar5 + 1;
			} while (iVar5 < this->field_0x174);
		}
	}
	this->nbScores = pByteCode->GetS32();
	if (this->nbScores != 0) {
		this->aScores = new S_MINI_GAME_SCORE[this->nbScores];
		iVar5 = 0;
		if (0 < this->nbScores) {
			do {
				pScore = this->aScores + iVar5;
				pScore->score = pByteCode->GetF32();
				pName = pByteCode->GetString();
				memcpy(pScore->name, pName, 4);
				iVar5 = iVar5 + 1;
			} while (iVar5 < this->nbScores);
		}
	}
	if (2.21f <= CScene::_pinstance->field_0x1c) {
		this->defaultScore.score = pByteCode->GetF32();
		pName = pByteCode->GetString();
		memcpy(this->defaultScore.name, pName, 4);
	}
	else {
		this->defaultScore.score = 0.0f;
		memcpy(this->defaultScore.name, "NAT", 4);
		this->defaultScore.name[3] = 0;
	}
	local_4.index = pByteCode->GetS32();
	local_4.Init();
	this->wayPointRef = local_4;
	this->field_0x190 = pByteCode->GetS32();
	this->field_0x1a8 = pByteCode->GetU64();
	this->field_0x19c.Create(pByteCode);
	iVar5 = this->nbScores + -1;
	iVar12 = 0;
	if (0 < iVar5) {
		do {
			fVar14 = this->aScores[iVar12].score;
			fVar13 = this->aScores[iVar12 + 1].score;
			if (fVar14 < fVar13) {
				this->field_0x1c4 = 1;
				return;
			}
			if (fVar13 < fVar14) {
				this->field_0x1c4 = 0;
				return;
			}
			iVar12 = iVar12 + 1;
		} while (iVar12 < iVar5);
	}
	return;
}

void CActorMiniGame::Init()
{
	CActor::Init();
	this->field_0x1bc = 1;
	this->field_0x1b9 = 0;
	this->bMustStop = false;
	this->field_0x1b4 = -1;
	this->field_0x1b0 = -1;
	this->field_0x170->Init();
	this->field_0x19c.Init();
	FUN_003ad480();
	this->field_0x1b8 = 0;
	this->field_0x1d4 = 0;
	this->field_0x1d8 = -1;
	this->field_0x1c0 = (CActorMiniGamesOrganizer*)0x0;
	return;
}

void CActorMiniGame::Reset()
{
	int iVar1;

	CActor::ResetActorSound();
	iVar1 = this->prevBehaviourId;
	if (iVar1 == -1) {
		SetBehaviour(this->subObjA->defaultBehaviourId, -1, -1);
	}
	else {
		SetBehaviour(iVar1, -1, -1);
	}
	this->field_0x1bc = 1;
	this->field_0x1b9 = 0;
	this->bMustStop = false;
	this->field_0x1b4 = -1;
	this->field_0x1b0 = -1;
	return;
}

void CActorMiniGame::CheckpointReset()
{
	this->field_0x1bc = 1;
	this->field_0x1b9 = 0;
	this->bMustStop = false;
	this->field_0x1b4 = -1;
	this->field_0x1b0 = -1;
	if (this->field_0x1b8 != 0) {
		this->field_0x19c.Switch(this);
	}
	SetState(5, -1);
	return;
}

void CActorMiniGame::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_MINI_GAME* pSaveData = static_cast<S_SAVE_CLASS_MINI_GAME*>(pData);
	CBehaviourMiniGameMulti* pMulti;
	CBehaviourMiniGameTraining* pTraining;
	int iVar5;

	pSaveData->field_0x0 = this->field_0x1d8;
	if (mode == 1) {
		pSaveData->defaultScore.score = this->defaultScore.score;
		memcpy(pSaveData->defaultScore.name, this->defaultScore.name, 4);
		pSaveData->defaultScore.name[3] = 0;
	}
	if (GetBehaviour(4) != (CBehaviour*)0x0) {
		pMulti = static_cast<CBehaviourMiniGameMulti*>(GetBehaviour(4));
		iVar5 = 0;
		if (0 < pMulti->scoreList.nbScores) {
			do {
				pSaveData->multiScores[iVar5].score = pMulti->scoreList.aScores[iVar5].score;
				memcpy(pSaveData->multiScores[iVar5].name, pMulti->scoreList.aScores[iVar5].name, 4);
				pSaveData->multiScores[iVar5].name[3] = 0;
				iVar5 = iVar5 + 1;
			} while (iVar5 < pMulti->scoreList.nbScores);
		}
	}
	if (GetBehaviour(2) != (CBehaviour*)0x0) {
		pTraining = static_cast<CBehaviourMiniGameTraining*>(GetBehaviour(2));
		pTraining->SaveContext(pData, mode, maxSize);
	}
	return;
}

void CActorMiniGame::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_MINI_GAME* pSaveData = static_cast<S_SAVE_CLASS_MINI_GAME*>(pData);
	CBehaviourMiniGameMulti* pMulti;
	CBehaviourMiniGameTraining* pTraining;
	int iVar6;

	if ((mode == 0) || (mode == 1)) {
		this->defaultScore.score = pSaveData->defaultScore.score;
		memcpy(this->defaultScore.name, pSaveData->defaultScore.name, 4);
		this->defaultScore.name[3] = 0;
		if (GetBehaviour(4) != (CBehaviour*)0x0) {
			pMulti = static_cast<CBehaviourMiniGameMulti*>(GetBehaviour(4));
			if (mode == 1) {
				iVar6 = 0;
				if (0 < pMulti->scoreList.nbScores) {
					do {
						pMulti->scoreList.aScores[iVar6].score = pSaveData->multiScores[iVar6].score;
						memcpy(pMulti->scoreList.aScores[iVar6].name, pSaveData->multiScores[iVar6].name, 4);
						pMulti->scoreList.aScores[iVar6].name[3] = 0;
						iVar6 = iVar6 + 1;
					} while (iVar6 < pMulti->scoreList.nbScores);
				}
			}
		}
		if (GetBehaviour(2) != (CBehaviour*)0x0) {
			pTraining = static_cast<CBehaviourMiniGameTraining*>(GetBehaviour(2));
			pTraining->LoadContext(pData, mode, maxSize);
		}
		this->field_0x1d8 = pSaveData->field_0x0;
		if (this->field_0x1d8 == -1) {
			this->field_0x1d4 = 0;
		}
		else {
			this->field_0x1d4 = FUN_003ab830();
		}
	}
	return;
}

CBehaviour* CActorMiniGame::BuildBehaviour(int behaviourType)
{
	return CActor::BuildBehaviour(behaviourType);
}

StateConfig CActorMiniGame::_gStateCfg_MIG[4] =
{
	StateConfig(0, 0),
	StateConfig(0, 0),
	StateConfig(0, 0),
	StateConfig(0, 0)
};

StateConfig* CActorMiniGame::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 4);
		pStateConfig = _gStateCfg_MIG + state + -5;
	}
	return pStateConfig;
}

int CActorMiniGame::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	S_ACTOR_STREAM_REF* pRef;
	int iVar4;

	if (msg == 0x58) {
		if (this->field_0x1b8 != 0) {
			if (pMsgParam == (void*)2) {
				this->field_0x1bc = 0;
			}
			else {
				if (pMsgParam == (void*)1) {
					this->field_0x1bc = this->field_0x168;
				}
				else {
					if (pMsgParam == (void*)0x0) {
						this->field_0x1bc = 1;
					}
				}
			}
			this->bMustStop = true;
			this->field_0x1b9 = 0;
		}
	}
	else {
		if (msg != 0x57) {
			if (msg != 0x56) {
				return CActor::InterpretMessage(pSender, msg, static_cast<_msg_params_get_position*>(pMsgParam));
			}
			this->field_0x1c0 = static_cast<CActorMiniGamesOrganizer*>(pSender);
			if (pMsgParam == (void*)2) {
				DoMessage(this->field_0x1c0->field_0x9f0, (ACTOR_MESSAGE)0x57, (void*)(uintptr_t)this->field_0x16c);
				GameFlags = GameFlags | 0x800;
				GetBhvMulti()->winner = 0;
				SetBehaviour(4, -1, -1);
				SetYouAreChosen(1);
			}
			else {
				if (pMsgParam == (void*)3) {
					DoMessage(this->field_0x1c0->field_0x9f0, (ACTOR_MESSAGE)0x57, (void*)(uintptr_t)this->field_0x16c);
					GameFlags = GameFlags | 0x800;
					SetBehaviour(2, -1, -1);
					SetYouAreChosen(1);
				}
				else {
					if (pMsgParam == (void*)1) {
						DoMessage(this->field_0x1c0->field_0x9f0, (ACTOR_MESSAGE)0x57, (void*)(uintptr_t)this->field_0x16c);
						GameFlags = GameFlags | 0x800;
						SetBehaviour(3, -1, -1);
						SetYouAreChosen(1);
					}
					else {
						if (pMsgParam == (void*)0x0) {
							SetYouAreChosen(0);
							GameFlags = GameFlags & 0xfffff7ff;
							iVar4 = 0;
							while (true) {
								pRef = this->field_0x170;
								if ((pRef == (S_ACTOR_STREAM_REF*)0x0) || (pRef->entryCount <= iVar4)) break;
								DoMessage(pRef->aEntries[iVar4].Get(), (ACTOR_MESSAGE)0x3c, 0);
								iVar4 = iVar4 + 1;
							}
						}
					}
				}
			}
			return 1;
		}
		if ((this->field_0x1b8 != 0) && (this->field_0x1b9 == 0)) {
			this->field_0x1b9 = 1;
			this->bMustStop = false;
		}
	}
	return 0;
}

void CActorMiniGame::FUN_003ad480()
{
	S_ACTOR_STREAM_REF* pRef;
	CActor* pActor;
	CActor* pOther;
	int iVar4;
	int iVar5;

	iVar4 = 0;
	while (true) {
		pRef = this->field_0x170;
		if ((pRef == (S_ACTOR_STREAM_REF*)0x0) || (pRef->entryCount + -1 <= iVar4)) break;
		iVar5 = iVar4 + 1;
		while (true) {
			pRef = this->field_0x170;
			if ((pRef == (S_ACTOR_STREAM_REF*)0x0) || (pRef->entryCount <= iVar5)) break;
			pOther = pRef->aEntries[iVar5].Get();
			pActor = pRef->aEntries[iVar4].Get();
			if (CActorFactory::gClassProperties[pOther->typeID].classPriority <
				CActorFactory::gClassProperties[pActor->typeID].classPriority) {
				pRef->aEntries[iVar5].pObj = STORE_POINTER(pActor);
				this->field_0x170->aEntries[iVar4].pObj = STORE_POINTER(pOther);
			}
			iVar5 = iVar5 + 1;
		}
		iVar4 = iVar4 + 1;
	}
	return;
}

// The name returned by FUN_00395080 is at PS2 offset 0x8, after the
// behaviour vtable and owner pointer. Keep those native pointer sizes here.
struct S_MINI_GAME_NAME_BEHAVIOUR : public CBehaviour
{
	CActor* pOwner;
	char name[4];
};

static char* FUN_00395080(CActorNativShop* pShop)
{
	S_ACTOR_STREAM_REF* pRef;
	CBehaviour* pBehaviour;
	int iVar8;
	int iVar6;
	int iVar2;
	ulong uVar5;

	iVar8 = 0;
	iVar6 = 0;
	while (true) {
		pRef = pShop->pActorStream;
		if ((pRef == (S_ACTOR_STREAM_REF*)0x0) || (pRef->entryCount <= iVar8)) break;
		pBehaviour = pRef->aEntries[iVar8].Get()->GetBehaviour(9);
		if (pBehaviour != (CBehaviour*)0x0) {
			iVar6 = iVar6 + 1;
		}
		iVar8 = iVar8 + 1;
	}
	uVar5 = CScene::_pinstance->field_0x38 * 0x343fd + 0x269ec3;
	CScene::_pinstance->field_0x38 = uVar5;
	iVar2 = (iVar6 + 1) * ((uint)(uVar5 >> 0x10) & 0x7fff);
	if (iVar2 < 0) {
		iVar2 = iVar2 + 0x7fff;
	}
	iVar8 = 0;
	iVar6 = 0;
	while (true) {
		pRef = pShop->pActorStream;
		if ((pRef == (S_ACTOR_STREAM_REF*)0x0) || (pRef->entryCount <= iVar6)) break;
		pBehaviour = pRef->aEntries[iVar6].Get()->GetBehaviour(9);
		if (pBehaviour != (CBehaviour*)0x0) {
			if ((iVar2 >> 0xf) == iVar8) {
				return static_cast<S_MINI_GAME_NAME_BEHAVIOUR*>(pBehaviour)->name;
			}
			iVar8 = iVar8 + 1;
		}
		iVar6 = iVar6 + 1;
	}
	return (char*)0x0;
}

bool CActorMiniGame::FUN_003ab830()
{
	int iVar4;
	float fVar5;
	float fVar6;
	char* pName;
	CActorNativShop* pShop;

	if (0 < this->nbScores) {
		fVar5 = this->defaultScore.score;
		iVar4 = this->field_0x174 + -1;
		while (-1 < iVar4) {
			fVar6 = this->field_0x178[iVar4];
			if (this->field_0x1c4 == 0) {
				if (fVar5 <= fVar6) break;
			}
			else {
				if (fVar6 <= fVar5) break;
			}
			iVar4 = iVar4 + -1;
		}
		if (iVar4 != -1) {
			fVar6 = this->field_0x178[iVar4];
			if (this->field_0x1c4 == 0) {
				if (fVar5 <= fVar6) {
					this->defaultScore.score = fVar6;
					return true;
				}
			}
			else {
				if (fVar6 <= fVar5) {
					this->defaultScore.score = fVar6;
					pName = (char*)0x0;
					pShop = static_cast<CActorNativShop*>(this->field_0x1c0->field_0x9f4->actorRef.Get());
					if (pShop != (CActorNativShop*)0x0) {
						pName = FUN_00395080(pShop);
					}
					if (pName != (char*)0x0) {
						memcpy(this->defaultScore.name, pName, 4);
						this->defaultScore.name[3] = 0;
						return true;
					}
				}
			}
		}
	}
	return false;
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

CBehaviourMiniGameBetting::CBehaviourMiniGameBetting()
{
	this->nbBets = 0;
	this->aBets = (S_MINI_GAME_BET*)0x0;
}

CBehaviourMiniGameBetting::~CBehaviourMiniGameBetting()
{
	if (this->aBets != (S_MINI_GAME_BET*)0x0) {
		delete[] this->aBets;
	}
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

CBehaviourMiniGameTraining::CBehaviourMiniGameTraining()
{
	this->scoreList.nbScores = 0;
}

CBehaviourMiniGameTraining::~CBehaviourMiniGameTraining()
{
	if (this->scoreList.nbScores != 0) {
		delete[] this->scoreList.aScores;
	}
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
	this->scoreList.nbScores = 5;
	if (this->scoreList.nbScores != 0) {
		this->scoreList.aScores = new S_MINI_GAME_SCORE[this->scoreList.nbScores];
	}

	iVar8 = 0;
	if (0 < this->scoreList.nbScores) {
		do {
			this->scoreList.aScores[iVar8].score = -1.0f;
			this->scoreList.aScores[iVar8].name[0] = CHighScoreArray::_STRING_Init[0];
			this->scoreList.aScores[iVar8].name[1] = CHighScoreArray::_STRING_Init[1];
			this->scoreList.aScores[iVar8].name[2] = CHighScoreArray::_STRING_Init[2];
			this->scoreList.aScores[iVar8].name[3] = CHighScoreArray::_STRING_Init[3];
			this->scoreList.aScores[iVar8].name[3] = 0;
			iVar8 = iVar8 + 1;
		} while (iVar8 < this->scoreList.nbScores);
	}

	iVar7 = 0;
	while (true) {
		if ((this->pOwner->nbScores <= iVar7) || (4 < iVar7)) break;
		pScore = this->pOwner->aScores + iVar7;
		if (iVar7 <= this->scoreList.nbScores) {
			local_4 = this->scoreList.aScores[iVar7].name[0];
			local_3 = this->scoreList.aScores[iVar7].name[1];
			local_2 = this->scoreList.aScores[iVar7].name[2];
			local_8 = this->scoreList.aScores[iVar7].score;
			this->scoreList.aScores[iVar7].score = pScore->score;
			if (pScore->name != (char*)0x0) {
				memcpy(this->scoreList.aScores[iVar7].name, pScore->name, 4);
				this->scoreList.aScores[iVar7].name[3] = 0;
			}

			iVar6 = iVar7 + 1;
			if (iVar6 < this->scoreList.nbScores) {
				do {
					cVar1 = this->scoreList.aScores[iVar6].name[0];
					cVar2 = this->scoreList.aScores[iVar6].name[1];
					cVar3 = this->scoreList.aScores[iVar6].name[2];
					fVar13 = this->scoreList.aScores[iVar6].score;
					this->scoreList.aScores[iVar6].name[0] = local_4;
					this->scoreList.aScores[iVar6].name[1] = local_3;
					this->scoreList.aScores[iVar6].name[2] = local_2;
					this->scoreList.aScores[iVar6].name[3] = 0;
					this->scoreList.aScores[iVar6].score = local_8;
					local_8 = fVar13;
					local_4 = cVar1;
					local_3 = cVar2;
					local_2 = cVar3;
					iVar6 = iVar6 + 1;
				} while (iVar6 < this->scoreList.nbScores);
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

void CBehaviourMiniGameTraining::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_MINI_GAME* pSaveData = static_cast<S_SAVE_CLASS_MINI_GAME*>(pData);
	int iVar4;

	iVar4 = 0;
	if ((mode == 1) && (0 < this->scoreList.nbScores)) {
		do {
			this->scoreList.aScores[iVar4].score = pSaveData->trainingScores[iVar4].score;
			memcpy(this->scoreList.aScores[iVar4].name, pSaveData->trainingScores[iVar4].name, 4);
			this->scoreList.aScores[iVar4].name[3] = 0;
			iVar4 = iVar4 + 1;
		} while (iVar4 < this->scoreList.nbScores);
	}
	return;
}

void CBehaviourMiniGameTraining::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_MINI_GAME* pSaveData = static_cast<S_SAVE_CLASS_MINI_GAME*>(pData);
	int iVar3;

	iVar3 = 0;
	if (0 < this->scoreList.nbScores) {
		do {
			pSaveData->trainingScores[iVar3].score = this->scoreList.aScores[iVar3].score;
			memcpy(pSaveData->trainingScores[iVar3].name, this->scoreList.aScores[iVar3].name, 4);
			pSaveData->trainingScores[iVar3].name[3] = 0;
			iVar3 = iVar3 + 1;
		} while (iVar3 < this->scoreList.nbScores);
	}
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

CBehaviourMiniGameMulti::CBehaviourMiniGameMulti()
{
	this->scoreList.nbScores = 0;
}

CBehaviourMiniGameMulti::~CBehaviourMiniGameMulti()
{
	if (this->scoreList.nbScores != 0) {
		delete[] this->scoreList.aScores;
	}
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
	this->scoreList.nbScores = 5;
	if (this->scoreList.nbScores != 0) {
		this->scoreList.aScores = new S_MINI_GAME_SCORE[this->scoreList.nbScores];
	}

	iVar8 = 0;
	if (0 < this->scoreList.nbScores) {
		do {
			this->scoreList.aScores[iVar8].score = -1.0f;
			this->scoreList.aScores[iVar8].name[0] = CHighScoreArray::_STRING_Init[0];
			this->scoreList.aScores[iVar8].name[1] = CHighScoreArray::_STRING_Init[1];
			this->scoreList.aScores[iVar8].name[2] = CHighScoreArray::_STRING_Init[2];
			this->scoreList.aScores[iVar8].name[3] = CHighScoreArray::_STRING_Init[3];
			this->scoreList.aScores[iVar8].name[3] = 0;
			iVar8 = iVar8 + 1;
		} while (iVar8 < this->scoreList.nbScores);
	}

	iVar7 = 0;
	while (true) {
		if ((this->pOwner->nbScores <= iVar7) || (4 < iVar7)) break;
		pScore = this->pOwner->aScores + iVar7;
		if (iVar7 <= this->scoreList.nbScores) {
			local_4 = this->scoreList.aScores[iVar7].name[0];
			local_3 = this->scoreList.aScores[iVar7].name[1];
			local_2 = this->scoreList.aScores[iVar7].name[2];
			local_8 = this->scoreList.aScores[iVar7].score;
			this->scoreList.aScores[iVar7].score = pScore->score;
			if (pScore->name != (char*)0x0) {
				memcpy(this->scoreList.aScores[iVar7].name, pScore->name, 4);
				this->scoreList.aScores[iVar7].name[3] = 0;
			}

			iVar6 = iVar7 + 1;
			if (iVar6 < this->scoreList.nbScores) {
				do {
					cVar1 = this->scoreList.aScores[iVar6].name[0];
					cVar2 = this->scoreList.aScores[iVar6].name[1];
					cVar3 = this->scoreList.aScores[iVar6].name[2];
					fVar13 = this->scoreList.aScores[iVar6].score;
					this->scoreList.aScores[iVar6].name[0] = local_4;
					this->scoreList.aScores[iVar6].name[1] = local_3;
					this->scoreList.aScores[iVar6].name[2] = local_2;
					this->scoreList.aScores[iVar6].name[3] = 0;
					this->scoreList.aScores[iVar6].score = local_8;
					local_8 = fVar13;
					local_4 = cVar1;
					local_3 = cVar2;
					local_2 = cVar3;
					iVar6 = iVar6 + 1;
				} while (iVar6 < this->scoreList.nbScores);
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
