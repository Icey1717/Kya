#ifndef ACTOR_BONUS_H
#define ACTOR_BONUS_H

#include "Types.h"
#include "Actor.h"

class CActorBonus : public CActorMovable {
public:
	CActorBonus(){
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual void Create(ByteCode* pByteCode);
};

#endif //ACTOR_BONUS_H