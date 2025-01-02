#ifndef ACTOR_HUNTER_H
#define ACTOR_HUNTER_H

#include "Types.h"
#include "Actor.h"

class CActorHunter : public CActor {
public:
	CActorHunter(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_HUNTER_H