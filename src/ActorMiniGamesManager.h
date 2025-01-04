#ifndef ACTOR_MINI_GAME_MANAGER_H
#define ACTOR_MINI_GAME_MANAGER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGamesManager : public CActor {
public:
	CActorMiniGamesManager() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_GAME_MANAGER_H