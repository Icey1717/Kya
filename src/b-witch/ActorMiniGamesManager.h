#ifndef ACTOR_MINI_GAME_MANAGER_H
#define ACTOR_MINI_GAME_MANAGER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGamesManager;

class CActorMiniGamesManager;

class CBehaviourMiniGamesManager : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorMiniGamesManager* pOwner;
};

class CBehaviourMiniGamesManagerStand : public CBehaviourMiniGamesManager
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorMiniGamesManager : public CActor
{
public:
	CActorMiniGamesManager() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CBehaviourMiniGamesManager behaviourMiniGamesManager;
	CBehaviourMiniGamesManagerStand behaviourStand;
};

#endif //ACTOR_MINI_GAME_MANAGER_H
