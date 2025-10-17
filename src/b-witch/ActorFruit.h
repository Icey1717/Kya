#ifndef ACTOR_FRUIT_H
#define ACTOR_FRUIT_H

#include "Types.h"
#include "ActorAutonomous.h"

class CActorFruit : public CActorAutonomous {
public:
	CActorFruit(){
		IMPLEMENTATION_GUARD_ACTOR()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_FRUIT_H