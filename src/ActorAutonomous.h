#ifndef ACTOR_AUTONOMOUS_H
#define ACTOR_AUTONOMOUS_H

#include "Types.h"
#include "Actor.h"
#include "ActorMovable.h"
#include "CollisionManager.h"

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
	float field_0x10[4];
	edF32VECTOR4 velocity;
	edF32VECTOR4 field_0x30;
	edF32VECTOR4 field_0x40;
	edF32VECTOR4 field_0x50;

	edF32VECTOR3 field_0x60;
	float field_0x6c;

	float scaledTotalTime;
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

	virtual float ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable);

	void _ManageDynamicFence(CActorsTable* pActorsTable);
	virtual void StoreCollisionSphere();
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);

	CDynamicExt dynamicExt;

	s_collision_contact collisionContact;

	edF32VECTOR3 field_0x280;
	edF32VECTOR3 field_0x28c;
	edF32VECTOR3 field_0x298;
	edF32VECTOR3 field_0x2a4;
	edF32VECTOR3 field_0x2b0;
	edF32VECTOR3 field_0x2bc;

	float field_0x2c8;
	undefined4 field_0x2cc;

	byte field_0x344;

	int field_0x348;
	CActor* field_0x34c;
};

#endif //ACTOR_AUTONOMOUS_H