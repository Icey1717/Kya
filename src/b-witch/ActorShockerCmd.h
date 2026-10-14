#ifndef ACTOR_SHOCKER_CMD_H
#define ACTOR_SHOCKER_CMD_H

#include "Types.h"
#include "Actor.h"

class CActorShockerCmd;

class CSequenceParam
{
public:
	CActor* GetActor();

	S_STREAM_REF<CActor> actorRef;
	int count_0x4;
	float* aFloats;
	byte field_0xc;
};

class CBehaviourShockerCmd : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorShockerCmd* pOwner;
};

class CBehaviourShockerCmdStand : public CBehaviourShockerCmd
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorShockerCmd : public CActor
{
public:
	static StateConfig _gStateCfg_SCM[2];

	CActorShockerCmd();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void StateShockerCmdSequence();

	uint field_0x168;
	int field_0x174;
	undefined4 field_0x178;
	int field_0x17c;
	int field_0x180;

	int sequenceParamsCount;
	CSequenceParam* aSequenceParams;

	CBehaviourShockerCmdStand behaviourStand;
};

#endif //ACTOR_SHOCKER_CMD_H