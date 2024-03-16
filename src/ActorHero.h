#ifndef _ACTOR_HERO_H
#define _ACTOR_HERO_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"

#define STATE_HERO_STAND 0x73
#define STATE_HERO_RUN 0x76
#define STATE_HERO_JUMP_1_1_STAND 0x78
#define STATE_HERO_JUMP_2_3_STAND 0x79
#define STATE_HERO_JUMP_3_3_STAND 0x7a

#define STATE_HERO_JUMP_1_1_RUN 0x7b
#define STATE_HERO_JUMP_2_3_RUN 0x7c
#define STATE_HERO_JUMP_3_3_RUN 0x7d

#define STATE_HERO_SLIDE_SLIP_A 0xe1
#define STATE_HERO_SLIDE_SLIP_B 0xe2
#define STATE_HERO_SLIDE_SLIP_C 0xe3

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
};

extern ulong gBoomyHashCodes[4];

#endif // _ACTOR_HERO_H
