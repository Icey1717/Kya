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

	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

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

class CFxHandle
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
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	// Autonomous
	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);
	virtual CActorWindState* GetWindState();
	virtual void StoreCollisionSphere();
	virtual CLifeInterface* GetLifeInterface();
	virtual void LifeDecrease(float amount);

	void ResetStdDefaultSettings();
	void ResetSlideDefaultSettings();
	void ResetBoomyDefaultSettings();
	void ResetGripClimbDefaultSettings();
	void ResetWindDefaultSettings();

	int StateEvaluate();
	int ChooseStateFall(int param_2);
	int ChooseStateLanding(float speed);

	bool CanEnterToboggan();

	void ClearLocalData();

	void BehaviourHero_InitState(int newState);
	void BehaviourHero_TermState(int oldState, int newState);
	void BehaviourHero_Manage();

	void StateHeroStandInit(int bCheckEffort);
	void StateHeroStandTerm();
	void StateHeroStand(int bCheckEffort);

	void StateHeroTobogganInit();
	void StateHeroToboggan(int param_2);

	void StateHeroTobogganJump(int param_2, int param_3, int param_4, int nextState);

	inline void StateHeroRunTerm() {}
	void StateHeroRun();

	void StateHeroSlideSlipInit();
	void StateHeroSlideSlip(int nextState, bool boolA, bool boolB);

	void StateHeroSlideInit(int param_2);
	void StateHeroSlide(int param_2);
	void StateHeroSlideTerm(int param_2);

	void StateHeroUTurnInit();
	void StateHeroUTurn();
	void StateHeroUTurnTerm();

	void StateHeroRollInit();
	void StateHeroRoll();
	void StateHeroRollTerm() {}

	void StateHeroJump_1_3(int nextState);

	void StateHeroJump_2_3Init();
	void StateHeroJump_2_3(int param_2, int param_3, int param_4);
	void StateHeroJump_2_3Term();

	void StateHeroJump_3_3(int param_2);

	void StateHeroGrip(float param_1, int nextState, int param_4);

	void StateHeroGripUp(float param_1, float param_2, int nextState, int param_5);

	void StateHeroGripUpToJumpInit();

	void StateHeroFall(float param_1, int param_3);

	void StateHeroFlyInit();

	void StateHeroGlideInit();
	void StateHeroGlide(int param_2, int nextState);

	void SetJumpCfgFromGround(float param_1);
	void SetJumpCfg(float param_1, float horizonalSpeed, float param_3, float param_4, float param_5, int unused_7, edF32VECTOR4* param_8);
	void SetBoomyFunc(int param_2);
	void IncreaseEffort(float param_1);

	void BuildHorizontalSpeedVector(float runSpeed, float param_2, float param_3, float param_4, float param_5);
	void ConvertSpeedSumForceExtToSpeedPlayer2D();

	void RestoreVerticalOrientation();

	bool DetectGripablePrecipice();

	int SlideOnGround(float param_1, float param_2, float param_3, float param_4, uint flags);
	void MoveInAir(float param_1, float param_2, float param_3, float param_4, float param_5);
	void MoveInFreeFall(float param_1, float param_2, float param_3, float param_4, float param_5, int param_7);

	void SetBoomyHairOff();
	void SetBoomyHairOn();

	void SetGripState();
	int DetectGripEdge(int param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, float* param_5, float* param_6, edF32VECTOR4* param_7);

	int EvolutionBounceCanJump();
	bool EvolutionTobogganCanJump();
	bool EvolutionTobogganUnknown();

	uint CanGrip(int param_2, edF32VECTOR4* pRotation);
	bool CanBounceAgainstWall();

	void ChangeCollisionSphereForLying(float param_2);
	void ChangeCollisionSphereForToboggan(float param_2);
	void ChangeCollisionSphereForGlide(float param_1, float param_2);

	void ConvertSpeedPlayerToSpeedSumForceExt();
	void BeginToboggan();
	void SlideOnToboggan(float param_1, float param_2, float param_3, float param_4, edF32VECTOR4* param_6, edF32VECTOR4* param_7, uint dynFlags);
	bool TobogganBounceOnWall(edF32VECTOR4* param_2, edF32VECTOR4* param_3, CActor* pActor);

	void Landing();

	void AdjustLocalMatrixFromNormal(float param_1, edF32VECTOR4* pNormal);

	bool UpdateOrientationFromWind(edF32VECTOR4* v0, edF32VECTOR4* v1);

	void ConvertSpeedSumForceExtToSpeedPlayer();
	bool GetNormalInFrontOf(float rayLength, edF32VECTOR4* pRayDirection, edF32VECTOR4* param_4, uint flags, edF32VECTOR4* pHitLocation, CActor** pActor);

	void SetInvincible(float t0, int param_3);

	CBehaviourHeroDefault behaviourHeroDefault;

	SPEED_DYN field_0xcb4;

	CActor* pTrappedByActor;
	uint trapLinkedBone;

	CActorWindState field_0x1190;

	CFxHandle* field_0x10fc;
	int* field_0x1100;

	float field_0x13cc;
	float field_0x13d0;

	float field_0x15c4;
	float field_0x15c8;

	CBehaviour behaviour_0x1c50;
	CBehaviour behaviour_0x1e10;
	CBehaviour behaviour_0x1fd0;

	float field_0x1558;

	int field_0x12e0;

	undefined4 field_0x1a50;
	int field_0x1094;

	edF32VECTOR4 field_0xf00;

	CActor* field_0xf54;
	CActor* field_0xf58;

	int field_0x1a4c;

	float field_0x1048;

	edF32VECTOR4 field_0x1400;
	edF32VECTOR4 bounceLocation;
	edF32VECTOR4 field_0x1460;
	edF32VECTOR4 field_0x1490;

	CActor* pCheckpointManagerSubObjA_0x14a4;

	float field_0x14b0;
	float field_0x14b4;
	float field_0x151c;

	edF32VECTOR4 field_0x10a0;
	edF32VECTOR4 field_0x1030;
	edF32VECTOR4 normalValue;

	uint field_0x15a0;

	float slideSlipIntensity;

	edF32VECTOR4 controlDirection;

	uint boomyState_0x1b70;

	float field_0x1b9c;

	int field_0x1020;
	int field_0x1420;
	int field_0x1424;
	int field_0x1428;

	int field_0x142c;
	bool field_0x1454;

	float field_0x116c;
	float field_0x1170;

	byte field_0x1455;
	int field_0x1168;

	// Run settings.
	float field_0x1068;
	float field_0x106c;
	float field_0x1074;
	float field_0x1078;
	float field_0x107c;
	float field_0x1080;
	float field_0x1084;

	float field_0x10c0;
	float field_0x10c4;
	float field_0x10c8;
	float field_0x10cc;
	float field_0x10d0;
	float field_0x10d4;
	edF32VECTOR4 field_0x10e0;
	float field_0x10f0;

	// Standard settings.
	float field_0x1040;
	float field_0x104c;
	float airRotationRate;
	float runSpeed;
	float field_0x1054;
	float field_0x1058;
	float field_0x105c;
	float airNoInputSpeed;
	float field_0x1184;
	float landSpeed;
	float field_0x1430;
	float field_0x1574;

	float animKey_0x157c;
	float field_0x11f0;

	float field_0x1160;
	float field_0x1164;

	float field_0x1174;
	float field_0x1180;

	CActor* field_0xf18;
	CActor* pSoccerActor;

	undefined4 field_0xcbc;

	CLifeInterface field_0xee4;

	CCamera* pMainCamera;
	CCamera* pCameraKyaJamgut;

	CCamera* pJamgutCamera_0x15b8;
	CCamera* pIntViewCamera_0x15bc;

	int field_0xe3c;

	float airMinSpeed;
	float airHorizontalSpeed;
	float airControlSpeed;
	float jmp_field_0x113c;
	float jmp_field_0x1140;

	float field_0x1150;
	float field_0x1154;
	float field_0x1158;

	int field_0x1a10;

	float field_0x118c;

	float field_0x10f4;
	float field_0x10f8;

	float field_0x1108;

	float field_0x11f4;
	float field_0x11f8;
	float field_0x11fc;

	float field_0x11ec;
	float field_0x11e0;
	float field_0x11e4;
	float field_0x11e8;

	float field_0x1410;
	float field_0x1414;
	float field_0x1418;
	float field_0x141c;

	float field_0x154c;
	float field_0x155c;
	undefined4 field_0x1560;

	undefined4 jmp_field_0x1144;
	edF32VECTOR4 jmp_field_0x1120;
};

#endif //ACTOR_HERO_PRIVATE_H