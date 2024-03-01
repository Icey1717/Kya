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

	void StateHeroStandInit(int bCheckEffort);
	void StateHeroStandTerm();
	void StateHeroStand(int bCheckEffort);

	inline void StateHeroRunTerm() {}
	void StateHeroRun();

	void StateHeroSlideSlipInit();
	void StateHeroSlideSlip(int nextState, bool boolA, bool boolB);

	void StateHeroJump_1_3(int nextState);

	void StateHeroJump_2_3Init();
	void StateHeroJump_2_3(int param_2, int param_3, int param_4);
	void StateHeroJump_2_3Term();

	void StateHeroJump_3_3(int param_2);

	void SetJumpCfg(float param_1, float horizonalSpeed, float param_3, float param_4, float param_5, edF32VECTOR4* param_8);
	void SetBoomyFunc(int param_2);
	void IncreaseEffort(float param_1);

	void BuildHorizontalSpeedVector(float runSpeed, float param_2, float param_3, float param_4, float param_5);
	void ConvertSpeedSumForceExtToSpeedPlayer2D();

	void MoveInAir(float param_1, float param_2, float param_3, float param_4, float param_5);

	CBehaviourHeroDefault behaviourHeroDefault;

	CBehaviour behaviour_0x1c50;
	CBehaviour behaviour_0x1e10;
	CBehaviour behaviour_0x1fd0;

	float field_0xa80;
	float field_0xa84;
	float field_0xa88;

	undefined4 field_0x1a50;

	edF32VECTOR4 field_0xf00;

	CActor* field_0xf54;
	CActor* field_0xf58;

	int field_0x1a4c;

	float field_0x1048;

	uint field_0x15a0;

	float slideSlipIntensity;

	edF32VECTOR4 controlDirection;

	uint boomyState_0x1b70;

	float field_0x1b9c;

	int field_0x1020;

	int field_0x142c;
	bool field_0x1454;

	float field_0x116c;
	float field_0x1170;

	byte field_0x1455;
	int field_0x1168;

	// Run settings.
	float field_0x1068;
	float field_0x1074;
	float field_0x107c;

	// Standard settings.
	float field_0x1040;
	float field_0x104c;
	float field_0x1044;
	float runSpeed;
	float field_0x1054;
	float field_0x1058;
	float field_0x105c;
	float field_0x1060;
	float field_0x1184;
	float field_0x1574;

	float animKey_0x157c;

	float field_0x1160;
	float field_0x1164;

	CActor* field_0xf18;
	CActor* pSoccerActor;

	undefined4 field_0xcbc;

	CLifeInterface field_0xee4;

	CCamera* pMainCamera;

	int field_0xe3c;

	float jmp_field_0x1130;
	float jmp_field_0x1134;
	float jmp_field_0x1138;
	float jmp_field_0x113c;
	float jmp_field_0x1140;

	float field_0x1150;
	float field_0x1154;
	float field_0x1158;

	int field_0x1a10;

	undefined4 jmp_field_0x1144 = 0;
	edF32VECTOR4 jmp_field_0x1120;
};

#endif //ACTOR_HERO_PRIVATE_H