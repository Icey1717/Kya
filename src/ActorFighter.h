#ifndef _ACTOR_FIGHTER_H
#define _ACTOR_FIGHTER_H

#include "Types.h"
#include "Actor.h"
#include "ActorAutonomous.h"
#include "ActorAmbre.h"
#include "Dynamic.h"

#define IMPLEMENTATION_GUARD_FIGHT(x)

#define FIGHTER_BEHAVIOUR_DEFAULT	0x3
#define FIGHTER_BEHAVIOUR_PROJECTED 0x4
#define FIGHTER_BEHAVIOUR_RIDDEN	0x5
#define FIGHTER_BEHAVIOUR_SLAVE		0x6

#define FIGHTER_DEFAULT_STATE_IDLE 0x6
#define FIGHTER_DEFAULT_STATE_RUN 0x9

#define FIGHTER_HIT_STEP_BACK 0x4f

#define FIGHTER_PROJECTED_HIT_FLY 0x55
#define FIGHTER_PROJECTED_HIT_FLY_TO_SLIDE 0x56
#define FIGHTER_PROJECTED_HIT_SLIDE 0x57
#define FIGHTER_PROJECTED_HIT_WAKING_UP 0x59
#define FIGHTER_PROJECTED_HIT_STAGGER_CHECK 0x5a

#define FIGHTER_BLOW_BEGIN 0x65
#define FIGHTER_EXECUTE_BLOW 0x66
#define FIGHTER_BLOW_END 0x67

struct s_fighter_anatomy_zones
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
	float field_0x20;
	float field_0x24;
};

struct s_fighter_multiways_anim
{
	byte field_0x0[4];
	uint field_0x4;
};

struct s_input_pattern
{
	union 
	{
		struct
		{
			byte field_0x0byte;
			byte field_0x1byte;
			byte field_0x2byte;
			byte field_0x3byte;
		};

		struct
		{
			ushort field_0x0ushort;
			ushort field_0x2ushort;
		};

		uint field_0x0uint;
	};

	uint nbInputs;
};

union s_fighter_action_hash
{
	uint hash;
	int pData; // void* (s_fighter_combo/blow/grab)
};

struct s_fighter_move
{
	s_fighter_action_hash hash;
	s_input_pattern field_0x4;
};

struct s_fighter_combo : public s_fighter_move
{
	float field_0x8;
	float field_0xc;
	s_input_pattern pattern;
	s_fighter_action_hash actionHash;
	s_fighter_action_hash* aBranches;
	uint nbBranches;
};

struct s_input_pattern_cmp
{
	uint field_0x0;
	uint field_0x4;
	float field_0x8;
};

struct CInputAnalyser
{
	CInputAnalyser();
	void _CumulateDirections(CPlayerInput* pInput, edF32VECTOR4* pDir);
	int Cumulate(CPlayerInput* pPalyerInput, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	void Reset();
	static bool Compare(s_input_pattern* pPatternA, s_input_pattern* pPatternB, s_input_pattern_cmp* pPatternCmp);
	void _SV_RotatePattern(int index);

	s_input_pattern patternA;
	s_input_pattern patternB;

	edF32VECTOR4 field_0x10;
	uint lastStickDirFlags;

	uint flags;
	s_fighter_combo* pComboA;
	s_fighter_combo* pComboB;
};

class StaticMeshComponentAdvanced : public StaticMeshComponent
{
public:
	virtual void Reset();
	virtual void SetHidden(ed_3D_Scene* pScene);
	virtual void SetVisible(ed_3D_Scene* pScene);
	virtual bool HasMesh();

	void Create(ByteCode* pByteCode);
	void ResetInternal(int textureIndex, int meshIndex);

	edF32VECTOR4* GetTextureAnimSpeedNormalExtruder();

	byte field_0x60;
	byte field_0x61;

	int field_0x64;

	undefined4 field_0x68;
	undefined4 field_0x6c;

	undefined4 field_0x70;
	float field_0x74;
	float field_0x78;
};

struct s_fighter_blow_bone_ref
{
	CActor* pActor;
	uint boneId;
};

struct s_fighter_blow_sub_obj {
	s_fighter_blow_bone_ref boneRefA;
	s_fighter_blow_bone_ref boneRefB;

	int materialId;
	union
	{
		struct {
			byte field_0x14;
			byte field_0x15;
			byte field_0x16;
			byte field_0x17;
		};

		uint field_0x14uint;
	};

	uint field_0x18;
	float field_0x1c;
	uint field_0x20;
};

struct _s_fighter_blow_stage
{
	int animId;
	float field_0x4;
	float field_0x8;
	uint flags;
	edF32VECTOR4 field_0x10;
};

struct s_fighter_blow : public s_fighter_move
{
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float damage;
	float field_0x18;
	float field_0x1c;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
	uint field_0x40;
	uint nbSubObjs;
	s_fighter_blow_sub_obj* field_0x48;
	float canActivateRange;
	float field_0x50;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	int nbBoneRefs;
	s_fighter_blow_bone_ref aBoneRefs[2];
	_s_fighter_blow_stage blowStageBegin;
	_s_fighter_blow_stage blowStageExecute;
	_s_fighter_blow_stage blowStageEnd;
	void* field_0xd0;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
};

struct s_fighter_grab : public s_fighter_move
{
	float field_0x8;
	float field_0xc;
	float field_0x10;
	s_fighter_blow_bone_ref field_0x14;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	float field_0x20;
	float field_0x24;
	float field_0x28;
	undefined4 field_0x2c;
	float field_0x30;
	float field_0x34;
	uint field_0x38;
	float field_0x3c;
	float field_0x40;
	float field_0x44;
	float field_0x48;
	undefined4 field_0x4c;
	edF32VECTOR3 field_0x50;
	undefined4 field_0x5c;
	float field_0x60;
	float field_0x64;
	uint field_0x68;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	int field_0x80;
	float field_0x84;
	float field_0x88;
	uint field_0x8c;
	float field_0x90;
	float field_0x94;
	float field_0x98;
	undefined4 field_0x9c;
	int field_0xa0;
	float field_0xa4;
	float field_0xa8;
	uint field_0xac;
	float field_0xb0;
	float field_0xb4;
	float field_0xb8;
	undefined4 field_0xbc;
	int field_0xc0;
	float field_0xc4;
	float field_0xc8;
	uint field_0xcc;
	float field_0xd0;
	float field_0xd4;
	float field_0xd8;
	undefined4 field_0xdc;
	int field_0xe0;
	float field_0xe4;
	float field_0xe8;
	uint field_0xec;
	float field_0xf0;
	float field_0xf4;
	float field_0xf8;
	undefined4 field_0xfc;
	int field_0x100;
	float field_0x104;
	float field_0x108;
	uint field_0x10c;
	float field_0x110;
	float field_0x114;
	float field_0x118;
	undefined4 field_0x11c;
};

struct s_fighter_grab_react
{
	s_fighter_action_hash hash;
	uint field_0x4;
	uint field_0x8;
};

class CActorWeapon;

struct s_fighter_hit_exclusion
{
	CActor* pActor;
	float currentDuration;
	float maxDuration;
};

class CFighterExcludedTable : public CFixedTable<s_fighter_hit_exclusion, 6>
{
public:
	void EmptyByTime(float time);
	bool IsInList(CActor* pActor);
	void Add(float maxDuration, CActor* pActor);
};


struct s_fighter_action
{
	union
	{
		struct
		{
			byte field_0x0;
			byte field_0x1;
			byte field_0x2;
			byte field_0x3;
		};

		struct
		{
			ushort field_0x0_2;
			ushort field_0x2_2;
		};

		uint all;
	};
};

struct s_fighter_action_param
{
	edF32VECTOR4* field_0x0;

	union 
	{
		uint field_0x4;
		void* pData;
	};
};

struct s_fighter_fight_collision
{
	CActor* pActor;
	uint boneId;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	edF32VECTOR4 prevBonePosition;
	edF32VECTOR4 curBonePosition;
};

struct s_fighter_collision_desc
{
	CActor* pActor;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	float field_0x30;
};

class CActorFighter : public CActorAutonomous
{
public:
	CActorFighter() {
		IMPLEMENTATION_GUARD_ACTOR()
	}

	static float _pStagger_check[8];

	static StateConfig _gStateCfg_FIG[0x6c];

	// CActor
	virtual bool IsKindOfObject(ulong kind);

	virtual void Create(ByteCode* pByteCode);

	virtual void Init();
	virtual void Term();

	virtual void Manage();

	virtual void Reset();

	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual bool SetBehaviour(int behaviourId, int newState, int animationType);

	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);

	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);

	virtual int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);

	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);

	virtual void TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5);

	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	// End Interface

	// CActorAutonomous
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	virtual void UpdateCollisionSphere();

	// CActorFighter
	virtual void SetInitialState();
	virtual void SetFightBehaviour();
	virtual int GetFightBehaviour();
	virtual void ProcessDeath() {}
	virtual bool IsFightRelated(int behaviourId);
	virtual bool IsInHitState();
	virtual bool IsAlive();
	virtual void EnableFightCamera() {}
	virtual void AcquireAdversary();
	virtual edF32VECTOR4* GetAdversaryPos();
	virtual int Func_0x18c();
	virtual bool Func_0x190(CActor* pOther);
	virtual void Func_0x194(float param_1);
	virtual void UpdateFightCommand();
	virtual bool Func_0x19c();
	virtual bool Func_0x1a0();
	virtual bool Func_0x1a4();
	virtual bool Func_0x1a8();
	virtual bool Func_0x1ac();
	virtual bool Func_0x1b0(CActor* pOther);
	virtual bool Func_0x1c0(s_fighter_combo* pCombo);
	virtual void _Execute_Std(s_fighter_action* pAction, s_fighter_action_param* pParam);
	virtual void _Std_GetPossibleHit(bool bPlayImpact);
	virtual void _Std_GetPossibleExit();
	virtual void _Std_OnFightActionSuccess();
	virtual CActorFighter* _Std_GetCaughtAdversary();
	virtual void _StateFighterRun(CActorsTable* pTable);
	virtual void _Ride_GetPossibleHit(bool bPlayImpact);
	virtual void _Ride_GetPossibleExit();
	virtual void _Hold_GetPossibleHit(bool bPlayImpact);
	virtual void _Hold_GetPossibleExit();
	virtual void _BeginFighterHold();
	virtual void _EndFighterHold();
	virtual void _Proj_GetPossibleExit();

	void ClearLocalData();

	//void _ExecuteCommand(s_fighter_action* pAction, s_fighter_action_param* pParam);
	bool Execute(s_fighter_action* pAction, s_fighter_action_param* pParams);

	void RunInternal(float param_1, edF32VECTOR4* pRotation);

	void SetAdversary(CActorFighter* pNewAdversary);

	void _ManageFighterDyn(uint param_2, uint inFlags, CActorsTable* pTable);

	void _ComputeLogicalPos(edF32VECTOR4* pPosition);

	void _InterpretCollisions(int param_2);
	void _InterpretSlides();

	void _StateFighterHitWakingUpInit();
	void _StateFighterHitFall(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, int bProcessCollisions);
	void _StateFighterHitFly();
	void _StateFighterHitFlyToSlide();
	void _StateFighterHitSlide();

	void _StateFighterHitStepBackInit(int animationId, int param_3);
	void _StateFighterHitStepBack(int nextState, int animationId, int param_4);

	float _StateFighterFightActionDynInit(_s_fighter_blow_stage* pStage);
	void _StateFighterExecuteBlow(int nextStateA, int nextStateB, int param_4);

	void _StateFighterHitStaggerCheck();

	void _StateFighterHitStaggerFallInit(float param_1);

	void _StateFighter_0xaInit();

	void _StateFighter_0xbInit(float param_1);
	void _StateFighter_0xb();

	void _LoadBlow(s_fighter_blow* pBlow, ByteCode* pByteCode);
	void _CreateBlowsDB(ByteCode* pByteCode);

	void _LoadGrab(s_fighter_grab* pGrab, ByteCode* pByteCode);
	void _CreateGrabsDB(ByteCode* pByteCode);

	int _MarkCombosRoots(s_fighter_combo* aCombos, int nbCombos);
	void _LoadCombo(s_fighter_combo* pCombo, ByteCode* pByteCode);
	void _CreateCombosDB(ByteCode* pByteCode);

	uint _SV_HIT_GetHitZoneFromImpact(s_fighter_anatomy_zones* pAnatomyZones, edF32VECTOR4* pHitPosition, edF32VECTOR4* pActorPosition);
	void _SV_HIT_FightCollisionsBegin();
	bool _SV_HIT_FightCollisionCheckIntersect(s_fighter_collision_desc* pCollisionDesc, edF32VECTOR4* param_3, edF32VECTOR4* param_4, uint kindMask);
	uint _SV_HIT_FightCollisionsGet(s_fighter_collision_desc* aCollisionDescs, uint param_3, uint param_4);
	void _SV_HIT_FightCollisionsEnd();
	void _SV_HIT_FightCollisionProcessHit(CActor* pHitActor, s_fighter_blow* pBlow, edF32VECTOR4* pPosition, int param_5);
	int _SV_HIT_ProcessActorsCollisions(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, CActorsTable* param_6, CFighterExcludedTable* param_7, bool param_8, ushort param_9, ushort param_10);

	void _SV_DYN_SetRotationAroundMassCenter(edF32MATRIX4* param_2);

	void _SV_VOLDYN_VolumeSpeedFromPointSpeed(edF32VECTOR4* param_2, edF32VECTOR4* param_3, float* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7);
	void _SV_VOLDYN_PointSpeedFromVolumeSpeed(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7);
	void _SV_VOLDYN_GetCollisionPosFromVector(edF32VECTOR4* v0, edF32VECTOR4* v1);

	int _SV_ANM_GetMultiWaysAnim3D(s_fighter_multiways_anim* pAnimation, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	int _SV_ANM_GetTwoSidedAnim(int param_2, int param_3);

	CActorWeapon* GetWeapon();

	s_fighter_combo* FindComboByName(char* szName);
	void SetFighterCombo(s_fighter_combo* pCombo);
	s_fighter_blow* FindBlowByName(char* szName);

	void SetStandAnim(int newStandAnim);

	bool FUN_0031b5d0(int state);
	void FUN_0031a7c0(int inState);

	void FUN_0031aad0(float animLength, s_fighter_blow* pBlow);
	bool FUN_0031b790(int state);
	uint FUN_0031b4d0(int state);
	bool FUN_0031ac10(float param_1, s_fighter_blow* pBlow);
	int FUN_0030a6a0();
	bool FUN_003175e0(s_fighter_action* pFighterAction, float* param_3);
	bool FUN_0031b7f0(s_fighter_action* pAction, s_fighter_action_param* pActionParam);

	void PlayOrientedFx(edF32VECTOR4* param_2, edF32VECTOR4* param_3, uint param_4, int* param_5);
	void PlayImpactFx(edF32VECTOR4* pPosition, edF32VECTOR4* param_3, int param_4, bool param_5);

	float _GetFighterAnimationLength(int animId);

	void _InitBoneRef(s_fighter_blow_bone_ref* pBoneRef);

	void _InitBlow(s_fighter_blow* pBlow);
	void _InitGrab(s_fighter_grab* pGrab);

	void _InitBlowsDB();
	void _DeleteCombosDB();
	void _InitGrabsDB();
	void _DeleteGrabsDB();
	void _InitCombosDB();
	void _DeleteBlowsDB();

	void _UpdateComboSituation();
	s_fighter_combo* _FindComboBestRoot(s_input_pattern* pPattern, uint flags);
	s_fighter_combo* _FindComboBestNext(s_input_pattern* pPattern, uint flags);
	s_fighter_combo* _FindBestCombo(s_input_pattern* pPattern, uint flags);
	bool _ValidateCommand(s_fighter_action* pActionA, s_fighter_action* pActionB);
	void _SetRelativeSpeedOnGround(float speed, edF32VECTOR4* pDirection);
	void _BuildCommandFromAbsoluteVector(edF32VECTOR4* param_2, s_fighter_action* pFighterAction, edF32VECTOR4* param_4);
	void _UpdateDynForExplosion(_msg_hit_param* pHitParams);
	void _UpdateDynForHit(float maxDuration, _msg_hit_param* pHitParams);

	void UpdateFightCommandInternal(CPlayerInput* pPlayerInput, int param_3);

	CActorFighter* pAdversary;
	CActorFighter* field_0x354;
	edF32VECTOR4 logicalPosition;

	float field_0x358;
	float field_0x360;
	int field_0x36c;
	float adversaryDistance;
	undefined4 field_0x374;
	float adversaryAngleDiff;

	uint fightFlags;

	s_fighter_anatomy_zones fighterAnatomyZones;

	CScalarDyn scalarDynA;
	CScalarDyn scalarDynB;
	CScalarDyn scalarDynJump;

	float field_0x3f4;
	float field_0x3f8;

	edF32VECTOR4 field_0x400;
	edF32VECTOR4 field_0x410;

	float field_0x420;
	float field_0x424;
	float field_0x428;
	float field_0x42c;

	float hitMultiplier;
	uint field_0x444;

	union
	{
		uint all;
		byte flags[4];
	} validCommandMask;

	s_fighter_action_param field_0x454;
	float field_0x45c;
	edF32VECTOR4 field_0x460;

	CInputAnalyser* pInputAnalyser;

	float field_0x478;

	edF32VECTOR4 field_0x4a0;
	edF32VECTOR4 field_0x4b0;

	union 
	{
		struct {
			byte field_0x44c;
			byte field_0x44d;
			byte field_0x44e;
			byte field_0x44f;
		};

		uint field_0x44cuint;
	};

	s_fighter_action field_0x450;

	undefined4 field_0x474;
	float field_0x47c;
	int standAnim;

	float field_0x4c0;
	float field_0x4c4;
	float field_0x4c8;
	float field_0x4cc;

	edF32VECTOR4 field_0x4d0;
	edF32VECTOR4 field_0x4e0;

	float field_0x4f0;
	float field_0x4f4;
	float field_0x4f8;
	float field_0x4fc;

	float field_0x504;
	float field_0x50c;
	float field_0x510;
	float field_0x514;

	uint field_0x518;
	uint field_0x51c;
	uint field_0x520;
	uint field_0x524;
	uint field_0x528;
	uint field_0x52c;
	uint field_0x530;
	uint field_0x534;
	uint field_0x538;
	uint field_0x53c;
	uint field_0x540;
	uint field_0x544;
	uint field_0x548;
	uint field_0x54c;

	StaticMeshComponentAdvanced staticMeshComponentAdvanced;
	// Maybe part of above
	uint field_0x5f0;

	edF32VECTOR4 field_0x600;

	CActor* field_0x634;

	CFighterExcludedTable actorsExcludeTable;

	short field_0x684;
	ushort field_0x686;

	edF32VECTOR4 field_0x690;

	edF32VECTOR4 field_0x6a0;
	float field_0x6b0;
	float hitDamage;
	int hitFlags;
	float field_0x6bc;
	float field_0x6c0;
	float field_0x6c4;
	float field_0x6c8;
	float field_0x6cc;

	s_fighter_multiways_anim field_0x6d0[12];
	s_fighter_multiways_anim field_0x730;

	uint field_0x738;
	edF32VECTOR4 field_0x740;
	edF32VECTOR4 field_0x750;

	edF32MATRIX4 field_0x760;

	edF32VECTOR4 field_0x7a0;
	float field_0x7b0;
	float field_0x7b4;
	
	float field_0x7c0;
	float field_0x7c4;
	float field_0x7c8;
	float field_0x7d0;
	float field_0x7d4;
	float field_0x7d8;
	float field_0x7dc;

	float field_0x7cc;

	uint field_0x7e0;
	uint field_0x7e4;
	edF32VECTOR4 field_0x7f0;

	float field_0x800;
	edF32VECTOR4 field_0x810[2];

	s_fighter_blow* pBlow;
	s_fighter_blow* field_0x834;
	s_fighter_blow* field_0x840;

	undefined4 field_0x860;
	s_fighter_blow* field_0x864;
	uint comboFlags;

	s_fighter_combo* pFighterCombo;

	uint field_0x630;

	s_fighter_fight_collision aFightCollisions[2];
	int nbFightCollisions;
	int field_0x8e4;

	edF32VECTOR4 field_0x8f0;
	edF32VECTOR4 field_0x900;

	S_STREAM_REF<CActor> pWeaponActor;

	uint nbBlows;
	s_fighter_blow* aBlows;

	uint nbGrabs;
	s_fighter_grab* aGrabs;

	uint nbGrabReacts;
	s_fighter_grab_react* aGrabReacts;

	uint nbCombos;
	uint nbComboRoots;
	s_fighter_combo* aCombos;
};

class CBehaviourFighter : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);
	virtual void Term() {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void SetInitialState();
	virtual bool Execute(s_fighter_action* param_2, s_fighter_action_param* param_3);
	virtual void _ManageHit(bool bPlayImpact);
	virtual void _ManageExit();

	bool Conditional_Execute(s_fighter_action* pAction, s_fighter_action_param* pActionParam);

	CActorFighter* pOwner;
	int behaviourId;
};

class CBehaviourFighterSlave : public CBehaviourFighter
{

};

class CBehaviourFighterRidden : public CBehaviourFighter
{

};

class CBehaviourFighterProjected : public CBehaviourFighter
{
public:
	virtual void Init(CActor* pOwner) {}
	virtual void Term() {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual bool Execute(s_fighter_action* param_2, s_fighter_action_param* param_3);
	virtual void _ManageHit(bool bPlayImpact);

	void _ComputeDynamics();
};

#endif // _ACTOR_FIGHTER_H
