#ifndef ACTOR_MINI_GAME_BOOMY_H
#define ACTOR_MINI_GAME_BOOMY_H

#include "Types.h"
#include "ActorMiniGame.h"

class CActorMiniGameBoomy : public CActorMiniGame {
public:
	CActorMiniGameBoomy() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_GAME_BOOMY_H
