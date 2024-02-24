#include "ActorHero.h"

CActorHero* CActorHero::_gThis = (CActorHero*)0x0;

AnimResultHero CActorHero::_gStateCfg_HRO[HERO_STATE_COUNT] = {
	AnimResultHero( 0, 0, 0 ),
	AnimResultHero( 0, 0x100, 0xC0401F ),
	AnimResultHero( 0x83, 0x100, 0xC0401F ),
	AnimResultHero( 0x84, 0x100, 0xC0401F ),
	AnimResultHero( 0xfe, 0x102, 0xC4401F ),
};

uint CActorHero::_gStateCfg_ELE[HERO_BHVR_COUNT] = {
	0, 0, 0, 0, 0, 0, 
	0, 0x768, 0x3e0, 0x0,
	0, 0, 0, 0
};

AnimResult* CActorHero::GetStateCfg(int state)
{
	AnimResultHero* pHeroStateCfg;

	if (state < 0x72) {
		pHeroStateCfg = (AnimResultHero*)CActorFighter::GetStateCfg(state);
	}
	else {
		assert(state - 0x72 < HERO_STATE_COUNT);
		pHeroStateCfg = _gStateCfg_HRO + state - 0x72;
	}
	return (AnimResult*)pHeroStateCfg;
}

uint CActorHero::GetBehaviourFlags(int state)
{
	uint bhvrFlags;

	if (state < 7) {
		bhvrFlags = CActorFighter::GetBehaviourFlags(state);
	}
	else {
		bhvrFlags = _gStateCfg_ELE[state];
	}
	return bhvrFlags;
}

uint CActorHero::GetStateHeroFlags(int state)
{
	uint uVar1;

	uVar1 = 0;
	if (state != AS_None) {
		if (state < 0x72) {
			uVar1 = 0;
		}
		else {
			assert(state - 0x72 < HERO_STATE_COUNT);
			uVar1 = _gStateCfg_HRO[state - 0x72].heroFlags;
		}
	}
	return uVar1;
}

uint CActorHero::TestState_IsInHit(uint inFlags)
{
	EActorState currentState;

	if (inFlags == 0xffffffff) {
		currentState = this->actorState;
		inFlags = 0;
		if ((currentState != AS_None) && (inFlags = 0, 0x71 < (int)currentState)) {

			assert(currentState - 0x72 < HERO_STATE_COUNT);

			inFlags = _gStateCfg_HRO[currentState - 0x72].heroFlags;
		}
	}
	return inFlags & 0x80000;
}

uint CActorHero::TestState_IsOnAToboggan(uint inFlags)
{
	EActorState currentState;

	if (inFlags == 0xffffffff) {
		currentState = this->actorState;
		inFlags = 0;
		if ((currentState != AS_None) && (inFlags = 0, 0x71 < (int)currentState)) {

			assert(currentState - 0x72 < HERO_STATE_COUNT);

			inFlags = _gStateCfg_HRO[currentState - 0x72].heroFlags;
		}
	}
	return inFlags & 0x8000;
}

