#ifndef ACTOR_MICKEN_H
#define ACTOR_MICKEN_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "WayPoint.h"

#define MICKEN_BEHAVIOUR_EAT 4

#define MICKEN_EAT_STATE_STAND				0x6
#define MICKEN_EAT_STATE_WALK_TO_FRUIT		0x8
#define MICKEN_EAT_STATE_EAT				0x9
#define MICKEN_EAT_STATE_CHEW				0xa
#define MICKEN_EAT_STATE_HOLE				0xf
#define MICKEN_EAT_STATE_ROLL_IN_THE_WIND	0x10

class CActorMicken;
class CActorHeroPrivate;

class CBehaviourMicken : public CBehaviour {
public:
	CActorMicken* pOwner;

	virtual void Create(ByteCode* pByteCode) {}
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType) {}
	virtual void InitState(int newState);
};

class CBehaviourMickenEat : public CBehaviourMicken {
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

class CBehaviourMickenKicked : public CBehaviourMicken {
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int newState, int);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourMickenSquashed : public CBehaviourMicken {
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

class CActorMicken : public CActorAutonomous {
public:
	CActorMicken();

	static StateConfig _gStateCfg_MCK[NUM_MCK_STATES];

	// CActor
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	// CActorAutonomous
	virtual void StoreCollisionSphere();

	void StateMickenStand(CBehaviourMickenEat* pBehaviour);
	void StateMickenRollOnGround(CBehaviourMickenKicked* pBehaviour, int param_3, int param_4);
	void StateMickenKickImpactFly(CBehaviourMicken* pBehaviour, int param_3);
	void StateMickenKickFly(CBehaviourMicken* pBehaviour, int param_3);

	void BehaviourMickenKicked_Manage(CBehaviourMickenKicked* pBehaviour);
	void BehaviourMickenEat_Manage(CBehaviourMickenEat* pBehaviour);
	void BehaviourMickenSquashed_Manage(CBehaviourMickenSquashed* pBehaviour);

	void BehaviourMicken_Manage(int state, CBehaviourMicken* pBehaviourMicken);

	int WalkToPos(float param_1, CBehaviourMickenEat* pBehaviour, edF32VECTOR4* pPosition, int param_5);
	void StateMickenHole();


	S_STREAM_REF<CWayPoint> streamRefWayPoint;
	S_STREAM_REF<CActor> streamRefActor;
	uint field_0x358;
	float field_0x35c;
	undefined4 field_0x360;
	undefined4 field_0x364;
	int field_0x368;
	undefined field_0x36c;
	undefined field_0x36d;
	undefined field_0x36e;
	undefined field_0x36f;
	edF32VECTOR4 field_0x370;
	undefined field_0x380;
	undefined field_0x381;
	undefined field_0x382;
	undefined field_0x383;
	undefined field_0x384;
	undefined field_0x385;
	undefined field_0x386;
	undefined field_0x387;
	undefined field_0x388;
	undefined field_0x389;
	undefined field_0x38a;
	undefined field_0x38b;
	undefined field_0x38c;
	undefined field_0x38d;
	undefined field_0x38e;
	undefined field_0x38f;
	undefined field_0x390;
	undefined field_0x391;
	undefined field_0x392;
	undefined field_0x393;
	undefined field_0x394;
	undefined field_0x395;
	undefined field_0x396;
	undefined field_0x397;
	undefined field_0x398;
	undefined field_0x399;
	undefined field_0x39a;
	undefined field_0x39b;
	undefined field_0x39c;
	undefined field_0x39d;
	undefined field_0x39e;
	undefined field_0x39f;
	undefined field_0x3a0;
	undefined field_0x3a1;
	undefined field_0x3a2;
	undefined field_0x3a3;
	undefined field_0x3a4;
	undefined field_0x3a5;
	undefined field_0x3a6;
	undefined field_0x3a7;
	undefined field_0x3a8;
	undefined field_0x3a9;
	undefined field_0x3aa;
	undefined field_0x3ab;
	undefined field_0x3ac;
	undefined field_0x3ad;
	undefined field_0x3ae;
	undefined field_0x3af;
	undefined field_0x3b0;
	undefined field_0x3b1;
	undefined field_0x3b2;
	undefined field_0x3b3;
	undefined field_0x3b4;
	undefined field_0x3b5;
	undefined field_0x3b6;
	undefined field_0x3b7;
	undefined field_0x3b8;
	undefined field_0x3b9;
	undefined field_0x3ba;
	undefined field_0x3bb;
	undefined field_0x3bc;
	undefined field_0x3bd;
	undefined field_0x3be;
	undefined field_0x3bf;
	undefined field_0x3c0;
	undefined field_0x3c1;
	undefined field_0x3c2;
	undefined field_0x3c3;
	undefined field_0x3c4;
	undefined field_0x3c5;
	undefined field_0x3c6;
	undefined field_0x3c7;
	undefined field_0x3c8;
	undefined field_0x3c9;
	undefined field_0x3ca;
	undefined field_0x3cb;
	undefined field_0x3cc;
	undefined field_0x3cd;
	undefined field_0x3ce;
	undefined field_0x3cf;
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
	undefined4 field_0x404;
	undefined4 field_0x408;
	undefined4 field_0x40c;
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