#ifndef ACTOR_MINI_GAME_ORGANIZER_H
#define ACTOR_MINI_GAME_ORGANIZER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGamesOrganizer;
class CActorMiniGamesManager;

class CBehaviourMiniGamesOrganizer : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorMiniGamesOrganizer* pOwner;
};

class CBehaviourMiniGamesOrganizerStand : public CBehaviourMiniGamesOrganizer
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void ManageFrozen();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorMiniGamesOrganizer : public CActor {
public:
	CActorMiniGamesOrganizer() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// Unrecovered organizer data surrounding the fields used by the manager.
	undefined field_0x160[0x1c];
	S_ACTOR_STREAM_REF* field_0x17c = (S_ACTOR_STREAM_REF*)0x0;
	undefined field_0x180[0x874];
	CActorMiniGamesManager* field_0x9f4;
};

#endif //ACTOR_MINI_GAME_ORGANIZER_H
