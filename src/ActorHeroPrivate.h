#ifndef ACTOR_HERO_PRIVATE_H
#define ACTOR_HERO_PRIVATE_H

#include "Types.h"
#include "Actor.h"
#include "ActorHero.h"

class CBehaviourHero : public CBehaviourFighter
{

};

class CBehaviourHeroDefault : public CBehaviourHero
{
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	class CActorHeroPrivate* field_0xc;
};

class CBehaviourRideJamGut : public CBehaviour
{

};

class CBehaviourHeroRideJamGut : public CBehaviourRideJamGut
{

};

class CActorHeroPrivate : public CActorHero 
{
public:
	CActorHeroPrivate() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void SetState(int newState, int animType);
	virtual void CinematicMode_Leave(int behaviourId);

	int StateEvaluate();

	CBehaviourHeroDefault behaviourHeroDefault;

	CBehaviour behaviour_0x1c50;
	CBehaviour behaviour_0x1e10;
	CBehaviour behaviour_0x1fd0;
};

#endif //ACTOR_HERO_PRIVATE_H