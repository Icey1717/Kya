#ifndef ACTOR_FRUIT_H
#define ACTOR_FRUIT_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "CinematicManager.h"

#define FRUIT_BEHAVIOUR_SUSPENDED	3
#define FRUIT_BEHAVIOUR_SQUASHED	4
#define FRUIT_BEHAVIOUR_INVENTORY	5
#define FRUIT_BEHAVIOUR_BONUS		6
#define FRUIT_BEHAVIOUR_PROJECTILE 	7

class CActorFruit;

class CBehaviourFruit : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	CActorFruit* pOwner;
};

class CBehaviourFruitSuspended : public CBehaviourFruit
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourFruitBonus : public CBehaviourFruit
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	S_STREAM_REF<CActor> actorRef;
	int initialBehaviourId;
};

class CBehaviourFruitSquashed : public CBehaviourFruit
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	float field_0x8;
};

class CActorFruit : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_FRT[12];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual void Func_0xd4(ed_zone_3d* param_2);
	virtual void Func_0xd8(ed_zone_3d* param_2);
	virtual CInventoryInfo* GetInventoryInfo();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	void BehaviourFruitSquashed_InitState(int newState);
	void BehaviourFruitSquashed_Manage(CBehaviourFruitSquashed* pBehaviour);
	void BehaviourFruitSquashed_TermState(int oldState);

	void FUN_001de2f0(float param_1, int nextState);

	S_NTF_SWITCH ntfSwitch;
	S_NTF_SWITCH_EX_LIST ntfSwitchExList;

	uint field_0x360;
	CActorMovable* field_0x364;
	CActorMovable* field_0x368;
	float field_0x36c;
	float field_0x370;
	ed_zone_3d* field_0x374[3];
	undefined4 field_0x380;
	CBehaviourFruitSuspended behaviourSuspended;
	CBehaviourFruitSquashed behaviourSquashed;
};

#endif //ACTOR_FRUIT_H