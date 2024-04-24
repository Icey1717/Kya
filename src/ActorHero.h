#ifndef _ACTOR_HERO_H
#define _ACTOR_HERO_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"

#define SCENE_VAR_BOUNCE_JUMP 0x7
#define SCENE_VAR_MAX_HEALTH 0x14

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

#define STATE_HERO_ROLL 0x8a

#define STATE_HERO_GRIP_B 0xbc
#define STATE_HERO_GRIP_C 0xbd
#define STATE_HERO_GRIP_HANG_IDLE 0xbf

#define STATE_HERO_GRIP_UP 0xc6
#define STATE_HERO_JUMP_2_3_GRIP 0xc7

#define STATE_HERO_GRIP_GRAB 0xca

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

#define STATE_HERO_GLIDE 0xf0

struct CPlayerInput;
class CActorBoomy;

struct AnimResultHero : public AnimResult {
	AnimResultHero(int inA, uint inB, uint inC)
		: AnimResult(inA, inB)
		, heroFlags(inC)
	{}

	uint heroFlags;
};

#define HERO_STATE_COUNT 200
#define HERO_BHVR_COUNT 14

#define ACTOR_HERO_LOG(level, format, ...) MY_LOG_CATEGORY("ActorHero", level, format, ##__VA_ARGS__)

class CActorHero : public CActorFighter
{
public:
	static CActorHero* _gThis;

	uint heroFlags;

	int lastCheckPointSector;
	int field_0xea0;

	int bCanUseCheats;
	int field_0xaa4;

	int field_0x1610;
	int field_0x18dc;
	int field_0xee0;

	float time_0x1538;
	float time_0x153c;

	int bFacingControlDirection;
	int field_0xff4;
	int animIdleSequenceIndex;
	float effort;

	char* field_0xf24;

	CPlayerInput* pPlayerInput;

	CActorBoomy* pActorBoomy;

	// Hero goes at least up to 0x1548 given CCameraStack::GetCurHeroState
	float field_0x1548;
	float field_0xa80;
	float field_0xa84;
	float field_0xa88;

	static AnimResultHero _gStateCfg_HRO[HERO_STATE_COUNT];
	static uint _gStateCfg_ELE[HERO_BHVR_COUNT];

	virtual AnimResult* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);

	uint GetStateHeroFlags(int state);

	uint TestState_IsInHit(uint inFlags);
	uint TestState_IsOnAToboggan(uint inFlags);
	uint TestState_IsGrippedOrClimbing(uint inFlags);
	bool TestState_IsInCheatMode();
	uint TestState_IsInTheWind(uint inFlags);
	uint TestState_IsFlying(uint inFlags);
	uint TestState_IsCrouched(uint inFlags);
	uint TestState_IsOnCeiling(uint inFlags);

	bool FUN_0014cb60(edF32VECTOR4* v0);
};

extern ulong gBoomyHashCodes[4];

#endif // _ACTOR_HERO_H
