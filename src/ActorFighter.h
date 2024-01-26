#ifndef _ACTOR_FIGHTER_H
#define _ACTOR_FIGHTER_H

#include "Types.h"
#include "Actor.h"


class CActorFighter : public CActorAutonomous
{
public:
	CActorFighter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual CBehaviour* BuildBehaviour(int behaviourType);

	virtual bool IsFightRelated(int param_2);
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
