#ifndef ACTOR_EVENT_GENERATOR_H
#define ACTOR_EVENT_GENERATOR_H

#include "Types.h"
#include "Actor.h"

class CBehaviourEventGen : public CBehaviour
{

};

class CActorEventGenerator : public CActor
{
public:

	CActorEventGenerator() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourEventGen behaviourEventGen;
};

#endif //ACTOR_EVENT_GENERATOR_H