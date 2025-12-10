#ifndef ACTOR_HERO_PRIVATE_H
#define ACTOR_HERO_PRIVATE_H

#include "Types.h"
#include "Actor.h"
#include "ActorHero.h"
#include "Fx_Tail.h"

#define IMPLEMENTATION_GUARD_BOOMY(x)
#define IMPLEMENTATION_GUARD_INVENTORY(x)

#define ACTION_SPEAK 0xf

class PlayerSubStruct_64 : public CObject
{
public:
	PlayerSubStruct_64();

	void Init(uint param_2);
	void FUN_00401460(float param_1, int param_3);
	void FUN_004012f0(float param_1, uint param_3, int materialId, uint param_5);

	undefined4 field_0x4;
	undefined* field_0x8[2];
	int field_0x10;
	undefined4 field_0x14;
	undefined* field_0x18[2];
	float field_0x20;
	float field_0x24;
	int field_0x28;
	uint flags;
	float field_0x30;
	undefined4 field_0x34;
	float field_0x38;
	float field_0x3c;
	edDList_material* pMaterial;
	uint field_0x44;
};

enum EBoomyThrowState
{
	BTS_None,
	BTS_Unknown_1,
	BTS_Melee,
	BTS_StandardThrow,
	BTS_TargettedThrow,
	BTS_ControlledThrow,
	BTS_Unknown_6
};

class CCamera;
class CActorBoomy;
class CActorHeroPrivate;
class CFightLock_SE;

struct _ray_info_out;

struct AdversaryEntry
{
	CActorFighter* pActorFighter;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	uint field_0x18;
	int field_0x1c;
};

// typedef for function that takes in a CFightLock_SE and an int and returns an int
typedef uint(*InferenceRuleFunc)(CFightLock_SE*, uint);

struct InferenceRuleEntry
{
	InferenceRuleFunc pFunc;
	uint result;
};

class CFightLock_SE
{
public:
	void BuildKnowledgeBase(edF32VECTOR4* pDirection);
	CActorFighter* ApplyInferenceRules();

	void _Heuristic_Aim(edF32VECTOR4* pDirection);
	void _Heuristic_Danger();

	static uint _Rule_Flee(CFightLock_SE* pLock, uint prevValue);
	static uint _Rule_Attack(CFightLock_SE* pLock, uint prevValue);
	static uint _Rule_DangerCritical(CFightLock_SE* pLock, uint prevValue);
	static uint _Rule_DangerMin(CFightLock_SE* pLock, uint prevValue);
	static uint _Rule_KeepFocus(CFightLock_SE* pLock, uint prevValue);
	static uint _Rule_DangerMax(CFightLock_SE* pLock, uint prevValue);
	static uint _Rule_DistMin(CFightLock_SE* pLock, uint prevValue);

	CActorFighter* pOwner;

	byte bUseOcclusion;

	uint nbPotentialAdversaries;

	AdversaryEntry aAdversaries[0x10];

	uint field_0x214;
	uint field_0x218;

	CActorFighter* pAdversary;
	int adversaryIndex;

	InferenceRuleEntry aInferenceRules[7];
};

class CBehaviourHero : public CBehaviourFighter
{
public:
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	virtual void SetInitialState();

	CActorHeroPrivate* pHero;
};

class CBehaviourHeroDefault : public CBehaviourHero
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourRideJamGut : public CBehaviour
{

};

class CBehaviourHeroRideJamGut : public CBehaviourRideJamGut
{

};

struct S_DETECT_WALL_CFG
{
	float field_0x0;
	float field_0x4;
	float field_0x8;
};

class CActorHeroPrivate : public CActorHero 
{
public:
	CActorHeroPrivate();

	// CActor
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Manage();

	virtual void PreCheckpointReset();
	virtual void CheckpointReset();

	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);
	virtual void SetState(int newState, int animType);

	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);

	virtual void CinematicMode_Leave(int behaviourId);

	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);

	virtual bool IsMakingNoise();

	virtual void GetVisualDetectionPoint(edF32VECTOR4* pOutPoint, int index);

	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);
	// End CActor

	// CActorMovable
	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	// CActorAutonomous
	virtual CActorWindState* GetWindState();
	virtual void StoreCollisionSphere();

	virtual void LifeDecrease(float amount);
	virtual void LifeRestore();
	virtual void LifeAnnihilate();
	virtual CLifeInterface* GetLifeInterface();
	virtual CLifeInterface* GetLifeInterfaceOther();

	// CActorFighter
	virtual void SetInitialState();
	virtual void ProcessDeath();
	virtual void EnableFightCamera(int bEnable);
	virtual void AcquireAdversary();
	virtual edF32VECTOR4* GetAdversaryPos();
	virtual int Func_0x18c();
	virtual void Func_0x194(float param_1);
	virtual void UpdateFightCommand();
	virtual bool Func_0x1a4();
	virtual bool Func_0x1a8();
	virtual bool Func_0x1b0(CActor* pOther);
	virtual bool Func_0x1c0(s_fighter_combo* pCombo);
	virtual void _Proj_GetPossibleExit();

	bool AccomplishHit(CActor* pHitBy, _msg_hit_param* pHitParam, edF32VECTOR4* param_4);
	bool AccomplishAttack();
	bool AccomplishMagic();
	bool AccomplishAction(int bUpdateActiveActionId);

	void GetPossibleAction();
	void GetPossibleMagicalTargets(CActorsTable* pTable);

	bool AbleTo_AttackByBoomyBlow();
	bool AbleTo_AttackByBoomyLaunch();
	bool AbleTo_AttackByBoomySnipe();
	bool AbleTo_AttackByBoomyControl();

	EBoomyThrowState ManageEnterAttack();
	bool ManageActions();

	void FxManageGlideTail();

	void UpdateMedallion();

	float GetTargetBeta();
	void DisableLayer(uint layerId);
	void SetLayerProperty(float param_1, uint layerId);
	bool IsLayerAnimFinished(uint layerId);
	bool IsLayerAnimEndReached(uint layerId);
	void SetLayerAnim(float param_1, uint layerId, int animId);
	void SetMagicMode(int bEnable);

	void ResetStdDefaultSettings();
	void ResetSlideDefaultSettings();
	void ResetBoomyDefaultSettings();
	void ResetGripClimbDefaultSettings();
	void ResetWindDefaultSettings();
	void ResetJamGutSettings();

	void ComputeBlendingWeightsInHitGlide();

	void GetClimbVector(edF32VECTOR4* pOutVector, edF32VECTOR4* pInVector);
	bool CheckClimbZone(edF32VECTOR4* pInVector);
	bool SetNextClimbState(int param_2, int param_3);

	int StateEvaluate();
	int ChooseStateFall(int param_2);
	int ChooseStateLanding(float speed);
	int ChooseStateHit(CActor* pHitBy, _msg_hit_param* pHitParams, edF32VECTOR4* param_4, int param_5);
	int ChooseStateDead(int hitType, int dieType, int param_4);

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

	void StateHeroWindSlideInit();
	void StateHeroWindSlide(int nextState);

	void StateHeroWindWallMove(float horizontalSpeed, float verticalSpeed, int param_4, int param_5);

	void StateHeroTrampolineJump_1_2Init();
	void StateHeroTrampolineJump_1_2Term();
	void StateHeroTrampolineJump_1_2(float param_1);

	void StateHeroTrampolineStomachToFall(float param_1);

	void StateHeroCeilingClimb(int param_2, int param_3, int param_4);

	void StateHeroGrip2Ceiling();
	void StateHeroCeiling2Grip();

	inline void StateHeroRunTerm() {}
	void StateHeroRun();
	void StateHeroRun_B();

	void StateHeroJoke();

	void StateHeroSlideSlipInit();
	void StateHeroSlideSlip(int nextState, bool boolA, bool boolB);

	void StateHeroSlideInit(int param_2);
	void StateHeroSlide(int param_2);
	void StateHeroSlideTerm(int param_2);

	void StateHeroUTurnInit();
	void StateHeroUTurn();
	void StateHeroUTurnTerm();

	void StateHeroStandToCrouch(int param_2);

	void StateHeroCrouchInit();
	void StateHeroCrouch(int nextState);

	void StateHeroCrouchWalk();

	void StateHeroRollInit();
	void StateHeroRoll();
	void StateHeroRollTerm() {}

	void StateHeroRoll2Crouch();

	void StateHeroHit();

	void StateHeroKickInit();
	void StateHeroKickInitSimple();
	void StateHeroKick(int param_2, int param_3);
	void StateHeroKickTerm();
	void StateHeroKickTermSimple();

	void StateHeroJump_1_3(int nextState);

	void StateHeroJump_2_3Init();
	void StateHeroJump_2_3(int param_2, int param_3, int param_4);
	void StateHeroJump_2_3Term();

	void StateHeroJump_3_3(int param_2);

	void FUN_00138700();
	void FUN_00138520();

	void StateHeroClimbStand(int param_2, int param_3);
	void StateHeroClimbMove(float param_1, float param_2, int param_4, int param_5, int param_6);
	void StateHeroClimbJump();

	void StateHeroGrip(float param_1, int nextState, int param_4);
	void StateHeroGripAngle(int nextState, int param_3);
	void StateHeroGripUp(float param_1, float param_2, int nextState, int param_5);
	void StateHeroGripUpToJumpInit();

	void StateHeroBounceSomersault1();
	void StateHeroBounceSomersault2();

	void StateHeroBoomyPrepareFightBlowInit();
	void StateBoomyExecuteFightBlowInit();
	void StateBoomyExecuteFightBlow();
	void StateHeroBoomyReturnFightBlowInit();
	void StateBoomyTerm();

	void StateHeroFall(float param_1, int param_3);
	void StateHeroDead(float time);

	void StateHeroColWall();

	void StateHeroFlyInit();

	void StateHeroLever_1_2Init();
	void StateHeroLever_1_2();
	void StateHeroLever_1_2Term();

	void StateHeroLever_2_2();
	void StateHeroLever_2_2Term();

	void StateHeroDCAInit();
	void StateHeroDCA();
	void StateHeroDCATerm();

	void StateHeroUnlockInit();
	void StateHeroUnlock(ACTOR_CLASS typeId);
	void StateHeroUnlockTerm();

	void StateHeroExorciseInit();
	void StateHeroExorcise();
	void StateHeroExorciseTerm(int nextState);

	void StateHeroWindFly(int param_2);
	void StateHeroFlyJumpInit();
	void StateHeroFlyTerm();

	void StateHeroGlideInit();
	void StateHeroGlide(int param_2, int nextState);
	void StateHeroGlideTerm() {}

	void StateHeroWindCanonInit();
	void StateHeroWindCanonTerm();

	void StateHeroBasic(float param_1, float param_2, int nextState);

	void StateHeroFallBounce_2_2Init();
	void StateHeroFallBounce_2_2Term();

	void SetJumpCfgFromGround(float param_1);
	void SetJumpCfg(float param_1, float horizonalSpeed, float param_3, float param_4, float param_5, int unused_7, edF32VECTOR4* param_8);
	void SetBoomyFunc(int param_2);
	void IncreaseEffort(float param_1);

	void ManageBoomyStateInit();
	void ManageBoomyState();
	void ManageBoomyStateTerm();

	void BuildHorizontalSpeedVector(float runSpeed, float param_2, float param_3, float param_4, float param_5);
	void ConvertSpeedSumForceExtToSpeedPlayer2D();

	void RestoreVerticalOrientation();

	bool DetectMultipleWallBounces();
	bool DetectGripablePrecipice();
	void DetectStickableWalls(edF32VECTOR4* v0, int* param_3, int* param_4, edF32VECTOR4* v1);

	bool InClimbZone(edF32VECTOR4* pPosition);
	void UngripAllObjects();

	int SlideOnGround(float param_1, float param_2, float param_3, float param_4, uint flags);
	void MoveInAir(float param_1, float param_2, float param_3, float param_4, float param_5);
	void MoveRelativeToWallPlaneInTheWind(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5);
	void MoveInFreeFall(float param_1, float param_2, float param_3, float param_4, float param_5, int param_7);

	void SetBoomyHairOff();
	void SetBoomyHairOn();

	void GetPadRelativeToPlane(edF32VECTOR4* param_2, float* param_3, float* param_4);

	void SetGripState();

	bool DetectWall(uint param_2, uint param_3, edF32VECTOR4* param_4, S_DETECT_WALL_CFG* pDetectWallCfg, edF32VECTOR4* param_6, edF32VECTOR4* param_7, _ray_info_out* pRayInfoOut);
	int DetectClimbCeiling(edF32VECTOR4* v0, CActor** pOutActor);
	bool DetectClimbWall(int param_2, CActor** pOutActor, float* param_4);
	int DetectClimbCeilingFromGrip(CActor** pOutActor, edF32VECTOR4* pPosition);
	int DetectGripEdge(int param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, float* param_5, float* param_6, edF32VECTOR4* param_7);

	bool EvolutionBoomyCanControl();
	bool EvolutionBoomyCanSnipe();
	bool EvolutionBoomyCanLaunch();
	int EvolutionBounceCanJump();
	bool EvolutionCanClimb();
	bool EvolutionTobogganCanJump();
	bool EvolutionTobogganUnknown();

	uint CanGrip(int param_2, edF32VECTOR4* pRotation);
	bool CanBounceAgainstWall();

	void ChangeCollisionSphereForLying(float param_2);
	void ChangeCollisionSphereForToboggan(float param_2);
	void ChangeCollisionSphereForGlide(float param_1, float param_2);

	void ConvertSpeedPlayerToSpeedSumForceExt();
	void ConvertSpeedPlayerToSpeedSumForceExt2D();

	void BeginToboggan();
	void SlideOnToboggan(float param_1, float param_2, float param_3, float param_4, edF32VECTOR4* param_6, edF32VECTOR4* param_7, uint dynFlags);
	void EndToboggan();
	bool TobogganBounceOnWall(edF32VECTOR4* param_2, edF32VECTOR4* param_3, CActor* pActor);

	void Landing();

	void AdjustLocalMatrixFromNormal(float param_1, edF32VECTOR4* pNormal);

	bool UpdateOrientationFromWind(edF32VECTOR4* v0, edF32VECTOR4* v1);

	void ConvertSpeedSumForceExtToSpeedPlayer();
	bool GetNormalInFrontOf(float rayLength, edF32VECTOR4* pRayDirection, edF32VECTOR4* param_4, uint flags, edF32VECTOR4* pHitLocation, CActor** pActor);

	int GetPossibleWind(float param_1, edF32VECTOR4* param_3, CWayPoint* pWayPoint);

	void SetInvincible(float t0, int param_3);
	bool CheckHitAndDeath();

	void ComputeSoccerMoving(float param_1, float param_2, CActorMovable* pSoccerActor);
	void SoccerOff();
	void PlaySoccer();

	bool GetSomethingInFrontOf_001473e0();

	int ChooseFightAnim(int newState, int initialAnim);

	void _InitHeroFight();
	void _ResetHeroFight();

	void UpdateNextAdversary();

	void FUN_00325c40(float t0, int param_3);
	void FUN_00325d00();
	void UpdateBracelet(uint flags);

	void ManageInternalView();

	void WhatsInVision(CActorsTable* pTable, edF32VECTOR4* pDirection);

	bool FUN_00133fb0();

	uint FUN_00132c60(uint state);

	void GripObject(CActor* pOtherActor);

	CBehaviourHeroDefault behaviourHeroDefault;

	int field_0xe44;

	SPEED_DYN field_0xcb4;

	edF32MATRIX3* field_0xff0;

	CActor* pTrappedByActor;
	uint trapLinkedBone;

	CFightLock_SE fightLock;

	CActorMovable* pKickedActor;
	CActor* field_0xf50;

	CActorFighter* field_0x1618;

	CActorWindState field_0x1190;

	CFxTail fxTrailA;
	CFxTail fxTrailB;

	float field_0x1000;
	float field_0x1004;
	float field_0x1008;

	int field_0x100c;
	int field_0x1010;

	float field_0x102c;

	CFxHandle* field_0x10fc;
	int* field_0x1100;

	CActor* pBoomyTarget;
	edF32VECTOR4 boomyTargetPosition;

	int bUnknownBool;

	int boomyBlowStage;
	undefined4 field_0x1ba8;

	float field_0x13cc;
	float field_0x13d0;

	float field_0x15c4;
	float field_0x15c8;

	float field_0x1b74;
	int field_0x1b78;
	int field_0x1b6c;

	s_fighter_blow* aBoomyBlows[3];

	PlayerSubStruct_64 playerSubStruct_64_0x1bf0;

	int field_0x187c;

	int field_0x1c38;
	undefined4 field_0x1c3c;

	CBehaviour behaviour_0x1c50;
	CBehaviour behaviour_0x1e10;
	CBehaviour behaviour_0x1fd0;

	int field_0x12e0;

	undefined4 field_0x1a50;
	float field_0x1a58;
	int field_0x1094;

	float jokeWarnRadius;

	edF32VECTOR4 field_0xf00;

	CActor* field_0xf54;
	CActor* field_0xf58;

	int field_0xd28;
	int field_0xd30;

	int field_0x1a48;
	int field_0x1a4c;

	float field_0x1048;

	edF32VECTOR4 field_0x13f0;
	edF32VECTOR4 field_0x1400;
	edF32VECTOR4 bounceLocation;
	edF32VECTOR4 field_0x1460;
	edF32VECTOR4 field_0x1470;
	edF32VECTOR4 field_0x1480;
	edF32VECTOR4 field_0x1490;

	CActor* pGrippedActor;

	float field_0x14ac;
	float field_0x14b0;
	float field_0x14b4;
	float field_0x14b8;
	float field_0x14bc;
	float field_0x14c0;
	float field_0x14c4;
	float field_0x14c8;
	float field_0x14cc;
	float field_0x14d0;
	float field_0x14d4;
	float field_0x14d8;
	float field_0x14dc;
	float field_0x14e0;
	float field_0x14e4;
	float field_0x14e8;
	float field_0x14ec;
	float field_0x14f0;
	float field_0x1510;
	float field_0x1514;
	float field_0x1520;
	float field_0x1524;
	float field_0x1528;
	float field_0x152c;
	float field_0x1530;
	float field_0x1534;
	float gripHorizontalMoveSpeed;
	float gripUpMoveSpeed;

	edF32VECTOR4 field_0x10a0;
	edF32VECTOR4 field_0x1030;
	edF32VECTOR4 normalValue;

	uint field_0x15a0;

	float slideSlipIntensity;

	edF32VECTOR4 controlDirection;

	uint boomyState_0x1b70;

	float field_0x1b9c;

	int field_0x1020;
	int field_0x1424;
	int field_0x1428;

	int field_0x142c;
	bool field_0x1454;

	float field_0x116c;
	float field_0x1170;

	byte field_0x1455;
	int field_0x1168;

	float field_0x14a8;
	float field_0x14f4;

	// Run settings.
	float field_0x1068;
	float field_0x106c;
	float field_0x1070;
	float field_0x1074;
	float field_0x1078;
	float field_0x107c;
	float field_0x1080;
	float field_0x1084;
	float field_0x1088;
	float field_0x108c;

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
	float field_0x1570;
	int field_0x1574;

	float animKey_0x157c;
	float field_0x11f0;

	float field_0x1160;
	float field_0x1164;

	float field_0x1174;
	float field_0x1178;
	float field_0x117c;
	float field_0x1180;

	CActor* pBounceOnActor;
	CActorMovable* pSoccerActor;

	CActor* field_0xcbc;

	CLifeInterface field_0xee4;

	int field_0xe3c;

	float airMinSpeed;
	float airHorizontalSpeed;
	float airControlSpeed;
	float jmp_field_0x113c;
	float jmp_field_0x1140;

	float field_0x1150;
	float field_0x1154;
	float field_0x1158;
	float field_0x115c;

	int boomyThrowState;
	int activeBoomyThrowState;

	int field_0x1a00;

	uint bounceBoneId;

	struct HeroActionParams
	{
		int actionId;
		int activeActionId;
		CActor* pActor;
		undefined4 field_0xc;
		edF32VECTOR4 field_0x10;
		edF32VECTOR4 field_0x20;
	} heroActionParams;

	int field_0x1a40;
	int field_0x1a44;

	float field_0x118c;

	int field_0x1450;

	float field_0x10f4;
	float field_0x10f8;

	float field_0x1104;
	float field_0x1108;

	float windBoostStrength;
	float windRotationStrength;
	float field_0x11fc;

	float field_0x11ec;
	float field_0x11e0;
	float field_0x11e4;
	float field_0x11e8;

	float field_0x1200;
	float field_0x1204;
	float field_0x1208;

	float field_0x13d4;
	float field_0x13d8;
	float windWallHorizontalSpeed;
	float windWallVerticalSpeed;

	float field_0x13e4;

	float field_0x1410;
	float field_0x1414;
	float field_0x1418;
	float field_0x141c;

	float field_0x154c;
	float field_0x155c;
	undefined4 field_0x1560;

	float field_0x1564;
	int field_0x1568;
	uint field_0x156c;

	undefined4 jmp_field_0x1144;
	edF32VECTOR4 jmpDirection;

	CFxHandleExt field_0x1880[4];

	undefined4 field_0x18b0;

	CFxHandleExt field_0x18b4;
	CFxHandleExt field_0x18c0;
	CFxHandleExt field_0x18cc;


	float boomyTargetRayDist;
	int field_0x1b64;
	undefined4 field_0x1b68;

	int field_0x1b94;
	float field_0x1b98;

	float field_0x1ba0;

	CActorsTable boomyTargetTable;
	edF32VECTOR4 vector_0x1be0;
};

#endif //ACTOR_HERO_PRIVATE_H