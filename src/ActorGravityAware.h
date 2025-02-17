#ifndef ACTOR_GRAVITY_AWARE_H
#define ACTOR_GRAVITY_AWARE_H

#include "Types.h"
#include "ActorAutonomous.h"

class CActorGravityAware : public CActorAutonomous {
public:
	CActorGravityAware(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_GRAVITY_AWARE_H