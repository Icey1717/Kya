#ifndef _ACTOR_FIGHTER_H
#define _ACTOR_FIGHTER_H

#include "Types.h"
#include "Actor.h"
#include "ActorAutonomous.h"
#include "ActorAmbre.h"

#define IMPLEMENTATION_GUARD_FIGHT(x)

#define FIGHTER_BEHAVIOUR_DEFAULT	0x3
#define FIGHTER_BEHAVIOUR_PROJECTED 0x4
#define FIGHTER_BEHAVIOUR_RIDDEN	0x5
#define FIGHTER_BEHAVIOUR_SLAVE		0x6

#define FIGHTER_DEFAULT_STATE_IDLE 0x6

struct s_fighter_anatomy_zones
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
};

struct s_fighter_multiways_anim
{
	byte field_0x0[4];
	uint field_0x4;
};

struct FighterSubObj_40
{
	FighterSubObj_40();

	undefined4 field_0x0;
	undefined4 field_0x4;
	undefined4 field_0x8;
	undefined4 field_0xc;

	edF32VECTOR4 field_0x10;
	undefined4 field_0x20;

	uint field_0x30;
	undefined4 field_0x34;
	undefined4 field_0x38;
};

class StaticMeshComponentAdvanced : public StaticMeshComponent
{
public:
	virtual void Reset();
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

struct s_fighter_blow_sub_obj {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	uint field_0x4;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	uint field_0xc;
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

struct s_fighter_blow {
	undefined4 field_0x0;
	char field_0x4;
	char field_0x5;
	char field_0x6;
	undefined field_0x7;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	float field_0x28;
	undefined4 field_0x2c;
	edF32VECTOR4 field_0x30;
	uint field_0x40;
	uint nbSubObjs;
	s_fighter_blow_sub_obj* field_0x48;
	float field_0x4c;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	int field_0x5c;
	int field_0x60;
	uint field_0x64;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	int field_0x70;
	float field_0x74;
	float field_0x78;
	uint field_0x7c;
	float field_0x80;
	float field_0x84;
	float field_0x88;
	undefined4 field_0x8c;
	int field_0x90;
	float field_0x94;
	float field_0x98;
	uint field_0x9c;
	float field_0xa0;
	float field_0xa4;
	float field_0xa8;
	undefined4 field_0xac;
	int field_0xb0;
	float field_0xb4;
	float field_0xb8;
	uint field_0xbc;
	float field_0xc0;
	float field_0xc4;
	float field_0xc8;
	undefined4 field_0xcc;
	undefined4 field_0xd0;
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

struct s_fighter_grab
{
	undefined4 field_0x0;
	char field_0x4;
	char field_0x5;
	short field_0x6;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	uint field_0x18;
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
	int field_0x0;
	uint field_0x4;
	uint field_0x8;
};

struct s_fighter_combo
{
	undefined4 field_0x0;
	short field_0x4;
	short field_0x6;
	float field_0x8;
	float field_0xc;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	byte field_0x13;
	uint field_0x14;
	undefined4 field_0x18;
	void* field_0x1c;
	uint field_0x20;
};

class CActorWeapon;

class CFighterExcludedTable
{
public:
	CFighterExcludedTable();

	undefined4 field_0x0;
};

class CActorFighter : public CActorAutonomous
{
public:
	CActorFighter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	static StateConfig _gStateCfg_FIG[0x6c];

	virtual bool IsKindOfObject(ulong kind);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);
	virtual void Create(ByteCode* pByteCode);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);
	virtual void TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5);

	// CActorAutonomous
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	virtual void UpdateCollisionSphere();

	// CActorFighter
	virtual void SetInitialState();
	virtual void SetFightBehaviour();
	virtual int GetFightBehaviour();
	virtual bool IsFightRelated(int behaviourId);
	virtual void ProcessDeath() {}
	virtual bool IsAlive();
	virtual void EnableFightCamera() {}
	virtual void AcquireAdversary();
	virtual edF32VECTOR4* GetAdversaryPos();
	virtual int Func_0x18c();
	virtual void Func_0x194(float param_1);
	virtual int UpdateFightCommand();
	virtual bool Func_0x1a8();
	virtual void _Std_GetPossibleExit();
	virtual void _Ride_GetPossibleExit();
	virtual void _Hold_GetPossibleExit();
	virtual void _Proj_GetPossibleExit();

	virtual void AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim);
	void ClearLocalData();

	void SetAdversary(CActorFighter* pNewAdversary);

	void _ManageFighterDyn(uint param_2, uint inFlags, CActorsTable* pTable);

	void _ComputeLogicalPos(edF32VECTOR4* pPosition);

	void _InterpretCollisions(int param_2);
	void _InterpretSlides();

	void _StateFighterHitFall(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, int bProcessCollisions);
	void _StateFighterHitFlyToSlide();
	void _StateFighterHitSlide();

	void _LoadBlow(s_fighter_blow* pBlow, ByteCode* pByteCode);
	void _CreateBlowsDB(ByteCode* pByteCode);

	void _LoadGrab(s_fighter_grab* pGrab, ByteCode* pByteCode);
	void _CreateGrabsDB(ByteCode* pByteCode);

	void _LoadCombo(s_fighter_combo* pCombo, ByteCode* pByteCode);
	void _CreateCombosDB(ByteCode* pByteCode);

	int _SV_HIT_ProcessActorsCollisions(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, CActorsTable* param_6, CFighterExcludedTable* param_7, bool param_8, ushort param_9, ushort param_10);

	void _SV_DYN_SetRotationAroundMassCenter(edF32MATRIX4* param_2);

	void _SV_VOLDYN_VolumeSpeedFromPointSpeed(edF32VECTOR4* param_2, edF32VECTOR4* param_3, float* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7);
	void _SV_VOLDYN_PointSpeedFromVolumeSpeed(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, edF32VECTOR4* param_7);
	void _SV_VOLDYN_GetCollisionPosFromVector(edF32VECTOR4* v0, edF32VECTOR4* v1);

	int _SV_ANM_GetMultiWaysAnim3D(s_fighter_multiways_anim* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	int _SV_ANM_GetTwoSidedAnim(int param_2, int param_3);

	CActorWeapon* GetWeapon();

	void SetFighterCombo(s_fighter_combo* pCombo);

	bool FUN_0031b790(int state);
	uint FUN_0031b4d0(int state);
	int FUN_0030a6a0();

	CActorFighter* pAdversary;
	CActorFighter* field_0x354;
	edF32VECTOR4 logicalPosition;

	float field_0x360;
	int field_0x36c;
	float field_0x370;
	undefined4 field_0x374;
	float field_0x378;

	uint fightFlags;

	s_fighter_anatomy_zones fighterAnatomyZones;

	CScalarDyn field_0x37c;
	CScalarDyn field_0x3a4;
	CScalarDyn scalarDynJump;

	float field_0x3f4;
	float field_0x3f8;

	edF32VECTOR4 field_0x400;
	edF32VECTOR4 field_0x410;

	float field_0x420;
	float field_0x424;
	float field_0x428;
	float field_0x42c;

	float field_0x440;

	uint field_0x444;

	union
	{
		uint all;
		byte flags[4];
	} field_0x448;

	FighterSubObj_40* field_0x470;

	float field_0x478;

	edF32VECTOR4 field_0x4a0;
	edF32VECTOR4 field_0x4b0;

	undefined4 field_0x44c;

	undefined4 field_0x474;
	float field_0x47c;
	int field_0x480;

	float field_0x4f0;

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
	int field_0x6b8;

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

	float field_0x800;
	float field_0x810;
	float field_0x814;
	float field_0x818;
	float field_0x81c;
	float field_0x820;
	float field_0x824;
	float field_0x828;
	float field_0x82c;

	undefined4 field_0x860;
	undefined4 field_0x864;
	undefined4 field_0x868;

	s_fighter_combo* pFighterCombo;

	int field_0x8e0;
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
	virtual int Execute();
	virtual void _ManageHit();
	virtual void _ManageExit();

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
	virtual void Init(CActor* pOwner) {}
	virtual void Term() {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual int Execute();
	virtual void _ManageHit();

	void _ComputeDynamics();
};

#endif // _ACTOR_FIGHTER_H
