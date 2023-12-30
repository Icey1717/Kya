#ifndef ACTOR_BOOMY_H
#define ACTOR_BOOMY_H

#include "Types.h"
#include "Actor.h"

class CActorBoomy : public CActor {
public:
	CActorBoomy(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_BOOMY_H