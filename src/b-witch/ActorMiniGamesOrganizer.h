#ifndef ACTOR_MINI_GAME_ORGANIZER_H
#define ACTOR_MINI_GAME_ORGANIZER_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGamesManager;

class CActorMiniGamesOrganizer : public CActor {
public:
	CActorMiniGamesOrganizer() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	// Unrecovered organizer data surrounding the fields used by the manager.
	undefined field_0x160[0x1c];
	S_ACTOR_STREAM_REF* field_0x17c = (S_ACTOR_STREAM_REF*)0x0;
	undefined field_0x180[0x874];
	CActorMiniGamesManager* field_0x9f4;
};

#endif //ACTOR_MINI_GAME_ORGANIZER_H
