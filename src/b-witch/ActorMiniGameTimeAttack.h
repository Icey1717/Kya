#ifndef ACTOR_MINI_GAME_TIME_ATTACK_H
#define ACTOR_MINI_GAME_TIME_ATTACK_H

#include "Types.h"
#include "Actor.h"

class CActorMiniGameTimeAttack : public CActor {
public:
	CActorMiniGameTimeAttack() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_MINI_GAME_TIME_ATTACK_H