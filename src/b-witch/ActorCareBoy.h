#ifndef ACTOR_CARE_BOY_H
#define ACTOR_CARE_BOY_H

#include "Types.h"
#include "ActorMovable.h"
#include "Fx.h"

#define CARE_BOY_BEHAVIOUR_STEADY 2

class CActorCareBoy;

class CBehaviourCareBoy : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorCareBoy* pOwner;
};

class CBehaviourCareBoySteady : public CBehaviourCareBoy
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorCareBoy : public CActorMovable
{
public:
	static StateConfig _gStateCfg_CRB[4];

	CActorCareBoy();

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void LinkToParent(CActor* pLinkedActor, uint key);

	CBehaviourCareBoySteady behaviourSteady;
	
	CActor* pParent;
	int field_0x1dc;
	int field_0x1e0;

	CFxHandle field_0x1e4;
};

#endif //ACTOR_CARE_BOY_H