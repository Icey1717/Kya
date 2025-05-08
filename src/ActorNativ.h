#ifndef ACTOR_NATIV_H
#define ACTOR_NATIV_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ed3D.h"
#include "PathFollow.h"
#include "ActorNativCmd.h"

#define NATIVE_BEHAVIOUR_SPEAK 0x3
#define NATIVE_BEHAVIOUR_EXORCISM 0x4
#define NATIVE_BEHAVIOUR_LIVE 0x5
#define NATIVE_BEHAVIOUR_TAKE_AND_PUT 0x6
#define NATIVE_BEHAVIOUR_SELLER 0x8

#define NATIVE_STATE_TAKE_PUT_WALK 0x15
#define NATIVE_STATE_TAKE_PUT_TURN_TO 0x16

struct S_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

template<typename T>
class CRndChooser
{
public:
	undefined4 field_0x0;
	undefined4 field_0x4;
};

class CActorNativ;

class CBehaviourNativ : public CBehaviour
{
public:
	CActorNativ* pOwner;
};

class CBehaviourNativExorcisme : public CBehaviourNativ
{
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

struct s_fighter_combo;

struct NativSubObjB
{
	NativSubObjB();

	void Create(ByteCode* pByteCode);
	bool IsRequiredCombo(s_fighter_combo* pCombo);

	int nbRequiredCombos;
	s_fighter_combo* aRequiredCombos[2];
	undefined4 field_0xc;
};

struct NativSubObjA
{
	NativSubObjA();

	int nbSubObjB;
	NativSubObjB* aSubObjsB;
};

struct NativSellerSubObjA
{
	NativSellerSubObjA();

	NativSubObjB aSubObjB[0x8];
	undefined4 field_0x88;
	undefined4 field_0x8c;
};

struct NativSellerSubObjC
{
	NativSellerSubObjC();
	virtual void Init();

	int Add_0xc();

	int nbSubObjA;
	NativSubObjA* aSubObjsA;
	int activeSubObjIndex;
};

class CBehaviourNativSeller : public CBehaviourNativ
{
public:
	CBehaviourNativSeller();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }

	void FUN_003f2150();
	void FUN_003ed150();

	void FUN_003ebd90();

	void FUN_003eccb0(s_fighter_combo* pCombo, float* param_3, float* param_4);
	void FUN_003f1da0(s_fighter_combo* pCombo);
	bool FUN_003ebee0(int param_2);
	bool IsEventActive();
	void SetBehaviourState(int newState);

	uint field_0x8;
	int field_0xc;

	NativSellerSubObjC subObjC;

	CAddOnNativ addOn;

	S_TARGET_STREAM_REF* field_0x38;
	S_STREAM_EVENT_CAMERA* streamEventCamera_0x3c;

	S_TARGET_STREAM_REF* field_0x40;
	S_STREAM_EVENT_CAMERA* streamEventCamera_0x44;

	S_TARGET_STREAM_REF* field_0x48;
	S_STREAM_EVENT_CAMERA* streamEventCamera_0x4c;

	S_TARGET_STREAM_REF* field_0x50;
	S_STREAM_EVENT_CAMERA* streamEventCamera_0x54;

	NativSellerSubObjA field_0x68[0x1e];

	int initialAnimId;

	int activeSubObjBIndex;
	undefined4 field_0x16a4;
	int field_0x16ac;
	int field_0x16b4;
	int currentBehaviourState;

	float field_0x16c0;
	float field_0x16c4;
	float field_0x16c8;
	float field_0x16cc;

	float field_0x16d0;
	float field_0x16d4;
	float field_0x16d8;
};

class CTakePutTrajectoryParam
{
public:
	bool IsWayPointValidPosition(CPathFinderClient* pPathFinderClient);

	float field_0x0;
	CPathFollowReader pathFollowReader;
	uint field_0x14;
	float field_0x18;
	uint field_0x1c;
};

class CBehaviourNativTakeAndPut : public CBehaviourNativ
{
public:
	CBehaviourNativTakeAndPut();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }

	int nbTrajectoryParams;
	CTakePutTrajectoryParam* aTrajectoryParams;
	undefined4 curTrajectoryParamIndex;
};

class CBehaviourNativSpeak : public CBehaviourNativ
{
public:
	CBehaviourNativSpeak();

	void Reset();

	int nbTrajectoryParams;
	CTakePutTrajectoryParam* aTrajectoryParams;

	int field_0x24;
	float field_0x28;
};

class CBehaviourNativLive : public CBehaviourNativ
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }

	float field_0x8;
	float field_0xc;
};

struct ActionEntry {
	int field_0x0;
	float field_0x4;
};

class CActorNativ : public CActorAutonomous 
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Manage();
	virtual void ComputeLighting();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void UpdateAnimEffects();
	virtual void ChangeManageState(int state);
	virtual void ChangeVisibleState(int bVisible);
	virtual void CinematicMode_Enter(bool bSetState);
	virtual void CinematicMode_Leave(int behaviourId);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CActorAutonomous
	virtual CPathFinderClient* GetPathfinderClient();

	void InitActionsTable();
	void ClearLocalData();

	void ChooseBehaviour();
	void ManageOrientationAndLookingAt();

	void BehaviourExorcisme_Manage();

	void BehaviourNativLive_Manage();
	void BehaviourNativLive_TermState(int oldState);

	void BehaviourNativTakeAndPut_InitState(int newState);
	void BehaviourNativTakeAndPut_Manage(CBehaviourNativTakeAndPut* pBehaviour);
	void BehaviourNativTakeAndPut_TermState(int oldState);

	void BehaviourNativSeller_InitState(int newState, CBehaviourNativSeller* pBehaviour);
	void BehaviourNativSeller_Manage(CBehaviourNativSeller* pBehaviour);
	void BehaviourNativSeller_TermState(int oldState);

	void StateNativTakePutTurnTo();
	void StateNativTakePutWalk(CBehaviourNativTakeAndPut* pBehaviour);

	void SetHasObject(bool bHasObject);
	void ChooseToolState();

	uint FUN_00162270();
	float FUN_00164070();

	void State_0x21(CBehaviourNativSeller* pBehaviour);
	void State_0x22(CBehaviourNativSeller* pBehaviour);
	void State_0x23(CBehaviourNativSeller* pBehaviour);

	static StateConfig _gStateCfg_NTV[54];

	CPathFinderClient pathFinderClient;
	
	uint field_0x350;
	uint field_0x354;
	uint field_0x358;
	float moveSpeedNoHeldObject;
	float moveSpeedHeldObject;
	uint field_0x370;
	int field_0x368;
	int field_0x364;
	uint field_0x378;
	uint field_0x37c;
	uint field_0x380;

	CEmotionInfo emotionInfo;

	CActor* field_0x3f0;
	ActionEntry* aActions;
	int nbActions;

	byte bHasObject;

	float field_0x408;

	ed_3D_Light_Config lightConfig;

	ed_3d_hierarchy_setup hierSetup;
	edF32VECTOR4 hierBoundingSphere;
	ed_g3d_hierarchy* field_0x444;
	float hierClipInfo;

	edF32VECTOR4 lightAmbient;
	edF32MATRIX4 lightDirections;
	edF32MATRIX4 lightColours;

	int cinematicCachedBehaviour;
	int cinematicCachedState;
	int cinematicCachedAnim;
	edF32VECTOR4 cinematicCachedLocation;

	int field_0x4f8;
	int field_0x4fc;
	int field_0x500;

	float field_0x508;

	byte field_0x530;
	int field_0x534;

	undefined4 field_0x53c;

	edF32VECTOR4 field_0x540;

	byte field_0x550;
	byte field_0x551;
	byte field_0x504;

	edF32VECTOR4 curWayPointLocation;

	edF32MATRIX4 field_0x580;

	ed_3d_hierarchy* pHeldItemHierarchy;
	edNODE* pHeldItemNode;

	CBehaviourNativSpeak behaviourSpeak;
	CBehaviourNativExorcisme behaviourExorcism;
	CBehaviourNativLive behaviourLive;
	CBehaviourNativTakeAndPut behaviourTakeAndPut;
};

#endif //ACTOR_NATIV_H