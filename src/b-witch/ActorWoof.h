#ifndef ACTOR_WOOF_H
#define ACTOR_WOOF_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "Vision.h"
#include "Dynamic.h"

#define WOOF_BEHAVIOUR_GUARD 3
#define WOOF_BEHAVIOUR_VERTICAL_JUMP 4

#define WOOF_STATE_IDLE 0x6
#define WOOF_STATE_YAP 0x7
#define WOOF_STATE_WAIT_JUMP 0x9
#define WOOF_STATE_WAY_POINT 0x12
#define WOOF_STATE_CHASE 0x13
#define WOOF_STATE_CHASE_INTRUDER_LOST 0x14
#define WOOF_STATE_COME_BACK 0x15
#define WOOF_STATE_PATH 0x16
#define WOOF_STATE_SLIDE 0x17
#define WOOF_STATE_FALL 0x18
#define WOOF_STATE_ATTACK_HERO_2_4 0x1e
#define WOOF_STATE_ATTACK_HERO_3_4 0x1f
#define WOOF_STATE_ATTACK_HERO_4_4 0x20
#define WOOF_STATE_JUMP_1_4 0x22
#define WOOF_STATE_JUMP_2_4 0x23
#define WOOF_STATE_JUMP_3_4 0x24
#define WOOF_STATE_JUMP_4_4 0x25

class CActorWoof;
class CActorHero;

class CBehaviourWoof : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorWoof* pOwner;
};

class CBehaviourWoofGuard : public CBehaviourWoof
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

class CBehaviourWoofVerticalJump : public CBehaviourWoof
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

class CActorWoof : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_WOF[38];

	CActorWoof();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();

	virtual void Reset();
	virtual void CheckpointReset();

	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	virtual bool Can_0x9c();

	virtual CVision* GetVision();

	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	virtual void StoreCollisionSphere();

	virtual CPathFinderClient* GetPathfinderClient();
	virtual CPathFinderClient* GetPathfinderClientAlt();

	void ClearLocalData();

	void UpdateVision();
	bool DetectWithVision(edF32VECTOR4* param_2, edF32VECTOR4* param_3, bool param_4);

	int CheckArea();

	void BehaviourWoofGuard_InitState(int newState);
	void BehaviourWoofGuard_Manage();
	void BehaviourWoofGuard_TermState(int oldState);

	void BehaviourWoofVerticalJump_InitState(int newState);
	void BehaviourWoofVerticalJump_Manage();
	void BehaviourWoofVerticalJump_TermState(int oldState);

	void StateWoofYap();
	void StateWoofWaitJump();
	void StateWoofComeBack();
	void StateWoofPath();
	void StateWoofSlide();
	void StateWoofFall();
	void StateWoofChase();
	void StateWoofChaseIntruderLost();
	void StateWoofAttackHero_2_4();
	void StateWoofAttackHero_3_4();
	void StateWoofJump_2_4();

	CBehaviourWoofGuard behaviourWoofGuard;
	CBehaviourWoofVerticalJump behaviourWoofVerticalJump;

	CPathFollowReader pathFollowReader;
	CVision vision;

	CPathFinderClient pathFinderClient;

	float field_0x390;
	float field_0x394;

	uint field_0x398;
	uint field_0x39c;

	S_ACTOR_STREAM_REF* field_0x3a0;
	S_STREAM_REF<CWayPoint> field_0x3a4;

	uint field_0x410;
	uint field_0x414;
	float field_0x418;
	int field_0x41c;
	uint field_0x420;
	uint field_0x424;
	uint field_0x428;
	CActor* field_0x42c;
	CActorMovable* field_0x430;
	edF32VECTOR4 field_0x440;
	CVision field_0x460;
	float field_0x4c4;
	edF32VECTOR4 field_0x520;
	byte field_0x530;

	edF32VECTOR4 field_0x540;
	float lastGroundY; // Last known good Y position on the ground, used to detect falling.
	uint field_0x554;
	undefined4 field_0x558;
	undefined4 field_0x55c;
	uint field_0x560;
	uint field_0x564;
	edF32VECTOR4 field_0x570;
	edF32VECTOR4 field_0x580;
	edF32VECTOR4 field_0x590;
	CScalarDyn field_0x5a0;
};

#endif //ACTOR_WOOF_H