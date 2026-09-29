#ifndef ACTOR_HEDGEHOG_H
#define ACTOR_HEDGEHOG_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"

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
	uint field_0x380;
	edF32VECTOR4 field_0x390;

	CAddOnGenerator addOnGenerator;
};

#endif //ACTOR_HEDGEHOG_H