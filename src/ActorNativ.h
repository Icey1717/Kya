#ifndef ACTOR_NATIV_H
#define ACTOR_NATIV_H

#include "Types.h"
#include "ActorAutonomous.h"

class CBehaviourNativ : public CBehaviour
{

};

class CBehaviourNativExorcisme : public CBehaviourNativ
{

};

class CBehaviourNativTakeAndPut : public CBehaviourNativ
{

};

class CActorNativ : public CActorAutonomous 
{
public:
	CActorNativ(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourNativExorcisme behaviourExorcism;
	CBehaviourNativTakeAndPut behaviourTakeAndPut;
};

#endif //ACTOR_NATIV_H