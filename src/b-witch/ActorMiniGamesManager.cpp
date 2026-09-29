#include "ActorMiniGamesManager.h"
#include "MemoryStream.h"

void CActorMiniGamesManager::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

void CActorMiniGamesManager::Init()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorMiniGamesManager::Reset()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorMiniGamesManager::CheckpointReset()
{
	IMPLEMENTATION_GUARD_LOG();
}

CBehaviour* CActorMiniGamesManager::BuildBehaviour(int behaviourType)
{
	IMPLEMENTATION_GUARD_LOG();
	return CActor::BuildBehaviour(behaviourType);
}

StateConfig* CActorMiniGamesManager::GetStateCfg(int state)
{
	IMPLEMENTATION_GUARD_LOG();
	return CActor::GetStateCfg(state);
}

int CActorMiniGamesManager::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD_LOG();
	return 0;
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
