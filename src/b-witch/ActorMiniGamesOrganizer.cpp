#include "ActorMiniGamesOrganizer.h"
#include "MemoryStream.h"

void CActorMiniGamesOrganizer::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);
}

void CActorMiniGamesOrganizer::Init()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorMiniGamesOrganizer::Term()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorMiniGamesOrganizer::Draw()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorMiniGamesOrganizer::Reset()
{
	IMPLEMENTATION_GUARD_LOG();
}

void CActorMiniGamesOrganizer::CheckpointReset()
{
	IMPLEMENTATION_GUARD_LOG();
}

CBehaviour* CActorMiniGamesOrganizer::BuildBehaviour(int behaviourType)
{
	IMPLEMENTATION_GUARD_LOG();
	return CActor::BuildBehaviour(behaviourType);
}

StateConfig* CActorMiniGamesOrganizer::GetStateCfg(int state)
{
	IMPLEMENTATION_GUARD_LOG();
	return CActor::GetStateCfg(state);
}

int CActorMiniGamesOrganizer::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD_LOG();
	return 0;
}

void CBehaviourMiniGamesOrganizer::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGamesOrganizer::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorMiniGamesOrganizer*>(pOwner);

	return;
}

int CBehaviourMiniGamesOrganizer::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourMiniGamesOrganizer::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourMiniGamesOrganizerStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourMiniGamesOrganizerStand::Manage()
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_Manage();

	return;
}

void CBehaviourMiniGamesOrganizerStand::ManageFrozen()
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_Manage();

	return;
}

void CBehaviourMiniGamesOrganizerStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourMiniGamesOrganizer::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourMiniGamesOrganizerStand::InitState(int newState)
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_InitState(newState);

	return;
}

void CBehaviourMiniGamesOrganizerStand::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourMiniGamesOrganizerStand_TermState(oldState);

	return;
}

int CBehaviourMiniGamesOrganizerStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
