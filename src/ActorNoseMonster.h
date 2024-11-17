#ifndef ACTOR_NOSE_MONSTER_H
#define ACTOR_NOSE_MONSTER_H

#include "Types.h"
#include "Actor.h"

class CActorNoseMonster : public CActor {
public:
	CActorNoseMonster(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_NOSE_MONSTER_H