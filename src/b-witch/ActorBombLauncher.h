#ifndef ACTOR_BOMB_LAUNCHER_H
#define ACTOR_BOMB_LAUNCHER_H

#include "Types.h"
#include "Actor.h"

class CActorBombLauncher : public CActor {
public:
	CActorBombLauncher() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_BOMB_LAUNCHER_H