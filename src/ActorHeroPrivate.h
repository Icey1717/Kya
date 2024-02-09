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

	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	class CActorHeroPrivate* pHero;
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
	CActorHeroPrivate();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void SetState(int newState, int animType);
	virtual void CinematicMode_Leave(int behaviourId);

	// Autonomous
	virtual float ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);
	virtual void StoreCollisionSphere();

	void ResetStdDefaultSettings();
	void ResetBoomyDefaultSettings();

	int StateEvaluate();

	void ClearLocalData();

	void BehaviourHero_InitState(int newState);
	void BehaviourHero_TermState(int oldState, int newState);
	void BehaviourHero_Manage();

	void StateHeroStandInit(int param_2);
	void StateHeroStand(int param_2);

	CBehaviourHeroDefault behaviourHeroDefault;

	CBehaviour behaviour_0x1c50;
	CBehaviour behaviour_0x1e10;
	CBehaviour behaviour_0x1fd0;

	float field_0xa80;
	float field_0xa84;
	float field_0xa88;

	CActor* field_0xf54;

	float field_0x105c;

	float animKey_0x157c;
};

#endif //ACTOR_HERO_PRIVATE_H