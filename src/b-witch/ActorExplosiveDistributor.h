#ifndef ACTOR_EXPLOSIVE_DISTRIBUTOR_H
#define ACTOR_EXPLOSIVE_DISTRIBUTOR_H

#include "Types.h"
#include "Actor.h"

struct S_STREAM_EVENT_CAMERA;

struct _s_projectile
{
	edF32VECTOR4 field_0x0;
	float ejectForce;
	S_STREAM_REF<CActor> actorRef;
};

class CActorExplosiveDistributor;

class CBehaviourExplosiveDistributor : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorExplosiveDistributor* pOwner;
};

class CActorExplosiveDistributor : public CActor
{
public:
	static StateConfig _gStateCfg_ExD[2];

	CActorExplosiveDistributor();

	// CActor Interface
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Draw();
	virtual void Reset();
	virtual void CheckpointReset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);

	// New
	bool DistributeExplosive();

	S_STREAM_EVENT_CAMERA* pStreamEventCamera;

	float field_0x160;
	float field_0x164;
	undefined4 field_0x168;
	int nbProjectiles;
	_s_projectile* aProjectiles;
	edF32VECTOR4 projectileSpawnLocation;
	int field_0x190;
	_s_projectile* pProjectile;
	CBehaviourExplosiveDistributor behaviourExplosiveDistributor;
};

#endif //ACTOR_EXPLOSIVE_DISTRIBUTOR_H