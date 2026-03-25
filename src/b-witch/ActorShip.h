#ifndef ACTOR_SHIP_H
#define ACTOR_SHIP_H

#include "Types.h"
#include "Actor.h"
#include "Fx.h"

class CActorShip;

class CBehaviourShip : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorShip* pOwner;
};

class CBehaviourShipFLY : public CBehaviourShip
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorShip : public CActor
{
public:
	CActorShip();
	virtual void Create(ByteCode* pByteCode);

	float field_0x1b4;

	CFxHandle field_0x26c;
};

#endif //ACTOR_SHIP_H