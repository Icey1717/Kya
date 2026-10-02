#ifndef ACTOR_WOOD_MONSTER_H
#define ACTOR_WOOD_MONSTER_H

#include "Types.h"
#include "Actor.h"
#include "ActorAmortos.h"

#define WOODMONSTER_BEHAVIOUR_GET 2
#define WOODMONSTER_STATE_STAND 5
#define WOODMONSTER_STATE_ATTRACT 6
#define WOODMONSTER_STATE_GET 7
#define WOODMONSTER_STATE_WAIT 8
#define WOODMONSTER_STATE_RELEASE 9
#define WOODMONSTER_STATE_RELEASE_END 10
#define WOODMONSTER_STATE_WAIT_HERO_ANIM 12

class CActorWoodMonster;
class CActorAutonomous;

class CBehaviourWoodMonster : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorWoodMonster* pOwner;
};

class CBehaviourWoodMonsterGet : public CBehaviourWoodMonster
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	float field_0x8;
	S_STREAM_REF<CWayPoint> wayPointRef;
	int field_0x10;
};

class CActorWoodMonster : public CActor
{
public:
	static StateConfig _gStateCfg_WOODMONSTER[8];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void StateWoodMonsterStand(CBehaviourWoodMonsterGet* pBehaviour);
	void StateWoodMonsterAttract();
	void BehaviourWoodMonsterGet_Manage(CBehaviourWoodMonsterGet* pBehaviour);

	edF32VECTOR4 field_0x160;
	CActorAutonomous* field_0x170;
	CCylinderDetection field_0x180;
	CBehaviourWoodMonsterGet behaviourGet;
};

#endif //ACTOR_WOOD_MONSTER_H
