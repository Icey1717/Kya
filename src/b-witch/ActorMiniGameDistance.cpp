#include "ActorMiniGameDistance.h"
#include "ActorHero.h"
#include "ActorMovable.h"
#include "MemoryStream.h"
#include "MathOps.h"

StateConfig CActorMiniGameDistance::_gStateCfg_MD[1] =
{
	StateConfig(0, 0)
};

void CActorMiniGameDistance::Create(ByteCode* pByteCode)
{
	CActorMiniGame::Create(pByteCode);

	this->pathPlaneA.pathFollowReader.Create(pByteCode);
	this->pathPlaneB.pathFollowReader.Create(pByteCode);
}

void CActorMiniGameDistance::Init()
{
	CActorMiniGame::Init();

	this->pathPlaneA.Init();
	this->pathPlaneB.Init();
	this->pathPlaneA.Reset();
	this->pathPlaneB.Reset();
	this->field_0x1d0 = 0.0f;
	this->lapCountA = 0;
	this->lapCountB = 0;
	this->previousSegmentA = 0;
	this->previousSegmentB = 0;
}

void CActorMiniGameDistance::Reset()
{
	CActorMiniGame::Reset();
	this->pathPlaneA.Reset();
	this->pathPlaneB.Reset();
	this->field_0x1d0 = 0.0f;
	this->lapCountA = 0;
	this->lapCountB = 0;
	this->previousSegmentA = 0;
	this->previousSegmentB = 0;
}

CBehaviour* CActorMiniGameDistance::BuildBehaviour(int behaviourType)
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

StateConfig* CActorMiniGameDistance::GetStateCfg(int state)
{
	if (state < 8) {
		return CActorMiniGame::GetStateCfg(state);
	}

	return _gStateCfg_MD + state - 8;
}

int CActorMiniGameDistance::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	if (msg == 0x59) {
		return 0;
	}

	return CActorMiniGame::InterpretMessage(pSender, msg, pMsgParam);
}

CBehaviourMiniGameBetting* CActorMiniGameDistance::GetBhvBetting()
{
	return &this->bettingBehaviour;
}

CBehaviourMiniGameTraining* CActorMiniGameDistance::GetBhvTraining()
{
	return &this->trainingBehaviour;
}

CBehaviourMiniGameMulti* CActorMiniGameDistance::GetBhvMulti()
{
	return &this->multiBehaviour;
}

int CActorMiniGameDistance::GetUnity()
{
	return 2;
}

ulong CActorMiniGameDistance::GetScoreLabelMessageHash()
{
	return 0x1e161d0a08130845;
}

ulong CActorMiniGameDistance::GetHighScoreLabelMessageHash()
{
	return 0x5d595e4914151216;
}

bool CActorMiniGameDistance::MustStop()
{
	return this->bMustStop;
}

void CActorMiniGameDistance::InitDistanceState()
{
	this->pathPlaneA.InitTargetPos(&CActorHero::_gThis->currentLocation, &this->pathPlaneA.outData);
	this->pathPlaneB.InitTargetPos(&CActorHero::_gThis->currentLocation, &this->pathPlaneB.outData);
}

void CActorMiniGameDistance::StateDistance()
{
	this->field_0x1d0 = 0.0f;
	float distanceA = ComputePathDistance(&this->pathPlaneA, &this->lapCountA, &this->previousSegmentA);
	float distanceB = ComputePathDistance(&this->pathPlaneB, &this->lapCountB, &this->previousSegmentB);

	uint nbPaths = 0;
	if (this->pathPlaneA.pathFollowReader.pPathFollow != (CPathFollow*)0x0) {
		this->field_0x1d0 = this->field_0x1d0 + distanceA;
		nbPaths = nbPaths + 1;
	}

	if (this->pathPlaneB.pathFollowReader.pPathFollow != (CPathFollow*)0x0) {
		this->field_0x1d0 = this->field_0x1d0 + distanceB;
		nbPaths = nbPaths + 1;
	}

	if (nbPaths != 0) {
		this->field_0x1d0 = this->field_0x1d0 / (float)nbPaths;
	}

	FUN_003ac170(0xffffffff);

	if (MustStop()) {
		FUN_003ac030(this->field_0x1d0, 7, this->field_0x1bc);
	}

	return;
}

float CActorMiniGameDistance::ComputePathDistance(CPathPlane* pPathPlane, int* pLapCount, int* pPreviousSegment)
{
	CPathFollowReader* pReader = &pPathPlane->pathFollowReader;
	CPathFollow* pPath = pReader->pPathFollow;
	if (pPath == (CPathFollow*)0x0) {
		return 0.0f;
	}

	pPathPlane->ExternComputeTargetPosWithPlane(&CActorHero::_gThis->currentLocation, &pPathPlane->outData);
	int segment = pPathPlane->outData.field_0x0;
	if (*pPreviousSegment != segment) {
		if ((segment == 0) && (*pPreviousSegment == pPath->splinePointCount - 1)) {
			*pLapCount = *pLapCount + 1;
		}
		if ((*pPreviousSegment == 0) && (segment == pPath->splinePointCount - 1)) {
			*pLapCount = *pLapCount - 1;
		}
		*pPreviousSegment = segment;
	}

	edF32VECTOR4 projectedPosition;
	float segmentFraction;
	CActorMovable::FUN_00115380(&CActorHero::_gThis->currentLocation, pReader, segment, 0, &projectedPosition, &segmentFraction);
	float distance = pReader->FUN_001c2b50(0, segment);
	int nextSegment = pReader->GetNextPlace(segment, 1);
	edF32VECTOR4 segmentVector;
	edF32Vector4SubHard(&segmentVector, pReader->GetWayPoint(nextSegment), pReader->GetWayPoint(segment));

	return segmentFraction * edF32Vector4GetDistHard(&segmentVector) + distance + (float)*pLapCount * pPath->GetLength();
}

void CBehaviourMiniGameDistanceTraining::Manage()
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
			static_cast<CActorMiniGameDistance*>(pMiniGame)->StateDistance();
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

void CBehaviourMiniGameDistanceTraining::Begin(CActor* pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameDistanceTraining::InitState(int newState)
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
				static_cast<CActorMiniGameDistance*>(pMiniGame)->InitDistanceState();
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

void CBehaviourMiniGameDistanceTraining::TermState(int oldState, int newState)
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

int CBehaviourMiniGameDistanceTraining::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameTraining::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameDistanceBetting::Manage()
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
			static_cast<CActorMiniGameDistance*>(pMiniGame)->StateDistance();
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

void CBehaviourMiniGameDistanceBetting::Begin(CActor * pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameDistanceBetting::InitState(int newState)
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
				static_cast<CActorMiniGameDistance*>(pMiniGame)->InitDistanceState();
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

void CBehaviourMiniGameDistanceBetting::TermState(int oldState, int newState)
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

int CBehaviourMiniGameDistanceBetting::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameBetting::InterpretMessage(pSender, msg, pMsgParam);
}

void CBehaviourMiniGameDistanceMulti::Manage()
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
			static_cast<CActorMiniGameDistance*>(pMiniGame)->StateDistance();
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

void CBehaviourMiniGameDistanceMulti::Begin(CActor * pOwner, int newState, int newAnimationType)
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

void CBehaviourMiniGameDistanceMulti::InitState(int newState)
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
				static_cast<CActorMiniGameDistance*>(pMiniGame)->InitDistanceState();
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

void CBehaviourMiniGameDistanceMulti::TermState(int oldState, int newState)
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

int CBehaviourMiniGameDistanceMulti::InterpretMessage(CActor * pSender, int msg, void* pMsgParam)
{
	return CBehaviourMiniGameMulti::InterpretMessage(pSender, msg, pMsgParam);
}
