#ifndef ACTOR_MINI_GAME_H
#define ACTOR_MINI_GAME_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGame : public CActor {
public:
	CActorMiniGame(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);

	void FUN_003ace00();

	// Unrecovered mini-game data preceding the ordering field.
	undefined field_0x160[0x78];
	int field_0x1d8;
};

#endif //ACTOR_MINI_GAME_H
