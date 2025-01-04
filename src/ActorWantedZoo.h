#ifndef ACTOR_WANTED_ZOO_H
#define ACTOR_WANTED_ZOO_H

#include "Types.h"
#include "Actor.h"

class CActorWantedZoo : public CActor {
public:
	CActorWantedZoo() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_WANTED_ZOO_H