#ifndef ACTOR_ROPE_H
#define ACTOR_ROPE_H

#include "Types.h"
#include "Actor.h"

class CBehaviourRopeStand : public CBehaviour
{

};

class CActorRope : public CActor {
public:
	CActorRope(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual CBehaviour* BuildBehaviour(int behaviourType);

	CBehaviourRopeStand behaviourRopeStand;
};

#endif //ACTOR_ROPE_H