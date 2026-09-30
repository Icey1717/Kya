#ifndef ACTOR_MINI_GAME_TIME_ATTACK_H
#define ACTOR_MINI_GAME_TIME_ATTACK_H

#include "Types.h"
#include "ActorMiniGame.h"

class CActorMiniGameTimeAttack : public CActorMiniGame {
public:
	CActorMiniGameTimeAttack() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_GAME_TIME_ATTACK_H
