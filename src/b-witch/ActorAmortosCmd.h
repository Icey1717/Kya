#ifndef ACTOR_AMORTOS_CMD_H
#define ACTOR_AMORTOS_CMD_H

#include "Types.h"
#include "Actor.h"

#define AMORTOS_CMD_BEHAVIOUR_STAND 2
#define AMORTOS_CMD_STATE_STAND 5
#define AMORTOS_CMD_STATE_IDLE 6

class CActorAmortosCmd;

class CBehaviourAmortosCmd : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorAmortosCmd* pOwner;
};

class CBehaviourAmortosCmdStand : public CBehaviourAmortosCmd
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorAmortosCmd : public CActor
{
public:
	static StateConfig _gStateCfg_AMC[2];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void StateAmortosCmdStand();

	S_ACTOR_STREAM_REF* aAmortos;
	CActor* field_0x164;
	CBehaviourAmortosCmdStand behaviourStand;
};

#endif //ACTOR_AMORTOS_CMD_H
