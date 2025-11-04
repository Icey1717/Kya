#ifndef ACTOR_WOLFEN_H
#define ACTOR_WOLFEN_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"
#include "ActorCommander.h"
#include "CinematicManager.h"
#include "SwitchBehaviour.h"
#include "TimeController.h"
#include "ActorWeapon.h"
#include "Vision.h"
#include "Fx.h"

// <ACTOR_NAME>_BEHAVIOUR_<BEHAVIOUR_NAME>

#define WOLFEN_BEHAVIOUR_WATCH_DOG 0x8
#define WOLFEN_BEHAVIOUR_GUARD_AREA 0xa
#define WOLFEN_BEHAVIOUR_SLEEP 0xc
#define WOLFEN_BEHAVIOUR_ESCAPE 0xd
#define WOLFEN_BEHAVIOUR_EXORCISM 0xe
#define WOLFEN_BEHAVIOUR_TRACK 0xf
#define WOLFEN_BEHAVIOUR_TRACK_STAND 0x10
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON 0x11
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND 0x12
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE 0x13
#define WOLFEN_BEHAVIOUR_SNIPE 0x15
#define WOLFEN_BEHAVIOUR_LOST 0x16
#define WOLFEN_BEHAVIOUR_WOLFEN_DCA 0x17
#define WOLFEN_BEHAVIOUR_AVOID 0x18
#define WOLFEN_BEHAVIOUR_UNKNOWN 0x19

// <ACTOR_NAME>_STATE_<BEHAVIOUR_NAME>_<STATE_NAME>

#define WOLFEN_STATE_WATCH_DOG_GUARD 0x72
#define WOLFEN_STATE_TRACK_CHASE 0x73
#define WOLFEN_STATE_BREAK_OBJECT 0x74
#define WOLFEN_STATE_TRACK_DEFEND 0x76
#define WOLFEN_STATE_COME_BACK 0x77
#define WOLFEN_STATE_EXORCISE_IDLE 0x79
#define WOLFEN_STATE_EXORCISE_EXORCIZE 0x7a
#define WOLFEN_STATE_EXORCISE_TRANSFORM 0x7b
#define WOLFEN_STATE_EXORCISE_TRANSFORM_COMPLETE 0x7c
#define WOLFEN_STATE_EXORCISE_END 0x7d
#define WOLFEN_STATE_EXORCISE_AWAKE 0x7e
#define WOLFEN_STATE_EXORCISE_LIVING_DEAD 0x7f
#define WOLFEN_STATE_RELOAD 0x91
#define WOLFEN_STATE_AIM 0x94
#define WOLFEN_STATE_FIRE 0x95
#define WOLFEN_STATE_BOOMY_HIT 0x98
#define WOLFEN_STATE_SURPRISE 0x9a
#define WOLFEN_STATE_LOCATE 0x9c

#define WOLFEN_STATE_INSULT 0x9d
#define WOLFEN_STATE_INSULT_STAND 0x9e
#define WOLFEN_STATE_INSULT_RECEIVE 0x9f
#define WOLFEN_STATE_INSULT_END 0xa0

#define WOLFEN_STATE_BOMB_FLIP 0xa1
#define WOLFEN_STATE_BOMB_STAND 0xa2
#define WOLFEN_STATE_BOMB_WALK_TO 0xa3
#define WOLFEN_STATE_BOMB_ORIENT_TO 0xa4
#define WOLFEN_STATE_BOMB_SHOOT 0xa5

#define WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION 0xab

struct S_STREAM_EVENT_CAMERA;

struct EnemyComponent80
{
	void* pEnemy_0x60;
};

class CActorWolfen;
class CActorCommander;
class CActorProjectile;
class CActorHero;

struct RndData
{
	uint field_0x0;
	uint commandId;
};

class CFightIA
{
public:
	struct WFIGS_Chain
	{
		byte field_0x0;
		float field_0x4;
		int field_0x8;
		RndData rndData;
	};
};

union f4data
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

struct CActorWolfenKnowledge_0x14
{
	s_fighter_combo* field_0x0;
	f4data* field_0x4;
	uint field_0x8;
	uint field_0xc;
	uint field_0x10;
};

class CActorWolfenKnowledge
{
public:
	void Init(int memMode, uint param_3, uint param_4, uint nbObjs, uint param_6);
	void Reset();
	void Term();

	int BeginMemory(s_fighter_combo* pFighterCombo);
	int NextStage(s_fighter_combo* pFighterCombo);
	void EndMemory();

	CActorWolfenKnowledge_0x14* _AddComboRoot(s_fighter_combo* pFighterCombo);

	CActorWolfenKnowledge_0x14* aSubObjs;
	int field_0x4;
	uint nbSubObjs;
	int field_0xc;
	int memMode;
	uint field_0x14;
	uint field_0x18;
	int field_0x1c;
	CActorWolfenKnowledge_0x14* field_0x20;
	f4data* field_0x24;
	s_fighter_combo* field_0x28;
	byte field_0x2c;
};

enum EEnemyCombatMode
{
	ECM_None,
	ECM_Alerted,
	ECM_InCombat,
};

template<typename T>
class CNotificationTargetArray
{
public:
	CNotificationTargetArray()
	{
		this->field_0x0 = 0;
		this->field_0x4 = 0;
		this->field_0x34 = -1;

		return;
	}

	void FUN_003c3a30()
	{
		this->field_0x4 = 0.0f;

		return;
	}

	bool FUN_003c38c0(CActorWolfen* pWolfen);

	int GetState_003c37c0(CActorWolfen* pActor);

	float field_0x0;
	float field_0x4;

	EEnemyCombatMode combatMode;

	int field_0x34;
};

template<typename T>
class CRndChooser
{
public:
	CRndChooser()
	{
		this->nbItems = 0;
		this->pItems = (T*)0x0;

		this->prevPickedIndex = -1;
		this->field_0x10 = 0;
		this->field_0x14 = 3;
		this->field_0x8 = 0;
	}

	bool CanPickRndData()
	{
		bool bCanPick = false;
		uint currentItemIndex = 0;

		while (true) {
			bool bVar6 = false;
			if ((!bCanPick) && (currentItemIndex < this->nbItems)) {
				bVar6 = true;
			}

			if (!bVar6) break;

			T* pItem = this->pItems + currentItemIndex;
			bVar6 = false;

			if ((pItem->field_0x0 == 1) && (0.0f < pItem->field_0x4)) {
				bVar6 = true;
			}

			if (bVar6) {
				bCanPick = true;
			}

			currentItemIndex = currentItemIndex + 1;
		}

		return bCanPick;
	}

	RndData* PickRndData()
	{
		uint currentItemIndex;
		uint uVar17;
		bool bVar7;

		if (this->field_0x8 == 0) {
			float local_4 = 0.0f;

			for (currentItemIndex = 0; currentItemIndex < this->nbItems; currentItemIndex = currentItemIndex + 1) {
				CFightIA::WFIGS_Chain* pCVar18 = this->pItems + currentItemIndex;
				if (pCVar18->field_0x0 != 0) {
					local_4 = local_4 + pCVar18->field_0x4;
				}

				this->pItems[currentItemIndex].field_0x8 = (int)local_4;
			}

			for (currentItemIndex = 0; uVar17 = this->nbItems, currentItemIndex < uVar17; currentItemIndex = currentItemIndex + 1) {
				CFightIA::WFIGS_Chain* pCVar18 = this->pItems + currentItemIndex;
				pCVar18->field_0x8 = (int)(((float)pCVar18->field_0x8 * 32767.0f) / local_4);
			}

			this->pItems[uVar17 - 1].field_0x8 = 0x7fff;
			this->field_0x8 = 1;
		}

		for (currentItemIndex = 0; currentItemIndex < this->nbItems; currentItemIndex = currentItemIndex + 1) {
			if ((int)CScene::Rand() <= this->pItems[currentItemIndex].field_0x8) {
				if (currentItemIndex == this->prevPickedIndex) {
					int iVar12 = this->field_0x10;

					if (iVar12 < (int)this->field_0x14) {
						this->field_0x10 = iVar12 + 1;
					}
					else {
						do {
							uint uVar17 = this->nbItems;

							currentItemIndex = (int)(currentItemIndex + 1) % (int)uVar17;
							if (uVar17 == 0) {
								trap(7);
							}

							CFightIA::WFIGS_Chain* pCVar18 = this->pItems + currentItemIndex;
							bVar7 = true;
							if ((pCVar18->field_0x4 != 0.0) && (pCVar18->field_0x0 != 0)) {
								bVar7 = false;
							}
						} while (bVar7);

						this->prevPickedIndex = currentItemIndex;
						this->field_0x10 = 0;
					}
				}
				else {
					this->prevPickedIndex = currentItemIndex;
					this->field_0x10 = 0;
				}

				break;
			}
		}

		return &this->pItems[currentItemIndex].rndData;
	}

	T* pItems;
	uint nbItems;

	undefined4 field_0x8;
	int prevPickedIndex;
	undefined4 field_0x10;
	int field_0x14;
};

struct WolfenComboData
{
	uint field_0x0;
	int field_0x4;
	int field_0x8;
	int field_0xc;
};

struct WFIGS_Capability
{
	void Create(ByteCode* pByteCode, WolfenComboData* pComboData, float multiplier, uint* pOutCount);
	void Begin();

	bool Get();

	uint semaphoreId;
	uint field_0x4;
	uint field_0x8;
	uint field_0xc;
	uint field_0x10;
	uint field_0x14;

	CRndChooser<CFightIA::WFIGS_Chain> rndChooser;

	uint nbItems;
};

class CBehaviourWolfen : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual edF32VECTOR4* GetComeBackPosition();
	virtual edF32VECTOR4* GetComeBackAngles();
	virtual int GetTrackBehaviour();
	virtual CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* GetNotificationTargetArray();
	virtual int GetStateWolfenComeBack();
	virtual int GetStateWolfenTrack();
	virtual int GetStateWolfenGuard();
	virtual int GetStateWolfenWeapon();

	void CheckDetection();
	void CheckDetection_Intruder();

	int FUN_001f0ab0();

	int TestState_001f09b0();
	int TestState_001f0a00();
	int TestState_001f0a30();
	int TestState_001f0a70();
	int TestState_001f0a90();

	int GetState_001f0930();

	int GetState_001f0b30();
	int GetState_001f08a0();

	uint flags_0x4;
	CActorWolfen* pOwner;

	edF32VECTOR4 baseLocation;
	edF32VECTOR4 rotationEuler;
	edF32VECTOR4 comeBackPosition;
	edF32VECTOR4 comeBackAngles;

	CSwitchBehaviour switchBehaviour;

	bool bool_0x68;

	S_TARGET_ON_OFF_STREAM_REF* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA_CONTAINER pCameraStreamEvent;
};

class CBehaviourLost : public CBehaviourWolfen
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

struct astruct_18
{
	CActorWolfen* pWolfen;
	float field_0xc;
	int field_0x10;
};

struct astruct_17;

class CBehaviourExorcism : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void ChangeManageState(int state);

	bool FUN_001edbe0();
	void DecreaseNbBonusReq();
	void ClearStruct_001edb50();

	CActorWolfen* pOwner;

	int behaviourId;

	float rateMagicDecreaseForExorcism;
	float field_0x10;
	float field_0x14;
	float field_0x18;

	astruct_17* aSubObjA;
	CFxDigits fxDigits;
	int field_0x24;
	int digitMaterialId;
	CFxHandle fxHandle;

	edF32VECTOR4 field_0x40;
};

class CBehaviourTrackStand : public CBehaviourWolfen
{
public:
	virtual CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* GetNotificationTargetArray();

	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF> notificationTargetArray;
};

class CBehaviourTrack : public CBehaviourTrackStand
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	virtual int Func_0x70();
};

class CBehaviourWatchDog : public CBehaviourWolfen
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CBehaviourWolfen
	virtual int GetTrackBehaviour();

	S_STREAM_REF<CWayPoint> field_0x80;
	int trackBehaviourId;
};

class CBehaviourSleep : public CBehaviourWatchDog
{
public:
};

class CBehaviourEscape : public CBehaviourWolfen
{
public:
};

class CBehaviourWolfenWeapon : public CBehaviourWolfen
{
public:
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
};

class CBehaviourSnipe : public CBehaviourWolfenWeapon
{
public:
};

struct astruct_16
{
	astruct_16();

	void Create(ByteCode* pByteCode);

	uint field_0x0;
	float field_0x4;
	float field_0x8;
};

struct TrackSubObj
{
	int field_0x0;
	astruct_16 field_0x4;
	int field_0x10;
};

class CBehaviourTrackWeaponStand : public CBehaviourWolfenWeapon
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Term();
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	virtual CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* GetNotificationTargetArray();

	virtual int GetStateWolfenWeapon();
	virtual int Func_0x70();
	virtual int Func_0x74();
	virtual int Func_0x78();
	virtual void Func_0x80(int* param_2, int* param_3, CActor* pTarget);

	int FUN_002faf40();

	edF32VECTOR4 field_0x80;

	float field_0x90;
	int field_0x94;
	int field_0x98;

	TrackSubObj* aSubObjs;
	int nbSubObjs;

	CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF> notificationTargetArray;

	int field_0xe8;
	undefined4 field_0xec;
};

class CBehaviourTrackWeapon : public CBehaviourTrackWeaponStand
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void TermState(int oldState, int newState);

	virtual int Func_0x70();
	virtual void Func_0x80(int* param_2, int* param_3, CActor* pTarget);

	float field_0xf0;
};

class CBehaviourTrackWeaponSnipe : public CBehaviourTrackWeaponStand
{
public:
};

class CBehaviourAvoid : public CBehaviourWolfen
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);

	float field_0x80;
	float field_0x84;
	int returnBehaviourId;
};

class CBehaviourGuardArea : public CBehaviourWolfen
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual edF32VECTOR4* GetComeBackPosition();
	virtual int GetTrackBehaviour();
	virtual int GetStateWolfenGuard();

	CPathFollowReader pathFollowReader;
	int field_0x90;
	int trackBehaviourId;
	float field_0x98;
};

class CBehaviourDCA : public CBehaviourWolfen
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);

	virtual edF32VECTOR4* GetComeBackPosition();
	virtual edF32VECTOR4* GetComeBackAngles();
	virtual int GetTrackBehaviour();
	virtual int GetStateWolfenComeBack();
	virtual int GetStateWolfenTrack();

	CActor* GetActor();
	void GetPosition(edF32VECTOR4* pOutPosition);

	bool CanShoot();
	void HasShoot();

	S_STREAM_REF<CActor> actorRef;

	float lastShootTime;
	float shootCooldown;
	int trackBehaviourId;
	edF32VECTOR4 field_0x90;
	edF32VECTOR4 angleRotY;
};

class CBehaviourUnknown : public CBehaviour
{
public:
};

class CFightContext
{
public:
	byte field_0x0;
	int field_0x4;
	int field_0x8;
	float field_0xc;
	int state_0x10;
	int state_0x14;
};

class CBehaviourFighterWolfen : public CBehaviourFighter
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Draw();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// New virtual functions
	virtual void ManageExit();
	virtual void ManageCombatMusic(int state);
	virtual void Func_0x64();

	void SetPositionToHold(float param_1, edF32VECTOR4* pPosition);
	void InputPunch(uint cmd);
	void FlushInput();
	void UpdateFightContext(CFightContext* pFightContext);
	void ManageWFigState(uint commandId);
	bool TreatContext(CFightContext* pFightContext);

	void InitCommand(uint commandId);
	void ExecuteCommand(uint param_2, uint param_3);
	bool IsCommandFinished(uint param_2);
	void ValidateCommand();
	void PickCommand();
	s_fighter_combo* PickCombo_Attack(CFightContext* pFightContext, bool param_3, bool param_4, bool param_5, bool param_6);

	void FunReset();

	struct Rule
	{
		float (*pFunc)(CActorWolfen* pWolfen);
		float value;
	};

	Rule aRules[3];

	int field_0x3c;
	edF32VECTOR4 holdPosition;

	int field_0x24;
	int field_0x28;
	int field_0x2c;
	int field_0x30;
	float field_0x34;
	int field_0x38;
	float field_0x50;
	float adversaryBlowDuration;

	float field_0x58;
	float field_0x5c;

	int currentCommandId;

	float field_0x64;

	int field_0x68;

	byte field_0x70;
	byte field_0x71;

	s_fighter_action field_0x74;

	s_fighter_combo* pActiveCombo;
	s_fighter_blow* pActiveBlow;

	s_fighter_action_param field_0x90;

	edF32VECTOR4 field_0x80;

	CFightContext fightContext;

	byte field_0xb8;
};

class CBehaviourWolfenFighterRidden : public CBehaviourFighterRidden
{
public:

};

class CBehaviourWolfenFighterSlave : public CBehaviourFighterSlave
{
public:

};

class CBehaviourWolfenFighterProjected : public CBehaviourFighterProjected
{
public:
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
};

struct WolfenConfig : public StateConfig
{
	WolfenConfig(int inA, uint inB, int inC)
		: StateConfig(inA, inB)
		, field_0x8(inC)
	{

	}

	uint field_0x8;
};

struct WolfenCollisionSphere
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
};

struct HearingDetectionProps
{
	float rangeSquared;
	float maxHeightDifference;

	void Create(ByteCode* pByteCode);
};

struct VisionDetectionProps
{
	float field_0x0;

	void Create(ByteCode* pByteCode);
};

struct InitPathfindingClientMsgParams
{
	uint msgId;

	union
	{
		int newId;
		float time;
	};

	CActorFighter* pActor;
};

struct _msg_fight_action_success_params
{
	int field_0x0;
	CActorFighter* pAdversary;
};

struct ComboData
{
	byte field_0x0;

	float field_0x4;
	float field_0x8;

	s_fighter_combo* pCombo;
};

class CActorWolfen : public CActorFighter 
{
public:
	static WolfenConfig _gStateCfg_WLF[68];
	static WolfenCollisionSphere _pDefCollisions[4];
	static int _waitStandAnimArray[8];
	static int _waitDefendAnimArray[4];

	virtual bool IsKindOfObject(ulong kind);
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Manage();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);
	virtual void UpdateLookingAt() { IMPLEMENTATION_GUARD(); }
	virtual void UpdatePostAnimEffects();
	virtual void SetState(int newState, int animType);
	virtual void ChangeManageState(int state);
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual bool IsMakingNoise();
	virtual CVision* GetVision();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { IMPLEMENTATION_GUARD(); }

	// CActorAutonomous
	virtual CActorWindState* GetWindState();

	virtual void LifeDecrease(float amount);
	virtual void LifeAnnihilate();

	virtual float GetWalkSpeed();
	virtual float GetWalkRotSpeed();
	virtual float GetWalkAcceleration();
	virtual float GetRunSpeed();
	virtual float GetRunRotSpeed();
	virtual float GetRunAcceleration();
	virtual CPathFinderClient* GetPathfinderClient();
	virtual CPathFinderClient* GetPathfinderClientAlt();

	// CActorFighter
	virtual void SetFightBehaviour();
	virtual int GetFightBehaviour();
	virtual void ProcessDeath();
	virtual void AcquireAdversary();
	virtual bool IsFightRelated(int behaviourId);

	virtual bool Func_0x19c();
	virtual bool Func_0x1ac();

	virtual void _Std_OnFightActionSuccess();
	virtual CActorFighter* _Std_GetCaughtAdversary();
	virtual void _StateFighterRun(CActorsTable* pTable);

	virtual void _BeginFighterHold();
	virtual void _EndFighterHold();

	virtual void Func_0x204(CActorFighter* pOther);
	virtual bool AcquireAdversary(CActorFighter* pTarget);
	virtual void SetRunSpeed(float param_1);

	bool IsCurrentPositionValid();

	void SetCombatMode(EEnemyCombatMode newCombatMode);
	uint GetStateWolfenFlags(int state);

	void AnimEvaluate_0xa2(uint layerId);
	void AnimEvaluate_0017c930(uint layerId, edAnmMacroAnimator* pAnimator);

	void Create_FightParam(ByteCode* pByteCode);

	void ManageKnowledge();

	void BehaviourStand_Manage(CBehaviourWolfen* pBehaviour);
	void BehaviourWatchDog_Manage(CBehaviourWatchDog* pBehaviour);
	void BehaviourTrackWeapon_Manage(CBehaviourTrackWeapon* pBehaviour);
	void BehaviourTrackWeaponStand_Manage(CBehaviourTrackWeaponStand* pBehaviour);
	void BehaviourTrack_Manage(CBehaviourTrack* pBehaviour);
	void BehaviourGuardArea_Manage(CBehaviourGuardArea* pBehaviour);
	void BehaviourExorcism_Manage(CBehaviourExorcism* pBehaviour);
	void BehaviourDCA_Manage(CBehaviourDCA* pBehaviour);

	void BehaviourFighterStd_Exit(CBehaviourFighterWolfen* pBehaviour);

	void WaitingAnimation_Defend();
	void WaitingAnimation_Guard();

	void StateStandGuard();
	void StateTrackWeaponStandDefend(CBehaviourTrackWeaponStand* pBehaviour);
	void StateTrackWeaponStandFire(CBehaviourTrackWeaponStand* pBehaviour);
	void StateTrackStandAim(CBehaviourTrackWeaponStand* pBehaviour);
	void StateTrackWeaponReload(CBehaviourTrackWeaponStand* pBehaviour);
	void StateWolfenComeBack(CBehaviourWolfen* pBehaviour);
	void StateWolfen_00179db0(CBehaviourWolfen* pBehaviour);
	void StateWolfenSurprise(CBehaviourWolfen* pBehaviour);
	void StateWolfenLocate(CBehaviourWolfen* pBehaviour);
	void StateWatchDogGuard(CBehaviourWatchDog* pBehaviour);
	void StateTrackWeaponAim(CBehaviourTrackWeapon* pBehaviour);
	void StateTrackWeaponChase(CBehaviourTrackWeapon* pBehaviour);
	void StateTrackWeaponDefend(CBehaviourTrackWeapon* pBehaviour);
	void StateTrackCheckPosition(CBehaviourWolfen* pBehaviour);
	void StateTrackChase(CBehaviourTrack* pBehaviour);
	void StateTrackDefend(CBehaviourTrack* pBehaviour);
	void StateWolfenBombShoot(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenBombOrientTo(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenBombWalkTo(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenBombStand(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenBombFlip(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenInsultEnd(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenInsultReceive(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenInsultStand(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenInsult(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenBoomyHit(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void StateWolfenBreakObject(CBehaviourWolfen* pBehaviour) { IMPLEMENTATION_GUARD(); }
	void State_00174dc0(CBehaviourDCA* pBehaviour);
	void State_00174f20(CBehaviourDCA* pBehaviour);
	void State_001750a0(CBehaviourDCA* pBehaviour);
	void State_00174cb0(CBehaviourDCA* pBehaviour);
	void StateDCAStand(CBehaviourDCA* pBehaviour);
	void StateDCADefend(CBehaviourDCA* pBehaviour);

	void StateExorcizeIdle(CBehaviourExorcism* pBehaviour);
	void StateExorcizeAwake(CBehaviourExorcism* pBehaviour);

	void StateExorcizeInit(CBehaviourExorcism* pBehaviour);
	void StateExorcize(CBehaviourExorcism* pBehaviour);
	void StateExorciseTerm();

	void StateExorcizeTransformInit();
	void StateExorcizeTransform();

	void StateExorcizeTransformCompleteInit();
	void StateExorcizeTransformComplete();
	void StateExorcizeTransformCompleteTerm();

	void StateExorcizeEndInit();
	void StateExorcizeEndTerm(CBehaviourExorcism* pBehaviour);

	void StateDeadLivingDead();
	void StateDeadLivingDeadTerm(CBehaviourExorcism* pBehaviour);

	void StateTrackWeaponCheckPosition(CBehaviourTrackWeaponStand* pBehaviour);
	void StateTrackWeaponDefend(CBehaviourTrackWeaponStand* pBehaviour);

	int SV_WLF_CheckBoxOnWay(CActorsTable* pTable);
	void SV_WLF_ExorcismComputeCenter(edF32VECTOR4* pCenter);

	void ClearLocalData();

	int CheckDetectArea(edF32VECTOR4* pPosition);

	void SV_WLF_MoveTo(CActorMovParamsOut* pMovParamsOut, CActorMovParamsIn* pMovParamsIn, edF32VECTOR4* pPosition);
	bool SV_WLF_CanMoveTo(edF32VECTOR4* pPosition);
	bool SV_WLF_UpdateOrientation2D(float param_1, edF32VECTOR4* v0, int rotationType);
	bool SV_WLF_IsIntruderMakingNoise(CActor* pActor);
	bool SV_WLF_IsIntruderInVitalSphere(CActor* pActor);
	bool SV_WLF_IsIntruderInVision(CActor* pActor);

	bool FUN_001738e0(CActor* pActor);
	bool FUN_0035f1e0(CActorsTable* pTable, edF32VECTOR4* pPosition);
	int GetState_00174190();

	void EnterCombatState(CActor* pSender);
	void UpdateInRange_001744a0(bool bFlag);

	bool IsAlive(CActor* pActor);

	bool CanSwitchToFight_Area(CActor* pTarget);

	void InternState_WolfenLocate();

	void TermFightAction();

	bool CheckLost();

	void CheckValidPatterns(CRndChooser<CFightIA::WFIGS_Chain>* pRndChooser);
	void GetComboMatchValues(int index, float* param_3, float* param_4, float* param_5, float* param_6);
	bool CanPerformWeaponCommand();

	bool RequestFightAction(int index);

	float SemaphoreEval();
	bool SemaphoreKeepIt();

	void EnableFightAction();
	void DisableFightAction();

	bool FUN_00173de0(CActorFighter* pAdversary);
	bool CanBeExorcised();

	bool IsExorcizable(CActorHero* pHero);
	int GetExorciseAnim();

	// New
	void PostManageA();
	void PostManageB(CBehaviourWolfen* pBehaviour);
	void PostManageC(CBehaviourWolfen* pBehaviour);
	void PostManageD(CBehaviourWolfen* pBehaviour); // new
	void ManageSwitches(CBehaviourWolfen* pBehaviour);
	void UpdateCombatMode(); // new
	WFIGS_Capability* GetActiveCapability(); // new

	CBehaviourWolfenFighterProjected behaviourWolfenFighterProjected;
	CBehaviourWolfenFighterRidden behaviourWolfenFighterRidden;
	CBehaviourWolfenFighterSlave behaviourWolfenFighterSlave;
	CBehaviourTrack behaviourTrack;
	CBehaviourFighterWolfen behaviourFighterWolfen;
	CBehaviourLost behaviourLost;
	CBehaviourExorcism behaviourExorcism;

	CActorWindState windState;

	float field_0xa80;
	float field_0xa84;
	float field_0xa88;
	float field_0xa8c;

	WFIGS_Capability aCapabilities[3];

	int activeCapabilityIndex;
	int field_0xb30;

	uint field_0xb34;
	uint field_0xb38;
	uint field_0xb3c;
	uint field_0xb40;

	uint field_0xb44;

	float field_0xb48;
	float field_0xb4c;

	float field_0xb50;
	float field_0xb54;
	float field_0xb58;
	float field_0xb5c;

	uint field_0xb60;
	WolfenComboData* field_0xb64;

	undefined4 field_0xb6c;
	int field_0xb70;

	uint field_0xb74;
	uint combatFlags_0xb78 = 0; // delete init
	int exorcisedState;
	float nbRequiredMagicForExorcism;
	float nbConsumedMagicForExorcism;
	int startSectorId;

	float field_0xb90;
	float field_0xb94;
	float field_0xb98;

	int field_0xbd8;
	uint field_0xbdc;

	undefined4 field_0xbe0;
	int field_0xbe4;
	float field_0xbec;
	float field_0xbf0;

	ComboData* field_0xbd0;
	uint nbComboMatchValues;
	edF32VECTOR4* aComboMatchValues;

	float walkSpeed;
	float walkAcceleration;
	float walkRotSpeed;
	float defaultRunSpeed;

	float runSpeed;
	float runAcceleration;
	float rotRunSpeed;
	float runSpeedScale;

	edF32VECTOR4 pathOriginPosition;

	float field_0xcf0;
	float field_0xcf4;
	int field_0xcf8;

	float field_0xd00;
	CActor* field_0xd04;
	float field_0xd08;
	float field_0xd18;

	float field_0xd24;
	float field_0xd28;
	uint field_0xd30;

	CActorProjectile* pTrackedProjectile;
	CActor* pBoxInWay;
	float field_0xd1c;
	float field_0xd20;

	CPathFinderClient pathFinderClient;

	HearingDetectionProps hearingDetectionProps;
	VisionDetectionProps visionDetectionProps;

	EEnemyCombatMode combatMode_0xb7c;

	EnemyComponent80* pEnemyComponent80_0xd34 = nullptr; // delete init

	CVision vision;

	CActorCommander* pCommander;

	CActorFighter* pTargetActor_0xc80;
	float field_0xcfc;

	CActorWolfenKnowledge* pWolfenKnowledge;
};

template<typename T>
bool CNotificationTargetArray<T>::FUN_003c38c0(CActorWolfen* pWolfen)
{
	Timer* pTVar1;
	bool bVar2;

	bVar2 = true;
	if (((pWolfen->pCommander->flags_0x18c & 8) == 0) &&
		(pTVar1 = Timer::GetTimer(), this->field_0x4 < pTVar1->scaledTotalTime)) {
		bVar2 = false;
	}

	return bVar2;
}

template<typename T>
int CNotificationTargetArray<T>::GetState_003c37c0(CActorWolfen* pActor)
{
	bool bVar1;
	int iVar5;

	bVar1 = true;
	iVar5 = WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION;
	if (pActor->GetWeapon() != (CActorWeapon*)0x0) {
		if (pActor->GetWeapon()->FUN_002d58a0() != 0) {
			if (pActor->GetWeapon()->FUN_002d5830() == 0) {
				bVar1 = false;
			}
		}
	}

	if (!bVar1) {
		iVar5 = 0x97;
		pActor->field_0xcf8 = WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION;
	}

	if (((pActor->pCommander->flags_0x18c & 8) == 0) && (1 < (int)pActor->combatMode_0xb7c)) {
		if (this->field_0x4 < Timer::GetTimer()->scaledTotalTime) {
			this->field_0x4 = Timer::GetTimer()->scaledTotalTime + this->field_0x0;
		}
	}

	return iVar5;
}

#endif //ACTOR_WOLFEN_H