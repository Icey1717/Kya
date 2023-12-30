#ifndef ACTOR_FRUIT_H
#define ACTOR_FRUIT_H

#include "Types.h"
#include "Actor.h"

class CActorFruit : public CActorAutonomous {
public:
	CActorFruit(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_FRUIT_H