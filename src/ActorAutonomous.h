#ifndef ACTOR_AUTONOMOUS_H
#define ACTOR_AUTONOMOUS_H

#include "Types.h"
#include "Actor.h"
#include "ActorMovable.h"
#include "CollisionManager.h"

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
	CWayPoint* field_0x28;
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
	CWayPoint* field_0x2c;
	float field_0x30;
	float field_0x34;

	// Potentially just for CActorHero
	float field_0x38;
	float field_0x3c;
	float field_0x40;
	float field_0x44;
	float field_0x48;
	float field_0x4c;

	void AddWind(_wind_param_in* pParams);
	void SubWind(_wind_param_in* pParams);
	void Reset();
};

class CBehaviourAutonomous : public CBehaviour
{
public:

};

class CBehaviourCatchByTrap : public CBehaviourAutonomous
{
public:

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
	float aVelocityMagnitudes[3];
	float field_0x1c;
	edF32VECTOR4 aVelocity[3];
	edF32VECTOR4 normalizedTranslation;

	edF32VECTOR3 field_0x60;
	float field_0x6c;

	float scaledTotalTime;
};

class CInterface {
public:
	virtual float GetValue() = 0;
	virtual void SetValue(float value) = 0;
};

class CLifeInterface : public CInterface
{
public:
	CLifeInterface();

	int priority;
	float currentValue;
	float valueMax;
	undefined4 field_0x10;

	// CInterface
	virtual float GetValue();
	virtual void SetValue(float value);

	float GetValueMax();

	void SetValueMax(float max);
	void SetPriority(int newPriority);
};

class CActorAutonomous : public CActorMovable
{
public:
	CActorAutonomous() {
		IMPLEMENTATION_GUARD_LOG();
	}

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	virtual void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);
	virtual CActorWindState* GetWindState() { return (CActorWindState*)0x0; }

	void _ManageDynamicFence(CActorsTable* pActorsTable);
	virtual void StoreCollisionSphere();
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	virtual void UpdateCollisionSphere();
	virtual void RestoreCollisionSphere(float param_2);

	void ComputeFrictionForceWithSpeedMax(float param_1, edF32VECTOR4* pFrictionForce, int param_4);
	void ComputeFrictionForce(float param_1, edF32VECTOR4* pFrictionForce, int param_4);
	void ComputeSlidingForce(edF32VECTOR4* param_2, int param_3);

	virtual void LifeRestore();
	virtual CLifeInterface* GetLifeInterface();
	virtual CLifeInterface* GetLifeInterfaceOther();
	virtual void LifeDecrease(float amount);

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

	byte field_0x344;

	int field_0x348;
	CActor* field_0x34c;
};

#endif //ACTOR_AUTONOMOUS_H