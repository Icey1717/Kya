#ifndef ACTOR_NATIV_H
#define ACTOR_NATIV_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ed3D.h"
#include "PathFollow.h"

#define NATIVE_BEHAVIOUR_SPEAK 0x3
#define NATIVE_BEHAVIOUR_EXORCISM 0x4
#define NATIVE_BEHAVIOUR_LIVE 0x5
#define NATIVE_BEHAVIOUR_TAKE_AND_PUT 0x6

#define NATIVE_STATE_TAKE_PUT_WALK 0x15
#define NATIVE_STATE_TAKE_PUT_TURN_TO 0x16

struct s_fighter_combo {};

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

	int nbTrajectoryParams;
	CTakePutTrajectoryParam* aTrajectoryParams;

	int field_0x24;
};

class CBehaviourNativLive : public CBehaviourNativ
{
public:
	int field_0x8;
};

struct ActionEntry {
	int field_0x0;
	float field_0x4;
};

class CEmotionInfo {
public:
	void DoAnimation(float, float, CActor*);

	int macroAnimId;
	int field_0x4;
	int field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
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

	void BehaviourNativTakeAndPut_InitState(int newState);
	void BehaviourNativTakeAndPut_Manage(CBehaviourNativTakeAndPut* pBehaviour);
	void BehaviourNativTakeAndPut_TermState(int oldState);

	void StateNativTakePutTurnTo();
	void StateNativTakePutWalk(CBehaviourNativTakeAndPut* pBehaviour);

	void SetHasObject(bool bHasObject);
	void ChooseToolState();

	uint FUN_00162270();
	float FUN_00164070();

	static StateConfig _gStateCfg_NTV[54];

	CPathFinderClient pathFinderClient;
	
	uint field_0x350;
	uint field_0x354;
	uint field_0x358;
	float field_0x35c;
	float field_0x360;
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