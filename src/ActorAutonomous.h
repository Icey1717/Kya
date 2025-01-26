#ifndef ACTOR_AUTONOMOUS_H
#define ACTOR_AUTONOMOUS_H

#include "Types.h"
#include "Actor.h"
#include "ActorMovable.h"
#include "CollisionManager.h"
#include "ScenaricCondition.h"
#include "PathFinderClient.h"

class CWayPoint;

struct _wind_param_in {
	float field_0x0;
	float field_0x4;
	int field_0x8;
	int field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	uint field_0x24;
	CWayPoint* pWayPoint;
	float field_0x2c;
	float field_0x30;
};

struct CActorWindState {
	int field_0x0;
	int field_0x4;
	int field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	int field_0x28;
	CWayPoint* pWayPoint;
	float field_0x30;
	float field_0x34;

	// Potentially just for CActorHero
	float field_0x38;
	float field_0x3c;
	edF32VECTOR4 field_0x40;

	void AddWind(_wind_param_in* pParams);
	void SubWind(_wind_param_in* pParams);
	void Reset();
};

class CActorAutonomous;

class CBehaviourAutonomous : public CBehaviour
{
public:
	virtual void Init(CActor* pOwner);

	uint boneId;
	CActorAutonomous* pOwner;
	CActor* field_0x10;
};

class CBehaviourCatchByTrap : public CBehaviourAutonomous
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void End(int newBehaviourId);
	virtual void InitState(int newState) {}
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	ConditionedOperationArray condOpArray;
};

class CDynamicExt
{
public:
	CDynamicExt();

	void ClearLocalData();

	static edF32VECTOR4 gForceGravity;

	float gravityScale;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float aImpulseVelocityMagnitudes[3];
	float field_0x1c;
	edF32VECTOR4 aImpulseVelocities[3];
	edF32VECTOR4 normalizedTranslation;

	edF32VECTOR3 field_0x60;
	float field_0x6c;

	float scaledTotalTime;
};

class CLifeInterface : public CInterface
{
public:
	CLifeInterface();

	int priority;
	float currentValue;
	float valueMax;
	int field_0x10;

	// CInterface
	virtual bool Manage();
	virtual void Draw() {}
	virtual void Reset() {}
	virtual float GetValue();
	virtual void SetValue(float value);

	float GetValueMax();
	void SetValueMax(float max);

	void SetPriority(int newPriority);
	float GetPriority();
};

class CActorAutonomous : public CActorMovable
{
public:
	CActorAutonomous();

	virtual bool IsKindOfObject(ulong kind);
	virtual void Create(ByteCode* pByteCode);
	virtual void Reset();
	virtual void Init();
	virtual void Term();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual bool IsLockable();
	virtual void SetLookingAtOn();

	// CActorMovable
	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	virtual CVision* GetVision() { return (CVision*)0x0; }

	void SetLookingAtOn(float param_1);
	void SetLookingAtRotationHeight(float height, edF32VECTOR4* pRotation);
	void SetLookingAt(float x, float y, float z);
	void SetLookingAtBounds(float param_1, float param_2, float param_3, float param_4);
	void SetLookingAtBones(uint leftBoneId, uint rightBoneId);

	void _ManageDynamicFence(CActorsTable* pActorsTable);
	virtual void StoreCollisionSphere();
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	virtual void UpdateCollisionSphere();
	virtual void RestoreCollisionSphere(float param_2);

	virtual CActorWindState* GetWindState();

	virtual float GetWalkSpeed();
	virtual float GetWalkRotSpeed();
	virtual float GetWalkAcceleration();
	virtual float GetRunSpeed();
	virtual float GetRunRotSpeed();
	virtual float GetRunAcceleration();
	
	static StateConfig gStateCfg_AUT[1];

	void ComputeFrictionForceWithSpeedMax(float param_1, edF32VECTOR4* pFrictionForce, int param_4);
	void ComputeFrictionForce(float param_1, edF32VECTOR4* pFrictionForce, int param_4);
	void ComputeSlidingForce(edF32VECTOR4* param_2, int param_3);
	void ClearAllSumForceExt();

	void StateAutSoccer(float param_1, int param_3, int param_4, CActorMovable* param_5);

	void SV_AUT_WarnActors(float radius, float param_2, CActor* pActor);
	void SV_AUT_MoveTo_Pathfinding(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation);
	void SV_AUT_MoveTo(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation);
	void SV_AUT_MoveTo_DynFence(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation);
	void SV_AUT_MoveTo_FixDyn(CActorMovParamsOut* pParamsIn, CActorMovParamsIn* pParamsOut, edF32VECTOR4* pLocation);
	void SV_AUT_PathfindingEnd();
	bool SV_AUT_CanMoveTo(edF32VECTOR4* v0);

	virtual void LifeRestore();
	virtual CLifeInterface* GetLifeInterface();
	virtual CLifeInterface* GetLifeInterfaceOther();
	virtual void LifeDecrease(float amount);

	virtual CPathFinderClient* GetPathfinderClient() { return (CPathFinderClient*)0x0; }
	virtual CPathFinderClient* GetPathfinderClientAlt() { return (CPathFinderClient*)0x0; }

	float field_0x2e4;

	CLifeInterface lifeInterface;

	CDynamicExt dynamicExt;

	s_collision_contact collisionContact;

	edF32VECTOR3 field_0x280;
	edF32VECTOR3 field_0x28c;
	edF32VECTOR3 field_0x298;
	edF32VECTOR3 field_0x2a4;
	edF32VECTOR3 field_0x2b0;
	edF32VECTOR3 field_0x2bc;

	float field_0x2c8;
	undefined4 bCollisionSphereDirty;

	float field_0x2e8;
	float vector_0x2f0;

	int field_0x340;
	byte field_0x344;

	int field_0x348;
	CActor* field_0x34c;
};

#endif //ACTOR_AUTONOMOUS_H