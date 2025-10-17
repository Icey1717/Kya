#ifndef ACTOR_WOOD_MONSTER_H
#define ACTOR_WOOD_MONSTER_H

#include "Types.h"
#include "Actor.h"

class CActorWoodMonster : public CActor {
public:
	CActorWoodMonster(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_WOOD_MONSTER_H