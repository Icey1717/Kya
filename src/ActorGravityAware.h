#ifndef ACTOR_GRAVITY_AWARE_H
#define ACTOR_GRAVITY_AWARE_H

#include "Types.h"
#include "ActorAutonomous.h"


#define GRAVITY_AWARE_BEHAVIOUR_FALL 0x3
#define GRAVITY_AWARE_BEHAVIOUR_BELL 0x4

class CActorGravityAware;
struct S_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

class CBehaviourGravityAwareBell : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorGravityAware* pOwner;
	S_STREAM_REF<CSound> soundRef;

	S_TARGET_STREAM_REF* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;
};

class CBehaviourGravityAwareFall : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	CActorGravityAware* pOwner;
	S_STREAM_REF<CSound> soundRef;
};

class CActorGravityAware : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_GRV[4];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	float field_0x350;
	byte field_0x354;

	CBehaviourGravityAwareFall fallBehaviour;
	CActorSound* pActorSound;
};

#endif //ACTOR_GRAVITY_AWARE_H