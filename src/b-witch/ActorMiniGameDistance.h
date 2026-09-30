#ifndef ACTOR_MINI_DISTANCE_H
#define ACTOR_MINI_DISTANCE_H

#include "Types.h"
#include "ActorMiniGame.h"

class CActorMiniGameDistance : public CActorMiniGame {
public:
	CActorMiniGameDistance() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual int GetScoreType() { return 2; }
};

#endif //ACTOR_MINI_DISTANCE_H
