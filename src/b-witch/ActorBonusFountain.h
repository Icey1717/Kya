#ifndef ACTOR_BONUS_FOUNTAIN_H
#define ACTOR_BONUS_FOUNTAIN_H

#include "Types.h"
#include "Actor.h"
#include "ActorBonusServices.h"

#define BONUS_FOUNTAIN_BEHAVIOUR_STAND 2

class CActorBonusFountain;

class CBehaviourBonusFountainStand : public CBehaviour
{
public:
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	CActorBonusFountain* pOwner;
	int field_0x8;
};

class CActorBonusFountain : public CActor
{
public:
	static StateConfig _gStateCfg_BNF[4];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	CBehaviour* BuildBehaviour(int behaviourType);
	StateConfig* GetStateCfg(int state);

	void FUN_003ea280(edF32VECTOR4* pPosition, int index);

	CAddOnGenerator addOnGenerator;

	CBehaviourBonusFountainStand behaviourStand;

	uint field_0x1a0[4];
	float field_0x1b0[2][4];
};

#endif //ACTOR_BONUS_FOUNTAIN_H