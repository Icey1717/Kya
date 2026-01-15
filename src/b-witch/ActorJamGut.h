#ifndef ACTOR_JAMGUT_H
#define ACTOR_JAMGUT_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "Fx.h"

#define JAMGUT_BEHAVIOUR_STAND 3
#define JAMGUT_BEHAVIOUR_RIDDEN 4

#define JAMGUT_STAND_STATE_STAND 6
#define JAMGUT_STAND_STATE_REJECT_HERO_A 0x19
#define JAMGUT_STAND_STATE_REJECT_HERO_B 0x1a

#define JAMGUT_CMD_RECEPT 1
#define JAMGUT_CMD_JUMP 5
#define JAMGUT_CMD_ATTACK 8
#define JAMGUT_CMD_WALK 0xd

class CActorHero;
class CActorJamGut;
class CMusic;

struct S_JMG_PATH
{
	S_STREAM_REF<ed_zone_3d> zoneRef;
	CPathPlane pathPlane;
	S_STREAM_REF<CActor> actorRef;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
	CPathFollowReaderAbsolute pathFollowReader;
};

struct S_JMG_CHECKPOINT
{
	S_STREAM_REF<ed_zone_3d> zoneRef;
	S_STREAM_REF<CWayPoint> waypointRef;
};

class CBehaviourJamGut : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorJamGut* pOwner;
};

class CBehaviourJamGutStand : public CBehaviourJamGut
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourJamGutRidden : public CBehaviourJamGut
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	undefined4 field_0x8;
	int field_0xc;
	CMusic* field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
};

struct StateConfigRider : public StateConfig
{
	StateConfigRider(int inA, uint inB, int inC) :
		StateConfig(inA, inB), field_0x8(inC)
	{
	}

	int field_0x8;
};

class CActorJamGut : public CActorAutonomous
{
public:
	static StateConfigRider gStateCfg_JMG[21];

	virtual void Create(ByteCode* pByteCode);

	virtual void Init();

	virtual void Manage();
	virtual void ComputeLighting();

	virtual void Reset();
	virtual void CheckpointReset();

	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	virtual StateConfig* GetStateCfg(int state);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual uint GetBehaviourFlags(int state);

	virtual void SetState(int newState, int animType);

	virtual bool CanPassThrough();

	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);

	virtual void CinematicMode_Leave(int behaviourId);

	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);

	virtual CPlayerInput* GetInputManager(int, int);

	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	// CActorMovable Interface
	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	// CActorAutonomous Interface
	virtual CActorWindState* GetWindState();
	virtual float GetRunSpeed();

	// CActorJamGut
	void ClearLocalData();

	void ManagePaths();
	void ManageCheckpoints();

	float ComputeDistanceToPath(edF32VECTOR4* param_2, CPathPlane* param_3, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, float* param_7);

	void FUN_00377030();
	void FUN_003767d0(float param_1, float param_2, float param_3, float param_4, float param_5,
		int param_7, edF32VECTOR4* param_8);

	int GetHeroAction(float param_1, CActorHero* pHero);
	int AccomplishHit(CActor* pActor, CActor* pSender, _msg_hit_param* pParams, edF32VECTOR4* param_5);

	uint GetStateRider(int state);
	int ChooseStateDead(int param_2, int param_3, int param_4);
	int ChooseStateWind(float param_1, edF32VECTOR4* param_3);
	bool CheckHitAndDeath(CActorHero* pHero);

	void BehaviourJamGutStand_Manage(CBehaviourJamGutStand* pBehaviour);
	void BehaviourJamGutRidden_Manage(CBehaviourJamGutRidden* pBehaviour);

	void StateJamGutStand(float minTime, CBehaviourJamGutStand* pBehaviour, CActor* param_4, int nextState);
	void StateJamGutRideStand(CBehaviourJamGutRidden* pBehaviour, int param_3, int param_4);

	void FUN_00375fe0();
	void FUN_00364810(float param_1, float param_2, float param_3);

	int RiderCmdAttack();
	int RiderCmdJump();
	int RiderCmdRecept();
	int RiderCmdWalk();
	int RiderCmdRun();
	int RiderCmdNew();

	edF32VECTOR4* RiderGetForce();

	void RiderSetState(int state, int anim);

	void UpdatePercentLeftRight(float param_1, float param_2);

	bool FUN_00376710();

	CActorHero* field_0x350;
	CActorHero* field_0x354;
	uint flags_0x358;
	undefined4 field_0x35c;
	int field_0x360;
	int field_0x364;
	CFxHandleExt field_0x368;
	uint field_0x374;
	float field_0x378;
	float field_0x37c;
	float field_0x380;
	uint field_0x384;
	edF32VECTOR4 field_0x390;
	int field_0x3a0;
	float field_0x3a4;
	float field_0x3a8;

	CActorWindState field_0x3b0;

	float field_0x400;
	float field_0x404;
	float field_0x408;
	float field_0x40c;
	float field_0x410;
	float field_0x414;
	float field_0x418;
	float field_0x41c;
	edF32VECTOR4 field_0x430;

	float field_0x444;
	float field_0x448;

	edF32VECTOR4 field_0x450;
	edF32VECTOR4 field_0x470;

	float field_0x480;
	float field_0x484;
	float field_0x488;
	float field_0x48c;
	float field_0x490;
	float field_0x494;

	float field_0x4a0;
	float field_0x4a4;
	float field_0x4a8;
	float field_0x4ac;
	float field_0x4b0;
	float field_0x4b4;

	int field_0x4ec;

	float field_0x4f0;
	float field_0x4f4;
	float field_0x4fc;

	CLightConfig lightConfig;
	int field_0x500;

	edF32VECTOR4 field_0x5b0;
	edF32VECTOR3 field_0x5c0;
	int field_0x5cc;

	edF32VECTOR4 field_0x5d0;
	edF32VECTOR3 field_0x5e0;
	int field_0x5ec;

	edF32VECTOR4 field_0x5f0;
	edF32VECTOR3 field_0x600;
	int field_0x60c;

	int field_0x618;
	uint field_0x61c;
	undefined4 field_0x620;
	int field_0x62c;
	edF32VECTOR4 field_0x630;
	float field_0x640;
	int field_0x644;
	int field_0x648;
	float field_0x64c;
	float field_0x650;
	CActor* field_0x654;
	CPathPlaneOutData field_0x658;

	int nbPaths;
	S_JMG_PATH* aPaths;

	int nbCheckpoints;
	S_JMG_CHECKPOINT* aCheckpoints;

	CBehaviourJamGutRidden behaviourRidden;
	CBehaviourJamGutStand behaviourStand;
};

#endif //ACTOR_JAMGUT_H