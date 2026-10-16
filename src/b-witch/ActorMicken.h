#ifndef ACTOR_MICKEN_H
#define ACTOR_MICKEN_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "WayPoint.h"
#include "Fx.h"

#define MICKEN_BEHAVIOUR_EAT 4

#define MICKEN_EAT_STATE_STAND				0x6
#define MICKEN_EAT_STATE_WALK_TO_FRUIT		0x8
#define MICKEN_EAT_STATE_EAT				0x9
#define MICKEN_EAT_STATE_CHEW				0xa
#define MICKEN_EAT_STATE_HOLE				0xf
#define MICKEN_EAT_STATE_ROLL_IN_THE_WIND	0x10

class CActorMicken;
class CActorHeroPrivate;

class CBehaviourMicken : public CBehaviour
{
public:
	CActorMicken* pOwner;

	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType) {}
	virtual void InitState(int newState);
};

class CBehaviourMickenEat : public CBehaviourMicken
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int newState, int oldState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	float field_0x8;
	float field_0xc;
	uint field_0x10;
	CPathFollowReader pathFollowReader;
};

class CBehaviourMickenKicked : public CBehaviourMicken
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int newState, int);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourMickenSquashed : public CBehaviourMicken
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int newState, int);

	float field_0x8;
	int field_0xc;
};

#define NUM_MCK_STATES 26

class CActorMicken : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_MCK[NUM_MCK_STATES];

	// CActor
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void Func_0xd4(ed_zone_3d* pZone);
	virtual void Func_0xd8(ed_zone_3d* pZone);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	// CActorAutonomous
	virtual CActorWindState* GetWindState() override;
	virtual void StoreCollisionSphere();

	void StateMickenStand(CBehaviourMickenEat* pBehaviour);
	void StateMickenRollOnGround(CBehaviourMickenKicked* pBehaviour, int param_3, int param_4);
	void StateMickenKickImpactFly(CBehaviourMicken* pBehaviour, int param_3);
	void StateMickenKickFly(CBehaviourMicken* pBehaviour, int param_3);

	void StateMickenEat(CBehaviourMickenEat* pBehaviour);
	void StateMickenChew(CBehaviourMickenEat* pBehaviour);

	void BehaviourMickenKicked_Manage(CBehaviourMickenKicked* pBehaviour);
	void BehaviourMickenEat_Manage(CBehaviourMickenEat* pBehaviour);
	void BehaviourMickenSquashed_Manage(CBehaviourMickenSquashed* pBehaviour);

	void BehaviourMicken_Manage(int state, CBehaviourMicken* pBehaviourMicken);

	int WalkToPos(float param_1, CBehaviourMickenEat* pBehaviour, edF32VECTOR4* pPosition, int param_5);
	void StateMickenHole();
	void StateMickenRollInTheWind();


	S_STREAM_REF<CWayPoint> streamRefWayPoint;
	S_STREAM_REF<CActor> streamRefActor;
	uint field_0x358;
	float field_0x35c;
	CFxHandleExt field_0x360;
	undefined field_0x36c;
	undefined field_0x36d;
	undefined field_0x36e;
	undefined field_0x36f;
	edF32VECTOR4 field_0x370;
	CActorWindState windState; // PS2 0x380..0x3cf.
	int field_0x3d0;
	int field_0x3d4;
	CActorHeroPrivate* field_0x3d8;
	CActorMovable* field_0x3dc;
	CActorMovable* field_0x3e0;
	CActor* pNearestFruit;
	uint flags_0x3e8;
	float field_0x3ec;
	float field_0x3f0;
	float field_0x3f4;
	float field_0x3f8;
	float field_0x3fc;
	int field_0x400;
	ed_zone_3d* field_0x404[3];
	int field_0x410;
	CBehaviourMickenEat behaviourMickenEat;
	undefined field_0x418;
	undefined field_0x419;
	undefined field_0x41a;
	undefined field_0x41b;
	undefined field_0x41c;
	undefined field_0x41d;
	undefined field_0x41e;
	undefined field_0x41f;
	undefined field_0x420;
	undefined field_0x421;
	undefined field_0x422;
	undefined field_0x423;
	undefined field_0x424;
	undefined field_0x425;
	undefined field_0x426;
	undefined field_0x427;
	CPathFollowReader followPathReader;
	undefined field_0x438;
	undefined field_0x439;
	undefined field_0x43a;
	undefined field_0x43b;
	undefined field_0x43c;
	undefined field_0x43d;
	undefined field_0x43e;
	undefined field_0x43f;
};

#endif //ACTOR_MICKEN_H
