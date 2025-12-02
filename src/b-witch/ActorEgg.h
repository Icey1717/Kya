#ifndef ACTOR_EGG_H
#define ACTOR_EGG_H

#define EGG_BEHAVIOUR_MOVE 2

#include "Types.h"
#include "ActorMovable.h"

class CActorEgg;

class CBehaviourEgg : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorEgg* pOwner;
};

class CBehaviourEggMove : public CBehaviourEgg
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorEgg : public CActorMovable
{
public:
	static StateConfig _gStateCfg_EGG[8];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void FUN_00356820();
	void BehaviourEggMove_Manage();

	CBehaviourEggMove behaviourEggMove;

	float field_0x1d8;
	uint field_0x1dc;
	uint field_0x1e0;

	SV_MOV_FLEE_ON_PATH_PARAM field_0x1f0;

	float field_0x200;
	byte field_0x204;

	float field_0x220;

	CPathPlaneArray pathPlaneArray;
};

#endif //ACTOR_EGG_H