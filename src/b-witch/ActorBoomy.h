#ifndef ACTOR_BOOMY_H
#define ACTOR_BOOMY_H

#include "Types.h"
#include "ActorMovable.h"
#include "Vision.h"
#include "ActorAmbre.h"
#include "Fx_Tail.h"
#include "FxLightEmitter.h"
#include "Dynamic.h"

#define BOOMY_STATE_DEFAULT 0x5
#define BOOMY_STATE_GET_TARGET 0x6
#define BOOMY_STATE_GOTO_TARGET 0x7
#define BOOMY_STATE_GOTO_LAUNCHER 0x8
#define BOOMY_STATE_CONTROL 0xa

class CActorBoomy;
class CActorHero;
class CedMathTCBSpline;
class CCameraGame;

class CBehaviourBoomyLaunch : public CBehaviour
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);

	CActorBoomy* pOwner;
};

struct BoomySubObj_0x1c {
	uint flags;
	float field_0x4;
	float visionRange;
	int hitDamage;
	uint fxColorA;
	int hitProjectileType;
};

class CActorBoomy : public CActorMovable
{
public:
	static CActorBoomy* _gThis;
	static StateConfig _gStateCfg_BMY[7];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual CVision* GetVision();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CActorMovable
	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	void BehaviourBoomyLaunch_Manage();

	void UpdateTypeInfo();

	void ClearLocalData();

	void StateBoomyGetTarget();
	void StateBoomyGotoTarget();
	void StateBoomyGotoLauncher();

	bool GotoTarget(CActorsTable* pTable, edF32VECTOR4* param_3, int param_4, int param_5);

	void CalculateTrajectory(byte param_2);
	void BSpline_UpdateTargetPos(CActor* pTargetActor);
	void _BSpline_ComputeWhenHit();
	void _BSpline_ComputeToTarget();
	void _BSpline_InitWhenHit(bool param_2);

	void UpdateFromOwner(int param_2, edF32VECTOR4* pDirection);
	void GetActorsInVision(float maxDistance, CActorsTable* pTable);
	void SetTarget(CActor* pActor, edF32VECTOR4* pPosition);
	void SetTarget(edF32VECTOR4* pPosition);

	CActor* GetBestActorInVision();

	float field_0x1dc;

	float field_0x1e0;
	float field_0x1e4;

	undefined4 field_0x1d0;
	uint field_0x1d4;
	float field_0x1d8;

	float field_0x1e8;

	float distanceToTarget;

	CFxTail fxTail;
	CBehaviourBoomyLaunch launchBehaviour;

	CedMathTCBSpline* pSpline;
	CActor* pTargetActor;

	edF32VECTOR4 targetPosition;
	edF32VECTOR4 field_0x210;
	edF32VECTOR4 field_0x220;

	CVision vision;

	BoomySubObj_0x1c aBoomyTypeInfo[4];
	int curBoomyTypeId;

	int field_0x294;

	edF32VECTOR4 targetPositionOffset;

	byte field_0x2b0;
	CActorSound* field_0x2b4;
	S_STREAM_REF<CSound> field_0x2b8;
	S_STREAM_REF<CSound> field_0x2bc;
	float field_0x2c0;
	byte field_0x2c4;
	CActorsTable hitActorsTable;

	uint field_0x3cc;

	int particleID_0x3d0;
	int particleID_0x3d4;
	int particleID_0x3d8;
	int particleID_0x3dc;

	CActorHero* pHero;

	CFxLightEmitter fxLightEmitterA;
	CFxLightEmitter fxLightEmitterB;

	StaticMeshComponent staticMeshComponent;

	CCameraGame* pCamera;

	edF32MATRIX4 field_0x610;

	undefined4 field_0x650;
	undefined4 field_0x654;

	float field_0x740;
	float field_0x744;

	SPEED_DYN speedDyn;	
};

#endif //ACTOR_BOOMY_H