#include "ActorHero.h"

CActorHero* CActorHero::_gThis = (CActorHero*)0x0;

AnimResultHero CActorHero::_gStateCfg_HRO[HERO_STATE_COUNT] = {
	{ 0, 0, 0 },
	{ 0, 0x100, 0xC0401F },
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

CBehaviour* CActorFighter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 6) {
		pNewBehaviour = new CBehaviourFighterSlave;
	}
	else {
		if (behaviourType == 5) {
			pNewBehaviour = new CBehaviourFighterRidden;
		}
		else {
			if (behaviourType == 4) {
				pNewBehaviour = new CBehaviourFighterProjected;
			}
			else {
				if (behaviourType == 3) {
					new CBehaviourFighter;
				}
				else {
					pNewBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
				}
			}
		}
	}
	return pNewBehaviour;
}

bool CActorFighter::IsFightRelated(int param_2)
{
	bool bVar1;

	if ((param_2 - 3U < 3) || (param_2 == 6)) {
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}
	return bVar1;
}