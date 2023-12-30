#ifndef ACTOR_MICKEN_H
#define ACTOR_MICKEN_H

#include "Types.h"
#include "Actor.h"

class CActorMicken : public CActorAutonomous {
public:
	CActorMicken(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MICKEN_H