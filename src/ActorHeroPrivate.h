#ifndef ACTOR_HERO_PRIVATE_H
#define ACTOR_HERO_PRIVATE_H

#include "Types.h"
#include "Actor.h"
#include "ActorHero.h"

class CCamera;
class CActorBoomy;

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
	virtual void Manage();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void SetState(int newState, int animType);
	virtual void AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual void CinematicMode_Leave(int behaviourId);

	// Autonomous
	virtual float ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);
	virtual void StoreCollisionSphere();
	virtual CLifeInterface* GetLifeInterface();

	void ResetStdDefaultSettings();
	void ResetSlideDefaultSettings();
	void ResetBoomyDefaultSettings();

	int StateEvaluate();
	bool CanEnterToboggan();

	void ClearLocalData();

	void BehaviourHero_InitState(int newState);
	void BehaviourHero_TermState(int oldState, int newState);
	void BehaviourHero_Manage();

	void StateHeroStandInit(int param_2);
	void StateHeroStandTerm();
	void StateHeroStand(int param_2);

	void StateHeroRun();

	void SetBoomyFunc(int param_2);
	void IncreaseEffort(float param_1);

	void BuildHorizontalSpeedVector(float runSpeed, float param_2, float param_3, float param_4, float param_5);

	CBehaviourHeroDefault behaviourHeroDefault;

	CBehaviour behaviour_0x1c50;
	CBehaviour behaviour_0x1e10;
	CBehaviour behaviour_0x1fd0;

	float field_0xa80;
	float field_0xa84;
	float field_0xa88;

	CActor* field_0xf54;

	int field_0x1a4c;

	float field_0x1048;

	edF32VECTOR4 controlDirection;

	uint boomyState_0x1b70;

	float field_0x1b9c;

	int field_0x1020;

	int field_0x142c;
	bool field_0x1454;

	// Run settings.
	float field_0x1068;
	float field_0x1074;
	float field_0x107c;

	// Standard settings.
	float field_0x1040;
	float field_0x104c;
	float runSpeed;
	float field_0x1054;
	float field_0x1058;
	float field_0x105c;
	float field_0x1184;
	float field_0x1574;

	float animKey_0x157c;

	CActor* pSoccerActor;

	undefined4 field_0xcbc;

	CLifeInterface field_0xee4;

	CCamera* pMainCamera;

	int field_0xe3c;
};

#endif //ACTOR_HERO_PRIVATE_H