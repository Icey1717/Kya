#ifndef ACTOR_WOOF_H
#define ACTOR_WOOF_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "Vision.h"

class CBehaviourWoof : public CBehaviour
{

};

class CBehaviourWoofGuard : public CBehaviourWoof
{

};

class CBehaviourWoofVerticalJump : public CBehaviourWoof
{

};

class CActorWoof : public CActorAutonomous
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourWoofGuard behaviourWoofGuard;
	CBehaviourWoofVerticalJump behaviourWoofVerticalJump;

	CPathFollowReader pathFollowReader;
	CVision vision;
};

#endif //ACTOR_WOOF_H