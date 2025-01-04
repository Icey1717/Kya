#ifndef ACTOR_MINI_GAME_ORGANIZER_H
#define ACTOR_MINI_GAME_ORGANIZER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGamesOrganizer : public CActor {
public:
	CActorMiniGamesOrganizer() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_GAME_ORGANIZER_H