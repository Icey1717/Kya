#ifndef ACTOR_SHOCKER_H
#define ACTOR_SHOCKER_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"
#include "CircularWaveShoot.h"
#include "CinematicManager.h"

#define SHOCKER_BEHAVIOUR_FIRE_WAVE 3
#define SHOCKER_STATE_SLEEP 6
#define SHOCKER_STATE_STAND_UP_1_3 7
#define SHOCKER_STATE_STAND_UP_2_3 8
#define SHOCKER_STATE_STAND_UP_3_3 9
#define SHOCKER_STATE_CHASE 10
#define SHOCKER_STATE_WAIT_COMMAND 11
#define SHOCKER_STATE_FIRE_WAVE_WIND_UP 12
#define SHOCKER_STATE_FIRE_WAVE_FALL 13
#define SHOCKER_STATE_FIRE_WAVE_LAND 14
#define SHOCKER_STATE_STAND_UP_TIRED_1_2 15
#define SHOCKER_STATE_STAND_UP_TIRED_2_2 16
#define SHOCKER_STATE_RECHARGE 17
#define SHOCKER_STATE_COME_BACK 19
#define SHOCKER_STATE_GO_TO_SLEEP_1_3 20
#define SHOCKER_STATE_GO_TO_SLEEP_2_3 21
#define SHOCKER_STATE_GO_TO_SLEEP_3_3 22
#define SHOCKER_STATE_DEATH 23
#define SHOCKER_STATE_DEATH_EXTRUDE 24
#define SHOCKER_STATE_DEATH_FADE 25

class CActorShocker;

class CBehaviourShocker : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);
	virtual void Reset();

	CActorShocker* pOwner;
};

class CBehaviourShockerFireWave : public CBehaviourShocker
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual void Reset();

	CCircularWaveShoot circularWaveShoot;
};

class CActorShocker : public CActorAutonomous
{
public:
	CActorShocker();
	~CActorShocker();
	static StateConfig _gStateCfg_SHK[20];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ComputeLighting();
	virtual bool Can_0x9c();
	virtual void ChangeVisibleState(int bVisible);
	virtual void ChangeDisplayState(int state);
	virtual void ChangeManageState(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void ClearLocalData();
	int CheckArea();
	void ComputeInvincibility();
	void ManageSparksBoomy();
	void FUN_003d11e0();
	void FUN_003d1b10();
	void FUN_003ce190();
	void FUN_003ce330();
	void BehaviourShockerFireWave_Manage(CBehaviourShockerFireWave* pBehaviour);
	void BehaviourShockerFireWave_InitState(int newState);
	void StateShockerStandUp_1_3(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerStandUp_2_3(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerStandUp_3_3(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerChase(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerStandUpTired_1_2(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerStandUpTired_2_2(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerRecharge();
	void StateShockerComeBack(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerGoToSleep_1_3(CBehaviourShockerFireWave* pBehaviour);
	void StateShockerGoToSleep_2_3(CBehaviourShockerFireWave* pBehaviour);

	float field_0x350;
	float field_0x354;
	float field_0x358;
	float field_0x35c;
	float field_0x360;
	float field_0x364;
	float field_0x368;
	float field_0x36c;
	int field_0x370;
	int field_0x374;
	S_NTF_SWITCH_ONOFF field_0x378;
	uint field_0x380;
	int field_0x384;
	int field_0x388;
	int field_0x38c;
	int field_0x390;
	int field_0x394;
	int field_0x398;
	uint boneId_0x39c;
	CActor* field_0x3a0;
	CActor* field_0x3a4;
	byte field_0x3a8;
	bool field_0x3a9;
	bool field_0x3aa;
	float field_0x3ac;
	edF32VECTOR4 lightAmbient;
	edF32MATRIX4 lightDirections;
	edF32MATRIX4 lightColors;
	ed_3D_Light_Config field_0x440;
	CFxHandle field_0x454;
	CFxHandle field_0x45c;
	CFxHandle field_0x464;
	CFxHandle field_0x46c;
	bool field_0x474;
	CFxSparkNoAlloc<2, 12> fxSpark;
	int field_0x740;
	float field_0x744;
	ed_3d_hierarchy_setup field_0x748;
	edF32VECTOR4 boundingSphere;
	float clipping;
	ed_3d_hierarchy* field_0x77c;
	edNODE* field_0x780;
	edF32VECTOR4 field_0x784;
	edF32VECTOR4 field_0x794;
	int field_0x7a4;
	int field_0x7a8;
	int field_0x7ac;
	int field_0x7b0;

	CAddOnGenerator addOnGenerator;
	CBehaviourShockerFireWave behaviourShockerFireWave;
};

#endif //ACTOR_SHOCKER_H
