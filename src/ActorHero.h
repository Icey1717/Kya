#ifndef _ACTOR_HERO_H
#define _ACTOR_HERO_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"
#include "ActorHero_Inventory.h"


#define SCENE_VAR_BOUNCE_JUMP 0x7
#define SCENE_VAR_CLIMB 0x8
#define SCENE_VAR_BOOMY 0x9
#define SCENE_VAR_MAX_HEALTH 0x14

#define STATE_HERO_NONE -1

#define STATE_HERO_STAND 0x73
#define STATE_HERO_RUN 0x76
#define STATE_HERO_JUMP_1_1_STAND 0x78
#define STATE_HERO_JUMP_2_3_STAND 0x79
#define STATE_HERO_JUMP_3_3_STAND 0x7a

#define STATE_HERO_JUMP_1_1_RUN 0x7b
#define STATE_HERO_JUMP_2_3_RUN 0x7c
#define STATE_HERO_JUMP_3_3_RUN 0x7d

#define STATE_HERO_FALL_A 0x7e
#define STATE_HERO_FALL_B 0x7f

#define STATE_HERO_FALL_BOUNCE_1_2 0x81
#define STATE_HERO_FALL_BOUNCE_2_2 0x82

#define STATE_HERO_STAND_TO_CROUCH_A 0x83
#define STATE_HERO_STAND_TO_CROUCH_B 0x84

#define STATE_HERO_CROUCH_A 0x85
#define STATE_HERO_CROUCH_WALK_A 0x86
#define STATE_HERO_CROUCH_B 0x88
#define STATE_HERO_JUMP_TO_CROUCH 0x89
#define STATE_HERO_ROLL 0x8a
#define STATE_HERO_CROUCH_C 0x8b
#define STATE_HERO_ROLL_2_CROUCH 0x8c
#define STATE_HERO_ROLL_FALL 0x8d

#define STATE_HERO_HURT_A 0x8e
#define STATE_HERO_HURT_B 0x8f

#define STATE_HERO_WIND_WALL_HURT 0x93
#define STATE_HERO_WIND_SLIDE_HURT 0x94
#define STATE_HERO_TOBOGGAN_JUMP_HURT 0x95
#define STATE_HERO_COL_WALL 0x96

#define STATE_HERO_FALL_DEATH 0xa1

#define STATE_HERO_KICK_A 0xa5
#define STATE_HERO_KICK_B 0xa6
#define STATE_HERO_KICK_C 0xa7

#define STATE_HERO_GRIP_B 0xbc
#define STATE_HERO_GRIP_C 0xbd
#define STATE_HERO_GRIP_HANG_IDLE 0xbf

#define STATE_HERO_GRIP_LEFT 0xc0
#define STATE_HERO_GRIP_RIGHT 0xc1
#define STATE_HERO_GRIP_UP 0xc6
#define STATE_HERO_JUMP_2_3_GRIP 0xc7

#define STATE_HERO_GRIP_GRAB 0xca

#define STATE_HERO_JOKE 0xe0

#define STATE_HERO_SLIDE_SLIP_A 0xe1
#define STATE_HERO_SLIDE_SLIP_B 0xe2
#define STATE_HERO_SLIDE_SLIP_C 0xe3

#define STATE_HERO_SLIDE_A 0xe4
#define STATE_HERO_SLIDE_B 0xe5

#define STATE_HERO_U_TURN 0xe7

#define STATE_HERO_TOBOGGAN_3 0xe8
#define STATE_HERO_TOBOGGAN_JUMP_1 0xea
#define STATE_HERO_TOBOGGAN_JUMP_2 0xeb
#define STATE_HERO_TOBOGGAN_2 0xed
#define STATE_HERO_TOBOGGAN 0xee

#define STATE_HERO_GLIDE_1 0xf0
#define STATE_HERO_GLIDE_2 0xf1
#define STATE_HERO_GLIDE_3 0xf2

#define STATE_HERO_WIND_CANON 0xf3
#define STATE_HERO_WIND_CANON_B 0xf4

#define STATE_HERO_WIND_FLY 0xf5
#define STATE_HERO_WIND_SLIDE 0xf6
#define STATE_HERO_WIND_WALL_MOVE_A 0xf7
#define STATE_HERO_WIND_WALL_MOVE_B 0xf8
#define STATE_HERO_WIND_WALL_MOVE_E 0xf9
#define STATE_HERO_WIND_WALL_MOVE_F 0xfa
#define STATE_HERO_WIND_WALL_MOVE_C 0xfb
#define STATE_HERO_WIND_WALL_MOVE_D 0xfc
#define STATE_HERO_GRIP_UP_A 0xfd
#define STATE_HERO_GRIP_UP_B 0xfe
#define STATE_HERO_WIND_WALL_MOVE_JUMP 0xff
#define STATE_HERO_WIND_FLY_B 0x100
#define STATE_HERO_WIND_FLY_C 0x101

#define STATE_HERO_TRAMPOLINE_JUMP_1_2_A 0x10b
#define STATE_HERO_TRAMPOLINE_JUMP_2_3 0x10c
#define STATE_HERO_TRAMPOLINE_JUMP_1_2_B 0x10d

#define STATE_HERO_TRAMPOLINE_STOMACH_TO_FALL 0x10e

#define STATE_HERO_LOOK_INTERNAL 0x11a

#define STATE_HERO_CAUGHT_TRAP_1 0x117
#define STATE_HERO_CAUGHT_TRAP_2 0x118

#define HERO_BEHAVIOUR_DEFAULT 0x7

struct CPlayerInput;
class CActorBoomy;
class CCamera;
class CActorNativ;

struct AnimResultHero : public StateConfig {
	AnimResultHero(int inA, uint inB, uint inC)
		: StateConfig(inA, inB)
		, heroFlags(inC)
	{}

	uint heroFlags;
};

#define HERO_STATE_COUNT 194
#define HERO_BHVR_COUNT 14

#define ACTOR_HERO_LOG(level, format, ...) MY_LOG_CATEGORY("ActorHero", level, format, ##__VA_ARGS__)

class CMagicInterface : public CInterface
{
public:
	// CInterface
	virtual bool Activate(int bActive);
	virtual bool CanActivate();
	virtual bool IsActive();
	virtual bool Manage();
	virtual void Draw();
	virtual void Reset() {}
	virtual void SetValue(float value);
	virtual float GetValue();

	float GetValueMax();
	void SetValueMax(float max);

	void SetTransit(float transit);

	void FUN_001dcd70();

	void SetHasMagicInteractionAround(int bHasMagicAround);

	int bActive;
	int bHasMagicAround;
	float field_0xc;
	float field_0x10;
	float value;
	float valueMax;
	float transit;
};

class CMoneyInterface : public CInterface
{
public:
	// CInterface
	virtual bool Manage();
	virtual void Draw() {}
	virtual void Reset() {}
	virtual void SetValue(float value) {}
	virtual float GetValue();
};

struct BounceParams
{
	edF32VECTOR4 field_0x0;
	float field_0x10;
};

struct HeroActionStateCfg
{
	uint field_0x0;
};

struct _evt_checkpoint_param
{
	CWayPoint* pWayPointA;
	int sectorId;
	CWayPoint* pWayPointB;
	int flags;
};

class CActorHero : public CActorFighter
{
public:
	static CActorHero* _gThis;

	uint heroFlags;

	int bCanUseCheats;
	int field_0xaa4;

	int field_0x1610;
	int field_0x18dc;

	int field_0xcc0;
	int bIsSettingUp;

	edF32VECTOR4 field_0xeb0;
	edF32VECTOR4 field_0xec0;
	int field_0xed0;

	int* field_0x14a4;

	// Bones
	uint field_0x157c;
	uint animKey_0x1584;
	uint animKey_0x1588;
	uint fxGlideBoneA;
	uint fxGlideBoneB;
	uint fxGlideBoneC;
	uint fxGlideBoneD;
	uint field_0x1598;
	uint braceletBone;

	uint medallionBone;

	float time_0x1538;
	float time_0x153c;
	float field_0x1540;

	int bFacingControlDirection;
	int field_0xff4;
	int animIdleSequenceIndex;
	float effort;

	CActorNativ* pTalkingToActor;

	CPlayerInput* pPlayerInput;

	CActorBoomy* pActorBoomy;

	edF32VECTOR4 field_0xe50;
	edF32VECTOR3 field_0xe60;
	int levelDataField1C_0xe6c;
	int lastCheckPointSector;

	byte field_0xd34[16][16];

	S_ZONE_STREAM_REF* field_0xe48;

	edF32VECTOR4 field_0xe80;
	edF32VECTOR3 field_0xe90;
	int levelDataField1C_0xe9c;
	int field_0xea0;

	// Hero goes at least up to 0x1558 given CanActivateCheckpoint
	float field_0x1544;
	float field_0x1548;
	float field_0x1550;
	float field_0x1554;
	float field_0xa80;
	float field_0xa84;
	float field_0xa88;

	int field_0x1420;

	int field_0x1018;

	float field_0x1558;

	int field_0x1a54;

	CInventoryInterface inventory;
	CMagicInterface magicInterface;
	CMoneyInterface moneyInterface;

	CCamera* pMainCamera;
	CCamera* pCameraViewBase_0x15b0;
	CCamera* pWindWallCamera_0x15b4;
	CCamera* pCameraKyaJamgut;
	CCamera* pFightCamera;

	CCamera* pDeathCamera;

	CCamera* pJamgutCamera_0x15b8;
	CCamera* pIntViewCamera_0x15bc;

	int braceletLevel;
	uint field_0x1874;
	uint field_0x1878;

	static AnimResultHero _gStateCfg_HRO[HERO_STATE_COUNT];
	static uint _gStateCfg_ELE[HERO_BHVR_COUNT];
	static HeroActionStateCfg _gActionCfg_HRO[16];

	~CActorHero();

	// CActor
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);

	// CActorFighter
	virtual edF32VECTOR4* GetAdversaryPos();

	int GetLastCheckpointSector();
	bool CanActivateCheckpoint(uint flags);
	void ActivateCheckpoint(_evt_checkpoint_param* pEventCheckpointParam);

	edF32VECTOR4* GetPosition_00369c80();

	uint GetStateHeroFlags(int state);

	HeroActionStateCfg* GetActionCfg(int index);

	uint TestState_IsInHit(uint inFlags);
	uint TestState_CanTrampo(uint inFlags);
	uint TestState_IsOnAToboggan(uint inFlags);
	uint TestState_IsGrippedOrClimbing(uint inFlags);
	bool TestState_IsInCheatMode();
	uint TestState_IsInTheWind(uint inFlags);
	uint TestState_BounceWalls(uint inFlags);
	uint TestState_AllowAction(uint inFlags);
	uint TestState_IsSliding(uint inFlags);
	uint TestState_IsFlying(uint inFlags);
	uint TestState_NoMoreHit(uint inFlags);
	uint TestState_IsCrouched(uint inFlags);
	uint TestState_AllowMagic(uint inFlags);
	uint TestState_IsOnCeiling(uint inFlags);
	uint TestState_IsGripped(uint inFlags);
	uint TestState_AllowAttack(uint inFlags);
	uint TestState_001328a0(uint inFlags);
	uint TestState_00132830(uint inFlags);
	uint TestState_00132b90(uint inFlags);
	uint TestState_CheckFight(uint inFlags);
	uint TestState_AllowFight(uint inFlags);
	uint TestState_CanPlaySoccer(uint inFlags);
	uint TestState_WindWall(uint inFlags);
	uint TestState_AllowInternalView(uint inFlags);

	bool FUN_0014cb60(edF32VECTOR4* v0);
	bool FUN_0031c9e0();

	float GetMagicalForce();
};

extern ulong gBoomyHashCodes[4];

#endif // _ACTOR_HERO_H
