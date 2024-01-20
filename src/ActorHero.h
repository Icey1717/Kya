#ifndef _ACTOR_HERO_H
#define _ACTOR_HERO_H

#include "Types.h"
#include "Actor.h"

class CActorFighter : public CActorAutonomous
{
public:
	CActorFighter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual bool IsFightRelated(int param_2);
};

class CBehaviourFighter : public CBehaviour
{

};

class CBehaviourFighterSlave : public CBehaviourFighter
{

};

class CBehaviourFighterRidden : public CBehaviourFighter
{

};

class CBehaviourFighterProjected : public CBehaviourFighter
{

};

struct AnimResultHero : public AnimResult {
	uint heroFlags;
};

#define HERO_STATE_COUNT 2
#define HERO_BHVR_COUNT 14

class CActorHero : public CActorFighter
{
public:
	static CActorHero* _gThis;

	uint heroFlags;

	int field_0x1610;
	int field_0x18dc;
	int field_0xee0;

	float time_0x1538;
	float time_0x153c;

	static AnimResultHero _gStateCfg_HRO[HERO_STATE_COUNT];
	static uint _gStateCfg_ELE[HERO_BHVR_COUNT];

	virtual AnimResult* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);

	uint GetStateHeroFlags(int state);

	uint TestState_IsInHit(uint inFlags);
	uint TestState_IsOnAToboggan(uint inFlags);
};


#endif // _ACTOR_HERO_H
