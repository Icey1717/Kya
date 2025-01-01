#ifndef ACTOR_ATON_H
#define ACTOR_ATON_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "PathManager.h"
#include "Path.h"

struct edDList_material;
class CActorAton;

class CFxTail : public CObject
{
public:
	CFxTail() : pData_0x18((undefined*)0x0) {}

	void Create(float param_1, int count, int param_4, int materialId);
	void Init(float param_1, int id);

	undefined* pData_0x18;
};

class CStaticMeshComponent
{
public:
	CStaticMeshComponent();

	virtual void Reset();
};

class CBehaviourAddOnAton : public CAddOn {
public:
	CBehaviourAddOnAton();
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pActor);
	virtual void Manage();

	int field_0x10;
	int* field_0x14;
	int field_0x1c;
	int* field_0x20;
};

class CBehaviourAton : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }

	CActorAton* pOwner;
};

class CBehaviourAtonEscape : public CBehaviourAton
{
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId) {}
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorAton : public CActorAutonomous {
public:
	CActorAton();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void SetState(int newState, int animType);
	virtual void AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	void ClearLocalData();
	bool ComputeSpeedAndAccelerationAndAnalyseForRun();
	void AnalysePathType();

	void BehaviourAtonEscape_InitState(int newState);
	void BehaviourAtonEscape_Manage();

	void StateAtonPathWaitJumpCallFar();

	void StateAtonPathStandCallNear();
	void StateAtonPathStandCallFar();
	void StateAtonPathStandCallBack();
	void StateAtonPathVertWindToJump_1_2();
	void StateAtonPathBeforeWaitJump();

	void StateAtonPathJump_1_4();
	void StateAtonPathJump_2_4();
	void StateAtonPathJump_3_4();
	void StateAtonPathJump_4_4();

	void StateAtonPathFall();

	void StateAtonPathVerticalWind();

	void StateAtonPathHorizontalWind();

	void StateAtonPathToboggan();

	void StateAtonPathRoll();

	void StateAtonPathStandWindTrack();

	void ChooseJumpState();
	void ChooseWaitJumpCallState();

	// very likely inlined
	void UpdateOrientationToWatchKim();
	
	// very likely inlined
	bool AnalyseForRun();

	static StateConfig _gStateCfg_ATO[46];

	CPathPlaneArray pathPlaneArray;
	CBehaviourAddOnAton behaviourAddOn;
	CBehaviourAtonEscape behaviourAtonEscape;

	CFxTail fxTailA;
	CFxTail fxTailB;

	float pathDefaultDelay;

	uint field_0x354;
	uint field_0x358;
	uint field_0x35c;
	uint field_0x360;

	float field_0x3b0;

	SV_MOV_FLEE_ON_PATH_PARAM fleeOnPathParams;

	edF32VECTOR4 field_0x3c0;

	float field_0x3d0;
	float field_0x3d4;
	float field_0x3d8;
	float field_0x3dc;

	undefined4 field_0x3e0;
	float field_0x3e4;
	int field_0x3e8;

	CVectorDyn vectorDyn;

	float field_0x470;

	byte field_0x474;
	byte field_0x475;

	float field_0x478;
	float field_0x47c;
	undefined4 field_0x480;
	float field_0x484;

	int trailMaterialId;

	int field_0x63c;
	int field_0x644;
	int field_0x648;

	edF32VECTOR4 waypointLocation;
	byte bWaypointSet;

	edDList_material* pTrailMaterial;

	CStaticMeshComponent staticMeshComponent;
};

#endif //ACTOR_ATON_H