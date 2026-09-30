#ifndef ACTOR_MINI_GAME_BOX_COUNTER_H
#define ACTOR_MINI_GAME_BOX_COUNTER_H

#include "Types.h"
#include "ActorMiniGame.h"

class CActorMiniGameBoxCounter : public CActorMiniGame {
public:
	CActorMiniGameBoxCounter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
	virtual int GetScoreType() { return 3; }
};

#endif //ACTOR_MINI_GAME_BOX_COUNTER_H
