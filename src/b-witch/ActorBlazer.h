#ifndef ACTOR_BLAZER_H
#define ACTOR_BLAZER_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "PathFollow.h"
#include "FireShot.h"

#define BLAZER_BEHAVIOUR_STAND 0x3

#define BLAZER_STATE_IDLE 0x6
#define BLAZER_STATE_ATTACK 0x8
#define BLAZER_STATE_STUNNED_START_A 0x9
#define BLAZER_STATE_STUNNED_START_B 0xa
#define BLAZER_STATE_STUNNED 0xb
#define BLAZER_STATE_RECOVER 0xe
#define BLAZER_STATE_RESET 0xf

class CActorBlazer;

class CBehaviourBlazer : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorBlazer* pOwner;
};

class CBehaviourBlazerStand : public CBehaviourBlazer
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorBlazer : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_BLA[12];

	CActorBlazer(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ClearLocalData();

	void BehaviourBlazerStand_InitState(int newState);
	void BehaviourBlazerStand_Manage();

	void StateBlazerAttack();

	void FUN_003d8640();
	void Blazer_UpdatePosition(edF32VECTOR4* pPosition, int param_3, CActorsTable* pActorsTable, edF32MATRIX4* param_5);
	int CheckArea();
	int FUN_003d7eb0(CActor* param_2);
	void FUN_003d8060();

	CBehaviourBlazerStand behaviourStand;
	uint field_0x358;
	uint field_0x35c;
	S_TILT_CONFIG* field_0x360;
	float field_0x364;
	float field_0x368;
	float field_0x36c;
	uint field_0x370;
	float field_0x374;
	float field_0x378;
	float field_0x37c;
	float stunnedDuration;
	float field_0x384;
	float field_0x388;
	S_STREAM_REF<CWayPoint> field_0x38c;
	S_STREAM_REF<CWayPoint> field_0x390;
	uint field_0x394;
	uint field_0x398;
	CPathFollowReader pathFollowReader;
	float field_0x3ac;
	float field_0x3b0;
	uint field_0x3b4;
	float field_0x3b8;
	undefined field_0x3bc;
	undefined field_0x3bd;
	undefined field_0x3be;
	undefined field_0x3bf;
	CFireShot fireShot;
	S_TILT_DATA field_0x660;
	undefined field_0x6b0;
	undefined field_0x6b1;
	undefined field_0x6b2;
	undefined field_0x6b3;
	undefined field_0x6b4;
	undefined field_0x6b5;
	undefined field_0x6b6;
	undefined field_0x6b7;
	undefined field_0x6b8;
	undefined field_0x6b9;
	undefined field_0x6ba;
	undefined field_0x6bb;
	undefined field_0x6bc;
	undefined field_0x6bd;
	undefined field_0x6be;
	undefined field_0x6bf;
	edF32VECTOR4 field_0x6c0;
	edF32VECTOR4 field_0x6d0;
	edF32VECTOR4 field_0x6e0;
	struct CActor* field_0x6f0;
	bool field_0x6f4;
	bool field_0x6f5;
	bool field_0x6f6;
	undefined field_0x6f7;
	float field_0x6f8;
	float field_0x6fc;
	float field_0x700;
	int field_0x704;
	float field_0x708;
	float field_0x70c;
	bool field_0x710;
	bool field_0x711;
	undefined field_0x712;
	undefined field_0x713;
	undefined field_0x714;
	undefined field_0x715;
	undefined field_0x716;
	undefined field_0x717;
	undefined field_0x718;
	undefined field_0x719;
	undefined field_0x71a;
	undefined field_0x71b;
	undefined field_0x71c;
	undefined field_0x71d;
	undefined field_0x71e;
	undefined field_0x71f;
};

#endif //ACTOR_BLAZER_H