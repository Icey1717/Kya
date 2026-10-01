#include "ActorMiniGameBoomy.h"
#include "ActorMiniGamesOrganizer.h"
#include "Animation.h"
#include "CollisionManager.h"
#include "MemoryStream.h"
#include "Pause.h"
#include "Rendering/edCTextFormat.h"
#include "TranslatedTextData.h"
#include "TimeController.h"
#include "FileManager3D.h"
#include "kya.h"

extern void DrawInputBorder(edF32VECTOR2* pPosition, edF32VECTOR2* pSize, _rgba* pColor, edDList_material* pMaterial, int param_5);

StateConfig CActorMiniGameBoomy::_gStateCfg_MB[2] =
{
	StateConfig(0, 0),
	StateConfig(0, 0)
};

CActorMiniGameBoomy::CActorMiniGameBoomy()
{
	this->nbTargets = 0;
	this->aTargets = (S_MINI_GAME_BOOMY_TARGET*)0x0;
	this->nbPenaltyTargets = 0;
	this->aPenaltyTargets = (S_MINI_GAME_BOOMY_TARGET*)0x0;
	this->nbSequences = 0;
	this->aSequences = (S_MINI_GAME_BOOMY_SEQUENCE*)0x0;
}

CActorMiniGameBoomy::~CActorMiniGameBoomy()
{
	delete[] this->aTargets;
	delete[] this->aPenaltyTargets;
	for (int i = 0; i < this->nbSequences; i++) {
		delete[] this->aSequences[i].aTargetIndices;
	}
	delete[] this->aSequences;
}

void S_MINI_GAME_BOOMY_TARGET::Create(ByteCode* pByteCode)
{
	this->actorRef.index = pByteCode->GetS32();
	this->hitSwitch.Create(pByteCode);
}

void S_MINI_GAME_BOOMY_TARGET::Init()
{
	this->actorRef.Init();
	this->bArmed = 1;
	this->hitSwitch.Init();
}

bool S_MINI_GAME_BOOMY_TARGET::CheckCollision()
{
	CActor* pActor = this->actorRef.Get();
	bool bTriggered = false;
	if (this->bArmed && (pActor->pCollisionData->flags_0x4 & 2)) {
		this->bArmed = 0;
		bTriggered = true;
		this->hitSwitch.Switch(pActor);
	}

	if (!this->bArmed && ((pActor->pCollisionData->flags_0x4 & 2) == 0)) {
		this->bArmed = 1;
	}

	return bTriggered;
}

bool S_MINI_GAME_BOOMY_TARGET::CheckHit()
{
	CActor* pActor = this->actorRef.Get();
	bool bTriggered = false;
	if (pActor != (CActor*)0x0) {
		if (this->bArmed && ((pActor->GetStateFlags(pActor->actorState) & 0x20) != 0)) {
			this->bArmed = 0;
			bTriggered = true;
			this->hitSwitch.Switch(pActor);
		}

		if (!this->bArmed && ((pActor->GetStateFlags(pActor->actorState) & 0x20) == 0)) {
			this->bArmed = 1;
		}
	}

	return bTriggered;
}

void S_MINI_GAME_BOOMY_SEQUENCE::Create(ByteCode* pByteCode)
{
	this->nbTargets = pByteCode->GetS32();
	this->aTargetIndices = (int*)0x0;

	if (this->nbTargets != 0) {
		this->aTargetIndices = new int[this->nbTargets];
	}

	for (int i = 0; i < this->nbTargets; i++) {
		this->aTargetIndices[i] = pByteCode->GetS32();
	}
}

int S_MINI_GAME_BOOMY_SEQUENCE::GetTargetIndex(int index)
{
	if (index < this->nbTargets) {
		return this->aTargetIndices[index];
	}

	return -1;
}

void S_MINI_GAME_BOOMY_SEQUENCE::GetTargetColor(int index, _rgba* pColor)
{
	switch (GetTargetIndex(index)) {
	case 0: *pColor = _rgba(0xff, 0, 0xff, 0x80); break;
	case 1: *pColor = _rgba(0xff, 0, 0, 0x80); break;
	case 2: *pColor = _rgba(0, 0xff, 0, 0x80); break;
	case 3: *pColor = _rgba(0, 0xff, 0xff, 0x80); break;
	case 4: *pColor = _rgba(0xff, 0x80, 0, 0x80); break;
	case 5: *pColor = _rgba(0x80, 0xff, 0x80, 0x80); break;
	}
}

void S_MINI_GAME_BOOMY_SEQUENCE::Draw()
{
	edF32VECTOR2 position;
	position.x = (float)gVideoConfig.screenWidth * 0.4f;
	position.y = (float)gVideoConfig.screenHeight * 0.91f;
	for (int i = 0; i < this->nbTargets; i++) {
		edF32VECTOR2 size;
		size.x = (float)gVideoConfig.screenWidth * 0.04f;
		size.y = (float)gVideoConfig.screenHeight * 0.05f;
		float width = (float)gVideoConfig.screenWidth * 0.02f * 2.0f * 1.75f;
		float height = (float)gVideoConfig.screenHeight * 0.025f * 2.0f * 2.15f;
		float scale = 1.0f;
		_rgba color;

		GetTargetColor(i, &color);
		if (i == this->curTarget) {
			scale = 1.2f;
			width = width * scale;
			height = height * scale;
			size.x = size.x * scale;
			size.y = size.y * scale;
		}

		CPauseManager::DrawRectangleBorder(position.x, position.y, width, height,
			(float)gVideoConfig.screenWidth * 0.01f, (float)gVideoConfig.screenHeight * 0.01f,
			0x40101030, 0, 0);

		DrawInputBorder(&position, &size, &color, (edDList_material*)0x0, 1);

		position.x = position.x + scale * ((float)gVideoConfig.screenWidth * 0.02f +
			(float)gVideoConfig.screenWidth * 0.02f * 2.0f * 1.75f);
	}
}

void CActorMiniGameBoomy::Create(ByteCode* pByteCode)
{
	CActorMiniGame::Create(pByteCode);

	this->nbTargets = pByteCode->GetS32();
	if (this->nbTargets != 0) {
		this->aTargets = new S_MINI_GAME_BOOMY_TARGET[this->nbTargets];
		for (int i = 0; i < this->nbTargets; i++) {
			this->aTargets[i].Create(pByteCode);
		}
	}

	this->nbPenaltyTargets = pByteCode->GetS32();
	if (this->nbPenaltyTargets != 0) {
		this->aPenaltyTargets = new S_MINI_GAME_BOOMY_TARGET[this->nbPenaltyTargets];
		for (int i = 0; i < this->nbPenaltyTargets; i++) {
			this->aPenaltyTargets[i].Create(pByteCode);
		}
	}

	this->timeLimit = pByteCode->GetF32();
	this->maxPenalties = pByteCode->GetS32();
	this->gameFlags = pByteCode->GetU32();
	this->nbSequences = pByteCode->GetS32();

	if (this->nbSequences != 0) {
		this->aSequences = new S_MINI_GAME_BOOMY_SEQUENCE[this->nbSequences];
		for (int i = 0; i < this->nbSequences; i++) {
			this->aSequences[i].Create(pByteCode);
		}
	}

	return;
}

void CActorMiniGameBoomy::Init()
{
	CActorMiniGame::Init();

	for (int i = 0; i < this->nbTargets; i++) {
		this->aTargets[i].Init();
	}
	for (int i = 0; i < this->nbPenaltyTargets; i++) {
		this->aPenaltyTargets[i].Init();
	}

	ResetTargets();

	return;
}

void CActorMiniGameBoomy::ResetTargets()
{
	this->field_0x1d0 = 0.0f;
	this->timeRemaining = this->timeLimit;
	this->penaltyCount = 0;
	this->hitCount = 0;
	this->field_0x21c = 0;
	this->bSequenceComplete = 0;

	for (int i = 0; i < this->nbTargets; i++) {
		this->aTargets[i].bArmed = 1;
	}

	for (int i = 0; i < this->nbPenaltyTargets; i++) {
		this->aPenaltyTargets[i].bArmed = 1;
	}

	for (int i = 0; i < this->nbSequences; i++) {
		this->aSequences[i].curTarget = 0;
	}

	this->curSequence = 0;

	return;
}

void CActorMiniGameBoomy::Reset()
{
	CActorMiniGame::Reset();
	ResetTargets();
}

void CActorMiniGameBoomy::CheckpointReset()
{
	CActorMiniGame::CheckpointReset();
	ResetTargets();
}

CBehaviour* CActorMiniGameBoomy::BuildBehaviour(int behaviourType)
{
	if (behaviourType == 4) {
		return &this->multiBehaviour;
	}

	if (behaviourType == 3) {
		return &this->bettingBehaviour;
	}

	if (behaviourType == 2) {
		return &this->trainingBehaviour;
	}

	return CActor::BuildBehaviour(behaviourType);
}

StateConfig* CActorMiniGameBoomy::GetStateCfg(int state)
{
	if (state < 8) {
		return CActorMiniGame::GetStateCfg(state);
	}

	return _gStateCfg_MB + state - 8;
}

int CActorMiniGameBoomy::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x67) {
		if ((this->gameFlags & 0x10) != 0) {
			this->curSequence = (this->curSequence + 1) % this->nbSequences;
			this->aSequences[this->curSequence].curTarget = 0;
			if (this->bSequenceComplete == 0) {
				this->penaltyCount = this->penaltyCount + 1;
			}
			else {
				this->bSequenceComplete = 0;
			}

			SetState(8, -1);
		}
	}
	else if (msg == 0x5a) {
		this->penaltyCount = this->penaltyCount + 1;
		return 1;
	}
	else if (msg == 0x59) {
		this->hitCount = this->hitCount + 1;
		return 1;
	}

	return CActorMiniGame::InterpretMessage(pSender, msg, pMsgParam);
}

CBehaviourMiniGameBetting* CActorMiniGameBoomy::GetBhvBetting()
{
	return &this->bettingBehaviour;
}

CBehaviourMiniGameTraining* CActorMiniGameBoomy::GetBhvTraining()
{
	return &this->trainingBehaviour;
}

CBehaviourMiniGameMulti* CActorMiniGameBoomy::GetBhvMulti()
{
	return &this->multiBehaviour;
}

int CActorMiniGameBoomy::GetUnity()
{
	return 3;
}

ulong CActorMiniGameBoomy::GetScoreLabelMessageHash()
{
	return 0x1e161d0a08130845;
}

ulong CActorMiniGameBoomy::GetHighScoreLabelMessageHash()
{
	return 0x5d595e4914151216;
}

bool CActorMiniGameBoomy::MustStop()
{
	if ((this->timeLimit != -1.0f) && (this->field_0x1b9 != 0)) {
		this->timeRemaining = this->timeRemaining - GetTimer()->cutsceneDeltaTime;
		if (this->timeRemaining < GetTimer()->cutsceneDeltaTime) {
			return true;
		}
	}

	if ((this->maxPenalties != -1) && (this->penaltyCount >= this->maxPenalties)) {
		return true;
	}

	return this->bMustStop;
}

float CActorMiniGameBoomy::GetExtraHudHeight()
{
	float height = 0.0f;

	if (this->timeLimit != -1.0f) {
		height = (float)gVideoConfig.screenHeight * 0.06f;
	}

	if ((this->gameFlags & 4) != 0) {
		height = height + (float)gVideoConfig.screenHeight * 0.06f;
	}

	return height;
}

S_MINI_GAME_BOOMY_SEQUENCE* CActorMiniGameBoomy::GetCurSequence()
{
	S_MINI_GAME_BOOMY_SEQUENCE* pSequence = this->aSequences + this->curSequence;
	while (pSequence->nbTargets < 1) {
		this->curSequence = (this->curSequence + 1) % this->nbSequences;
		pSequence = this->aSequences + this->curSequence;
		pSequence->curTarget = 0;
	}

	return pSequence;
}

void CActorMiniGameBoomy::DrawExtraHud(float x, float y)
{
	edCTextFormat textFormat;
	if (this->timeLimit != -1.0f) {
		char* pLabel = gMessageManager.get_message(0x5b5b5b4944041217);
		FormatScore(this->timeRemaining * 100.0f, &textFormat, pLabel, 1, 1);
		textFormat.Display(x + (float)gVideoConfig.screenWidth * 0.09f, y + (float)gVideoConfig.screenHeight * 0.06f);
	}

	if ((this->gameFlags & 4) != 0) {
		_rgba color(0x80808080);
		edDList_material* pMaterial = CScene::ptable.g_C3DFileManager_00451664->GetMaterialFromId(this->field_0x1c0->materialId_0x178, 2);
		edF32VECTOR2 position;
		edF32VECTOR2 size;
		position.x = (float)gVideoConfig.screenWidth * 0.5f;
		position.y = (float)gVideoConfig.screenHeight * 0.13f;
		size.x = (float)gVideoConfig.screenWidth * 0.07f;
		size.y = (float)gVideoConfig.screenHeight * 0.07f;

		for (int i = 0; i < this->maxPenalties - this->penaltyCount; i++) {
			DrawInputBorder(&position, &size, &color, pMaterial, 1);
			position.x = position.x + ((float)gVideoConfig.screenWidth * 0.07f) / 2.0f + (float)gVideoConfig.screenWidth * 0.06f;
		}
	}

	if ((this->gameFlags & 0x10) != 0) {
		S_MINI_GAME_BOOMY_SEQUENCE* pSequence = GetCurSequence();
		if ((pSequence != (S_MINI_GAME_BOOMY_SEQUENCE*)0x0) && (this->curSequence != -1)) {
			pSequence->Draw();
		}
	}
}

void CActorMiniGameBoomy::DisableUnusedSequenceTargets()
{
	if ((this->gameFlags & 0x10) != 0) {
		S_MINI_GAME_BOOMY_SEQUENCE* pSequence = GetCurSequence();

		for (int i = 0; i < this->nbTargets; i++) {
			bool bInSequence = false;
			for (int j = 0; j < pSequence->nbTargets; j++) {
				if (i == pSequence->GetTargetIndex(j)) {
					bInSequence = true;
					break;
				}
			}

			CActor* pActor = this->aTargets[i].actorRef.Get();
			if (!bInSequence && (pActor != (CActor*)0x0)) {
				DoMessage(pActor, (ACTOR_MESSAGE)3, 0);
			}
		}
	}
}

void CActorMiniGameBoomy::StateBoomy()
{
	if ((this->gameFlags & 0x10) == 0) {
		if ((this->gameFlags & 8) != 0) {
			int nbCollisions = 0;
			for (int i = 0; i < this->nbTargets; i++) {
				if (this->aTargets[i].CheckCollision()) {
					nbCollisions = nbCollisions + 1;
				}
			}

			this->penaltyCount = this->penaltyCount + nbCollisions;
			nbCollisions = 0;
			for (int i = 0; i < this->nbPenaltyTargets; i++) {
				if (this->aPenaltyTargets[i].CheckCollision()) {
					nbCollisions = nbCollisions + 1;
				}
			}
			this->penaltyCount = this->penaltyCount + nbCollisions;
		}

		int nbHits = 0;
		for (int i = 0; i < this->nbTargets; i++) {
			if (this->aTargets[i].CheckHit()) {
				nbHits = nbHits + 1;
			}
		}

		this->hitCount = this->hitCount + nbHits;
		nbHits = 0;
		for (int i = 0; i < this->nbPenaltyTargets; i++) {
			if (this->aPenaltyTargets[i].CheckHit()) {
				nbHits = nbHits + 1;
			}
		}

		this->penaltyCount = this->penaltyCount + nbHits;
	}
	else {
		S_MINI_GAME_BOOMY_SEQUENCE* pSequence = GetCurSequence();
		DisableUnusedSequenceTargets();
		int targetIndex = pSequence->GetTargetIndex(pSequence->curTarget);
		for (int i = 0; i < this->nbTargets; i++) {
			this->aTargets[i].CheckCollision();
		}

		int penaltiesBeforeHit = this->penaltyCount;
		while (this->aTargets[targetIndex].CheckHit()) {
			if (pSequence->curTarget == pSequence->nbTargets - 1) {
				if (this->bSequenceComplete == 0) {
					this->hitCount = this->hitCount + 1;
					if (penaltiesBeforeHit == this->penaltyCount) {
						this->bSequenceComplete = 1;
					}
					else {
						this->penaltyCount = penaltiesBeforeHit;
						this->bSequenceComplete = 0;
					}
				}
				break;
			}

			pSequence->curTarget = pSequence->curTarget + 1;
			targetIndex = pSequence->GetTargetIndex(pSequence->curTarget);
		}

		for (int i = 0; i < this->nbTargets; i++) {
			this->aTargets[i].CheckHit();
		}
	}

	this->field_0x1d0 = (float)this->hitCount;
	if ((this->gameFlags & 2) != 0) {
		this->field_0x1d0 = this->field_0x1d0 - (float)this->penaltyCount;
	}

	if ((this->gameFlags & 1) != 0) {
		for (int i = 0; i < this->nbTargets; i++) {
			CActor* pActor = this->aTargets[i].actorRef.Get();
			if ((pActor->pAnimationController != (CAnimation*)0x0) &&
				((pActor->GetStateFlags(pActor->actorState) & 0x20) != 0) &&
				pActor->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				pActor->Reset();
			}
		}
	}

	FUN_003ac170(0xffffffff);

	if (MustStop()) {
		FUN_003ac030(this->field_0x1d0, 7, this->field_0x1bc);
	}
}

void CBehaviourMiniGameBoomyTraining::Manage()
{
	int curState;
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	curState = pMiniGame->actorState;
	if (curState == 9) {
		CActorMiniGameBoomy* pBoomy = static_cast<CActorMiniGameBoomy*>(pMiniGame);
		if (pBoomy->curSequence != -1) {
			pBoomy->aSequences[pBoomy->curSequence].Draw();
		}
		return;
	}

	if (curState == 6) {
		pMiniGame->FUN_003a9f70();
	}
	else {
		if (curState == 8) {
			static_cast<CActorMiniGameBoomy*>(pMiniGame)->StateBoomy();
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

void CBehaviourMiniGameBoomyTraining::Begin(CActor* pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameBoomyTraining::InitState(int newState)
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
				CActorMiniGameBoomy* pBoomy = static_cast<CActorMiniGameBoomy*>(pMiniGame);
				pBoomy->timeRemaining = pBoomy->timeLimit;
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

void CBehaviourMiniGameBoomyTraining::TermState(int oldState, int newState)
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

int CBehaviourMiniGameBoomyTraining::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameTraining::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameBoomyBetting::Manage()
{
	int curState;
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	curState = pMiniGame->actorState;
	if (curState == 9) {
		CActorMiniGameBoomy* pBoomy = static_cast<CActorMiniGameBoomy*>(pMiniGame);
		if (pBoomy->curSequence != -1) {
			pBoomy->aSequences[pBoomy->curSequence].Draw();
		}
		return;
	}
	if (curState == 6) {
		pMiniGame->FUN_003a9f70();
	}
	else {
		if (curState == 8) {
			static_cast<CActorMiniGameBoomy*>(pMiniGame)->StateBoomy();
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

void CBehaviourMiniGameBoomyBetting::Begin(CActor * pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameBoomyBetting::InitState(int newState)
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
				CActorMiniGameBoomy* pBoomy = static_cast<CActorMiniGameBoomy*>(pMiniGame);
				pBoomy->timeRemaining = pBoomy->timeLimit;
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

void CBehaviourMiniGameBoomyBetting::TermState(int oldState, int newState)
{
	CActorMiniGame* pMiniGame = this->pOwner;
	if (oldState == 7) {
		pMiniGame->FUN_003a9d60();
	}
	else if (oldState == 6) {
		pMiniGame->FUN_003a9f50();
	}

	return;
}

int CBehaviourMiniGameBoomyBetting::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameBetting::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameBoomyMulti::Manage()
{
	int curState;
	CActorMiniGame* pMiniGame;

	pMiniGame = this->pOwner;
	curState = pMiniGame->actorState;
	if (curState == 9) {
		CActorMiniGameBoomy* pBoomy = static_cast<CActorMiniGameBoomy*>(pMiniGame);
		if (pBoomy->curSequence != -1) {
			pBoomy->aSequences[pBoomy->curSequence].Draw();
		}
		return;
	}
	if (curState == 6) {
		pMiniGame->FUN_003a9f70();
	}
	else {
		if (curState == 8) {
			static_cast<CActorMiniGameBoomy*>(pMiniGame)->StateBoomy();
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

void CBehaviourMiniGameBoomyMulti::Begin(CActor * pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameBoomyMulti::InitState(int newState)
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
				CActorMiniGameBoomy* pBoomy = static_cast<CActorMiniGameBoomy*>(pMiniGame);
				pBoomy->timeRemaining = pBoomy->timeLimit;
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

void CBehaviourMiniGameBoomyMulti::TermState(int oldState, int newState)
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

int CBehaviourMiniGameBoomyMulti::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameMulti::InterpretMessage(pSender, msg, pMsgParam);
}
