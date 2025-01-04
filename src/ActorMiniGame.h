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
};

#endif //ACTOR_MINI_GAME_H