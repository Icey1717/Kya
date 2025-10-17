#ifndef ACTOR_MINI_GAME_BOX_COUNTER_H
#define ACTOR_MINI_GAME_BOX_COUNTER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGameBoxCounter : public CActor {
public:
	CActorMiniGameBoxCounter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_GAME_BOX_COUNTER_H