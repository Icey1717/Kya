#ifndef ACTOR_HEDGEHOG_H
#define ACTOR_HEDGEHOG_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"
#include "PathFollow.h"

class CActorHedgehog;

class CBehaviourHedgehog : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CBehaviourHedgehog
	virtual bool HasArmor() = 0;
	virtual edF32VECTOR4* GetComeBackPosition();

	edF32VECTOR4 comeBackPosition;
	CActorHedgehog* pOwner;
};

class CBehaviourHedgehogWatchDog : public CBehaviourHedgehog
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool HasArmor();
};

class CBehaviourHedgehogGuardArea : public CBehaviourHedgehog
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool HasArmor();
	virtual edF32VECTOR4* GetComeBackPosition();

	CPathFollowReader pathFollowReader;
};

class CBehaviourHedgehogWatchDogArmor : public CBehaviourHedgehogWatchDog
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool HasArmor();
};

class CBehaviourHedgehogGuardAreaArmor : public CBehaviourHedgehogGuardArea
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Init(CActor* pOwner);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual bool HasArmor();
};

class CActorHedgehog : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_ABV[28];

	virtual void Create(ByteCode* pByteCode);

	virtual void Init();
	virtual void Term();

	virtual void Reset();

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual StateConfig* GetStateCfg(int state);

	virtual float GetWalkSpeed();
	virtual float GetWalkRotSpeed();
	virtual float GetWalkAcceleration();
	virtual float GetRunSpeed();
	virtual float GetRunRotSpeed();
	virtual float GetRunAcceleration();

	void BehaviourGuardArea_Manage(CBehaviourHedgehogGuardArea* pBehaviour);
	void BehaviourWatchDog_Manage(CBehaviourHedgehogWatchDog* pBehaviour);

	uint TreatBoomyHit(CBehaviourHedgehog* pBehaviour);

	void StateGuardChase(CBehaviourHedgehog* pBehaviour);
	void StateGuardChaseStand(CBehaviourHedgehog* pBehaviour);
	void StateGuardComeBack(CBehaviourHedgehog* pBehaviour);
	void StateGuardSurprised(CBehaviourHedgehog* pBehaviour);
	void StateFly(CBehaviourHedgehog* pBehaviour);
	void StateGuardUpsideDown();

	void State_0x10(CBehaviourHedgehogGuardArea* pBehaviour);

	bool SV_IsCylinderIntersect(float param_1, float param_2, CActor* pOtherActor);
	int CheckArea();

	uint field_0x350;
	CActor* field_0x354;
	float walkSpeed;
	float walkAcceleration;
	float walkRotSpeed;
	float runSpeed;
	float field_0x368;
	float field_0x36c;
	float field_0x370;
	uint field_0x374;
	uint field_0x378;
	CActor* field_0x37c;
	uint field_0x380;
	edF32VECTOR4 field_0x390;
	CBehaviourHedgehogWatchDog behaviourWatchDog;
	CBehaviourHedgehogGuardArea behaviourGuardArea;
	CBehaviourHedgehogWatchDogArmor behaviourWatchDogArmor;
	CBehaviourHedgehogGuardAreaArmor behaviourGuardAreaArmor;

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_HEDGEHOG_H
