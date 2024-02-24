#ifndef _ACTOR_HERO_H
#define _ACTOR_HERO_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"

#define STATE_HERO_STAND 0x73
#define STATE_HERO_RUN 0x76

struct CPlayerInput;
class CActorBoomy;

struct AnimResultHero : public AnimResult {
	AnimResultHero(int inA, uint inB, uint inC)
		: AnimResult(inA, inB)
		, heroFlags(inC)
	{}

	uint heroFlags;
};

#define HERO_STATE_COUNT 5
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
};


#endif // _ACTOR_HERO_H
