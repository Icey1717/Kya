#ifndef ACTOR_HUNTER_H
#define ACTOR_HUNTER_H

#include "Types.h"
#include "ActorAutonomous.h"

#define HUNTER_BEHAVIOUR_WATCH 3

class CActorHunter;

class CBehaviourHunter : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorHunter* pOwner;
};

class CBehaviourHunterWatch : public CBehaviourHunter
{
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorHunter : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_HTR[5];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void SetupCollisionMatrix();

	float field_0x350;
	uint field_0x354;
	S_STREAM_REF<CActor> actorRef;
	uint field_0x35c;
	uint boneId;
	CActor* field_0x364;
	CBehaviourHunterWatch behaviourWatch;
	byte field_0x370;
	float animLength;
	float field_0x3a4;

};

#endif //ACTOR_HUNTER_H