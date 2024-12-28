#ifndef ACTOR_WOLFEN_H
#define ACTOR_WOLFEN_H

#include "Types.h"
#include "Actor.h"
#include "ActorFighter.h"
#include "CinematicManager.h"
#include "SwitchBehaviour.h"
#include "TimeController.h"
#include "Vision.h"

#define WOLFEN_BEHAVIOUR_WATCH_DOG 0x8
#define WOLFEN_BEHAVIOUR_GUARD_AREA 0xa
#define WOLFEN_BEHAVIOUR_SLEEP 0xc
#define WOLFEN_BEHAVIOUR_ESCAPE 0xd
#define WOLFEN_BEHAVIOUR_EXORCISM 0xe
#define WOLFEN_BEHAVIOUR_TRACK 0xf
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON 0x11
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON_STAND 0x12
#define WOLFEN_BEHAVIOUR_TRACK_WEAPON_SNIPE 0x13
#define WOLFEN_BEHAVIOUR_SNIPE 0x15
#define WOLFEN_BEHAVIOUR_LOST 0x16
#define WOLFEN_BEHAVIOUR_WOLFEN_UNKNOWN 0x17
#define WOLFEN_BEHAVIOUR_DCA 0x18
#define WOLFEN_BEHAVIOUR_AVOID 0x19

#define WOLFEN_STATE_WATCH_DOG_GUARD 0x72
#define WOLFEN_STATE_TRACK_WEAPON_CHASE 0x73
#define WOLFEN_STATE_TRACK_DEFEND 0x76
#define WOLFEN_STATE_COME_BACK 0x77
#define WOLFEN_STATE_SURPRISE 0x9a
#define WOLFEN_STATE_LOCATE 0x9c

#define WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION 0xab

struct S_STREAM_EVENT_CAMERA;

struct EnemyComponent80
{
	void* pEnemy_0x60;
};

class CActorWolfen;
class CActorCommander;
class CActorProjectile;

class CActorWolfenKnowledge
{ };

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

	bool FUN_003c38c0(CActorWolfen* pWolfen)
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

	int GetState_003c37c0(CActorWolfen* pActor)
	{
		bool bVar1;
		CActorWeapon* pCVar2;
		Timer* pTVar3;
		long lVar4;
		int iVar5;

		bVar1 = true;
		iVar5 = WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION;
		//pCVar2 = CActorFighter::GetWeapon((CActorFighter*)pActor);
		//if (pCVar2 != (CActorWeapon*)0x0) {
		//	pCVar2 = CActorFighter::GetWeapon((CActorFighter*)pActor);
		//	lVar4 = FUN_002d58a0(pCVar2);
		//	if (lVar4 != 0) {
		//		pCVar2 = CActorFighter::GetWeapon((CActorFighter*)pActor);
		//		lVar4 = FUN_002d5830((int)pCVar2);
		//		if (lVar4 == 0) {
		//			bVar1 = false;
		//		}
		//	}
		//}

		if (!bVar1) {
			iVar5 = 0x97;
			pActor->field_0xcf8 = WOLFEN_STATE_TRACK_WEAPON_CHECK_POSITION;
		}

		if (((pActor->pCommander->flags_0x18c & 8) == 0) && (1 < (int)pActor->combatMode_0xb7c)) {
			pTVar3 = Timer::GetTimer();
			if (this->field_0x4 < pTVar3->scaledTotalTime) {
				this->field_0x4 = pTVar3->scaledTotalTime + this->field_0x0;
			}
		}

		return iVar5;
	}

	float field_0x0;
	float field_0x4;

	EEnemyCombatMode combatMode;

	int field_0x34;
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

	S_STREAM_EVENT_CAMERA* field_0x70;
	S_STREAM_REF<CWayPoint> field_0x80;
	int trackBehaviourId;
};

class CBehaviourLost : public CBehaviourWolfen
{
public:

};

class CBehaviourExorcism : public CBehaviour
{
public:
};

class CBehaviourTrackStand : public CBehaviourWolfen
{
public:
};

class CBehaviourTrack : public CBehaviourTrackStand
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType) { IMPLEMENTATION_GUARD(); }
	virtual void End(int newBehaviourId) { IMPLEMENTATION_GUARD(); }
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

	S_TARGET_STREAM_REF_CONTAINER pTargetStreamRef;
	S_STREAM_EVENT_CAMERA_CONTAINER pCameraStreamEvent;
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
	virtual void Term() { IMPLEMENTATION_GUARD(); }
	virtual void Manage() { IMPLEMENTATION_GUARD(); }
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState) { IMPLEMENTATION_GUARD(); }

	virtual CNotificationTargetArray<S_STREAM_NTF_TARGET_ONOFF>* GetNotificationTargetArray();

	virtual int GetStateWolfenWeapon();
	virtual int Func_0x70() { IMPLEMENTATION_GUARD(); }
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

	undefined4 field_0xe8;
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

class CBehaviourDCA : public CBehaviourWolfen
{
public:
};

class CBehaviourGuardArea : public CBehaviourWolfen
{
public:
};

class CBehaviourWolfenUnknown : public CBehaviourWolfen
{
public:
};

class CBehaviourAvoid : public CBehaviour
{
public:
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
	float field_0x0;
	float field_0x4;

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
	int newId;

	CActorFighter* pActor;
};

class CActorWolfen : public CActorFighter 
{
public:
	static WolfenConfig _gStateCfg_WLF[68];
	static WolfenCollisionSphere _pDefCollisions[3];
	static int _waitStandAnimArray[8];
	static int _waitDefendAnimArray[4];

	virtual bool IsKindOfObject(ulong kind);
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term() { IMPLEMENTATION_GUARD(); }
	virtual void Manage();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SectorChange(int oldSectorId, int newSectorId);
	virtual void SaveContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual void LoadContext(uint*, int) { IMPLEMENTATION_GUARD(); }
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state) { IMPLEMENTATION_GUARD(); }
	virtual void UpdateLookingAt() { IMPLEMENTATION_GUARD(); }
	virtual void UpdatePostAnimEffects();
	virtual void SetState(int newState, int animType);
	virtual void ChangeManageState(int state);
	virtual void AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual bool IsMakingNoise() { IMPLEMENTATION_GUARD(); }
	virtual CVision* GetVision();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { IMPLEMENTATION_GUARD(); }

	// CActorAutonomous
	virtual float GetWalkSpeed();
	virtual float GetWalkRotSpeed();
	virtual float GetWalkAcceleration();
	virtual float GetRunSpeed();
	virtual float GetRunRotSpeed();
	virtual float GetRunAcceleration();
	virtual CPathFinderClient* GetPathfinderClient();
	virtual CPathFinderClient* GetPathfinderClientAlt();

	// CActorFighter
	virtual bool Func_0x19c();
	virtual bool Func_0x1ac();

	void SetCombatMode(EEnemyCombatMode newCombatMode);
	uint GetStateWolfenFlags(int state);

	void AnimEvaluate_0xa2(uint layerId);

	void Create_FightParam(ByteCode* pByteCode);

	void ManageKnowledge();

	void BehaviourWatchDog_Manage(CBehaviourWatchDog* pBehaviour);
	void BehaviourTrackWeapon_Manage(CBehaviourTrackWeapon* pBehaviour);

	void WaitingAnimation_Defend();
	void WaitingAnimation_Guard();

	void StateWolfenComeBack(CBehaviourWolfen* pBehaviour);
	void StateWolfen_00179db0(CBehaviourWolfen* pBehaviour);
	void StateWolfenSurprise(CBehaviourWolfen* pBehaviour);
	void StateWolfenLocate(CBehaviourWolfen* pBehaviour);
	void StateWatchDogGuard(CBehaviourWatchDog* pBehaviour);
	void StateTrackWeaponChase(CBehaviourTrackWeapon* pBehaviour);
	void StateTrackWeaponDefend(CBehaviourTrackWeapon* pBehaviour);
	void StateTrackWeaponCheckPosition(CBehaviourTrackWeapon* pBehaviour);

	int SV_WLF_CheckBoxOnWay(CActorsTable* pTable);

	void ClearLocalData();

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

	CBehaviourLost behaviourLost;
	CBehaviourTrack behaviourTrack;
	CBehaviourExorcism behaviourExorcism;

	uint field_0xb74;
	uint combatFlags_0xb78 = 0; // delete init
	undefined4 field_0xb80;
	float field_0xb84;
	undefined4 field_0xb88;
	int field_0xb8c;

	float field_0xb90;
	float field_0xb94;
	float field_0xb98;

	float walkSpeed;
	float walkAcceleration;
	float walkRotSpeed;
	float field_0xc5c;

	float runSpeed;
	float runAcceleration;
	float rotRunSpeed;
	float field_0xc6c;

	edF32VECTOR4 pathOriginPosition;

	float field_0xcf0;
	float field_0xcf4;
	int field_0xcf8;

	CActor* field_0xd04;
	float field_0xd08;

	float field_0xd24;
	float field_0xd28;
	uint field_0xd30;

	CActorProjectile* pTrackedProjectile;
	undefined4 field_0xd14;
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

#endif //ACTOR_WOLFEN_H