#ifndef _ACTOR_FIGHTER_H
#define _ACTOR_FIGHTER_H

#include "Types.h"
#include "Actor.h"
#include "ActorAutonomous.h"


class CActorFighter : public CActorAutonomous
{
public:
	CActorFighter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Init();
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	// CActorAutonomous
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);

	virtual bool IsFightRelated(int param_2);

	edF32VECTOR4 field_0x400;
	edF32VECTOR4 field_0x410;
};

class CBehaviourFighter : public CBehaviour
{
	virtual void Init(CActor* pOwner);

	CActor* pOwner;
	int field_0x8;
};

class CBehaviourFighterSlave : public CBehaviourFighter
{

};

class CBehaviourFighterRidden : public CBehaviourFighter
{

};

class CBehaviourFighterProjected : public CBehaviourFighter
{

};

#endif // _ACTOR_FIGHTER_H
