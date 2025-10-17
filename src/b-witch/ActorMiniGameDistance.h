#ifndef ACTOR_MINI_DISTANCE_H
#define ACTOR_MINI_DISTANCE_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGameDistance : public CActor {
public:
	CActorMiniGameDistance() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_DISTANCE_H